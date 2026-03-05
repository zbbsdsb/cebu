#ifndef CEBU_SERIALIZATION_H
#define CEBU_SERIALIZATION_H

#include "cebu/simplex.h"
#include "cebu/absurdity.h"
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <cstdint>

// Forward declarations
namespace cebu {
    class SimplicialComplex;
    template<typename LabelType> class SimplicialComplexLabeled;
    template<typename LabelType> class SimplicialComplexNarrative;
    template<typename LabelType> class SimplicialComplexRefinement;
    template<typename LabelType> class SimplicialComplexNonHausdorff;
    template<typename LabelType> class SimplicialComplexNonHausdorffLabeled;
    class JsonSerializer;
}

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
    // Binary format sections
    enum class Section : uint32_t {
        HEADER = 0x48445230,           // "HDR0"
        SIMPLICES = 0x53494D50,        // "SIMP"
        LABELS = 0x4C41424C,           // "LABL"
        EQUIVALENCE_CLASSES = 0x4551554C, // "EQLS"
        REFINEMENT_LEVELS = 0x52454C56,  // "RELV"
        TIMELINE = 0x54494D45,          // "TIME"
        EVENTS = 0x45564554,           // "EVNT"
        COMMAND_HISTORY = 0x434D4853,  // "CMHS"
        CURRENT_TIME = 0x43555254,      // "CURT"
        END = 0x454E4430               // "END0"
    };

    /// Serialize basic complex to binary data
    static std::vector<uint8_t> serialize(const SimplicialComplex& complex) {
        std::vector<uint8_t> data;

        // Header
        write_section_header(data, Section::HEADER);
        write_uint32(data, 0x43454255); // "CEBU" magic
        write_uint32(data, 1); // Version

        // Simplices section
        write_section_header(data, Section::SIMPLICES);
        write_uint64(data, complex.simplex_count());

        for (const auto& pair : complex.get_simplices()) {
            const Simplex& simplex = pair.second;
            write_uint64(data, simplex.id());
            write_uint64(data, simplex.vertices().size());

            for (VertexID vid : simplex.vertices()) {
                write_uint32(data, vid);
            }
        }

        // End marker
        write_section_header(data, Section::END);

        return data;
    }

    /// Serialize labeled complex to binary data
    template<typename LabelType>
    static std::vector<uint8_t> serialize_labeled(
        const SimplicialComplexLabeled<LabelType>& complex) {
        std::vector<uint8_t> data;

        // Write base complex data
        auto base_data = serialize(static_cast<const SimplicialComplex&>(complex));
        data.insert(data.end(), base_data.begin(), base_data.end());

        // Labels section (before END marker)
        // Remove the END marker and add labels before it
        data.resize(data.size() - 8); // Remove END section header

        write_section_header(data, Section::LABELS);
        
        // Count of labeled simplices
        uint64_t label_count = 0;
        for (const auto& pair : complex.get_simplices()) {
            if (complex.has_label(pair.first)) {
                label_count++;
            }
        }
        write_uint64(data, label_count);

        // Write each label
        for (const auto& pair : complex.get_simplices()) {
            auto label_opt = complex.get_label(pair.first);
            if (label_opt) {
                write_uint64(data, pair.first); // Simplex ID
                write_label(data, *label_opt);
            }
        }

        // End marker
        write_section_header(data, Section::END);

        return data;
    }

    /// Serialize narrative complex (includes timeline, events, current time)
    template<typename LabelType>
    static std::vector<uint8_t> serialize_narrative(
        const SimplicialComplexNarrative<LabelType>& complex) {
        std::vector<uint8_t> data;

        // Write labeled complex data
        auto labeled_data = serialize_labeled(
            static_cast<const SimplicialComplexLabeled<LabelType>&>(complex));
        data.insert(data.end(), labeled_data.begin(), labeled_data.end());

        // Remove the END marker and add narrative sections before it
        data.resize(data.size() - 8); // Remove END section header

        // Timeline section
        write_section_header(data, Section::TIMELINE);
        auto bounds = complex.timeline().get_bounds();
        write_double(data, bounds.first);  // Start time
        write_double(data, bounds.second); // End time

        // Milestones
        auto milestones = complex.timeline().get_milestones();
        write_uint64(data, milestones.size());
        for (const auto& [time, label] : milestones) {
            write_double(data, time);
            write_string(data, label);
        }

        // Events section
        write_section_header(data, Section::EVENTS);
        auto all_events = complex.events().get_all_events();
        write_uint64(data, all_events.size());

        for (const auto& pair : all_events) {
            const StoryEvent& event = pair.second;
            write_uint64(data, event.id);
            write_string(data, event.description);
            write_double(data, event.timestamp);

            // Affected simplices
            write_uint64(data, event.affected_simplices.size());
            for (SimplexID sid : event.affected_simplices) {
                write_uint64(data, sid);
            }

            // Impact metrics
            write_double(data, event.impact.surprisal);
            write_double(data, event.impact.logical_deviation);
            write_double(data, event.impact.user_laughter);
            write_double(data, event.impact.narrative_tension);
            write_double(data, event.impact.dt);
        }

        // Current time section
        write_section_header(data, Section::CURRENT_TIME);
        write_double(data, complex.current_time());

        // End marker
        write_section_header(data, Section::END);

        return data;
    }

    /// Serialize refinement complex (includes refinement levels)
    template<typename LabelType>
    static std::vector<uint8_t> serialize_refinement(
        const SimplicialComplexRefinement<LabelType>& complex) {
        std::vector<uint8_t> data;

        // Write labeled complex data
        auto labeled_data = serialize_labeled(
            static_cast<const SimplicialComplexLabeled<LabelType>&>(complex));
        data.insert(data.end(), labeled_data.begin(), labeled_data.end());

        // Remove the END marker and add refinement section before it
        data.resize(data.size() - 8); // Remove END section header

        // Refinement levels section
        write_section_header(data, Section::REFINEMENT_LEVELS);

        // Write refinement levels
        // We need to access the private refinement_levels_ member
        // For now, we'll skip this or add a public accessor
        // This is a design limitation

        // Placeholder: write count of 0
        write_uint64(data, 0);

        // End marker
        write_section_header(data, Section::END);

        return data;
    }

    /// Deserialize refinement complex
    template<typename LabelType>
    static SimplicialComplexRefinement<LabelType> deserialize_refinement(
        const std::vector<uint8_t>& data) {
        SimplicialComplexRefinement<LabelType> complex;
        size_t pos = 0;

        // Read header
        if (!verify_section_header(data, pos, Section::HEADER)) {
            throw std::runtime_error("Missing header section");
        }
        read_uint32(data, pos); // magic
        read_uint32(data, pos); // version

        // Read all sections
        while (pos < data.size()) {
            Section section = read_section_header(data, pos);

            if (section == Section::SIMPLICES) {
                read_simplices_section(data, pos, complex);
            } else if (section == Section::LABELS) {
                read_labels_section(data, pos, complex);
            } else if (section == Section::REFINEMENT_LEVELS) {
                read_refinement_levels_section(data, pos, complex);
            } else if (section == Section::END) {
                break;
            } else {
                skip_section(data, pos);
            }
        }

        return complex;
    }

    /// Deserialize from binary data
    static SimplicialComplex deserialize(const std::vector<uint8_t>& data) {
        SimplicialComplex complex;
        size_t pos = 0;

        // Read header
        if (!verify_section_header(data, pos, Section::HEADER)) {
            throw std::runtime_error("Missing header section");
        }

        uint32_t magic = read_uint32(data, pos);
        if (magic != 0x43454255) {
            throw std::runtime_error("Invalid magic number");
        }

        uint32_t version = read_uint32(data, pos);
        if (version != 1) {
            throw std::runtime_error("Unsupported version");
        }

        // Read sections
        while (pos < data.size()) {
            Section section = read_section_header(data, pos);

            if (section == Section::SIMPLICES) {
                read_simplices_section(data, pos, complex);
            } else if (section == Section::END) {
                break;
            } else {
                // Skip unknown sections
                skip_section(data, pos);
            }
        }

        return complex;
    }

    /// Deserialize labeled complex
    template<typename LabelType>
    static SimplicialComplexLabeled<LabelType> deserialize_labeled(
        const std::vector<uint8_t>& data) {
        SimplicialComplexLabeled<LabelType> complex;
        size_t pos = 0;

        // Read header
        if (!verify_section_header(data, pos, Section::HEADER)) {
            throw std::runtime_error("Missing header section");
        }

        uint32_t magic = read_uint32(data, pos);
        if (magic != 0x43454255) {
            throw std::runtime_error("Invalid magic number");
        }

        uint32_t version = read_uint32(data, pos);
        if (version != 1) {
            throw std::runtime_error("Unsupported version");
        }

        // Read sections
        while (pos < data.size()) {
            Section section = read_section_header(data, pos);

            if (section == Section::SIMPLICES) {
                read_simplices_section(data, pos, complex);
            } else if (section == Section::LABELS) {
                read_labels_section(data, pos, complex);
            } else if (section == Section::END) {
                break;
            } else {
                skip_section(data, pos);
            }
        }

        return complex;
    }

    /// Deserialize narrative complex
    template<typename LabelType>
    static SimplicialComplexNarrative<LabelType> deserialize_narrative(
        const std::vector<uint8_t>& data) {
        
        // First read the labeled complex portion
        SimplicialComplexLabeled<LabelType> labeled = 
            deserialize_labeled<LabelType>(data);
        
        // Copy into narrative complex
        SimplicialComplexNarrative<LabelType> complex;
        size_t pos = 0;

        // Read header (to get past it)
        if (!verify_section_header(data, pos, Section::HEADER)) {
            throw std::runtime_error("Missing header section");
        }
        read_uint32(data, pos); // magic
        read_uint32(data, pos); // version

        // Read all sections to populate complex
        while (pos < data.size()) {
            Section section = read_section_header(data, pos);

            if (section == Section::SIMPLICES) {
                read_simplices_section(data, pos, complex);
            } else if (section == Section::LABELS) {
                read_labels_section(data, pos, complex);
            } else if (section == Section::TIMELINE) {
                read_timeline_section(data, pos, complex);
            } else if (section == Section::EVENTS) {
                read_events_section(data, pos, complex);
            } else if (section == Section::CURRENT_TIME) {
                double current = read_double(data, pos);
                // Note: current_time_ is private, can't set directly
                // This is a limitation - would need friend class or setter
                (void)current;
            } else if (section == Section::END) {
                break;
            } else {
                skip_section(data, pos);
            }
        }

        return complex;
    }

