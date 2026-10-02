#include "wavecore/fields/FieldStorage.hpp"
#include "wavecore/fields/FieldStorage.hpp"
#include "wavecore/fields/Entries.hpp"
#include "wavecore/fields/FieldRow.hpp"
#include "wavecore/archetypes/NodeArchetype.hpp"
#include "wavecore/systems/FiniteElementSystems.hpp"

#include <doctest/doctest.h>
#include <type_traits>

template <class Storage, class Entry>
concept IndexedBy = requires(Storage& storage, Entry entry) { storage[entry]; };

TEST_CASE("Field storage enforces names and constness and aliases views") {
    using Storage = wavecore::FieldStorage<wavecore::StressEntry2D>;
    using Entry = wavecore::StressEntry2D;
    static_assert(IndexedBy<Storage,Entry>);
    static_assert(!IndexedBy<Storage,wavecore::VelocityEntry2D>);
    static_assert(!IndexedBy<Storage,std::size_t>);
    static_assert(!IndexedBy<wavecore::FieldView<Entry,double>,wavecore::StressEntry3D>);
    static_assert(std::is_trivially_copyable_v<wavecore::FieldView<Entry,double>>);
    static_assert(!std::is_constructible_v<wavecore::FieldView<Entry,double>,wavecore::FieldView<Entry,const double>>);
    Storage stress(3);
    stress[Entry::sxx][1] = 4.0;
    auto view = stress.view();
    CHECK(view.size() == 3);
    CHECK(view[Entry::sxx].data() == stress[Entry::sxx].data());
    view[Entry::sxy][1] = 7.0;
    CHECK(stress[Entry::syx][1] == 7.0);
    auto tail = view.subview(1,2);
    tail[Entry::syy][0] = 9.0;
    CHECK(stress[Entry::syy][1] == 9.0);
    const auto& owner = stress;
    auto read = owner.view();
    static_assert(std::same_as<decltype(read[Entry::sxx][0]),const double&>);
    CHECK(read[Entry::sxy][1] == 7.0);
    CHECK_THROWS_AS(static_cast<void>(view.subview(2,2)),std::out_of_range);
    stress.reserve(20);
    stress.resize(5);
    CHECK(stress.size() == 5);
    CHECK(stress[Entry::sxy][1] == 7.0);
    CHECK(stress[Entry::sxy][4] == 0.0);
    stress.resize(1);
    stress.push_back({2.0,3.0,4.0});
    CHECK(stress[Entry::sxx][1] == 2.0);
    stress.clear();
    CHECK(stress.empty());
}

TEST_CASE("Scalar storage uses direct integer indexing and read-only views") {
    wavecore::ScalarStorage<std::size_t> rows;
    rows.push_back(17); rows.push_back(2);
    auto view = rows.view();
    CHECK(view.data() == &rows[0]);
    view[1] = 23;
    CHECK(rows[1] == 23);
    const auto& owner = rows;
    static_assert(std::same_as<decltype(owner.view()[0]),const std::size_t&>);
    static_assert(!std::is_constructible_v<wavecore::ScalarView<std::size_t>,wavecore::ScalarView<const std::size_t>>);
    CHECK(owner.view().subview(1,1)[0] == 23);
    CHECK_THROWS_AS(static_cast<void>(rows.at(2)),std::out_of_range);
    rows.resize(4);
    CHECK(rows[3] == 0);
    wavecore::ScalarStorage<double> density(2);
    density[0] = 7800.0;
    CHECK(density.view()[0] == 7800.0);
}

TEST_CASE("Symmetric tensors alias reversed shear in 2D and 3D") {
    wavecore::FieldStorage<wavecore::StressEntry3D> stress(2);
    wavecore::SymmetricMatrixRow<wavecore::StressEntry3D,double,3> row{stress.view(),1};
    row(2,0) = 3.0; row(2,1) = 5.0;
    CHECK(row(0,2) == 3.0);
    CHECK(row(1,2) == 5.0);
    CHECK(stress[wavecore::StressEntry3D::sxz][1] == 3.0);
    CHECK(stress[wavecore::StressEntry3D::szy][1] == 5.0);
    const wavecore::Matrix<double,3,3> matrix = row;
    CHECK(matrix(2,0) == matrix(0,2));
    wavecore::FieldStorage<wavecore::StressEntryPlaneStrain> plane(1);
    plane[wavecore::StressEntryPlaneStrain::szz][0] = 8.0;
    plane[wavecore::StressEntryPlaneStrain::sxx][0] = 2.0;
    CHECK(plane[wavecore::StressEntryPlaneStrain::szz][0] == 8.0);
}

TEST_CASE("Node views alias 3D vector and scalar fields") {
    wavecore::NodeArchetype<3> nodes;
    static_cast<void>(nodes.add_node(wavecore::Node3D{{1.0,2.0,3.0}}));
    nodes.node(0).velocity() = {4.0,5.0,6.0};
    auto view = nodes.values();
    view.velocity[wavecore::VelocityEntry3D::vz][0] = 7.0;
    view.mass[0] = 9.0;
    CHECK(nodes.node(0).velocity()[2] == 7.0);
    CHECK(nodes.node(0).mass() == 9.0);
    static_cast<void>(nodes.add_node(wavecore::Node3D{{2.0,3.0,4.0}}));
    nodes.node(1).velocity() = nodes.node(0).velocity();
    nodes.node(0).velocity()[2] = 10.0;
    CHECK(nodes.node(1).velocity()[2] == 7.0);
    nodes.node(1) = nodes.node(0);
    CHECK(nodes.node(1).coordinates()[0] == 1.0);
    CHECK(nodes.node(1).velocity()[2] == 10.0);
    nodes.node(0).displacement()[1] = 0.5;
    CHECK(nodes.node(0).current_coordinates()[1] == 2.5);
    const auto& owner = nodes;
    static_assert(std::same_as<decltype(owner.values()[0].velocity()[0]),const double&>);
    CHECK_THROWS_AS(static_cast<void>(nodes.node(2)),std::out_of_range);
}

