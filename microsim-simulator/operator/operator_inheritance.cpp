// (C) Averisera Ltd 2014-2020
#include "operator_inheritance.hpp"
#include "../contexts.hpp"
#include "../history.hpp"
#include "../immutable_context.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"
#include "core/array_2d.hpp"
#include "core/copula_gaussian.hpp"
#include "core/math_utils.hpp"
#include "core/multivariate_distribution_gaussian.hpp"
#include "core/normal_distribution.hpp"
#include "core/rng.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        static const char* validate_input_distributions(const Array2D<std::shared_ptr<const Distribution>>& distributions,
                                                        const size_t expected_nrows,
                                                        const size_t expected_ncols) {
            if (distributions.size() != expected_nrows) {
                return "distribution row number mismatch";
            }
            for (auto rit = distributions.begin(); rit != distributions.end(); ++rit) {
                if (rit->size() != expected_ncols) {
                    return "distribution row length mismatch";
                }
                if (std::any_of(rit->begin(), rit->end(), [](const std::shared_ptr<const Distribution>& ptr) { return !ptr; })) {
                    return "one or more distributions is null";
                }
            }
            return nullptr;
        }
        
        OperatorInheritance::OperatorInheritance(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<Person>> predicate,
                                                 const Array2D<std::shared_ptr<const Distribution>>& mother_distributions,
                                                 const Array2D<std::shared_ptr<const Distribution>>& child_distributions,
                                                 const std::vector<std::shared_ptr<const CopulaGaussian>>& copulas,
                                                 unsigned int date_offset, std::shared_ptr<const ReferenceDateType> ref_date_type,
                                                 const std::vector<HistoryFactory::factory_t>& history_factories)
            : Operator<Person>(true, FeatureUser<Feature>::feature_set_t(variables.begin(), variables.end())),
            hist_gen_(variables.begin(), variables.end(), history_factories.begin(), history_factories.end(), predicate),
            _variables(variables),
            _predicate(predicate),
            _distributions(copulas.size(), 2 * variables.size()),
            _copulas(copulas),
            _date_offset(date_offset), _rdt(ref_date_type), _zero(2 * variables.size()) {
            _zero.fill(0.0);
            if (mother_distributions.size() != child_distributions.size()) {
                throw std::domain_error("OperatorInheritance: distribution vectors size mismatch");
            }
            if (_copulas.size() != mother_distributions.size()) {
                throw std::domain_error("OperatorInheritance: distribution and variable vectors size mismatch");
            }
            const size_t nvars = _variables.size();
            const char* msg = validate_input_distributions(mother_distributions, _copulas.size(), nvars);
            if (msg != nullptr) {
                throw std::domain_error(std::string("OperatorInheritance: invalid mother distributions: ") + msg);
            }
            msg = validate_input_distributions(child_distributions, _copulas.size(), nvars);
            if (msg != nullptr) {
                throw std::domain_error(std::string("OperatorInheritance: invalid child distributions: ") + msg);
            }
            if (std::any_of(_copulas.begin(), _copulas.end(), [](const std::shared_ptr<const CopulaGaussian>& ptr) { return !ptr; })) {
                throw std::domain_error("OperatorInheritance: null copula");
            }
            if (std::any_of(_copulas.begin(), _copulas.end(), [nvars](const std::shared_ptr<const CopulaGaussian>& ptr) { return ptr->dim() != nvars; })) {
                throw std::domain_error("OperatorInheritance: copula dimension mismatch");
            }
            if (!_rdt) {
                throw std::domain_error("OperatorInheritance: null reference data type");
            }
            if (std::any_of(history_factories.begin(), history_factories.end(), [](HistoryFactory::factory_t factory) { return !factory; })) {
                throw std::domain_error("OperatorInheritance: one or more history factories is null");
            }
            auto mit = mother_distributions.begin();
            auto cit = child_distributions.begin();
            auto dit = _distributions.row_begin();
            for (; mit != mother_distributions.end(); ++mit, ++cit, ++dit) {
                assert(cit != child_distributions.end());
                assert(dit != _distributions.row_end());
                std::copy(mit->begin(), mit->end(), (*dit).begin());
                std::copy(cit->begin(), cit->end(), (*dit).begin() + nvars);
            }
        }
            
        void OperatorInheritance::apply(const std::vector<std::shared_ptr<Person>>& selected, const Contexts& contexts) const {
            if (contexts.immutable_ctx().schedule().nbr_dates() > _copulas.size() + _date_offset) {
                throw std::out_of_range("OperatorInheritance: not enough data");
            }
            const Date asof = contexts.asof();
            const size_t asof_idx = contexts.asof_idx();
            if (asof_idx < _date_offset) {
                throw std::runtime_error("OperatorInheritance: applied too early");
            }
            const size_t idx = asof_idx - _date_offset;
            const unsigned int cond_dim = MathUtils::safe_cast<unsigned int>(_variables.size());
            Eigen::VectorXd a(2 * cond_dim);
            a.fill(std::numeric_limits<double>::quiet_NaN());
            Eigen::MatrixXd cond_cov;
            Eigen::VectorXd cond_mean;
            cond_mean.resize(cond_dim);
            cond_cov.resize(cond_dim, cond_dim);
            const CopulaGaussian& copula = (*_copulas.at(idx));
            const auto dist_row = _distributions[idx];
            for (auto cit = selected.begin(); cit != selected.end(); ++cit) {
                Person& child = **cit;
                const std::weak_ptr<const Person>& weak_mother = child.mother();
                if (weak_mother.expired()) {
                    // We cannot distinguish between a weak pointer which expired and a weak pointer which never pointed to anything.
                    continue;
                }
                const std::shared_ptr<const Person> mother = weak_mother.lock();
                const Date reference_date = _rdt->reference_date(*mother, child, contexts);
                const HistoryRegistry& mother_registry = mother->get_history_registry(contexts.immutable_ctx());
                for (unsigned int i = 0; i < cond_dim; ++i) {
                    const std::string& variable = _variables[i];
                    const auto hist_idx = mother_registry.variable_index(variable);
                    const History& mother_history = mother->history(hist_idx);
                    const double x = mother_history.last_as_double(reference_date);
                    if (!std::isnan(x)) {
                        a[i] = NormalDistribution::normsinv(dist_row[i]->cdf(x));
                    } else {
                        a[i] = x;
                    }
                }
                MultivariateDistributionGaussian::conditional(_zero, copula.rho(), a, cond_mean, cond_cov);
                const MultivariateDistributionGaussianSimple cond_distr(cond_mean, cond_cov);
                cond_distr.draw(contexts.mutable_ctx().rng(), a.head(cond_dim));
                const HistoryRegistry& child_registry = child.get_history_registry(contexts.immutable_ctx());                
                for (unsigned int i = 0; i < cond_dim; ++i) {
                    const std::string& variable = _variables[i];
                    const auto hist_idx = child_registry.variable_index(variable);
                    History& child_history = child.history(hist_idx);
                    const double x = dist_row[i + cond_dim]->icdf(NormalDistribution::normcdf(a[i]));
                    child_history.append(asof, x);
                }
            }
        }

        OperatorInheritance::ReferenceDateType::~ReferenceDateType() {
        }

        namespace {
            class ReferenceDataTypeConception: public OperatorInheritance::ReferenceDateType {
            public:
                Date reference_date(const Person& /*parent*/, const Person& child, const Contexts& /*ctx*/) const override {
                    return child.conception_date();
                }
            };

            class ReferenceDataTypeBirth: public OperatorInheritance::ReferenceDateType {
            public:
                Date reference_date(const Person& /*parent*/, const Person& child, const Contexts& /*ctx*/) const override {
                    return child.date_of_birth();
                }
            };
        }

        const std::shared_ptr<const OperatorInheritance::ReferenceDateType> OperatorInheritance::ReferenceDateType::CONCEPTION = std::make_shared<ReferenceDataTypeConception>();

        const std::shared_ptr<const OperatorInheritance::ReferenceDateType> OperatorInheritance::ReferenceDateType::BIRTH = std::make_shared<ReferenceDataTypeBirth>();
    }
}
