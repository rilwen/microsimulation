#include "multivariate_distribution_independent.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <stdexcept>

namespace averisera {
    MultivariateDistributionIndependent::MultivariateDistributionIndependent(const std::vector<std::shared_ptr<const MultivariateDistribution>>& members)
        : _members(members), _nbr_members(members.size()) {
        if (std::any_of(_members.begin(), _members.end(), [](const std::shared_ptr<const MultivariateDistribution>& distr) {
                    return !distr;
                })) {
            throw std::domain_error("MultivariateDistributionIndependent: one or more members is null");
        }
        _dim = std::accumulate(_members.begin(), _members.end(), size_t(0), [](size_t tot_dim, const std::shared_ptr<const MultivariateDistribution>& distr) {
                return tot_dim + distr->dim();
            });
    }

    void MultivariateDistributionIndependent::draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        size_t j = 0;
        for (auto dit = _members.begin(); dit != _members.end(); ++dit) {
            const MultivariateDistribution& distr = **dit;
            const size_t dim_i = distr.dim();
            distr.draw(rng, x.segment(j, dim_i));
            j += dim_i;
        }
    }

    void MultivariateDistributionIndependent::draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const {
        size_t j = 0;
        for (auto dit = _members.begin(); dit != _members.end(); ++dit) {
            const MultivariateDistribution& distr = **dit;
            const size_t dim_i = distr.dim();
            distr.draw_noncont(rng, x.segment(j, dim_i));
            j += dim_i;
        }
    }

    void MultivariateDistributionIndependent::marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const {
        size_t j = 0;
        for (auto dit = _members.begin(); dit != _members.end(); ++dit) {
            const MultivariateDistribution& distr = **dit;
            const size_t dim_i = distr.dim();
            distr.marginal_cdf(x.segment(j, dim_i), p.segment(j, dim_i));
            j += dim_i;
        }
    }

    void MultivariateDistributionIndependent::marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const {
		size_t j = 0;
        for (auto dit = _members.begin(); dit != _members.end(); ++dit) {
            const MultivariateDistribution& distr = **dit;
            const size_t dim_i = distr.dim();
            distr.marginal_icdf(p.segment(j, dim_i), x.segment(j, dim_i));
            j += dim_i;
        }
    }

    void MultivariateDistributionIndependent::adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const {
        size_t j = 0;
        const auto nrows = sample.rows();
        for (auto dit = _members.begin(); dit != _members.end(); ++dit) {
            const MultivariateDistribution& distr = **dit;
            const size_t dim_i = distr.dim();
            distr.adjust_distribution(sample.block(0, j, nrows, dim_i));
            j += dim_i;
        }
    }
}
