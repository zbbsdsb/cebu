#ifndef CEBU_SERIALIZATION_H
#define CEBU_SERIALIZATION_H

#include "cebu/simplex.h"
#include "cebu/absurdity.h"
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <cstdint>

// Forward declarations
namespace cebu {
    class SimplicialComplex;
    template<typename LabelType> class SimplicialComplexLabeled;
    template<typename LabelType> class SimplicialComplexNarrative;
    template<typename LabelType> class SimplicialComplexRefinement;
    class SimplicialComplexNonHausdorff;
    template<typename LabelType> class SimplicialComplexNonHausdorffLabeled;
}

namespace cebu {

/// JSON serialization format for simplicial complexes
class JsonSerializer {
public:
    /// Serialize a simplicial complex to JSON string
    static std::string serialize(const SimplicialComplex& complex);

    /// Deserialize a simplicial complex from JSON string
    static SimplicialComplex deserialize_complex(const std::string& json);

    /// Serialize a labeled simplicial complex with double labels
    static std::string serialize_labeled_double(
        const SimplicialComplexLabeled<double>& complex);

    /// Serialize a narrative complex (includes timeline and events)
    template<typename LabelType>
    static std::string serialize_narrative(
        const SimplicialComplexNarrative<LabelType>& complex);
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
    static std::vector<uint8_t> serialize(const SimplicialComplex& complex);

    /// Serialize labeled complex to binary data
    template<typename LabelType>
    static std::vector<uint8_t> serialize_labeled(
        const SimplicialComplexLabeled<LabelType>& complex);

    /// Deserialize basic complex from binary data
    static SimplicialComplex deserialize(const std::vector<uint8_t>& data);

    /// Deserialize labeled complex from binary data
    template<typename LabelType>
    static SimplicialComplexLabeled<LabelType> deserialize_labeled(
        const std::vector<uint8_t>& data);

private:
    /// Write a section header to binary data
    static void write_section_header(std::vector<uint8_t>& data, Section section);

    /// Write a 32-bit unsigned integer
    static void write_uint32(std::vector<uint8_t>& data, uint32_t value);

    /// Write a 64-bit unsigned integer
    static void write_uint64(std::vector<uint8_t>& data, uint64_t value);

    /// Write a double precision float
    static void write_double(std::vector<uint8_t>& data, double value);

    /// Read a 32-bit unsigned integer
    static uint32_t read_uint32(const uint8_t* data, size_t& offset);

    /// Read a 64-bit unsigned integer
    static uint64_t read_uint64(const uint8_t* data, size_t& offset);

    /// Read a double precision float
    static double read_double(const uint8_t* data, size_t& offset);

    /// Read a section header
    static Section read_section_header(const uint8_t* data, size_t& offset);
};

} // namespace cebu

#endif // CEBU_SERIALIZATION_H
