#ifndef CEBU_SERIALIZATION_H
#define CEBU_SERIALIZATION_H

#include "cebu/simplicial_complex.h"
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>

namespace cebu {

/// JSON serialization format for simplicial complexes
class JsonSerializer {
public:
    /// Serialize a simplicial complex to JSON string
    static std::string serialize(const SimplicialComplex& complex) {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";
        oss << "  \"type\": \"basic\",\n";
        oss << "  \"simplices\": [\n";

        bool first = true;
        for (const auto& pair : complex.get_simplices()) {
            if (!first) oss << ",\n";
            first = false;

            const Simplex& simplex = pair.second;
            oss << "    {\n";
            oss << "      \"id\": " << simplex.id() << ",\n";
            oss << "      \"vertices\": [";
            bool first_v = true;
            for (auto vid : simplex.vertices()) {
                if (!first_v) oss << ", ";
                first_v = false;
                oss << vid;
            }
            oss << "],\n";
            oss << "      \"dimension\": " << simplex.dimension() << "\n";
            oss << "    }";
        }

        oss << "\n  ]\n";
        oss << "}";

        return oss.str();
    }

    /// Deserialize a simplicial complex from JSON string
    static SimplicialComplex deserialize(const std::string& json) {
        SimplicialComplex complex;
        // Simplified implementation - returns empty complex
        // Full implementation would use nlohmann::json parser
        return complex;
    }

    /// Deserialize from nlohmann::json object (for use with nlohmann/json library)
    static SimplicialComplex deserialize(const nlohmann::json& j) {
        SimplicialComplex complex;
        // Simplified implementation
        return complex;
    }

    /// Get JSON representation as nlohmann::json object
    static nlohmann::json to_json(const SimplicialComplex& complex) {
        nlohmann::json j;
        j["version"] = "1.0";
        j["type"] = "basic";
        j["simplices"] = nlohmann::json::array();

        for (const auto& pair : complex.get_simplices()) {
            const Simplex& simplex = pair.second;
            nlohmann::json s;
            s["id"] = simplex.id();
            s["vertices"] = simplex.vertices();
            s["dimension"] = simplex.dimension();
            j["simplices"].push_back(s);
        }

        return j;
    }
};

} // namespace cebu

#endif // CEBU_SERIALIZATION_H
