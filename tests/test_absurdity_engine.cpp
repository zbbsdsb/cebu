// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#include "cebu/absurdity_engine.h"
#include <cassert>
#include <iostream>

using namespace cebu;

int main() {
    std::cout << "Test: AbsurdityEngine integration... ";

    SimplicialComplex complex;
    auto v0 = complex.add_vertex();
    auto v1 = complex.add_vertex();
    auto v2 = complex.add_vertex();
    complex.add_triangle(v0, v1, v2);

    AbsurdityEngine engine(complex);

    NarrativeContext ctx(0.9, 0.8, 0.6, 0.7, 0.4);
    engine.tick(ctx, 0.1);

    assert(engine.time() > 0.0);
    assert(engine.field().size() == complex.simplex_count());

    auto mean = engine.field().mean();
    assert(mean.lower >= 0.0 && mean.upper <= 1.5); // sanity range

    std::cout << "PASSED\n";
    return 0;
}
