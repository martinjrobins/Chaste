/*

Copyright (C) University of Oxford, 2008

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

#ifndef TESTVERTEXBASEDTISSUE_HPP_
#define TESTVERTEXBASEDTISSUE_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "VertexBasedTissue.hpp"
#include "FixedCellCycleModel.hpp"
#include "AbstractCancerTestSuite.hpp"


class TestVertexBasedTissue : public AbstractCancerTestSuite
{
private:

    /**
     * Set up cells, one for each VertexElement. 
     * Give each cell a birth time of -elem_index, 
     * so its age is elem_index.
     */
    template<unsigned DIM>
    std::vector<TissueCell> SetUpCells(VertexMesh<DIM,DIM>& rMesh)
    {
        std::vector<TissueCell> cells;
        for (unsigned i=0; i<rMesh.GetNumElements(); i++)
        {
            TissueCell cell(DIFFERENTIATED, HEALTHY, new FixedCellCycleModel());
            double birth_time = 0.0-i;
            cell.SetLocationIndex(i);
            cell.SetBirthTime(birth_time);
            cells.push_back(cell);
        }
        return cells;
    }
    
public:

    // Test construction, accessors and iterator
    void TestCreateSmallVertexBasedTissue() throw(Exception)
    {
        // Create a simple 2D VertexMesh
        VertexMesh<2,2> mesh(5,3); // columns then rows

        // Set up cells
        std::vector<TissueCell> cells = SetUpCells(mesh);
        
        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);
        
        // Test we have the correct number of cells and elements
        TS_ASSERT_EQUALS(tissue.GetNumElements(), mesh.GetNumElements());
        TS_ASSERT_EQUALS(tissue.rGetCells().size(), cells.size());

        unsigned counter = 0;

        // Test VertexBasedTissue::Iterator
        for (VertexBasedTissue<2>::Iterator cell_iter = tissue.Begin();
             cell_iter != tissue.End();
             ++cell_iter)
        {
            // Test operator* and that cells are in sync
            TS_ASSERT_EQUALS((*cell_iter).GetLocationIndex(), counter);

            // Test operator-> and that cells are in sync
            TS_ASSERT_DELTA(cell_iter->GetAge(), (double)counter, 1e-12);

            // Test GetLocationIndex() on the iterator
            TS_ASSERT_EQUALS(cell_iter->GetLocationIndex(), mesh.GetElement(counter)->GetIndex());

            counter++;
        }

        // Test we have gone through all cells in the for loop
        TS_ASSERT_EQUALS(counter, tissue.GetNumRealCells());
        
        // Test GetNumNodes() method
        TS_ASSERT_EQUALS(tissue.GetNumNodes(), mesh.GetNumNodes());
    }
    
    void TestValidateVertexBasedTissue()
    {
        // Create a simple vertex-based mesh
        VertexMesh<2,2> mesh(4,6); // columns then rows

        // Set up cells
        std::vector<TissueCell> cells = SetUpCells(mesh);
        cells[0].SetLocationIndex(1);

        // This test fails as there is no cell to element 0
        TS_ASSERT_THROWS_ANYTHING(VertexBasedTissue<2> tissue(mesh, cells));
    }
    
    void TestUpdateWithoutBirthOrDeath()
    {
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetEndTimeAndNumberOfTimeSteps(10.0, 1);
        
        // Create a simple vertex-based mesh
        VertexMesh<2,2> mesh(4,6); // columns then rows

        // Set up cells
        std::vector<TissueCell> cells = SetUpCells(mesh);

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);
        
        /// \todo Coverage (can be removed once test below is completed - see #827) 
        unsigned num_cells_removed = tissue.RemoveDeadCells();
        TS_ASSERT_EQUALS(num_cells_removed, 0u);
        
        p_simulation_time->IncrementTimeOneStep();
        
        TS_ASSERT_THROWS_NOTHING(tissue.Update());    
    }
    
    /// \todo This test currently fails, since the method RemoveDeadCells() does not yet
    // delete the elements/nodes assoicated with dead cells (see #853)
    void DONTTestRemoveDeadCellsAndUpdate()
    {
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetEndTimeAndNumberOfTimeSteps(10.0, 1);
        
        // Create a simple vertex-based mesh
        VertexMesh<2,2> mesh(4,6); // columns then rows

        // Set up cells
        std::vector<TissueCell> cells = SetUpCells(mesh);
        cells[5].StartApoptosis();

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);

        TS_ASSERT_EQUALS(mesh.GetNumElements(), 24u);        
        TS_ASSERT_EQUALS(tissue.rGetCells().size(), 24u);
        TS_ASSERT_EQUALS(tissue.GetNumRealCells(), 24u);
        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 68u);

        p_simulation_time->IncrementTimeOneStep();
        
        // Remove dead cells
        unsigned num_cells_removed = tissue.RemoveDeadCells();
       
        /// \todo Currently RemoveDeadCells() does nothing, and is only 
        //        in a test for coverage. Cell death will be implemented 
        //        in #853.
        TS_ASSERT_EQUALS(num_cells_removed, 1u);
        
        // We should now have one less real cell, since one cell has been 
        // marked as dead, so is skipped by the tissue iterator
        TS_ASSERT_EQUALS(tissue.GetNumRealCells(), 23u);
        
        /// \todo Need some more tests here, on the new number of elements/nodes
                
        TS_ASSERT_EQUALS(tissue.rGetCells().size(), cells.size()); // the tissue now copies cells

        tissue.Update();
        
        // Finally, check the cells' element indices have updated

        // We expect the cell element indices to be {0,11,...,23}
        std::set<unsigned> expected_elem_indices;
        for (unsigned i=0; i<tissue.GetNumRealCells(); i++)
        {
            expected_elem_indices.insert(i);
        }

        // Get actual cell element indices
        std::set<unsigned> element_indices;

        for (AbstractTissue<2>::Iterator cell_iter = tissue.Begin();
             cell_iter != tissue.End();
             ++cell_iter)
        {
            // Record element index corresponding to cell
            unsigned element_index = cell_iter->GetLocationIndex();
            element_indices.insert(element_index);
        }

        TS_ASSERT_EQUALS(element_indices, expected_elem_indices);        
    }
    
    void TestVertexBasedTissueOutputWriters()
    {
        // Create a simple vertex-based mesh
        VertexMesh<2,2> mesh(4,6); // columns then rows

        // Set up cells
        std::vector<TissueCell> cells = SetUpCells(mesh);

        // Create tissue
        VertexBasedTissue<2> tissue(mesh, cells);
        
        // For coverage of WriteResultsToFiles()
        tissue.rGetCellUsingLocationIndex(0).SetCellType(TRANSIT);
        tissue.rGetCellUsingLocationIndex(0).SetMutationState(LABELLED);
        tissue.rGetCellUsingLocationIndex(1).SetCellType(DIFFERENTIATED);
        tissue.rGetCellUsingLocationIndex(1).SetMutationState(APC_ONE_HIT);
        tissue.rGetCellUsingLocationIndex(2).SetMutationState(APC_TWO_HIT);
        tissue.rGetCellUsingLocationIndex(3).SetMutationState(BETA_CATENIN_ONE_HIT);
        tissue.rGetCellUsingLocationIndex(4).SetCellType(APOPTOTIC);
        tissue.rGetCellUsingLocationIndex(4).StartApoptosis();
        tissue.rGetCellUsingLocationIndex(5).SetCellType(STEM);
        tissue.SetCellAncestorsToNodeIndices();

        std::string output_directory = "TestVertexBasedTissueOutputWriters";
        OutputFileHandler output_file_handler(output_directory, false);

        TS_ASSERT_THROWS_NOTHING(tissue.CreateOutputFiles(output_directory, false, true, true, false, true, true));

        tissue.WriteResultsToFiles(true, true, false, true, true);

        TS_ASSERT_THROWS_NOTHING(tissue.CloseOutputFiles(true, true, false, true, true));

        // Compare output with saved files of what they should look like
        std::string results_dir = output_file_handler.GetOutputDirectoryFullPath();

        TS_ASSERT_EQUALS(system(("diff " + results_dir + "results.viznodes     notforrelease-cancer/test/data/TestVertexBasedTissueOutputWriters/results.viznodes").c_str()), 0);
        TS_ASSERT_EQUALS(system(("diff " + results_dir + "results.vizelements     notforrelease-cancer/test/data/TestVertexBasedTissueOutputWriters/results.vizelements").c_str()), 0);
        TS_ASSERT_EQUALS(system(("diff " + results_dir + "results.vizcelltypes     notforrelease-cancer/test/data/TestVertexBasedTissueOutputWriters/results.vizcelltypes").c_str()), 0);
        TS_ASSERT_EQUALS(system(("diff " + results_dir + "results.vizancestors     notforrelease-cancer/test/data/TestVertexBasedTissueOutputWriters/results.vizancestors").c_str()), 0);
        TS_ASSERT_EQUALS(system(("diff " + results_dir + "cellmutationstates.dat     notforrelease-cancer/test/data/TestVertexBasedTissueOutputWriters/cellmutationstates.dat").c_str()), 0);

        // For coverage
        TS_ASSERT_THROWS_NOTHING(tissue.WriteResultsToFiles(true, false, false, true, false));
    }

    // At the moment the tissue cannot be properly archived since the mesh cannot be. This test
    // just checks that the cells are correctly archived.
    /// \todo This test should run once archiving of VertexMesh is sorted (see #821 and #862)
    void DONTTestArchivingVertexBasedTissue() throw (Exception)
    {
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetOutputDirectoryFullPath() + "tissue.arch";

        // Archive tissue
        {
            // Need to set up time
            unsigned num_steps=10;
            SimulationTime* p_simulation_time = SimulationTime::Instance();
            p_simulation_time->SetEndTimeAndNumberOfTimeSteps(1.0, num_steps+1);

            // Create a basic vertex-based mesh 'by hand'...
            
            // ... first create seven nodes...
            std::vector<Node<2>*> basic_nodes;
            basic_nodes.push_back(new Node<2>(0, false, 0.0, 0.0));
            basic_nodes.push_back(new Node<2>(1, false, 1.0, 0.0));
            basic_nodes.push_back(new Node<2>(2, false, 1.5, 1.0));
            basic_nodes.push_back(new Node<2>(3, false, 1.0, 2.0));
            basic_nodes.push_back(new Node<2>(4, false, 0.0, 1.0));
            basic_nodes.push_back(new Node<2>(5, false, 2.0, 0.0));
            basic_nodes.push_back(new Node<2>(6, false, 2.0, 3.0));
            
            std::vector<Node<2>*> nodes_elem_0, nodes_elem_1;
            
            nodes_elem_0.push_back(basic_nodes[0]);
            nodes_elem_0.push_back(basic_nodes[1]);
            nodes_elem_0.push_back(basic_nodes[2]);
            nodes_elem_0.push_back(basic_nodes[3]);
            nodes_elem_0.push_back(basic_nodes[4]);
            
            nodes_elem_1.push_back(basic_nodes[2]);
            nodes_elem_1.push_back(basic_nodes[5]);
            nodes_elem_1.push_back(basic_nodes[6]);
            
            // ... now create two elements associated with these nodes...
            std::vector<VertexElement<2,2>*> basic_vertex_elements;
            basic_vertex_elements.push_back(new VertexElement<2,2>(0, nodes_elem_0));
            basic_vertex_elements.push_back(new VertexElement<2,2>(1, nodes_elem_1));
            
            // ... finally, create mesh
            VertexMesh<2,2> mesh(basic_nodes, basic_vertex_elements);
    
            // Set up cells
            std::vector<TissueCell> cells = SetUpCells(mesh);
    
            // Create thetissue
            VertexBasedTissue<2>* const p_tissue = new VertexBasedTissue<2>(mesh, cells);

            // Cells have been given birth times of 0 and -1.
            // Loop over them to run to time 0.0;
            for (AbstractTissue<2>::Iterator cell_iter=p_tissue->Begin();
                 cell_iter!=p_tissue->End();
                 ++cell_iter)
            {
                cell_iter->ReadyToDivide();
            }
            
            // Create an output archive
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            // Write the tissue to the archive
            output_arch << static_cast<const SimulationTime&> (*p_simulation_time);
            output_arch << p_tissue;
            SimulationTime::Destroy();
            delete p_tissue;
        }

        // Restore tissue
        {
            // Need to set up time
            unsigned num_steps=10;
            SimulationTime* p_simulation_time = SimulationTime::Instance();
            p_simulation_time->SetStartTime(0.0);
            p_simulation_time->SetEndTimeAndNumberOfTimeSteps(1.0, num_steps+1);
            p_simulation_time->IncrementTimeOneStep();

            VertexBasedTissue<2>* p_tissue;

            // Restore the tissue
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);
            input_arch >> *p_simulation_time;

            // The following line is required because the loading of a tissue 
            // is usually called by the method TissueSimulation::Load()
            VertexBasedTissue<2>::meshPathname = "notforrelease-cancer/test/data/TestVertexMesh";

            input_arch >> p_tissue;

            // Cells have been given birth times of 0, -1, -2, -3, -4.
            // this checks that individual cells and their models are archived.
            unsigned counter = 0u;
            for (AbstractTissue<2>::Iterator cell_iter=p_tissue->Begin();
                 cell_iter!=p_tissue->End();
                 ++cell_iter)
            {
                TS_ASSERT_DELTA(cell_iter->GetAge(), (double)(counter), 1e-7);
                counter++;
            }

            // Check the simulation time has been restored (through the cell)
            TS_ASSERT_EQUALS(p_simulation_time->GetTime(), 0.0);

            // Check the tissue has been restored
            TS_ASSERT_EQUALS(p_tissue->rGetCells().size(), 2u);

            delete p_tissue;
        }
    }
};


#endif /*TESTVERTEXBASEDTISSUE_HPP_*/
