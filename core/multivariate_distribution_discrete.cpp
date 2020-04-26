// (C) Averisera Ltd 2014-2020
#include "multivariate_distribution_discrete.hpp"

namespace averisera {
    MultivariateDistributionDiscrete::MultivariateDistributionDiscrete(Eigen::Ref<const Eigen::VectorXd> probs,
                                                                       const std::vector<int>& lb,
                                                                       const std::vector<int>& ub)
        : _flat(0, probs),
          _lb(lb) {
        if (lb.empty() || ub.empty()) {
            throw std::domain_error("MultivariateDistributionDiscrete: empty bounds vectors");
        }
        if (lb.size() != ub.size()) {
            throw std::domain_error("MultivariateDistributionDiscrete: bound sizes mismatch");
        }
        std::vector<size_t> sizes;
        sizes.reserve(lb.size());
        auto ub_it = ub.begin();
        for (auto lb_it = _lb.begin(); lb_it != _lb.end(); ++lb_it, ++ub_it) {
            assert(ub_it != ub.end());
            int size = *ub_it - *lb_it + 1;
            if (size <= 0) {
                throw std::domain_error("MultivariateDistributionDiscrete: marginal size not positive");
            }
            sizes.push_back(static_cast<unsigned int>(size));
        }
        _mi = std::move(MultiIndexMultisize(std::move(sizes)));
        if (static_cast<unsigned int>(probs.size()) != _mi.flat_size()) {
            throw std::domain_error("MultivariateDistributionDiscrete: bad probabilities size");
        }
        std::vector<std::vector<double>> mp(_mi.dim());
        for (unsigned int i = 0; i < _mi.dim(); ++i) {
            mp[i].resize(_mi.sizes()[i]);
            std::fill(mp[i].begin(), mp[i].end(), 0.0);
        }
        assert(_mi.flat_index() == 0);
        while (_mi.flat_index() < _mi.flat_size()) {
            const double p = probs[_mi.flat_index()];
            for (unsigned int i = 0; i < _mi.dim(); ++i) {
                mp[i][_mi.indices()[i]] += p;
            }
            ++_mi;
        }
        _marginals.reserve(_mi.dim());
        auto lit = _lb.begin();
        for (auto dit = mp.begin(); dit != mp.end(); ++dit, ++lit) {
            assert(lit != _lb.end());
            _marginals.push_back(std::move(DiscreteDistribution(*lit, std::move(*dit))));
        }
     }

            // Flatten the matrix row-by-row
            static Eigen::VectorXd flatten(Eigen::Ref<const Eigen::MatrixXd> probs) {
            Eigen::VectorXd result(probs.size());
            const unsigned int nr = static_cast<unsigned int>(probs.rows());
            const unsigned int nc = static_cast<unsigned int>(probs.cols());
            for (unsigned int r = 0; r < nr; ++r) {
            result.segment(r * nc, nc) = probs.row(r);
        }
            return result;
        }

            MultivariateDistributionDiscrete::MultivariateDistributionDiscrete(Eigen::Ref<const Eigen::MatrixXd> probs, int lb0, int lb1)
                : MultivariateDistributionDiscrete(flatten(probs), std::vector<int>({lb0, lb1}), std::vector<int>({static_cast<int>(lb0 + probs.rows() - 1), static_cast<int>(lb1 + probs.cols() - 1)})) {
        }
    
    void MultivariateDistributionDiscrete::draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        const size_t idx = static_cast<size_t>(_flat.draw(rng));
        _mi.decompose<double>(idx, x);
        for (unsigned int i = 0; i < _mi.dim(); ++i) {
            x[i] += _lb[i];
        }
    }

    void MultivariateDistributionDiscrete::draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const {
        const size_t idx = static_cast<size_t>(_flat.draw(rng));
        _mi.decompose<double>(idx, x);
        for (unsigned int i = 0; i < _mi.dim(); ++i) {
            x[i] += _lb[i];
        }
    }

    void MultivariateDistributionDiscrete::marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const {
        auto mit = _marginals.begin();
        for (unsigned int i = 0; i < dim(); ++i, ++mit) {
            assert(mit != _marginals.end());
            p[i] = mit->cdf(x[i]);
        }
    }

    void MultivariateDistributionDiscrete::marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const {
        auto mit = _marginals.begin();
        for (unsigned int i = 0; i < dim(); ++i, ++mit) {
            assert(mit != _marginals.end());
            x[i] = mit->icdf(p[i]);
        }
    }
    
    void MultivariateDistributionDiscrete::adjust_distribution(Eigen::Ref<Eigen::MatrixXd> /*sample*/) const {
        // No adjustment done because of unknown correlation structure.
        // TODO: log the fact that we didn't do anything
    }
}
