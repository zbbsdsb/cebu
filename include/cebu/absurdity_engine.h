// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT
#pragma once

#include "cebu/absurdity.h"
#include "cebu/topology_morph.h"
#include "cebu/narrative_context.h"
#include "cebu/simplicial_complex.h"
#include <vector>
#include <functional>

namespace cebu {

/**
 * @brief High-level orchestrator that links narrative context, absurdity field
 * evolution, and topology morphing into a single tick() entry point.
 */
class AbsurdityEngine {
public:
    explicit AbsurdityEngine(SimplicialComplex& complex);

    /**
     * @brief Advance the simulation by one step.
     * @param ctx Narrative context driving absurdity.
     * @param dt  Time step.
     */
    void tick(const NarrativeContext& ctx, double dt);

    // Accessors
    AbsurdityField& field() { return field_; }
    const AbsurdityField& field() const { return field_; }
    TopologyMorph& morph() { return morph_; }
    const TopologyMorph& morph() const { return morph_; }
    double time() const { return time_; }

    // Register a callback for morph events.
    void on_morph(MorphEventCallback cb);

private:
    SimplicialComplex& complex_;
    AbsurdityField field_;
    TopologyMorph morph_;
    StochasticEvolution evolution_;
    double time_ = 0.0;
    std::vector<MorphEventCallback> callbacks_;

    void sync_field_with_complex(const FuzzyInterval& initial_value);
    void evolve_field_with_adjacency(double dt);
};

} // namespace cebu
