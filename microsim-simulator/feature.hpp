#ifndef __AVERISERA_MS_FEATURE_H
#define __AVERISERA_MS_FEATURE_H

#include <functional>
#include <iosfwd>
#include <string>
#include <unordered_set>
#include <vector>

namespace averisera {
    namespace microsim {
        /** Denotes a feature which is required or provided by a FeatureProvider */
        class Feature {
        public:
            Feature() = default;

            /** @throw std::domain_error If name is empty */
            Feature(const std::string& name);

            /** @throw std::domain_error If name is empty or null */
            Feature(const char* name);

            Feature(const Feature& other) = default;

            Feature& operator=(const Feature& other) = default;

            bool operator==(const Feature& other) const;

            bool operator!=(const Feature& other) const;

            bool operator<(const Feature& other) const;
            
            /** Return empty feature set; set type must be the same as in FeatureProvider::feature_set_t typedef */
            static const std::unordered_set<Feature>& empty();
            
            static std::unordered_set<Feature> from_names(const std::vector<std::string>& names);

			static std::unordered_set<Feature> from_name(const std::string& name);

            friend std::ostream& operator<<(std::ostream& stream, const Feature& feature);            

            friend struct std::hash<Feature>;
        private:
            std::string _name;
        };        
    }
}

namespace std {
    template <> struct hash<averisera::microsim::Feature> {
        typedef averisera::microsim::Feature argument_type;
        typedef size_t result_type;
        result_type operator()(const argument_type& feature) const {
            return std::hash<std::string>{}(feature._name);
        }
    };
}

#endif // __AVERISERA_MS_FEATURE_H