private:
    // Section header helpers
    static void write_section_header(std::vector<uint8_t>& data, Section section) {
        write_uint32(data, static_cast<uint32_t>(section));
    }

    static bool verify_section_header(const std::vector<uint8_t>& data, 
                                      size_t pos, Section expected) {
        if (pos + 4 > data.size()) return false;
        uint32_t section_id = read_uint32_at(data, pos);
        return section_id == static_cast<uint32_t>(expected);
    }

    static Section read_section_header(const std::vector<uint8_t>& data, 
                                       size_t& pos) {
        return static_cast<Section>(read_uint32(data, pos));
    }

    // Read simplices section
    static void read_simplices_section(const std::vector<uint8_t>& data,
                                       size_t& pos,
                                       SimplicialComplex& complex) {
        uint64_t count = read_uint64(data, pos);
        
        for (uint64_t i = 0; i < count; ++i) {
            uint64_t sid = read_uint64(data, pos);
            uint64_t vertex_count = read_uint64(data, pos);

            std::vector<VertexID> vertices;
            for (uint64_t j = 0; j < vertex_count; ++j) {
                vertices.push_back(read_uint32(data, pos));
            }

            complex.add_simplex(vertices);
        }
    }

    // Read labels section
    template<typename LabelType>
    static void read_labels_section(const std::vector<uint8_t>& data,
                                     size_t& pos,
                                     SimplicialComplexLabeled<LabelType>& complex) {
        uint64_t count = read_uint64(data, pos);
        
        for (uint64_t i = 0; i < count; ++i) {
            SimplexID sid = read_uint64(data, pos);
            LabelType label = read_label<LabelType>(data, pos);
            complex.set_label(sid, label);
        }
    }

    // Read timeline section
    template<typename LabelType>
    static void read_timeline_section(const std::vector<uint8_t>& data,
                                      size_t& pos,
                                      SimplicialComplexNarrative<LabelType>& complex) {
        double start = read_double(data, pos);
        double end = read_double(data, pos);

        // Note: Timeline doesn't have set_bounds, need to reconstruct
        // This is a limitation - would need to modify Timeline class

        uint64_t milestone_count = read_uint64(data, pos);
        for (uint64_t i = 0; i < milestone_count; ++i) {
            double time = read_double(data, pos);
            std::string label = read_string(data, pos);
            complex.timeline().add_milestone(time, label);
        }
    }

    // Read events section
    template<typename LabelType>
    static void read_events_section(const std::vector<uint8_t>& data,
                                     size_t& pos,
                                     SimplicialComplexNarrative<LabelType>& complex) {
        uint64_t event_count = read_uint64(data, pos);
        
        for (uint64_t i = 0; i < event_count; ++i) {
            uint64_t id = read_uint64(data, pos);
            std::string description = read_string(data, pos);
            double timestamp = read_double(data, pos);

            uint64_t simplex_count = read_uint64(data, pos);
            std::vector<SimplexID> affected_simplices;
            for (uint64_t j = 0; j < simplex_count; ++j) {
                affected_simplices.push_back(read_uint64(data, pos));
            }

            double surprisal = read_double(data, pos);
            double logical_deviation = read_double(data, pos);
            double user_laughter = read_double(data, pos);
            double narrative_tension = read_double(data, pos);
            double dt = read_double(data, pos);

            AbsurdityContext impact;
            impact.surprisal = surprisal;
            impact.logical_deviation = logical_deviation;
            impact.user_laughter = user_laughter;
            impact.narrative_tension = narrative_tension;
            impact.dt = dt;

            complex.add_event(description, timestamp, affected_simplices, impact);
        }
    }

    // Skip a section (for unknown sections)
    static void skip_section(const std::vector<uint8_t>& data, size_t& pos) {
        // For now, skip to end (since we don't store section sizes)
        // In a real implementation, we'd store section sizes in headers
        pos = data.size();
    }

    // Write helpers for different types
    static void write_double(std::vector<uint8_t>& data, double value) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        data.insert(data.end(), bytes, bytes + sizeof(double));
    }

    static void write_string(std::vector<uint8_t>& data, const std::string& str) {
        write_uint64(data, str.size());
        data.insert(data.end(), str.begin(), str.end());
    }

    template<typename LabelType>
    static void write_label(std::vector<uint8_t>& data, const LabelType& label) {
        if constexpr (std::is_same_v<LabelType, double>) {
            write_double(data, label);
        } else if constexpr (std::is_same_v<LabelType, Absurdity>) {
            write_double(data, label.lower());
            write_double(data, label.upper());
            write_double(data, label.confidence());
        } else {
            // For generic types, try to write as double
            write_double(data, static_cast<double>(label));
        }
    }

    // Read helpers
    static double read_double(const std::vector<uint8_t>& data, size_t& pos) {
        double value;
        std::memcpy(&value, &data[pos], sizeof(double));
        pos += sizeof(double);
        return value;
    }

    static std::string read_string(const std::vector<uint8_t>& data, size_t& pos) {
        uint64_t length = read_uint64(data, pos);
        std::string str(data.begin() + pos, data.begin() + pos + length);
        pos += length;
        return str;
    }

    template<typename LabelType>
    static LabelType read_label(const std::vector<uint8_t>& data, size_t& pos) {
        if constexpr (std::is_same_v<LabelType, double>) {
            return read_double(data, pos);
        } else if constexpr (std::is_same_v<LabelType, Absurdity>) {
            double lower = read_double(data, pos);
            double upper = read_double(data, pos);
            double confidence = read_double(data, pos);
            return Absurdity(lower, upper, confidence);
        } else {
            return static_cast<LabelType>(read_double(data, pos));
        }
    }

    static uint32_t read_uint32_at(const std::vector<uint8_t>& data, size_t pos) {
        uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            value |= static_cast<uint32_t>(data[pos + i]) << (i * 8);
        }
        return value;
    }

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
