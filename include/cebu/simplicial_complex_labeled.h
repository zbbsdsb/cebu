#pragma once

#include "cebu/simplicial_complex.h"
#include "cebu/label.h"
#include "cebu/absurdity.h"
#include <memory>
#include <type_traits>

namespace cebu {

/**
 * @brief Labeled simplicial complex class
 *
 * Extends SimplicialComplex with label support. Labels can be any type
 * and are managed through a label system.
 *
 * @tparam LabelType Type of the label (e.g., double, Absurdity, custom struct)
 * @tparam LabelSystemType Label system implementation (defaults to DefaultLabelSystem)
 */
template<typename LabelType, template<typename> class LabelSystemType = DefaultLabelSystem>
class SimplicialComplexLabeled {
public:
    using LabelSys = LabelSystemType<LabelType>;
    using Predicate = typename LabelSystem<LabelType>::Predicate;

    /**
     * @brief Constructor
     * @param label_system Optional label system (if nullptr, creates default)
     */
    explicit SimplicialComplexLabeled(std::unique_ptr<LabelSys> label_system = nullptr)
        : complex_(std::make_unique<SimplicialComplex>()),
          label_system_(label_system ? std::move(label_system) : std::make_unique<LabelSys>()) {}

    // Forward basic operations to underlying complex

    /**
     * @brief Add a vertex
     */
    VertexID add_vertex() {
        return complex_->add_vertex();
    }

    /**
     * @brief Add an edge
     */
    SimplexID add_edge(VertexID v1, VertexID v2) {
        return complex_->add_edge(v1, v2);
    }

    /**
     * @brief Add a triangle
     */
    SimplexID add_triangle(VertexID v1, VertexID v2, VertexID v3) {
        return complex_->add_triangle(v1, v2, v3);
    }

    /**
     * @brief Add a k-simplex
     */
    SimplexID add_simplex(const std::vector<VertexID>& vertices) {
        return complex_->add_simplex(vertices);
    }

    /**
     * @brief Remove a simplex (also removes its label)
     */
    bool remove_simplex(SimplexID simplex_id, bool cascade = false) {
        if (complex_->remove_simplex(simplex_id, cascade)) {
            label_system_->remove_label(simplex_id);
            return true;
        }
        return false;
    }

    /**
     * @brief Remove a vertex
     */
    bool remove_vertex(VertexID vertex_id, bool cascade = true) {
        return complex_->remove_vertex(vertex_id, cascade);
    }

    // Query operations (forwarded)

    const std::vector<VertexID>& get_vertices() const {
        return complex_->get_vertices();
    }

    const std::unordered_map<SimplexID, Simplex>& get_simplices() const {
        return complex_->get_simplices();
    }

    std::vector<SimplexID> get_simplices_of_dimension(size_t dimension) const {
        return complex_->get_simplices_of_dimension(dimension);
    }

    bool has_simplex(SimplexID id) const {
        return complex_->has_simplex(id);
    }

    const Simplex& get_simplex(SimplexID id) const {
        return complex_->get_simplex(id);
    }

    std::vector<SimplexID> get_simplices_containing_vertex(VertexID vertex_id) const {
        return complex_->get_simplices_containing_vertex(vertex_id);
    }

    std::vector<SimplexID> get_adjacent_simplices(SimplexID simplex_id) const {
        return complex_->get_adjacent_simplices(simplex_id);
    }

    std::vector<SimplexID> get_facets(SimplexID simplex_id) const {
        return complex_->get_facets(simplex_id);
    }

    size_t simplex_count() const {
        return complex_->simplex_count();
    }

    size_t vertex_count() const {
        return complex_->vertex_count();
    }

    // Label operations

    /**
     * @brief Set a label for a simplex
     * @param simplex_id ID of the simplex
     * @param label Label value
     */
    void set_label(SimplexID simplex_id, const LabelType& label) {
        label_system_->set_label(simplex_id, label);
    }

    /**
     * @brief Get the label for a simplex
     * @param simplex_id ID of the simplex
     * @return Optional containing the label, or empty if not set
     */
    std::optional<LabelType> get_label(SimplexID simplex_id) const {
        return label_system_->get_label(simplex_id);
    }

    /**
     * @brief Find all simplices with labels matching a predicate
     * @param predicate Function that returns true for matching labels
     * @return List of simplex IDs
     */
    std::vector<SimplexID> find_by_label(Predicate predicate) const {
        return label_system_->find_by_label(predicate);
    }

    /**
     * @brief Remove a label from a simplex
     * @param simplex_id ID of the simplex
     * @return true if label was removed, false if it didn't exist
     */
    bool remove_label(SimplexID simplex_id) {
        return label_system_->remove_label(simplex_id);
    }

    /**
     * @brief Check if a simplex has a label
     * @param simplex_id ID of the simplex
     */
    bool has_label(SimplexID simplex_id) const {
        return label_system_->has_label(simplex_id);
    }

    /**
     * @brief Get the number of labeled simplices
     */
    size_t labeled_count() const {
        return label_system_->labeled_count();
    }

    /**
     * @brief Clear all labels
     */
    void clear_labels() {
        label_system_->clear();
    }

    /**
     * @brief Get access to the label system
     */
    LabelSys& label_system() {
        return *label_system_;
    }

    /**
     * @brief Get const access to the label system
     */
    const LabelSys& label_system() const {
        return *label_system_;
    }

    /**
     * @brief Find simplices with high label values (only for numeric labels)
     */
    template<typename T = LabelType>
    std::enable_if_t<std::is_arithmetic_v<T>, std::vector<SimplexID>>
    find_high_labels(double threshold = 0.7) const {
        std::vector<SimplexID> result;
        const auto& labels = static_cast<const DefaultLabelSystem<T>*>(label_system_.get())->get_all_labels();
        for (const auto& pair : labels) {
            if (pair.second > threshold) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with low label values (only for numeric labels)
     */
    template<typename T = LabelType>
    std::enable_if_t<std::is_arithmetic_v<T>, std::vector<SimplexID>>
    find_low_labels(double threshold = 0.3) const {
        std::vector<SimplexID> result;
        const auto& labels = static_cast<const DefaultLabelSystem<T>*>(label_system_.get())->get_all_labels();
        for (const auto& pair : labels) {
            if (pair.second < threshold) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    /**
     * @brief Find simplices with labels in a range (only for numeric labels)
     */
    template<typename T = LabelType>
    std::enable_if_t<std::is_arithmetic_v<T>, std::vector<SimplexID>>
    find_labels_in_range(double min, double max) const {
        std::vector<SimplexID> result;
        const auto& labels = static_cast<const DefaultLabelSystem<T>*>(label_system_.get())->get_all_labels();
        for (const auto& pair : labels) {
            if (pair.second >= min && pair.second <= max) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

private:
    std::unique_ptr<SimplicialComplex> complex_;
    std::unique_ptr<LabelSystem<LabelType>> label_system_;
};

// Convenience aliases

/**
 * @brief Labeled complex with double labels (e.g., for generic numeric labels)
 */
using SimplicialComplexDouble = SimplicialComplexLabeled<double>;

/**
 * @brief Labeled complex with absurdity labels
 */
using SimplicialComplexAbsurdity = SimplicialComplexLabeled<Absurdity, AbsurdityLabelSystem::template rebind>;

} // namespace cebu
