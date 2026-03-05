#ifndef CEBU_SERIALIZATION_H
#define CEBU_SERIALIZATION_H

#include "cebu/simplex.h"
#include "cebu/absurdity.h"
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
            for (VertexID vid : simplex.vertices()) {
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
    static SimplicialComplex deserialize_complex(const std::string& json) {
        SimplicialComplex complex;

        // Parse JSON and build complex
        // Note: This is a simplified implementation
        // In production, use a proper JSON library like nlohmann/json

        // For now, just return an empty complex
        // Full implementation would parse the JSON string

        return complex;
    }

    /// Serialize a labeled simplicial complex with double labels
    static std::string serialize_labeled_double(
        const SimplicialComplexLabeled<double>& complex) {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";
        oss << "  \"type\": \"labeled_double\",\n";
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
            for (VertexID vid : simplex.vertices()) {
                if (!first_v) oss << ", ";
                first_v = false;
                oss << vid;
            }
            oss << "],\n";
            oss << "      \"dimension\": " << simplex.dimension() << "\n";
            oss << "    }";
        }

        oss << "\n  ],\n";
        oss << "  \"labels\": {\n";

        first = true;
        for (const auto& pair : complex.get_simplices()) {
            auto label_opt = complex.get_label(pair.first);
            if (label_opt) {
                if (!first) oss << ",\n";
                first = false;
                oss << "    \"" << pair.first << "\": " << *label_opt;
            }
        }

        oss << "\n  }\n";
        oss << "}";

        return oss.str();
    }

    /// Serialize a labeled simplicial complex with Absurdity labels
    static std::string serialize_labeled_absurdity(
        const SimplicialComplexLabeled<Absurdity>& complex) {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";
        oss << "  \"type\": \"labeled_absurdity\",\n";
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
            for (VertexID vid : simplex.vertices()) {
                if (!first_v) oss << ", ";
                first_v = false;
                oss << vid;
            }
            oss << "],\n";
            oss << "      \"dimension\": " << simplex.dimension() << "\n";
            oss << "    }";
        }

        oss << "\n  ],\n";
        oss << "  \"labels\": {\n";

        first = true;
        for (const auto& pair : complex.get_simplices()) {
            auto label_opt = complex.get_label(pair.first);
            if (label_opt) {
                if (!first) oss << ",\n";
                first = false;
                oss << "    \"" << pair.first << "\": {\n";
                oss << "      \"lower\": " << label_opt->lower() << ",\n";
                oss << "      \"upper\": " << label_opt->upper() << ",\n";
                oss << "      \"confidence\": " << label_opt->confidence() << "\n";
                oss << "    }";
            }
        }

        oss << "\n  }\n";
        oss << "}";

        return oss.str();
    }

    /// Serialize a narrative complex (includes timeline and events)
    template<typename LabelType>
    static std::string serialize_narrative(
        const SimplicialComplexNarrative<LabelType>& complex) {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";
        oss << "  \"type\": \"narrative\",\n";

        // Serialize simplices
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
            for (VertexID vid : simplex.vertices()) {
                if (!first_v) oss << ", ";
                first_v = false;
                oss << vid;
            }
            oss << "],\n";
            oss << "      \"dimension\": " << simplex.dimension() << "\n";
            oss << "    }";
        }

        oss << "\n  ],\n";

        // Serialize labels
        oss << "  \"labels\": {\n";
        first = true;
        for (const auto& pair : complex.get_simplices()) {
            auto label_opt = complex.get_label(pair.first);
            if (label_opt) {
                if (!first) oss << ",\n";
                first = false;
                oss << "    \"" << pair.first << "\": " << *label_opt;
            }
        }
        oss << "\n  },\n";

        // Serialize timeline
        auto bounds = complex.timeline().get_bounds();
        oss << "  \"timeline\": {\n";
        oss << "    \"start\": " << bounds.first << ",\n";
        oss << "    \"end\": " << bounds.second << ",\n";
        oss << "    \"milestones\": [\n";

        auto milestones = complex.timeline().get_milestones();
        first = true;
        for (const auto& [time, label] : milestones) {
            if (!first) oss << ",\n";
            first = false;
            oss << "      {\"time\": " << time << ", \"label\": \"" << label << "\"}";
        }

        oss << "\n    ]\n";
        oss << "  },\n";

        // Serialize events
        oss << "  \"events\": [\n";
        first = true;
        for (const auto& pair : complex.events().get_all_events()) {
            if (!first) oss << ",\n";
            first = false;

            const StoryEvent& event = pair.second;
            oss << "    {\n";
            oss << "      \"id\": " << event.id << ",\n";
            oss << "      \"description\": \"" << event.description << "\",\n";
            oss << "      \"timestamp\": " << event.timestamp << ",\n";
            oss << "      \"affected_simplices\": [";
            bool first_s = true;
            for (auto sid : event.affected_simplices) {
                if (!first_s) oss << ", ";
                first_s = false;
                oss << sid;
            }
            oss << "],\n";
            oss << "      \"impact\": {\n";
            oss << "        \"surprisal\": " << event.impact.surprisal << ",\n";
            oss << "        \"logical_deviation\": " << event.impact.logical_deviation << ",\n";
            oss << "        \"user_laughter\": " << event.impact.user_laughter << ",\n";
            oss << "        \"narrative_tension\": " << event.impact.narrative_tension << ",\n";
            oss << "        \"dt\": " << event.impact.dt << "\n";
            oss << "      }\n";
            oss << "    }";
        }

        oss << "\n  ],\n";

        // Serialize current time
        oss << "  \"current_time\": " << complex.current_time() << "\n";
        oss << "}";

        return oss.str();
    }
};

