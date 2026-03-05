#include "cebu/command_history.h"
#include "cebu/simplicial_complex_labeled.h"
#include "cebu/absurdity.h"
#include <iostream>
#include <cassert>

using namespace cebu;

void test_add_simplex_command() {
    std::cout << "Testing AddSimplexCommand..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    std::vector<VertexID> vertices = {v0, v1, v2};

    AddSimplexCommand<decltype(complex)> cmd(complex, vertices);

    // Execute
    cmd.execute();
    assert(complex.simplex_count() == 4); // 3 vertices + 1 triangle
    assert(cmd.simplex_id() != 0);

    // Undo
    cmd.undo();
    assert(complex.simplex_count() == 3); // Only vertices

    std::cout << "  PASSED" << std::endl;
}

void test_remove_simplex_command() {
    std::cout << "Testing RemoveSimplexCommand..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, v1, v2);

    RemoveSimplexCommand<decltype(complex)> cmd(complex, tri, false);

    // Execute
    cmd.execute();
    assert(complex.simplex_count() == 3); // Only vertices
    assert(!complex.has_simplex(tri));

    // Undo
    cmd.undo();
    assert(complex.simplex_count() == 4); // 3 vertices + 1 triangle
    assert(complex.has_simplex(tri));

    std::cout << "  PASSED" << std::endl;
}

void test_set_label_command() {
    std::cout << "Testing SetLabelCommand..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    VertexID v0 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, complex.add_vertex(), complex.add_vertex());

    SetLabelCommand<decltype(complex), double> cmd1(complex, tri, 0.5);

    // Execute (set new label)
    cmd1.execute();
    auto label = complex.get_label(tri);
    assert(label);
    assert(*label == 0.5);

    // Undo (remove label)
    cmd1.undo();
    label = complex.get_label(tri);
    assert(!label);

    // Set label again
    SetLabelCommand<decltype(complex), double> cmd2(complex, tri, 0.7);
    cmd2.execute();

    // Update label
    SetLabelCommand<decltype(complex), double> cmd3(complex, tri, 0.9);
    cmd3.execute();
    label = complex.get_label(tri);
    assert(label);
    assert(*label == 0.9);

    // Undo (restore old label)
    cmd3.undo();
    label = complex.get_label(tri);
    assert(label);
    assert(*label == 0.7);

    std::cout << "  PASSED" << std::endl;
}

void test_command_history_basic() {
    std::cout << "Testing CommandHistory (basic)..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();

    // Execute commands
    auto cmd1 = std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0});
    auto cmd2 = std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v1});

    history.execute(std::move(cmd1));
    assert(history.size() == 1);
    assert(history.can_undo());
    assert(!history.can_redo());

    history.execute(std::move(cmd2));
    assert(history.size() == 2);
    assert(complex.simplex_count() == 2);

    std::cout << "  PASSED" << std::endl;
}

void test_command_history_undo_redo() {
    std::cout << "Testing CommandHistory (undo/redo)..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    VertexID v0 = complex.add_vertex();

    // Execute command
    auto cmd1 = std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0});
    SimplexID sid = v0; // Vertex ID is same as simplex ID
    history.execute(std::move(cmd1));

    assert(complex.has_simplex(sid));

    // Undo
    history.undo();
    assert(!complex.has_simplex(sid));
    assert(!history.can_undo());
    assert(history.can_redo());

    // Redo
    history.redo();
    assert(complex.has_simplex(sid));
    assert(history.can_undo());
    assert(!history.can_redo());

    std::cout << "  PASSED" << std::endl;
}

void test_command_history_multiple_undo() {
    std::cout << "Testing CommandHistory (multiple undo)..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Execute 3 commands
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v1}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v2}));

    assert(complex.simplex_count() == 3);

    // Undo all
    history.undo();
    history.undo();
    history.undo();

    assert(complex.simplex_count() == 0);
    assert(!history.can_undo());
    assert(history.can_redo());

    // Redo all
    history.redo();
    history.redo();
    history.redo();

    assert(complex.simplex_count() == 3);

    std::cout << "  PASSED" << std::endl;
}

