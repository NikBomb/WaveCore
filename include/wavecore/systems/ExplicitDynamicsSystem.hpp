#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

#include "wavecore/systems/FiniteElementSystems.hpp"

namespace wavecore {

// Optional numerical policy, independent of the physical problem definition.
struct ExplicitDynamicsOptions {
    double maximum_timestep = std::numeric_limits<double>::infinity();
    double safety_factor = 0.9;
};

struct DynamicsOutputSchedule {
    double interval = std::numeric_limits<double>::infinity();
    std::vector<double> snapshot_times;
};

struct DynamicsOutputEvent {
    double time;
    bool initial;
    bool final;
    bool history;
    bool snapshot;
};

// Non-owning query: component ownership stays in the separate archetypes;
// all joins stay in the external relations. A run starts at time zero.
template <IElementConcept Element, IMaterialConcept Material, class Nodes = NodeView<Element::dimension>>
struct ExplicitDynamicsQuery {
    ElementArchetype<Element>& elements;
    Nodes nodes;
    const ElementNodeRelation<Element>& element_nodes;
    const ElementGaussPointRelation<Element>& element_points;
    const MaterialArchetype<Material>& materials;
    const GaussPointMaterialRelation& point_materials;
    GaussPointArchetype<Element, Material>& points;
};

// Orchestrates Belytschko, Box 6.1 (explicit time integration). Step numbers
// below refer to that flowchart; kernels operate on externally owned ECS data.
// Current scope: zero damping and stationary homogeneous BCs. General external
// loads, time-dependent prescribed motion, and the step-11 energy check are not
// yet wired into this driver. Output scheduling is a driver extension; writers
// remain application-owned.
struct ExplicitDynamicsSystem {
    // IC initializes nodal data directly. GP initial state is supplied by the
    // problem. BC currently supports stationary, zero-displacement constraints.
    // Output receives only scheduled completed states, including t=0 and tf.
    template <IElementConcept Element, IMaterialConcept Material, class Nodes,
              class InitialConditions, class Observer, class Constraints>
    static void run(
        ExplicitDynamicsQuery<Element, Material, Nodes> query,
        const Constraints& constraints,
        InitialConditions initial_conditions, double final_time,
        Observer observe, ExplicitDynamicsOptions options = {},
        DynamicsOutputSchedule output = {}) {
        if (!std::isfinite(final_time) || final_time < 0.0 ||
            std::isnan(options.maximum_timestep) || options.maximum_timestep <= 0.0 ||
            !std::isfinite(options.safety_factor) || options.safety_factor <= 0.0 ||
            options.safety_factor > 1.0 || constraints.size() != query.nodes.size())
            throw std::invalid_argument("Invalid explicit dynamics configuration");
        if (std::isnan(output.interval) || output.interval <= 0.0)
            throw std::invalid_argument("Output interval must be positive");
        for (double time : output.snapshot_times)
            if (!std::isfinite(time) || time < 0.0 || time > final_time)
                throw std::invalid_argument("Snapshot time outside simulation interval");
        std::sort(output.snapshot_times.begin(), output.snapshot_times.end());
        output.snapshot_times.erase(std::unique(output.snapshot_times.begin(),
            output.snapshot_times.end()), output.snapshot_times.end());

        auto& [elements, nodes, element_nodes, element_points,
               materials, point_materials, points] = query;
        // Box 6.1, step 1: nodal IC, admissible BCs, geometry and lumped M.
        // Initial GP stress/history is already supplied by the problem.
        // Legacy span callbacks get a temporary value snapshot allocated once
        // for the run. The canonical simulation storage remains component SoA.
        ScalarStorage<typename Element::node_type> callback_nodes;
        using ReadOnlyNodes = decltype(read_only_nodes(nodes));
        if constexpr (!std::invocable<InitialConditions,Nodes> ||
                      !std::invocable<Observer,DynamicsOutputEvent,ReadOnlyNodes>)
            callback_nodes.resize(nodes.size());
        if constexpr (std::invocable<InitialConditions,Nodes>) {
            std::invoke(initial_conditions, nodes);
        } else {
            for (std::size_t i=0;i<nodes.size();++i) callback_nodes[i]=nodes[i];
            std::invoke(initial_conditions,
                std::span<typename Element::node_type>{callback_nodes.view().data(),nodes.size()});
            for (std::size_t i=0;i<nodes.size();++i) {
                if constexpr (requires { nodes[i].assign(callback_nodes[i]); })
                    nodes[i].assign(callback_nodes[i]);
                else nodes[i]=callback_nodes[i];
            }
        }
        apply_velocity_constraints(nodes, constraints);
        refresh_element_geometry(elements, nodes, element_nodes);
        assemble_lumped_mass(elements, nodes, element_nodes, element_points,
                             materials, point_materials);
        // Steps 2-3: initial getforce and a^0 = M^-1 (f_ext - f_int).
        // dt=0 leaves the current rate-based material history unadvanced.
        refresh_material_force_state(elements, nodes, element_nodes, element_points,
                                     materials, point_materials, points, 0.0);
        compute_accelerations(nodes);
        const auto const_nodes = read_only_nodes(nodes);
        double time = 0.0;
        long double elapsed = 0.0L;
        std::size_t history_index = 1;
        std::size_t snapshot_index = 0;
        const auto emit = [&](bool history) {
            const bool snapshot = snapshot_index < output.snapshot_times.size() &&
                                  output.snapshot_times[snapshot_index] == time;
            if (snapshot) ++snapshot_index;
            if (history || snapshot || time == 0.0 || time == final_time)
            {
                const DynamicsOutputEvent event{time,time == 0.0,time == final_time,history,snapshot};
                if constexpr (std::invocable<Observer,DynamicsOutputEvent,ReadOnlyNodes>) {
                    std::invoke(observe,event,const_nodes);
                } else {
                    for (std::size_t i=0;i<nodes.size();++i) callback_nodes[i]=nodes[i];
                    std::invoke(observe,event,std::span<const typename Element::node_type>{
                        callback_nodes.view().data(),nodes.size()});
                }
            }
        };
        emit(true);
        while (time < final_time) {
            // getforce's critical-step reduction/safety factor is evaluated here
            // from the latest geometry. Cap dt at the next output/final time.
            const double next_history = static_cast<double>(history_index) * output.interval;
            const double next_snapshot = snapshot_index < output.snapshot_times.size()
                ? output.snapshot_times[snapshot_index] : final_time;
            const double target = std::min({final_time, next_history, next_snapshot});
            const double limit = std::min(options.maximum_timestep,
                critical_timestep(elements, element_points, point_materials,
                                  materials, options.safety_factor));
            if (!std::isfinite(limit) || limit <= 0.0)
                throw std::runtime_error("Invalid stable timestep");
            const double remaining = static_cast<double>(
                static_cast<long double>(target) - elapsed);
            // Avoid a spurious near-zero final step from accumulated roundoff.
            const bool last = remaining <= limit ||
                remaining - limit <= 32.0 * std::numeric_limits<double>::epsilon() *
                                     std::max(final_time, limit);
            const double dt = last ? remaining : limit;
            // Step 4: choose t^(n+1); the kernel uses dt/2 for both half steps.
            elapsed = last ? static_cast<long double>(target) : elapsed + dt;
            const double next = static_cast<double>(elapsed);
            if (next <= time)
                throw std::runtime_error("Timestep cannot advance simulation time");
            // Steps 5-10: half kick, BC, drift, getforce, acceleration, half kick.
            explicit_leapfrog_step(elements, nodes, element_nodes, element_points,
                                   materials, point_materials, points, dt, constraints);
            // Step 11 TODO: work-consistent energy-balance check. The standalone
            // energy diagnostics are not a balance check in this integration loop.
            // Steps 12-13: accept the new time and dispatch scheduled output;
            // continue until final_time. No application-owned stepping loop.
            time = next;
            const bool history = time == next_history;
            if (history) ++history_index;
            emit(history);
        }
    }
};

} // namespace wavecore
