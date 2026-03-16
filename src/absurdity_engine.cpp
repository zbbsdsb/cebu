#include "cebu/absurdity_engine.h"
#include <unordered_map>
#include <unordered_set>

namespace cebu {

AbsurdityEngine::AbsurdityEngine(SimplicialComplex& complex)
    : complex_(complex)
    , field_()
    , morph_()
    , evolution_()
    , time_(0.0) {
    // Initialize field with zeros for existing simplices
    sync_field_with_complex(absurdity_utils::crisp(0.0));
}

void AbsurdityEngine::sync_field_with_complex(const FuzzyInterval& initial_value) {
    const auto& simplices = complex_.get_simplices();
    for (const auto& pair : simplices) {
        SimplexID sid = pair.first;
        if (!field_.has(sid)) {
            field_.set(sid, initial_value);
        }
    }
}

void AbsurdityEngine::evolve_field_with_adjacency(double dt) {
    // Build contiguous indexing for existing simplices
    const auto& simplices = complex_.get_simplices();
    std::vector<SimplexID> ids;
    ids.reserve(simplices.size());
    std::unordered_map<SimplexID, size_t> index_map;

    size_t idx = 0;
    for (const auto& pair : simplices) {
        ids.push_back(pair.first);
        index_map[pair.first] = idx++;
    }

    std::vector<std::vector<size_t>> adjacency(ids.size());
    adjacency.reserve(ids.size());

    for (size_t i = 0; i < ids.size(); ++i) {
        auto neighbors = complex_.get_adjacent_simplices(ids[i]);
        for (SimplexID n : neighbors) {
            auto it = index_map.find(n);
            if (it != index_map.end()) {
                adjacency[i].push_back(it->second);
            }
        }
    }

    // Current values in contiguous order
    std::vector<FuzzyInterval> current(ids.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        current[i] = field_.get(ids[i]);
    }

    // Evolve with coupling
    auto next = evolution_.evolve_neighborhood(current, adjacency, dt);

    // Write back to field keyed by simplex IDs
    for (size_t i = 0; i < ids.size(); ++i) {
        field_.set(ids[i], next[i]);
    }
}

void AbsurdityEngine::tick(const NarrativeContext& ctx, double dt) {
    time_ += dt;

    // Ensure all simplices have an absurdity value; seed with driving force
    sync_field_with_complex(absurdity_utils::crisp(ctx.get_driving_force()));

    // Evolve absurdity field with coupling using current topology
    evolve_field_with_adjacency(dt);

    // Build morph context
    MorphContext mctx(complex_, field_, time_, dt);
    mctx.global_absurdity = field_.mean();

    auto results = morph_.apply_rules(mctx);

    // Emit callbacks
    if (!callbacks_.empty()) {
        for (const auto& res : results) {
            SimplexID sid = 0;
            if (!res.modified.empty()) {
                sid = res.modified.front();
            } else if (!res.created.empty()) {
                sid = res.created.front();
            } else if (!res.removed.empty()) {
                sid = res.removed.front();
            }

            MorphEvent evt(MorphEventType::MORPH_APPLIED,
                          "", // rule name not tracked here
                          sid,
                          time_);
            evt.result = res;
            for (const auto& cb : callbacks_) {
                cb(evt);
            }
        }
    }
}

void AbsurdityEngine::on_morph(MorphEventCallback cb) {
    callbacks_.push_back(std::move(cb));
}

} // namespace cebu
