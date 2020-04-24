#include "data_exception.hpp"
#include "discrete_distribution.hpp"
#include "distribution_empirical.hpp"
#include "distribution_linear_interpolated.hpp"
//#include "interpolator_impl_tanh.hpp"
#include "interpolator.hpp"
#include "interpolator_impl.hpp"
#include "interpolator_impl_piecewise_cubic.hpp"
#include "interpolator_impl_factory.hpp"
#include "kahan_summation.hpp"
#include "log.hpp"
#include "population_mover.hpp"
#include "population_mover_slope_calculator.hpp"
#include "preconditions.hpp"
#include "rng.hpp"
#include "stl_utils.hpp"
#include <boost/format.hpp>

namespace averisera {
	PopulationMover::PopulationMover(const Eigen::MatrixXd& pi, const std::vector<double>& ranges, bool smooth)
		: pi_(pi), ranges_(ranges), distrs_(static_cast<size_t>(pi.cols() + 1)), range_cnt_(ranges.size() - 1), 
		distr_cnt_(static_cast<size_t>(pi.cols())), smooth_(smooth) {
		check_that(ranges.size() > 1, "PopulationMover: Need at least 1 range with 2 values");
		check_that(pi.cols() > 0, "PopulationMover: Need at least 1 distribution");
		//check_that(pi.rows() == pi.cols(), "pi must be square");
		if (static_cast<size_t>(pi.rows()) != range_cnt_) {
			throw std::domain_error(boost::str(boost::format("PopulationMover: pi_expanded row dimension %d does not match number of ranges %d") % pi.rows() % range_cnt_));
		}		
		std::iota(distrs_.begin(), distrs_.end(), 0u);
		assert(distrs_.size() == distr_cnt_ + 1);
		assert(distrs_.back() == distr_cnt_);
		assert(distrs_.front() == 0);
	}

