/*

Copyright (C) University of Oxford, 2005-2011

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

*/
/*
 *
 *  Chaste tutorial - this page gets automatically changed to a wiki page
 *  DO NOT remove the comments below, and if the code has to be changed in
 *  order to run, please check the comments are still accurate
 *
 *
 */

#ifndef TESTRUNNINGMESHBASEDSIMULATIONSTUTORIAL_HPP_
#define TESTRUNNINGMESHBASEDSIMULATIONSTUTORIAL_HPP_

/*
 * = Examples showing how to create, run and visualize mesh-based simulations =
 *
 * EMPTYLINE
 *
 * == Introduction ==
 *
 * EMPTYLINE
 *
 * In this tutorial we show how Chaste can be used to create, run and visualize mesh-based simulations.
 * Full details of the mathematical model can be found in van Leeuwen ''et al.'' (2009) [doi:10.1111/j.1365-2184.2009.00627.x].
 *
 * EMPTYLINE
 *
 * == The test ==
 *
 * EMPTYLINE
 *
 * We begin by including the necessary header files. The first thing to do is include the 
 * following header file, which allows us to use certain methods in our test. This header 
 * file must be included in any Chaste test.
 */
#include <cxxtest/TestSuite.h>
/* Any test in which the {{{GetIdentifier()}}} method is used, even via the main 
 * `cell_based` code (through calls to {{{AbstractCellPopulation}}} output methods), 
 * must also include {{{CheckpointArchiveTypes.hpp}}} or {{{CellBasedSimulationArchiver.hpp}}} 
 * as the first Chaste header file. 
 */
#include "CheckpointArchiveTypes.hpp"
/* The next header includes the Boost shared_ptr smart pointer, and defines some useful 
 * macros to save typing when using it.
 */
#include "SmartPointers.hpp"
/* The remaining header files define classes that will be used in the cell population
 * simulation test. The first defines a helper class for generating a suitable collection 
 * of cells. */
#include "CellsGenerator.hpp"
/* The next header file defines a stochastic cell-cycle model class. */
#include "StochasticDurationCellCycleModel.hpp"
/* The next header file defines a helper class for generating a suitable mesh. */
#include "HoneycombMeshGenerator.hpp"
/* The next header file defines the class that simulates the evolution of an off-lattice {{{CellPopulation}}}. */
#include "OffLatticeSimulation.hpp"
/* The next header files define a mesh-based {{{CellPopulation}}} with and without ghost nodes class.*/
#include "MeshBasedCellPopulation.hpp"
#include "MeshBasedCellPopulationWithGhostNodes.hpp"
/* The next header file defines a force law for describing the mechanical interactions
 * between neighbouring cells in the cell population.
 */
#include "GeneralisedLinearSpringForce.hpp"
/* Next, we define the test class, which inherits from {{{CxxTest::TestSuite}}}
 * and defines some test methods.
 */
