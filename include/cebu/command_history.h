#ifndef CEBU_COMMAND_HISTORY_H
#define CEBU_COMMAND_HISTORY_H

#include <memory>
#include <vector>
#include <stdexcept>
#include <functional>

namespace cebu {

/// Base class for reversible commands
class Command {
public:
    virtual ~Command() = default;

    /// Execute the command
    virtual void execute() = 0;

    /// Undo the command
    virtual void undo() = 0;

    /// Get command description
    virtual std::string description() const = 0;
};

/// Command for adding a simplex to a complex
template<typename ComplexType>
class AddSimplexCommand : public Command {
public:
    AddSimplexCommand(
        ComplexType& complex,
        const std::vector<typename ComplexType::VertexID>& vertices,
        std::function<void(typename ComplexType::SimplexID)> on_execute = nullptr)
        : complex_(complex)
        , vertices_(vertices)
        , on_execute_(std::move(on_execute))
        , executed_(false) {}

    void execute() override {
        if (executed_) {
            throw std::runtime_error("Command already executed");
        }

        simplex_id_ = complex_.add_simplex(vertices_);
        executed_ = true;

        if (on_execute_) {
            on_execute_(simplex_id_);
        }
    }

    void undo() override {
        if (!executed_) {
            throw std::runtime_error("Command not executed");
        }

        complex_.remove_simplex(simplex_id_, false); // Don't cascade
        executed_ = false;
    }

    std::string description() const override {
        return "Add simplex with " + std::to_string(vertices_.size()) + " vertices";
    }

    typename ComplexType::SimplexID simplex_id() const { return simplex_id_; }

private:
    ComplexType& complex_;
    std::vector<typename ComplexType::VertexID> vertices_;
    std::function<void(typename ComplexType::SimplexID)> on_execute_;
    typename ComplexType::SimplexID simplex_id_;
    bool executed_;
};

/// Command for removing a simplex from a complex
template<typename ComplexType>
class RemoveSimplexCommand : public Command {
public:
    RemoveSimplexCommand(
        ComplexType& complex,
        typename ComplexType::SimplexID simplex_id,
        bool cascade = false)
        : complex_(complex)
        , simplex_id_(simplex_id)
        , cascade_(cascade)
        , executed_(false) {

        // Store simplex data for undo
        if (complex_.has_simplex(simplex_id)) {
            const auto& simplex = complex_.get_simplex(simplex_id);
            vertices_ = simplex.vertices();
            can_undo_ = true;
        } else {
            can_undo_ = false;
        }
    }

    void execute() override {
        if (executed_) {
            throw std::runtime_error("Command already executed");
        }

        success_ = complex_.remove_simplex(simplex_id_, cascade_);
        executed_ = true;
    }

    void undo() override {
        if (!executed_) {
            throw std::runtime_error("Command not executed");
        }

        if (!can_undo_) {
            throw std::runtime_error("Cannot undo: simplex did not exist");
        }

        if (success_) {
            complex_.add_simplex(vertices_);
        }

        executed_ = false;
    }

    std::string description() const override {
        return "Remove simplex " + std::to_string(simplex_id_);
    }

private:
    ComplexType& complex_;
    typename ComplexType::SimplexID simplex_id_;
    bool cascade_;
    std::vector<typename ComplexType::VertexID> vertices_;
    bool executed_;
    bool success_;
    bool can_undo_;
};

/// Command for setting a label on a simplex
template<typename ComplexType, typename LabelType>
class SetLabelCommand : public Command {
public:
    SetLabelCommand(
        ComplexType& complex,
        typename ComplexType::SimplexID simplex_id,
        const LabelType& new_label)
        : complex_(complex)
        , simplex_id_(simplex_id)
        , new_label_(new_label)
        , executed_(false)
        , had_label_(false) {

        // Store old label for undo
        auto old_label_opt = complex_.get_label(simplex_id);
        if (old_label_opt) {
            old_label_ = *old_label_opt;
            had_label_ = true;
        }
    }

    void execute() override {
        if (executed_) {
            throw std::runtime_error("Command already executed");
        }

        complex_.set_label(simplex_id_, new_label_);
        executed_ = true;
    }

    void undo() override {
        if (!executed_) {
            throw std::runtime_error("Command not executed");
        }

        if (had_label_) {
            complex_.set_label(simplex_id_, old_label_);
        } else {
            complex_.remove_label(simplex_id_);
        }

        executed_ = false;
    }