	void PopulationMover::move_between_ranges(std::vector<PopulationMover::Member>& population, RNG& rng) const {
		if (population.empty()) {
			return;
		}
		std::sort(population.begin(), population.end(), Member::comparator);
		for (const Member& p : population) {
			if (p.value < ranges_.front() || p.value > ranges_.back()) {
				throw DataException(boost::str(boost::format("PopulationMover: value %g outside bounds [%g, %g]") % p.value % ranges_.front() % ranges_.back()));
			}
			if (p.distribution_index >= distr_cnt_) {
				throw DataException(boost::str(boost::format("PopulationMover: distribution index %d outside bounds [0, %d)") % p.distribution_index % distr_cnt_));
			}
		}
		std::vector<size_t> distr_range_indices(get_range_indices(distrs_, population, Member::distr_getter));
		assert(distr_range_indices.size() == distr_cnt_ + 1);
		const size_t popsize = population.size();
		assert(distr_range_indices.front() == 0);
		assert(distr_range_indices.back() == popsize);
		std::vector<size_t> distr_range_counts(distr_cnt_);
		for (size_t i = 0; i < distr_cnt_; ++i) {
			assert(distr_range_indices[i + 1] >= distr_range_indices[i]);
			distr_range_counts[i] = distr_range_indices[i + 1] - distr_range_indices[i];
		}
		assert(std::accumulate(distr_range_counts.begin(), distr_range_counts.end(), size_t(0)) == population.size());
		std::vector<Member> new_population;
		new_population.reserve(popsize);
		std::vector<std::vector<size_t>> transferred_indices(range_cnt_);
		std::vector<double> local_probs(range_cnt_);
		std::vector<double> new_x_values;
		for (size_t from = 0; from < distr_cnt_; ++from) {
			const size_t from_size = distr_range_counts[from];
			LOG_DEBUG() << "from=" << from << ", from_size=" << from_size;
			if (from_size) {
				const double v0 = population[distr_range_indices[from]].value;
				const double v1 = population[distr_range_indices[from] + from_size - 1].value;
				LOG_DEBUG() << "v0=" << v0 << ", v1=" << v1;
				size_t r0 = SegmentSearch::binary_search_left_inclusive(ranges_, v0);
				size_t r1 = SegmentSearch::binary_search_left_inclusive(ranges_, v1);
				if (r0 == range_cnt_) {
					--r0;
				}
				if (r1 == range_cnt_) {
					--r1;
				}
				LOG_DEBUG() << "r0=" << r0 << ", r1=" << r1;
				assert(r0 < range_cnt_);
				assert(r1 < range_cnt_);
				if (r0 != r1) {
					throw std::domain_error(boost::str(boost::format("PopulationMover: division into distributions not compatible with value ranges: %d vs %d, %g vs %g for distribution %d") % r0 % r1 % v0 % v1 % from));
				}
				const auto& distr = pi_.col(from);				
				draw_moved_indices(distr, distr_range_indices[from], from_size, r0, transferred_indices, rng, local_probs);
				size_t transferred_cnt = 0;
				for (size_t to = 0; to < range_cnt_; ++to) {
					const std::vector<size_t>& ti = transferred_indices[to];
					const size_t n_i = ti.size();
					LOG_DEBUG() << "from=" << from << ", to=" << to << ", n_i=" << n_i;
					transferred_cnt += n_i;
					if (to != r0) {
						move_and_draw_new_values(ti, new_x_values, r0, to, population, new_population, rng);
					} else {
						for (size_t i : ti) {
							population[i].range_index = to;
							new_population.push_back(population[i]);
						}
					}
				}
				assert(from_size == transferred_cnt);
			}
		}

		if (smooth_) {
#ifndef NDEBUG
			const double max_value = ranges_.back();
			const double min_value = ranges_.front();
#endif //NDEBUG
			std::vector<double> old_xs(range_cnt_ + 1);
			{
				DistributionLinearInterpolated new_distr(std::vector<double>(ranges_), calc_new_probabilities(distr_range_counts, static_cast<double>(popsize)));
				DistributionEmpirical old_distr(population, Member::value_getter);
				for (size_t i = 0; i <= range_cnt_; ++i) {
					const double u = new_distr.cdf(ranges_[i]);
					const double old_x = old_distr.icdf(u);
					old_xs[i] = old_x;
					assert(old_x <= max_value + 1E-10);
					assert(old_x >= min_value - 1E-10);
					LOG_DEBUG() << ranges_[i] << " " << old_x;
				}
				old_xs.front() = ranges_.front();
				old_xs.back() = ranges_.back();
			}

			//InterpolatorImplTanh interp(std::move(old_xs), ranges_, 0.1);// , 1.0, 1.0);
			const auto interp = InterpolatorImplFactory::fritschButland(old_xs, ranges_);			
//#ifndef NDEBUG
//			const std::shared_ptr<InterpolatorImplPiecewiseCubic> interp_cubic = std::dynamic_pointer_cast<InterpolatorImplPiecewiseCubic>(interp);
//			assert(interp_cubic);
//			for (size_t i = 0; i < range_cnt_; ++i) {
//				assert(interp_cubic->is_monotonic(i));
//			}
//#endif


			std::sort(new_population.begin(), new_population.end(), Member::value_comparator);
			for (size_t i = 0; i < popsize; ++i) {
				double new_value = interp->evaluate(population[i].value);
				//new_value = std::min(max_value, new_value);
				//new_value = std::max(min_value, new_value);
				new_population[i].value = new_value;
			}
		}

		population = std::move(new_population);

	}

	double PopulationMover::draw_from_linear_pdf(double a, double b, RNG& rng) {
		const double u = rng.next_uniform();
		// CDF is quadratic
		return a + (b - a) * sqrt(u);
	}

	std::vector<double> PopulationMover::calc_new_probabilities(const std::vector<size_t>& distr_range_counts, const double popsize) const {
		std::vector<double> new_probs(range_cnt_, 0.0);
		for (size_t i = 0; i < distr_cnt_; ++i) {
			const double p0_i = static_cast<double>(distr_range_counts[i]) / popsize;
			const auto col = pi_.col(i);
			for (size_t j = 0; j < range_cnt_; ++j) {
				new_probs[j] += p0_i * col[j];
			}
		}
		KahanSummation<double> sum(0.0);
		for (size_t j = 0; j < range_cnt_; ++j) {
			new_probs[j] = std::max(1e-10, new_probs[j]);
			sum += new_probs[j];
		}
		for (size_t j = 0; j < range_cnt_; ++j) {
			new_probs[j] /= sum;
		}
		return new_probs;
	}

	/*size_t PopulationMover::draw_from_linear_pdf(size_t i0, size_t i1, RNG& rng) {
		const double x = floor(sqrt(rng.next_uniform()) * (i1 + 1 - i0));
		if (i1 >= i0) {
			return i0 + std::min(i1, static_cast<size_t>(x));
		} else {
			return i0 + std::max(i1, static_cast<size_t>(x));
		}
	}*/