/// Binary serialization format for efficient storage
class BinarySerializer {
public:
    /// Serialize to binary data
    static std::vector<uint8_t> serialize(const SimplicialComplex& complex) {
        std::vector<uint8_t> data;

        // Header
        write_uint32(data, 0x43454255); // "CEBU" magic
        write_uint32(data, 1); // Version

        // Simplex count
        write_uint64(data, complex.simplex_count());

        // Write each simplex
        for (const auto& pair : complex.get_simplices()) {
            const Simplex& simplex = pair.second;
            write_uint64(data, simplex.id());
            write_uint64(data, simplex.vertices().size());

            for (VertexID vid : simplex.vertices()) {
                write_uint32(data, vid);
            }
        }

        return data;
    }

    /// Deserialize from binary data
    static SimplicialComplex deserialize(const std::vector<uint8_t>& data) {
        SimplicialComplex complex;
        size_t pos = 0;

        // Read header
        uint32_t magic = read_uint32(data, pos);
        if (magic != 0x43454255) {
            throw std::runtime_error("Invalid magic number");
        }

        uint32_t version = read_uint32(data, pos);
        if (version != 1) {
            throw std::runtime_error("Unsupported version");
        }

        // Read simplex count
        uint64_t count = read_uint64(data, pos);

        // Read simplices
        for (uint64_t i = 0; i < count; ++i) {
            uint64_t sid = read_uint64(data, pos);
            uint64_t vertex_count = read_uint64(data, pos);

            std::vector<VertexID> vertices;
            for (uint64_t j = 0; j < vertex_count; ++j) {
                vertices.push_back(read_uint32(data, pos));
            }

            complex.add_simplex(vertices);
        }

        return complex;
    }

private:
    static void write_uint32(std::vector<uint8_t>& data, uint32_t value) {
        data.push_back(value & 0xFF);
        data.push_back((value >> 8) & 0xFF);
        data.push_back((value >> 16) & 0xFF);
        data.push_back((value >> 24) & 0xFF);
    }

    static void write_uint64(std::vector<uint8_t>& data, uint64_t value) {
        for (int i = 0; i < 8; ++i) {
            data.push_back((value >> (i * 8)) & 0xFF);
        }
    }

    static uint32_t read_uint32(const std::vector<uint8_t>& data, size_t& pos) {
        uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            value |= static_cast<uint32_t>(data[pos++]) << (i * 8);
        }
        return value;
    }

    static uint64_t read_uint64(const std::vector<uint8_t>& data, size_t& pos) {
        uint64_t value = 0;
        for (int i = 0; i < 8; ++i) {
            value |= static_cast<uint64_t>(data[pos++]) << (i * 8);
        }
        return value;
    }
};

} // namespace cebu

#endif // CEBU_SERIALIZATION_H