    std::string description() const override {
        return "Set label on simplex " + std::to_string(simplex_id_);
    }

private:
    ComplexType& complex_;
    typename ComplexType::SimplexID simplex_id_;
    LabelType new_label_;
    LabelType old_label_;
    bool executed_;
    bool had_label_;
};

/// Command for applying a story event
template<typename ComplexType>
class ApplyEventCommand : public Command {
public:
    ApplyEventCommand(
        ComplexType& complex,
        const std::vector<typename ComplexType::SimplexID>& affected_simplices,
        std::function<void(const std::vector<typename ComplexType::SimplexID>&)> on_apply)
        : complex_(complex)
        , affected_simplices_(affected_simplices)
        , on_apply_(std::move(on_apply))
        , executed_(false) {

        // Store old labels for undo
        for (auto sid : affected_simplices) {
            auto label_opt = complex_.get_label(sid);
            if (label_opt) {
                old_labels_[sid] = *label_opt;
            }
        }
    }

    void execute() override {
        if (executed_) {
            throw std::runtime_error("Command already executed");
        }

        if (on_apply_) {
            on_apply_(affected_simplices_);
        }

        executed_ = true;
    }

    void undo() override {
        if (!executed_) {
            throw std::runtime_error("Command not executed");
        }

        // Restore old labels
        for (const auto& [sid, label] : old_labels_) {
            complex_.set_label(sid, label);
        }

        executed_ = false;
    }

    std::string description() const override {
        return "Apply event to " + std::to_string(affected_simplices_.size()) + " simplices";
    }

private:
    ComplexType& complex_;
    std::vector<typename ComplexType::SimplexID> affected_simplices_;
    std::function<void(const std::vector<typename ComplexType::SimplexID>&)> on_apply_;
    std::unordered_map<typename ComplexType::SimplexID,
                      typename ComplexType::LabelType> old_labels_;
    bool executed_;
};

/// Manages command history with undo/redo support
class CommandHistory {
public:
    CommandHistory() = default;

    /// Execute a command and add to history
    void execute(std::unique_ptr<Command> command) {
        command->execute();

        // Remove any commands after current position (for redo)
        if (current_index_ < history_.size()) {
            history_.resize(current_index_);
        }

        history_.push_back(std::move(command));
        current_index_++;

        // Enforce max size
        if (max_size_ > 0 && history_.size() > max_size_) {
            history_.erase(history_.begin());
            current_index_--;
        }
    }

    /// Undo the last command
    /// @throws std::runtime_error if no commands to undo
    void undo() {
        if (current_index_ == 0) {
            throw std::runtime_error("No commands to undo");
        }

        current_index_--;
        history_[current_index_]->undo();
    }

    /// Redo the last undone command
    /// @throws std::runtime_error if no commands to redo
    void redo() {
        if (current_index_ >= history_.size()) {
            throw std::runtime_error("No commands to redo");
        }

        history_[current_index_]->execute();
        current_index_++;
    }

    /// Check if undo is available
    bool can_undo() const {
        return current_index_ > 0;
    }

    /// Check if redo is available
    bool can_redo() const {
        return current_index_ < history_.size();
    }

    /// Clear command history
    void clear() {
        history_.clear();
        current_index_ = 0;
    }

    /// Get maximum history size
    size_t max_size() const {
        return max_size_;
    }

    /// Set maximum history size (0 = unlimited)
    void set_max_size(size_t size) {
        max_size_ = size;

        // Trim if needed
        if (max_size_ > 0 && history_.size() > max_size_) {
            size_t to_remove = history_.size() - max_size_;
            history_.erase(history_.begin(), history_.begin() + to_remove);
            current_index_ -= to_remove;
        }
    }

    /// Get current history size
    size_t size() const {
        return history_.size();
    }

    /// Get description of command at index
    std::string get_command_description(size_t index) const {
        if (index >= history_.size()) {
            throw std::out_of_range("Command index out of range");
        }
        return history_[index]->description();
    }

private:
    std::vector<std::unique_ptr<Command>> history_;
    size_t current_index_ = 0;
    size_t max_size_ = 1000;
};

} // namespace cebu

#endif // CEBU_COMMAND_HISTORY_H
