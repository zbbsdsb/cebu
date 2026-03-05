#include "cebu/equivalence_classes.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace cebu;

void test_equivalence_class_basic() {
    std::cout << "Testing EquivalenceClass basic operations..." << std::endl;

    EquivalenceClass ec(100);

    assert(ec.representative() == 100);
    assert(ec.size() == 1);
    assert(ec.contains(100));

    ec.add_member(101);
    assert(ec.size() == 2);
    assert(ec.contains(101));

    ec.add_member(102);
    assert(ec.size() == 3);
    assert(ec.contains(102));

    bool removed = ec.remove_member(101);
    assert(removed);
    assert(!ec.contains(101));
    assert(ec.size() == 2);

    removed = ec.remove_member(999);
    assert(!removed);

    std::cout << "✓ EquivalenceClass basic operations passed" << std::endl;
}

void test_equivalence_class_representative_change() {
    std::cout << "Testing EquivalenceClass representative change..." << std::endl;

    EquivalenceClass ec(100);
    ec.add_member(101);
    ec.add_member(102);

    ec.remove_member(100);
    assert(ec.representative() != 100);
    assert(ec.contains(ec.representative()));

    ec.set_representative(102);
    assert(ec.representative() == 102);

    bool caught = false;
    try {
        ec.set_representative(999);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    std::cout << "✓ EquivalenceClass representative change passed" << std::endl;
}

void test_manager_add_simplex() {
    std::cout << "Testing EquivalenceClassManager::add_simplex..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    assert(manager.class_size(100) == 1);
    assert(manager.find_representative(100) == 100);
    assert(manager.class_count() == 1);

    manager.add_simplex(101);
    manager.add_simplex(102);
    assert(manager.class_count() == 3);

    std::cout << "✓ add_simplex passed" << std::endl;
}

void test_manager_find() {
    std::cout << "Testing EquivalenceClassManager::find..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    assert(manager.find_representative(100) == 100);

    // Test non-existent simplex returns itself
    assert(manager.find_representative(999) == 999);

    std::cout << "✓ find passed" << std::endl;
}

void test_manager_glue_basic() {
    std::cout << "Testing EquivalenceClassManager::glue (basic)..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    manager.add_simplex(101);

    SimplexID rep = manager.glue(100, 101);
    assert(rep == 100); // 100 should be the representative (first one)

    assert(manager.are_glued(100, 101));
    assert(manager.are_glued(101, 100)); // Symmetric
    assert(manager.is_glued(100));
    assert(manager.is_glued(101));
    assert(manager.class_size(100) == 2);
    assert(manager.class_size(101) == 2);

    // Both should have same representative
    assert(manager.find_representative(100) == manager.find_representative(101));

    std::cout << "✓ glue (basic) passed" << std::endl;
}

void test_manager_glue_three_way() {
    std::cout << "Testing EquivalenceClassManager::glue (three-way)..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    manager.add_simplex(101);
    manager.add_simplex(102);

    manager.glue(100, 101);
    manager.glue(101, 102);

    // All three should be in same class
    assert(manager.are_glued(100, 102));
    assert(manager.class_size(100) == 3);

    auto cls = manager.get_equivalence_class(100);
    assert(cls.size() == 3);
    assert(cls[0] == 100 || cls[1] == 100 || cls[2] == 100);
    assert(cls[0] == 101 || cls[1] == 101 || cls[2] == 101);
    assert(cls[0] == 102 || cls[1] == 102 || cls[2] == 102);

    std::cout << "✓ glue (three-way) passed" << std::endl;
}

void test_manager_separate() {
    std::cout << "Testing EquivalenceClassManager::separate..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    manager.add_simplex(101);
    manager.add_simplex(102);

    manager.glue(100, 101);
    manager.glue(101, 102);

    // Separate 101
    bool separated = manager.separate(101);
    assert(separated);

    // 101 should be alone now
    assert(!manager.are_glued(100, 101));
    assert(!manager.are_glued(101, 102));
    assert(manager.class_size(101) == 1);

    // 100 and 102 should still be glued
    assert(manager.are_glued(100, 102));
    assert(manager.class_size(100) == 2);

    // Try separating a simplex that's already alone
    separated = manager.separate(101);
    assert(!separated);

    std::cout << "✓ separate passed" << std::endl;
}

void test_manager_remove_simplex() {
    std::cout << "Testing EquivalenceClassManager::remove_simplex..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    manager.add_simplex(101);
    manager.add_simplex(102);

    manager.glue(100, 101);
    manager.glue(101, 102);

    // Remove middle simplex
    manager.remove_simplex(101);

    // 101 should be gone
    assert(manager.class_size(101) == 1);

    // 100 and 102 should still be glued
    assert(manager.are_glued(100, 102));
    assert(manager.class_size(100) == 2);

    std::cout << "✓ remove_simplex passed" << std::endl;
}

void test_manager_get_all_classes() {
    std::cout << "Testing EquivalenceClassManager::get_all_classes..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    manager.add_simplex(101);
    manager.add_simplex(102);
    manager.add_simplex(200);
    manager.add_simplex(201);

    manager.glue(100, 101);
    manager.glue(101, 102);
    manager.glue(200, 201);

    auto classes = manager.get_all_classes();
    assert(classes.size() == 2);

    // Check first class
    bool found_first = false;
    bool found_second = false;
    for (const auto& [rep, members] : classes) {
        if (members.size() == 3) {
            found_first = true;
        } else if (members.size() == 2) {
            found_second = true;
        }
    }
    assert(found_first && found_second);

    std::cout << "✓ get_all_classes passed" << std::endl;
}

void test_manager_path_compression() {
    std::cout << "Testing path compression..." << std::endl;

    EquivalenceClassManager manager;

    // Create a chain: 0-1-2-3-4-5-6-7-8-9
    for (int i = 0; i < 10; i++) {
        manager.add_simplex(i);
    }

    for (int i = 0; i < 9; i++) {
        manager.glue(i, i + 1);
    }

    // All should have same representative
    SimplexID rep = manager.find_representative(0);
    for (int i = 0; i < 10; i++) {
        assert(manager.find_representative(i) == rep);
    }

    // Path compression should make find operations efficient
    // (multiple calls should be fast due to path compression)
    for (int i = 0; i < 100; i++) {
        assert(manager.find_representative(i % 10) == rep);
    }

    std::cout << "✓ path compression passed" << std::endl;
}

void test_manager_clear() {
    std::cout << "Testing EquivalenceClassManager::clear..." << std::endl;

    EquivalenceClassManager manager;

    manager.add_simplex(100);
    manager.add_simplex(101);
    manager.glue(100, 101);

    assert(manager.class_count() == 1);

    manager.clear();

    assert(manager.class_count() == 0);
    assert(manager.class_size(100) == 1); // Non-existent simplex returns 1

    std::cout << "✓ clear passed" << std::endl;
}

void test_edge_cases() {
    std::cout << "Testing edge cases..." << std::endl;

    EquivalenceClassManager manager;

    // Glue non-existent simplices (should add them)
    manager.glue(100, 101);
    assert(manager.are_glued(100, 101));

    // Glue already glued simplices (should work)
    manager.glue(100, 101);
    assert(manager.are_glued(100, 101));

    // Separate non-existent simplex
    bool separated = manager.separate(999);
    assert(!separated);

    // Remove non-existent simplex (should not crash)
    manager.remove_simplex(999);

    std::cout << "✓ edge cases passed" << std::endl;
}

int main() {
    std::cout << "=== Equivalence Classes Tests ===" << std::endl << std::endl;

    test_equivalence_class_basic();
    test_equivalence_class_representative_change();
    test_manager_add_simplex();
    test_manager_find();
    test_manager_glue_basic();
    test_manager_glue_three_way();
    test_manager_separate();
    test_manager_remove_simplex();
    test_manager_get_all_classes();
    test_manager_path_compression();
    test_manager_clear();
    test_edge_cases();

    std::cout << std::endl;
    std::cout << "=== All Equivalence Classes Tests Passed! ===" << std::endl;
    return 0;
}