	void PopulationMover::move_and_draw_new_values(const std::vector<size_t>& ti, std::vector<double>& new_x_values, const size_t from_range, const size_t to_range, const std::vector<Member>& population, std::vector<Member>& new_population, RNG& rng) const {
		new_x_values.clear();
		new_x_values.reserve(ti.size());
		assert(from_range < range_cnt_);
		assert(to_range < range_cnt_);
		double a = ranges_[to_range];
		double b = ranges_[to_range + 1];
		assert(from_range != to_range);
		if (from_range < to_range) {
			std::swap(a, b);
		}
		for (auto it = ti.begin(); it != ti.end(); ++it) {
			const double new_x = draw_from_linear_pdf(a, b, rng);
			assert(new_x <= ranges_.back());
			assert(new_x >= ranges_.front());
			assert(new_x >= std::min(a, b));
			assert(new_x <= std::max(a, b));
			new_x_values.push_back(new_x);
		}
		std::sort(new_x_values.begin(), new_x_values.end());
		auto ti_it = ti.begin();
		for (auto nxv_it = new_x_values.begin(); nxv_it != new_x_values.end(); ++nxv_it, ++ti_it) {
			assert(ti_it != ti.end());
			Member new_member(population[*ti_it]);
			assert(new_member.distribution_index == population[*ti_it].distribution_index);
			assert(new_member.member_index == population[*ti_it].member_index);
			new_member.value = *nxv_it;
			new_member.range_index = to_range;
			new_population.push_back(new_member);
		}
	}

	//double PopulationMover::approximate_linear_pdf(double x, double dx) {
	//	//std::cout << "x == " << x << "\n";
	//	assert(x >= -1E-16);
	//	assert(x < 1 + 5E-16);
	//	assert(dx >= 0);
	//	if (dx > 1E-7) {
	//		return (pow(x + dx, 2) - x * x) / dx;
	//	} else {
	//		return 2 * x + dx;
	//	}
	//}

	void PopulationMover::draw_moved_indices(Eigen::Ref<const Eigen::VectorXd> distr, const size_t i0, const size_t from_size, const size_t range_from, std::vector<std::vector<size_t>>& transferred_indices, RNG& rng, std::vector<double>& local_probs) {		
		const size_t N = static_cast<size_t>(distr.size());
		assert(range_from < N);
		for (size_t to = 0; to < N; ++to) {
			transferred_indices[to].clear();
			transferred_indices[to].reserve(static_cast<size_t>(distr[to] * static_cast<double>(from_size)));
		}
		const size_t i1 = i0 + from_size;
		const double w = static_cast<double>(from_size);
		const double dx = 1.0 / w;
		LOG_DEBUG() << "i0=" << i0 << ", i1=" << i1 << ", from_size=" << from_size << ", w=" << w << ", dx=" << dx;
#ifndef NDEBUG
		std::vector<double> integrated_distr(N, 0.0); // for debugging
#endif
		PopulationMoverSlopeCalculator pmsc(1e-8);
		std::vector<double> a;
		std::vector<double> b;
		pmsc.calculate(distr, range_from, a, b);
		DiscreteDistribution local_distr(0, 0, static_cast<int>(N - 1));
		LOG_DEBUG() << "a=" << a << ", b=" << b;
		for (size_t i = i0; i < i1; ++i) {
			const double x = (static_cast<double>(i - i0) + 0.5) / w;
			assert(x >= 0);
			assert(x < 1);
			for (size_t to = 0; to < N; ++to) {
				local_probs[to] = a[to] + b[to] * x;
			}
			// normalise
			const double norm = std::accumulate(local_probs.begin(), local_probs.end(), 0.0);
			for (double& p : local_probs) {
				p /= norm;
			}
#ifndef NDEBUG
			for (size_t to = 0; to < N; ++to) {
				integrated_distr[to] += local_probs[to] * dx;
			}
#endif
			local_distr.assign_proba(local_probs);
			const size_t drawn_to = static_cast<size_t>(local_distr.draw(rng));
			assert(drawn_to < N);
			transferred_indices[drawn_to].push_back(i);
		}
#ifndef NDEBUG
		LOG_DEBUG() << "integrated_distr=" << integrated_distr;
#endif
	}
}
