/*
 * (C) Averisera Ltd 2015
 */
#include "copula_independent.hpp"
#include "rng.hpp"
#include "statistics.hpp"
#include <cassert>
#include <stdexcept>

namespace averisera {
    CopulaIndependent::CopulaIndependent(size_t dim)
    : _dim(dim) {
    }
 
    /*
    double CopulaIndependent::joint_cdf(const std::vector<double>& marginal_cdfs) const {
        if (marginal_cdfs.size() != _dim) {
            throw std::domain_error("CopulaIndependent: Incorrect argument size");
        }
        double prod = 1.;
        for (auto it = marginal_cdfs.begin(); it != marginal_cdfs.end(); ++it) {
            assert(*it >= 0 && *it <= 1);
            prod *= *it;
        }
        return prod;
    }
    */
    
    void CopulaIndependent::draw_cdfs(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        if (static_cast<size_t>(x.size()) != _dim) {
            throw std::domain_error("CopulaIndependent: Incorrect argument size");
        }
        const auto end = x.data() + _dim;
        for (auto it = x.data(); it != end; ++it) {
            *it = rng.next_uniform();
        }
    }
    
    void CopulaIndependent::adjust_cdfs(Eigen::Ref<Eigen::MatrixXd> sample) const {
        Statistics::percentiles_inplace(sample);
    }
}
