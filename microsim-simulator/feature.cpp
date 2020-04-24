#include "feature.hpp"
#include "core/log.hpp"
#include "core/stl_utils.hpp"
#include <algorithm>
#include <iterator>
#include <ostream>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        Feature::Feature(const std::string& name)
            : _name(name) {
            if (name.empty()) {
                throw std::domain_error("Feature: empty name");
            }
        }

        Feature::Feature(const char* name) {
            if (!name) {
                throw std::domain_error("Feature: null name");
            }
            _name = name;
            if (_name.empty()) {
                throw std::domain_error("Feature: empty name");
            }
        }

        bool Feature::operator==(const Feature& other) const {
            return _name == other._name;
        }
        bool Feature::operator!=(const Feature& other) const {
            return _name != other._name;
        }

        bool Feature::operator<(const Feature& other) const {
            return _name < other._name;
        }
                
        const std::unordered_set<Feature>& Feature::empty() {
            static const std::unordered_set<Feature> empty_features;
            return empty_features;
        }
        
        std::unordered_set<Feature> Feature::from_names(const std::vector<std::string>& names) {
			LOG_TRACE() << "Feature::from_names: names=" << names;
            std::unordered_set<Feature> features;
            std::transform(names.begin(), names.end(), std::inserter(features, features.begin()), [](const std::string& name){ return Feature(name); });
			LOG_TRACE() << "Feature::from_names: features=" << features;
            return features;
        }

		std::unordered_set<Feature> Feature::from_name(const std::string& name) {
			LOG_TRACE() << "Feature::from_name: name=" << name;
			std::unordered_set<Feature> features;
			features.insert(Feature(name));
			LOG_TRACE() << "Feature::from_name: features=" << features;
			return features;
		}

        std::ostream& operator<<(std::ostream& stream, const Feature& feature) {
            stream << feature._name;
            return stream;
        }
    }
}
