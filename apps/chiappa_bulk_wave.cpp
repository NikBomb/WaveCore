#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "wavecore/archetypes/ElementArchetype.hpp"
#include "wavecore/archetypes/GaussPointArchetype.hpp"
#include "wavecore/archetypes/MaterialArchetype.hpp"
#include "wavecore/archetypes/NodeArchetype.hpp"
#include "wavecore/benchmarks/ChiappaBulkWave.hpp"
#include "wavecore/elements/Quad4.hpp"
#include "wavecore/materials/LinearElasticPlaneStrain.hpp"
#include "wavecore/relations/ElementRelations.hpp"
#include "wavecore/systems/ExplicitDynamicsSystem.hpp"

int main(int argc, char** argv) {
    using Element = wavecore::Quad4;
    using Material = wavecore::LinearElasticPlaneStrain;
    const std::size_t subdivisions = argc > 2 ? std::stoul(argv[2]) : 160;
    if (subdivisions == 0 || subdivisions % 4 != 0) {
        std::cerr << "Mesh subdivisions must be positive and divisible by four\n";
        return 1;
    }
    constexpr double length = 1.0;
    constexpr double dt = 1.0e-8;
    constexpr double final_time = 470.0e-6;
    const std::filesystem::path output = argc > 1 ? argv[1] : "chiappa_snapshot.csv";
    const double spacing = length / static_cast<double>(subdivisions);
    const wavecore::ChiappaBulkWave reference;
    if (!output.parent_path().empty())
        std::filesystem::create_directories(output.parent_path());
    std::ofstream history(output.string() + ".history.csv");
    if (!history) return 1;
    history << "time,numerical_ux,numerical_uy,analytical_ux,analytical_uy\n";
    history << std::setprecision(17);
    std::ofstream initial(output.string() + ".initial.csv");
    if (!initial) return 1;
    initial << "x,y,vx,vy\n" << std::setprecision(17);
    std::size_t excited_nodes = 0;

    wavecore::NodeArchetype<2> nodes;
    for (std::size_t j = 0; j <= subdivisions; ++j)
        for (std::size_t i = 0; i <= subdivisions; ++i) {
            const std::array<double, 2> coordinate{
                static_cast<double>(i) * spacing,
                static_cast<double>(j) * spacing};
            static_cast<void>(nodes.add_node(wavecore::Node2D{coordinate}));
        }
    const auto initial_conditions = [&](std::span<wavecore::Node2D> initial_nodes) {
        for (auto& node : initial_nodes) {
            const auto& coordinate = node.coordinates();
            // Direct nodal IC. Patch-edge nodes receive the full prescribed
            // velocity. No smoothing, projection, or Fourier reconstruction.
            const bool in_patch = coordinate[0] >= 0.45 && coordinate[0] <= 0.55 &&
                                  coordinate[1] >= 0.45 && coordinate[1] <= 0.55;
            node.velocity() = {in_patch ? 1.0 : 0.0, 0.0};
            excited_nodes += in_patch ? 1 : 0;
            initial << coordinate[0] << ',' << coordinate[1] << ','
                    << node.velocity()[0] << ",0\n";
        }
    initial.close();
    std::cout << subdivisions << " x " << subdivisions << ": " << excited_nodes
              << " nodes initialized with vx=1 m/s; all other velocities zero\n" << std::flush;
    };

    wavecore::ElementArchetype<Element> elements;
    wavecore::MaterialArchetype<Material> materials;
    wavecore::GaussPointArchetype<Element, Material> points;
    wavecore::ElementNodeRelation<Element> element_nodes;
    wavecore::ElementGaussPointRelation<Element> element_points;
    wavecore::GaussPointMaterialRelation point_materials;
    const auto material = materials.add_material(Material{209.0e9, 1.0 / 3.0, 7800.0});

    for (std::size_t j = 0; j < subdivisions; ++j)
        for (std::size_t i = 0; i < subdivisions; ++i) {
            static_cast<void>(elements.add_element(wavecore::PlaneElementProperties{1.0}));
            const std::size_t n0 = j * (subdivisions + 1) + i;
            const std::size_t n1 = n0 + 1;
            const std::size_t n3 = (j + 1) * (subdivisions + 1) + i;
            const std::size_t n2 = n3 + 1;
            static_cast<void>(element_nodes.add({n0, n1, n2, n3}));
            const auto point = points.add_point(materials.material(material));
            static_cast<void>(element_points.add({point}));
            static_cast<void>(point_materials.add(material));
        }

    const auto constraints = wavecore::chiappa_bulk_constraints(nodes.values());
    const auto write_snapshot = [&](const std::filesystem::path& path, double time) {
    std::ofstream file(path);
    if (!file) throw std::runtime_error("Cannot write snapshot");
    file << "x,y,numerical_ux,numerical_uy,analytical_ux,analytical_uy,time\n";
    double max_error = 0.0;
    double error_squared = 0.0;
    double analytical_squared = 0.0;
    double analytical_max = 0.0;
    for (const auto& node : nodes.values()) {
        const auto& coordinate = node.coordinates();
        const auto expected = reference.displacement(
            coordinate[0], coordinate[1], time);
        const double error_x = node.displacement()[0] - expected(0);
        const double error_y = node.displacement()[1] - expected(1);
        const double error = std::hypot(error_x, error_y);
        const double expected_magnitude = std::hypot(expected(0), expected(1));
        max_error = std::max(max_error, error);
        error_squared += error * error;
        analytical_squared += expected_magnitude * expected_magnitude;
        analytical_max = std::max(analytical_max, expected_magnitude);
        file << std::setprecision(17) << coordinate[0] << ',' << coordinate[1] << ','
             << node.displacement()[0] << ',' << node.displacement()[1] << ','
             << expected(0) << ',' << expected(1) << ',' << time << '\n';
    }
    const double rms_error = std::sqrt(error_squared / static_cast<double>(nodes.size()));
    const double relative_l2 = std::sqrt(error_squared / analytical_squared);
    std::cout << "Chiappa bulk snapshot at t=" << time << " s\n"
              << "max vector error: " << max_error << " m\n"
              << "RMS vector error: " << rms_error << " m\n"
              << "relative L2 error: " << relative_l2 << "\n"
              << "analytical max displacement: " << analytical_max << " m\n" << std::flush;
    };

    const auto observe = [&](wavecore::DynamicsOutputEvent event,
                             std::span<const wavecore::Node2D> observed_nodes) {
        const double time = event.time;
        if (event.history || event.final) {
            const auto expected = reference.displacement(.25, .25, time);
            const auto& observed = observed_nodes[
                (subdivisions / 4) * (subdivisions + 1) + subdivisions / 4];
            history << time << ',' << observed.displacement()[0] << ','
                    << observed.displacement()[1] << ',' << expected(0) << ','
                    << expected(1) << '\n';
        }
        if (event.snapshot)
            write_snapshot(output.string() + ".71us.csv", time);
        if (event.final) write_snapshot(output, time);
    };
    wavecore::ExplicitDynamicsSystem::run(
        wavecore::ExplicitDynamicsQuery<Element, Material>{
            elements, nodes.values(), element_nodes, element_points,
            materials, point_materials, points},
        constraints, initial_conditions, final_time, observe,
        wavecore::ExplicitDynamicsOptions{dt, 0.9},
        wavecore::DynamicsOutputSchedule{1.0e-7, {71.0e-6}});
}
