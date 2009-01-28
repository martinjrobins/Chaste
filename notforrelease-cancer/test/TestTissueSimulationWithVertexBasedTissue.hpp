/*

Copyright (C) University of Oxford, 2005-2009

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
#ifndef TESTTISSUESIMULATIONWITHVERTEXBASEDTISSUE_HPP_
#define TESTTISSUESIMULATIONWITHVERTEXBASEDTISSUE_HPP_

#include <cxxtest/TestSuite.h>

// Must be included before other cancer headers
#include "TissueSimulationArchiver.hpp"

#include "TissueSimulation.hpp"
#include "FixedCellCycleModel.hpp"
#include "VertexBasedTissue.hpp"
#include "VertexBasedTissueForce.hpp"
#include "AbstractCancerTestSuite.hpp"


class TestTissueSimulationWithVertexBasedTissue : public AbstractCancerTestSuite
{
private:

    double mLastStartTime;
    void setUp()
    {
        mLastStartTime = std::clock();
        AbstractCancerTestSuite::setUp();
    }
    void tearDown()
    {
        double time = std::clock();
        double elapsed_time = (time - mLastStartTime)/(CLOCKS_PER_SEC);
        std::cout << "Elapsed time: " << elapsed_time << std::endl;
        AbstractCancerTestSuite::tearDown();
    }

public:

    void TestSolveThrowsNothing() throw (Exception)
    {
        // Create a simple 2D VertexMesh
        VertexMesh<2,2> mesh(6, 6, 0.01, 2.0);

        // Set up cells, one for each VertexElement. Give each cell
        // a random birth time of -elem_index, so its age is elem_index
        std::vector<TissueCell> cells;
        for (unsigned elem_index=0; elem_index<mesh.GetNumElements(); elem_index++)
        {
            TissueCell cell(DIFFERENTIATED, HEALTHY, new FixedCellCycleModel());
            double birth_time = 0.0 - elem_index;
            cell.SetBirthTime(birth_time);
            cells.push_back(cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        // Create a force system
        VertexBasedTissueForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestSolveThrowsNothing");
        simulator.SetEndTime(1.0);

        // Run simulation
        TS_ASSERT_THROWS_NOTHING(simulator.Solve());
    }


    void TestMonolayerWithCellBirth() throw (Exception)
    {
        // Create a simple 2D VertexMesh
        VertexMesh<2,2> mesh(3, 3, 0.01, 2.0);

        // Set up cells, one for each VertexElement. Give each cell
        // a random birth time of -elem_index, so its age is elem_index
        std::vector<TissueCell> cells;
        for (unsigned elem_index=0; elem_index<mesh.GetNumElements(); elem_index++)
        {
            CellType cell_type = DIFFERENTIATED;
            double birth_time = 0.0 - elem_index;

            // Cell 4 should divide at time t=0.5
            if (elem_index==4)
            {
                cell_type = STEM;
                birth_time = -23.5;          
            }

            TissueCell cell(cell_type, HEALTHY, new FixedCellCycleModel());
            cell.SetBirthTime(birth_time);
            cells.push_back(cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        unsigned old_num_nodes = tissue.GetNumNodes();
        unsigned old_num_elements = tissue.GetNumElements();
        unsigned old_num_cells = tissue.GetNumRealCells();

        // Create a force system
        VertexBasedTissueForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestMonolayerWithCellBirth");
        simulator.SetEndTime(1.0);

        // Run simulation
        simulator.Solve();

        // Check that cell 4 divided successfully
        unsigned new_num_nodes = simulator.rGetTissue().GetNumNodes();
        unsigned new_num_elements = (static_cast<VertexBasedTissue<2>*>(&(simulator.rGetTissue())))->GetNumElements();
        unsigned new_num_cells = simulator.rGetTissue().GetNumRealCells();

        TS_ASSERT_EQUALS(new_num_nodes, old_num_nodes+2);
        TS_ASSERT_EQUALS(new_num_elements, old_num_elements+1);
        TS_ASSERT_EQUALS(new_num_cells, old_num_cells+1);
    }

    /**
     * Test archiving of a TissueSimulation that uses a VertexBasedTissue.
     */
    void TestArchiving() throw (Exception)
    {
        // Create a simple 2D VertexMesh
        VertexMesh<2,2> mesh(6, 6, 0.01, 2.0);

        // Set up cells, one for each VertexElement. Give each cell
        // a random birth time of -elem_index, so its age is elem_index
        std::vector<TissueCell> cells;
        for (unsigned elem_index=0; elem_index<mesh.GetNumElements(); elem_index++)
        {
            TissueCell cell(DIFFERENTIATED, HEALTHY, new FixedCellCycleModel());
            double birth_time = 0.0 - elem_index;
            cell.SetBirthTime(birth_time);
            cells.push_back(cell);
        }

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        // Create a force system
        VertexBasedTissueForce<2> force;
        std::vector<AbstractForce<2>* > force_collection;
        force_collection.push_back(&force);

        // Set up tissue simulation
        TissueSimulation<2> simulator(tissue, force_collection);
        simulator.SetOutputDirectory("TestTissueSimulationWithVertexBasedTissueSaveAndLoad");
        simulator.SetEndTime(0.5);

        simulator.Solve();

        TissueSimulationArchiver<2, TissueSimulation<2> >::Save(&simulator);

        TissueSimulation<2>* p_simulator
            = TissueSimulationArchiver<2, TissueSimulation<2> >::Load("TestTissueSimulationWithVertexBasedTissueSaveAndLoad", 0.5);

        p_simulator->SetEndTime(1.0);

        TS_ASSERT_THROWS_NOTHING(p_simulator->Solve());

        /// \todo add further tests (see #821 and #862)
        
        // Tidy up 
        delete p_simulator;
    }
};

#endif /*TESTTISSUESIMULATIONWITHVERTEXBASEDTISSUE_HPP_*/
