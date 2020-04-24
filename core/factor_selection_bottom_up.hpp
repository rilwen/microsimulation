#pragma once
/*
(C) Averisera Ltd 
*/
#include "factor_selection.hpp"
#include "log.hpp"
#include "preconditions.hpp"
#include "stl_utils.hpp"
#include <set>

namespace averisera {
	/** @brief Bottom-up factor selection.

	Given X, y:
	1. accepted_factors := []
	2. choose next best candidate factor
	3. if candidate factor improves model, add it to accepted_factors and go to 2., else go to 4.
	4. return accepted_factors

	@tparam ModelFactory class with operator()() returning a properly initialised model with a fit(X, y) method.
	@tparam FactorRank class with operator()(X, y, model, accepted_factors, i) returning the rank of i-th factor conditioned on a model using accepted_factors, which exclude i and may be empty. Higher rank is better.
	@tparam ModelComparison class with operator()(model1, model2) returning true if model2 is significantly (in statistical sense) better than model1 (possibly empty).
	*/
	template <class ModelFactory, class FactorRank, class ModelComparison> class FactorSelectionBottomUp: public FactorSelection {
	public:
		FactorSelectionBottomUp(ModelFactory model_factory, FactorRank factor_rank, ModelComparison model_comparison);

		/**
		@param X n x p matrix (1 row per sample point, 1 column per dimension)
		@param y n-size vector
		@throw std::domain_error If X.rows() != y.size(). If X is empty.
		*/
		std::vector<size_t> select(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y) const override;
	private:
		ModelFactory model_factory_;
		FactorRank factor_rank_;
		ModelComparison model_comparison_;

		template <class Model> void fit(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y, const std::vector<size_t>& factors, Model& model) const;
	};

	template <class MF, class FR, class MC> FactorSelectionBottomUp<MF, FR, MC>::FactorSelectionBottomUp(MF model_factory, FR factor_rank, MC model_comparison)
		: model_factory_(model_factory),
		factor_rank_(factor_rank),
		model_comparison_(model_comparison) {}

	template <class MF, class FR, class MC> std::vector<size_t> FactorSelectionBottomUp<MF, FR, MC>::select(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y) const {
		const auto n = y.size();
		check_equals(X.rows(), n, "FactorSelectionBottomUp::select: X.rows() != y.size()");
		check_greater<Eigen::Index>(X.size(), 0, "FactorSelectionBottomUp::select: X.size() > 0");
		std::vector<size_t> accepted_factors;
		std::set<size_t> available_factors;
		const auto d = X.cols(); // number of factors available
		std::vector<double> ranks(d);
		for (Eigen::Index i = 0; i < d; ++i) {
			available_factors.insert(i);
		}
		auto model = model_factory_();
		while (!available_factors.empty()) {
			std::transform(available_factors.begin(), available_factors.end(), ranks.begin(), [this, &X, &y, &model, &accepted_factors](size_t idx) {
				return factor_rank_(X, y, model, accepted_factors, idx);
			});
			LOG_DEBUG() << "FactorSelectionBottomUp: factor ranks " << ranks;
			const size_t best_candidate_idx = *std::next(available_factors.begin(), static_cast<size_t>(std::distance(ranks.begin(), std::max_element(ranks.begin(), ranks.begin() + available_factors.size()))));
			LOG_DEBUG() << "FactorSelectionBottomUp: testing candidate factor " << best_candidate_idx;
			std::vector<size_t> new_accepted_factors(accepted_factors);
			new_accepted_factors.push_back(best_candidate_idx);
			auto new_model = model_factory_();
			fit(X, y, new_accepted_factors, new_model);
			if (model_comparison_(model, new_model)) {
				LOG_DEBUG() << "FactorSelectionBottomUp: accepted candidate factor " << best_candidate_idx;
				model = std::move(new_model);
				accepted_factors.swap(new_accepted_factors);
				available_factors.erase(best_candidate_idx);
				ranks.resize(available_factors.size());
			} else {
				LOG_DEBUG() << "FactorSelectionBottomUp: rejected candidate factor " << best_candidate_idx;
				// no new factor
				break;
			}
		}
		std::sort(accepted_factors.begin(), accepted_factors.end());
		return accepted_factors;
	}

	template <class MF, class FR, class MC> template <class M> void FactorSelectionBottomUp<MF, FR, MC>::fit(const Eigen::Ref<const Eigen::MatrixXd>& X, const Eigen::Ref<const Eigen::VectorXd>& y, const std::vector<size_t>& factors, M& model) const {
		const size_t p_sel = factors.size();
		Eigen::MatrixXd selX(X.rows(), p_sel);
		for (size_t i = 0; i < p_sel; ++i) {
			selX.col(i) = X.col(factors[i]);
		}
		model.fit(selX, y);
	}

	template <class MF, class FR, class MC> FactorSelectionBottomUp<MF, FR, MC> make_factor_selection_bottom_up(MF mf, FR fr, MC mc) {
		return FactorSelectionBottomUp<MF, FR, MC>(mf, fr, mc);
	}
}