TEST_CASE("Shared-node joins preserve permuted point rows and material bindings") {
    using namespace wavecore;
    using E = Quad4;
    using M = LinearElasticPlaneStrain;
    NodeArchetype<2> nodes;
    for (const auto& x : std::array<std::array<double,2>,6>{{{0,0},{1,0},{2,0},{0,1},{1,1},{2,1}}})
        static_cast<void>(nodes.add_node(Node2D{x}));
    ElementArchetype<E> elements;
    ElementNodeRelation<E> en;
    ElementGaussPointRelation<E> eg;
    MaterialArchetype<M> materials;
    GaussPointMaterialRelation gm;
    GaussPointArchetype<E,M> gp;
    const auto soft = materials.add_material(M{100.0,.25,1.0});
    const auto stiff = materials.add_material(M{200.0,.25,2.0});
    const auto g0 = gp.add_point(materials.material(stiff));
    const auto g1 = gp.add_point(materials.material(soft));
    static_cast<void>(gm.add(stiff)); static_cast<void>(gm.add(soft));
    static_cast<void>(elements.add_element(PlaneElementProperties{1.0}));
    static_cast<void>(elements.add_element(PlaneElementProperties{1.0}));
    static_cast<void>(en.add({0,1,4,3})); static_cast<void>(en.add({1,2,5,4}));
    static_cast<void>(eg.add({g1})); static_cast<void>(eg.add({g0}));
    for (std::size_t n=0;n<nodes.size();++n) nodes.node(n).velocity()={nodes.node(n).coordinates()[0],0};
    refresh_element_geometry(elements,nodes.values(),en);
    refresh_gauss_point_geometry(elements,nodes.values(),en,eg,gp);
    compute_strain_rates(elements,nodes.values(),en,eg,gp);
    update_material(materials,gm,gp,1.0);
    CHECK(gp.state(g1).stress(0,0) == doctest::Approx(120.0));
    CHECK(gp.state(g0).stress(0,0) == doctest::Approx(240.0));
    CHECK(gp.state(g1).stress_zz == doctest::Approx(40.0));
    CHECK(gp.state(g0).stress_zz == doctest::Approx(80.0));
    gp.state(g1).stress(0,1)=11.0;
    CHECK(gp.state(g1).stress(1,0) == 11.0);
    CHECK(gp.state(g0).stress(0,1) == 0.0);
    assemble_lumped_mass(elements,nodes.values(),en,eg,materials,gm);
    CHECK(nodes.node(1).mass() == doctest::Approx(.75));
    clear_nodal_forces(nodes.values());
    assemble_internal_forces(elements,nodes.values(),en,eg,materials,gm,gp);
    const auto first = static_cast<Node2D::Vector>(nodes.node(1).internal_force());
    assemble_internal_forces(elements,nodes.values(),en,eg,materials,gm,gp);
    CHECK(nodes.node(1).internal_force()[0] == doctest::Approx(2.0*first[0]));
    CHECK(nodes.node(1).internal_force()[1] == doctest::Approx(2.0*first[1]));
    double total_x=0.0,total_y=0.0;
    for (const auto& node:nodes.values()) { total_x+=node.internal_force()[0];total_y+=node.internal_force()[1]; }
    CHECK(total_x == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(total_y == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(en.view()[Quad4NodeEntry::n0][1] == 1);
    CHECK(eg.view()[Quad4PointEntry::g0][0] == g1);
    CHECK(gm.view()[g0] == stiff);
}

TEST_CASE("Legacy owning node arrays are updated rather than copied by joins") {
    using namespace wavecore;
    std::array<Node2D,4> nodes{Node2D{{0,0}},Node2D{{1,0}},Node2D{{1,1}},Node2D{{0,1}}};
    ElementArchetype<Quad4> elements;
    MaterialArchetype<LinearElasticPlaneStrain> materials;
    GaussPointArchetype<Quad4,LinearElasticPlaneStrain> points;
    ElementNodeRelation<Quad4> en;
    ElementGaussPointRelation<Quad4> eg;
    GaussPointMaterialRelation gm;
    static_cast<void>(elements.add_element(PlaneElementProperties{1.0}));
    const auto m=materials.add_material(LinearElasticPlaneStrain{100,.25,4});
    const auto g=points.add_point(materials.material(m));
    static_cast<void>(en.add({0,1,2,3}));
    static_cast<void>(eg.add({g}));
    static_cast<void>(gm.add(m));
    refresh_element_geometry(elements,nodes,en);
    assemble_lumped_mass(elements,nodes,en,eg,materials,gm);
    CHECK(nodes[0].mass() == doctest::Approx(1.0));
    nodes[0].external_force()[0]=2.0;
    compute_accelerations(nodes);
    CHECK(nodes[0].acceleration()[0] == doctest::Approx(2.0));
    first_half_velocity_update(nodes,.5);
    CHECK(nodes[0].velocity()[0] == doctest::Approx(.5));
    clear_nodal_forces(nodes);
    CHECK(nodes[0].external_force()[0] == 0.0);
}