class TestRunningMeshBasedSimulationsTutorial : public CxxTest::TestSuite
{
public:
    /* EMPTYLINE
     *
     * == Test 1 - a basic mesh-based simulation ==
     *
     * EMPTYLINE
     *
     * In the first test, we run a simple mesh-based simulation, in which we create a monolayer
     * of cells, using a mutable mesh. Each cell is assigned a stochastic cell-cycle model.
     */
    void TestMonolayer() throw(Exception)
    {
        /* As in most cell-based Chaste tutorials, we begin by setting up the start time. */
        SimulationTime::Instance()->SetStartTime(0.0);

        /* Next, we generate a mutable mesh. To create a {{{MutableMesh}}}, we can use
         * the {{{HoneycombMeshGenerator}}}. This generates a honeycomb-shaped mesh,
         * in which all nodes are equidistant. Here the first and second arguments
         * define the size of the mesh - we have chosen a mesh that is 2 nodes (i.e.
         * cells) wide, and 2 nodes high.
         */
        HoneycombMeshGenerator generator(2, 2);    // Parameters are: cells across, cells up
        MutableMesh<2,2>* p_mesh = generator.GetMesh();

        /* Having created a mesh, we now create a {{{std::vector}}} of {{{CellPtr}}}s.
         * To do this, we the `CellsGenerator` helper class, which is templated over the type
         * of cell model required (here {{{StochasticDurationCellCycleModel}}})
         * and the dimension. We create an empty vector of cells and pass this into the
         * method along with the mesh. The second argument represents the size of that the vector
         * {{{cells}}} should become - one cell for each node, the third argument specifies
         * the proliferative type of the cell STEM TRANSIT or DIFFERENTIATED. */
        std::vector<CellPtr> cells;
        CellsGenerator<StochasticDurationCellCycleModel, 2> cells_generator;
        cells_generator.GenerateBasicRandom(cells, p_mesh->GetNumNodes(),TRANSIT);

        /* Now we have a mesh and a set of cells to go with it, we can create a {{{CellPopulation}}}.
         * In general, this class associates a collection of cells with a set of elements or a mesh.
         * For this test, because we have a {{{MutableMesh}}}, we use a particular type of
         * cell population called a {{{MeshBasedCellPopulation}}}.
         */
        MeshBasedCellPopulation<2> cell_population(*p_mesh, cells);

        /* We then pass in the cell population into an {{{OffLatticeSimulation}}},
         * and set the output directory and end time. */
        OffLatticeSimulation<2> simulator(cell_population);
        simulator.SetOutputDirectory("MeshBasedMonolayer");
        simulator.SetEndTime(10.0);

        /*
         * For longer simulations, we may not want to output the results
         * every time step. In this case we can use the following method,
         * to print results every 12 time steps instead. As the time step
         * used by the simulator, is 30 seconds, this method will cause the
         * simulator to print results every 6 minutes.
         */
        simulator.SetSamplingTimestepMultiple(12);

        /* We must now create one or more force laws, which determine the mechanics of the centres
         * of each cell in a cell population. For this test, we use one force law, based on the
         * spring based model, and pass it to the {{{OffLatticeSimulation}}}.
         * For a list of possible forces see subclasses of {{{AbstractForce}}}.
         * These can be found in the inheritance diagram, here, [class:AbstractForce AbstractForce].
         * Note that some of these forces are not compatible with mesh-based simulations see the specific class documentation for details,
         * if you try to use an incompatible class then you will receive a warning.
         */
        MAKE_PTR(GeneralisedLinearSpringForce<2>, p_force);
        simulator.AddForce(p_force);

        /* To run the simulation, we call {{{Solve()}}}. */
        simulator.Solve();

        /* {{{SimulationTime::Destroy()}}} '''must''' be called at the end of the test.
         * If not, when {{{SimulationTime::Instance()->SetStartTime(0.0);}}} is called
         * at the beginning of the next test in this file, an assertion will be triggered.
         */
        SimulationTime::Destroy();
    }

