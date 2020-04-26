// (C) Averisera Ltd 2014-2020
//#include "population_mover_multidim.hpp"
//#include "csm.hpp"
//#include "log.hpp"
//#include "preconditions.hpp"
//#include "stl_utils.hpp"
//#include "statistics.hpp"
//#include <algorithm>
//
//namespace averisera {
//	PopulationMoverMultidim::PopulationMoverMultidim(const Eigen::MatrixXd& pi, const std::vector<std::vector<double>>& ranges, const bool smooth)
//	: ndims_(ranges.size()) {
//		check_equals(0, pi.cols() % pi.rows(), "PopulationMoverMultidim: pi dimensions are incorrect");
//		const unsigned int memory = Statistics::calc_memory(static_cast<unsigned int>(pi.cols()), static_cast<unsigned int>(pi.rows()));
//		dims_.reserve(ranges.size());
//		for (const auto& rng : ranges) {
//			check_that(rng.size() > 1, "PopulationMoverMultidim: need at least two range values");
//			dims_.push_back(static_cast<unsigned int>(rng.size() - 1));
//		}
//		const std::vector<Eigen::MatrixXd> pi_decomposed(CSM::to_hierarchical_compact_form(pi, dims_, memory));
//		auto pi_decomp_it = pi_decomposed.begin();
//		movers_.reserve(ranges.size());
//		for (const auto& rng : ranges) {
//			movers_.push_back(PopulationMover(*pi_decomp_it, rng, smooth));
//			++pi_decomp_it;
//		}
//	}
//
//	void PopulationMoverMultidim::move_between_ranges(std::vector<Member>& population, RNG& rng) const {
//		//const size_t N = population.size();
//		std::vector<PopulationMover::Member> pop1d;
//		initialize_populations(population, pop1d);
//		for (size_t i = 0; i < ndims_; ++i) {
//			if (i > 0) {
//				const auto prev_dim = dims_[i - 1];
//				auto it_1d = pop1d.begin();
//				for (auto it = population.begin(); it != population.end(); ++it, ++it_1d) {
//					assert(it->member_index == it_1d->member_index);
//					update_component(*it, *it_1d, i, prev_dim);					
//				}
//				assert(it_1d == pop1d.end());
//			}
//			movers_[i].move_between_ranges(pop1d, rng);
//			std::sort(pop1d.begin(), pop1d.end(), [](const PopulationMover::Member& l, const PopulationMover::Member& r) {
//				return l.member_index < r.member_index;
//			});
//			std::sort(population.begin(), population.end(), [](const Member& l, const Member& r) {
//				return l.member_index < r.member_index;
//			});
//			auto it_1d = pop1d.begin();
//			std::vector<size_t> cnt(dims_[i], 0);
//			for (auto it = population.begin(); it != population.end(); ++it, ++it_1d) {
//				assert(it->member_index == it_1d->member_index);
//				it->range_indices[i] = it_1d->range_index;
//				it->values[i] = it_1d->value;
//				++cnt[it_1d->range_index];
//			}
//			LOG_DEBUG() << "i=" << i << ", cnt=" << cnt;
//			assert(it_1d == pop1d.end());
//			//LOG_DEBUG() << "PopulationMoverMultidim: Handled dimension " << i;
//		}
//	}
//
//	void PopulationMoverMultidim::update_component(const Member& member, PopulationMover::Member& member_1d, size_t i, const unsigned int prev_dim) {
//		assert(i > 0);
//		assert(i < member.values.size());
//		member_1d.value = member.values[i];
//		member_1d.distribution_index = member_1d.distribution_index * prev_dim + member.range_indices[i - 1];
//	}
//
//	void PopulationMoverMultidim::initialize_populations(std::vector<Member>& population, std::vector<PopulationMover::Member>& pop1d) const {
//		pop1d.resize(population.size());
//		// make sure we have space for results and check values
//		std::for_each(population.begin(), population.end(), [this](Member& m) {
//			m.range_indices.resize(ndims_);
//			check_equals(ndims_, m.values.size(), "PopulationMoverMultidim: bad values size");
//		});
//		std::transform(population.begin(), population.end(), pop1d.begin(), [](const Member& m) {
//			PopulationMover::Member m1d;
//			m1d.distribution_index = m.distribution_index;
//			m1d.member_index = m.member_index;
//			m1d.value = m.values.front();
//			return m1d;
//		});
//	}
//}