void test_command_history_max_size() {
    std::cout << "Testing CommandHistory (max size)..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    history.set_max_size(3);

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();
    VertexID v3 = complex.add_vertex();
    VertexID v4 = complex.add_vertex();

    // Execute 5 commands
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v1}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v2}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v3}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v4}));

    // Only last 3 should be kept
    assert(history.size() == 3);
    assert(history.max_size() == 3);

    // Can't undo beyond 3
    assert(history.can_undo());

    std::cout << "  PASSED" << std::endl;
}

void test_command_history_clear() {
    std::cout << "Testing CommandHistory (clear)..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    VertexID v0 = complex.add_vertex();

    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0}));

    assert(history.size() == 1);

    history.clear();

    assert(history.size() == 0);
    assert(!history.can_undo());
    assert(!history.can_redo());

    std::cout << "  PASSED" << std::endl;
}

void test_command_history_truncate_on_execute() {
    std::cout << "Testing CommandHistory (truncate on execute)..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    VertexID v0 = complex.add_vertex();
    VertexID v1 = complex.add_vertex();
    VertexID v2 = complex.add_vertex();

    // Execute 3 commands
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v1}));
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v2}));

    assert(history.size() == 3);

    // Undo 2
    history.undo();
    history.undo();

    assert(history.size() == 3);

    // Execute new command (should truncate redo history)
    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{complex.add_vertex()}));

    assert(history.size() == 2);
    assert(!history.can_redo());

    std::cout << "  PASSED" << std::endl;
}

void test_set_label_with_absurdity() {
    std::cout << "Testing SetLabelCommand with Absurdity..." << std::endl;

    using ComplexType = SimplicialComplexLabeled<Absurdity>;
    ComplexType complex;

    VertexID v0 = complex.add_vertex();
    SimplexID tri = complex.add_triangle(v0, complex.add_vertex(), complex.add_vertex());

    Absurdity a1(0.3, 0.5, 0.8);

    SetLabelCommand<ComplexType, Absurdity> cmd1(complex, tri, a1);

    // Execute
    cmd1.execute();
    auto label = complex.get_label(tri);
    assert(label);
    assert(label->lower() == 0.3);
    assert(label->upper() == 0.5);
    assert(label->confidence() == 0.8);

    // Undo
    cmd1.undo();
    label = complex.get_label(tri);
    assert(!label);

    std::cout << "  PASSED" << std::endl;
}

void test_error_double_execute() {
    std::cout << "Testing error on double execute..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    CommandHistory history;

    VertexID v0 = complex.add_vertex();

    auto cmd = std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0});

    cmd->execute();

    bool threw = false;
    try {
        cmd->execute();
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_error_undo_without_execute() {
    std::cout << "Testing error on undo without execute..." << std::endl;

    SimplicialComplexLabeled<double> complex;
    VertexID v0 = complex.add_vertex();

    AddSimplexCommand<decltype(complex)> cmd(complex, std::vector<VertexID>{v0});

    bool threw = false;
    try {
        cmd.undo();
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_error_undo_without_commands() {
    std::cout << "Testing error on undo without commands..." << std::endl;

    CommandHistory history;

    bool threw = false;
    try {
        history.undo();
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

void test_error_redo_without_commands() {
    std::cout << "Testing error on redo without commands..." << std::endl;

    CommandHistory history;
    SimplicialComplexLabeled<double> complex;
    VertexID v0 = complex.add_vertex();

    history.execute(std::make_unique<AddSimplexCommand<decltype(complex)>>(complex, std::vector<VertexID>{v0}));

    bool threw = false;
    try {
        history.redo();
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);

    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "=== Command History Tests ===" << std::endl;
    std::cout << std::endl;

    test_add_simplex_command();
    test_remove_simplex_command();
    test_set_label_command();
    test_command_history_basic();
    test_command_history_undo_redo();
    test_command_history_multiple_undo();
    test_command_history_max_size();
    test_command_history_clear();
    test_command_history_truncate_on_execute();
    test_set_label_with_absurdity();
    test_error_double_execute();
    test_error_undo_without_execute();
    test_error_undo_without_commands();
    test_error_redo_without_commands();

    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;

    return 0;
}