    /*
     * EMPTYLINE
     *
     * To visualize the results, open a new terminal, {{{cd}}} to the Chaste directory,
     * then {{{cd}}} to {{{anim}}}. Then do: {{{java Visualize2dCentreCells /tmp/$USER/testoutput/MeshBasedMonolayer/results_from_time_0}}}.
     * We may have to do: {{{javac Visualize2dCentreCells.java}}} beforehand to create the
     * java executable.
     *
     * You will notice that half of each cell cell around the edge is missing.
     * This is because the Voronoi region for nodes on the edge of the mesh can be
     * infinite, therefore we only visualise the part inside the mesh.
     *
     * This also means there may be "long" edges in the mesh which can cause the cells
     * to move due long range interactions resulting in an artificially rounded shape.
     *
     * There are two solutiona to this. The first is to define a cut off length on the force,
     * which can be done by using the command
     *
     * {{{p_force->SetCutOffLength(1.5);}}}
     *
     * on the {{{GeneralisedLinearSpringForce}}}. Here there will be no forces exerted
     * on any "springs" which are longer than 1.5 cell radii.
     *
     * The second solution is to use 'ghost nodes'. Ghost nodes can be added to mesh-based
     * simulations to remove infinite voronoii regions and long edges. To do this, a set of
     * nodes (known as ghost nodes) are added around the original mesh which exert forces
     * on each other but do not exert forces on the nodes of the original mesh (known as
     * real nodes). In addition real nodes exert forces on ghost nodes so the ghost nodes
     * remain surrounding the cell population.
     *
     * EMPTYLINE
     *
     * == Test 2 - a basic mesh-based simulation with ghost nodes ==
     *
     * EMPTYLINE
     *
     * In the second test, we run a simple mesh-based simulation with ghost nodes, in which we
     * create a monolayer of cells, using a mutable mesh.
     * Each cell is assigned a stochastic cell-cycle model.
     */
    void TestMonolayerWithGhostNodes() throw(Exception)
    {
        /* Once again, we begin by setting up the start time. */
        SimulationTime::Instance()->SetStartTime(0.0);

        /* Next, we generate a mutable mesh. To create a {{{MutableMesh}}}, we can use
         * the {{{HoneycombMeshGenerator}}} as before. Here the first and second arguments
         * define the size of the mesh - we have chosen a mesh that is 2 nodes (i.e.
         * cells) wide, and 2 nodes high.The third argument specifies the number of layers
         * of ghost nodes to make.
         */
        HoneycombMeshGenerator generator(2, 2, 2);
        MutableMesh<2,2>* p_mesh = generator.GetMesh();

        /* We only want to create cells to attach to real nodes, so we
         * use the method {{{GetCellLocationIndices}}} to get the indices
         * of the real nodes in the mesh. This will be passed in to the
         * cell population later on.
         */
        std::vector<unsigned> location_indices = generator.GetCellLocationIndices();

        /* Having created a mesh, we now create a {{{std::vector}}} of {{{CellPtr}}}s.
         * To do this, we the `CellsGenerator` helper class again. This time the second
         * argument is different and is the number of real nodes in the mesh.
         * As before all cells are TRANSIT cells. */
        std::vector<CellPtr> cells;
        CellsGenerator<StochasticDurationCellCycleModel, 2> cells_generator;
        cells_generator.GenerateBasicRandom(cells, location_indices.size(), TRANSIT);

        /* Now we have a mesh and a set of cells to go with it, we can create a {{{CellPopulation}}}.
         * In general, this class associates a collection of cells with a set of elements or a mesh.
         * For this test, because we have a {{{MutableMesh}}}, and ghost nodes we use a particular type of
         * cell population called a {{{MeshBasedCellPopulationWithGhostNodes}}}. The third
         * argument of the constructor takes a vector of the indices of the real nodes and should be the
         * same length as the vector of cell pointers.
         */
        MeshBasedCellPopulationWithGhostNodes<2> cell_population(*p_mesh, cells, location_indices); //**Changed**//

        /* We then pass in the cell population into an {{{OffLatticeSimulation}}},
         * and set the output directory, output multiple and end time. */
        OffLatticeSimulation<2> simulator(cell_population);
        simulator.SetOutputDirectory("MeshBasedMonolayerWithGhostNodes");
        simulator.SetSamplingTimestepMultiple(12);
        simulator.SetEndTime(10.0);

        /* Again we create a force laws, and pass it to the {{{OffLatticeSimulation}}}. This
         * force law ensures that ghost nodes dont exert forces on real nodes but real nodes
         * exert forces on ghost nodes.*/
        MAKE_PTR(GeneralisedLinearSpringForce<2>, p_force);
        simulator.AddForce(p_force);

        /* To run the simulation, we call {{{Solve()}}}. */
        simulator.Solve();

        /* We conclude by calling {{{SimulationTime::Destroy()}}} as before. */
        SimulationTime::Destroy();
    }
    /*
     * EMPTYLINE
     *
     * To visualize the results, open a new terminal, {{{cd}}} to the Chaste directory,
     * then {{{cd}}} to {{{anim}}}. Then do: {{{java Visualize2dCentreCells /tmp/$USER/testoutput/MeshBasedMonolayerWithGhostNodes/results_from_time_0}}}.
     * We may have to do: {{{javac Visualize2dCentreCells.java}}} beforehand to create the
     * java executable.
     *
     * EMPTYLINE
     */
};

#endif /* TESTRUNNINGMESHBASEDSIMULATIONSTUTORIAL_HPP_ */
