#ifndef TESTCRYPTSIMULATION2DPERIODIC_HPP_
#define TESTCRYPTSIMULATION2DPERIODIC_HPP_

#include <cxxtest/TestSuite.h>
#include "ConformingTetrahedralMesh.cpp"
#include "TrianglesMeshReader.cpp"
#include <cmath>

#include <vector>
#include "OutputFileHandler.hpp"
#include "TissueSimulation.cpp"
#include "MeinekeCryptCell.hpp"
#include "FixedCellCycleModel.hpp"
#include "StochasticCellCycleModel.hpp"
#include "WntCellCycleModel.hpp"
#include "WntGradient.hpp"
#include "WntCellCycleOdeSystem.hpp"
#include "TysonNovakCellCycleModel.hpp"
#include "CancerParameters.hpp"
#include "ColumnDataReader.hpp"
#include "CryptHoneycombMeshGenerator.hpp"
#include "SimulationTime.hpp"

// Possible types of Cell Cycle Model (just for CreateVectorOfCells method)
typedef enum CellCycleType_
{
    FIXED,
    STOCHASTIC,
    WNT,
    TYSONNOVAK
} CellCycleType;


class TestCryptSimulation2DPeriodic : public CxxTest::TestSuite
{
    void CheckAgainstPreviousRun(std::string resultDirectory,std::string resultSet, unsigned maxCells, unsigned maxElements)
    {
        std::cout << "Comparing " << resultDirectory << std::endl << std::flush;
        
        ColumnDataReader computed_node_results = ColumnDataReader(resultDirectory+"/"+resultSet+"/tab_results",
                                                                  "tabulated_node_results",
                                                                  true);
                                                                  
        ColumnDataReader expected_node_results = ColumnDataReader("models/test/data/" + resultDirectory+"Results",
                                                                  "tabulated_node_results",
                                                                  false);
        ColumnDataReader computed_element_results = ColumnDataReader(resultDirectory+"/"+resultSet+"/tab_results",
                                                    "tabulated_element_results",
                                                    true);
                                                    
        ColumnDataReader expected_element_results = ColumnDataReader("models/test/data/" + resultDirectory+"Results",
                                                    "tabulated_element_results",
                                                    false);
                                                    
        for (unsigned cell=0; cell<maxCells; cell++)
        {
            std::stringstream cell_type_var_name;
            std::stringstream cell_x_position_var_name;
            std::stringstream cell_y_position_var_name;
            cell_type_var_name << "cell_type_" << cell;
            cell_x_position_var_name << "cell_x_position_" << cell;
            cell_y_position_var_name << "cell_y_position_" << cell;
            
            // Vector of Cell Types
            std::vector<double> expected_cell_types = expected_node_results.GetValues(cell_type_var_name.str());
            std::vector<double> computed_cell_types = computed_node_results.GetValues(cell_type_var_name.str());
            
            //Vector of Cell Positions
            std::vector<double> expected_cell_x_positions = expected_node_results.GetValues(cell_x_position_var_name.str());
            std::vector<double> computed_cell_x_positions = computed_node_results.GetValues(cell_x_position_var_name.str());
            
            std::vector<double> expected_cell_y_positions = expected_node_results.GetValues(cell_y_position_var_name.str());
            std::vector<double> computed_cell_y_positions = computed_node_results.GetValues(cell_y_position_var_name.str());
            
            //Comparing expected and computed vector length
            TS_ASSERT_EQUALS(expected_cell_types.size(), computed_cell_types.size());
            TS_ASSERT_EQUALS(expected_cell_x_positions.size(), computed_cell_x_positions.size());
            TS_ASSERT_EQUALS(expected_cell_y_positions.size(), computed_cell_y_positions.size());
            
            //Walkthrough of the expected and computed vectors
            for (unsigned time_step = 0; time_step < expected_cell_types.size(); time_step++)
            {
                TS_ASSERT_EQUALS(expected_cell_types[time_step], computed_cell_types[time_step]);
                TS_ASSERT_DELTA(expected_cell_x_positions[time_step], computed_cell_x_positions[time_step],1e-6);
                TS_ASSERT_DELTA(expected_cell_y_positions[time_step], computed_cell_y_positions[time_step],1e-6);
            }
        }
        
        for (unsigned element=0; element<maxElements; element++)
        {
            std::stringstream nodeA_var_name;
            std::stringstream nodeB_var_name;
            std::stringstream nodeC_var_name;
            nodeA_var_name << "nodeA_" << element;
            nodeB_var_name << "nodeB_" << element;
            nodeC_var_name << "nodeC_" << element;
            
            // Vector of Node A indexes
            std::vector<double> expected_NodeA_numbers = expected_element_results.GetValues(nodeA_var_name.str());
            std::vector<double> computed_NodeA_numbers = computed_element_results.GetValues(nodeA_var_name.str());
            
            // Vector of Node B indexes
            std::vector<double> expected_NodeB_numbers = expected_element_results.GetValues(nodeB_var_name.str());
            std::vector<double> computed_NodeB_numbers = computed_element_results.GetValues(nodeB_var_name.str());
            
            // Vector of Node C indexes
            std::vector<double> expected_NodeC_numbers = expected_element_results.GetValues(nodeC_var_name.str());
            std::vector<double> computed_NodeC_numbers = computed_element_results.GetValues(nodeC_var_name.str());
            
            TS_ASSERT_EQUALS(expected_NodeA_numbers.size(), computed_NodeA_numbers.size());
            TS_ASSERT_EQUALS(expected_NodeB_numbers.size(), computed_NodeB_numbers.size());
            TS_ASSERT_EQUALS(expected_NodeC_numbers.size(), computed_NodeC_numbers.size());
            
            for (unsigned time_step = 0; time_step < expected_NodeA_numbers.size(); time_step++)
            {
                TS_ASSERT_EQUALS(expected_NodeA_numbers[time_step], computed_NodeA_numbers[time_step]);
                TS_ASSERT_EQUALS(expected_NodeB_numbers[time_step], computed_NodeB_numbers[time_step]);
                TS_ASSERT_EQUALS(expected_NodeC_numbers[time_step], computed_NodeC_numbers[time_step]);
            }
        }
    }
    
    void CreateVectorOfCells(std::vector<MeinekeCryptCell>& rCells, 
                             ConformingTetrahedralMesh<2,2>& rMesh, 
                             CellCycleType cycleType, 
                             bool randomBirthTimes,
                             double y0 = 0.3,
                             double y1 = 2.0,
                             double y2 = 3.0,
                             double y3 = 4.0)
    {
        RandomNumberGenerator *p_random_num_gen=RandomNumberGenerator::Instance();
        unsigned num_cells = rMesh.GetNumNodes();

        AbstractCellCycleModel* p_cell_cycle_model = NULL;
        double typical_transit_cycle_time;
        double typical_stem_cycle_time;
        
        CancerParameters* p_params = CancerParameters::Instance();
        
        for (unsigned i=0; i<num_cells; i++)
        {
            CryptCellType cell_type;
            unsigned generation;

            double y = rMesh.GetNode(i)->GetPoint().rGetLocation()[1];
            
            if (cycleType==FIXED)
            {
                p_cell_cycle_model = new FixedCellCycleModel();
                typical_transit_cycle_time = p_params->GetTransitCellCycleTime();
                typical_stem_cycle_time = p_params->GetStemCellCycleTime();
            }
            else if (cycleType==STOCHASTIC)
            {
                p_cell_cycle_model = new StochasticCellCycleModel();
                typical_transit_cycle_time = p_params->GetTransitCellCycleTime();
                typical_stem_cycle_time = p_params->GetStemCellCycleTime();
            }
            else if (cycleType==WNT)
            {
                WntGradient wnt_gradient(LINEAR);
                double wnt = wnt_gradient.GetWntLevel(y);
                p_cell_cycle_model = new WntCellCycleModel(wnt,0);
                typical_transit_cycle_time = 16.0;
                typical_stem_cycle_time = typical_transit_cycle_time;
            }
            else if (cycleType==TYSONNOVAK)
            {
                p_cell_cycle_model = new TysonNovakCellCycleModel();
                typical_transit_cycle_time = 1.25;
                typical_stem_cycle_time = typical_transit_cycle_time;
            }
            else
            {
                EXCEPTION("Cell Cycle Type is not recognised");   
            }
            
            
            double birth_time = 0.0;
            
            if (y <= y0)
            {
                cell_type = STEM;
                generation = 0;
                if(randomBirthTimes)
                {
                    birth_time = -p_random_num_gen->ranf()*typical_stem_cycle_time; // hours
                }
            }
            else if (y < y1)
            {
                cell_type = TRANSIT;
                generation = 1;
                if(randomBirthTimes)
                {
                    birth_time = -p_random_num_gen->ranf()*typical_transit_cycle_time; // hours 
                }
            }
            else if (y < y2)
            {
                cell_type = TRANSIT;
                generation = 2;
                if(randomBirthTimes)
                {
                    birth_time = -p_random_num_gen->ranf()*typical_transit_cycle_time; // hours 
                }
            }
            else if (y < y3)
            {
                cell_type = TRANSIT;
                generation = 3;
                if(randomBirthTimes)
                {
                    birth_time = -p_random_num_gen->ranf()*typical_transit_cycle_time; // hours 
                }
            }
            else
            {
                if(randomBirthTimes)
                {
                    birth_time = -p_random_num_gen->ranf()*typical_transit_cycle_time; // hours 
                }
                if(cycleType==WNT || cycleType==TYSONNOVAK)
                {
                    // There are no fully differentiated cells!
                    cell_type = TRANSIT;
                    
                }
                else
                {
                    cell_type = DIFFERENTIATED;
                }                
                generation = 4;
            }

            MeinekeCryptCell cell(cell_type, HEALTHY, generation, p_cell_cycle_model);
            
            cell.SetNodeIndex(i);
            cell.SetBirthTime(birth_time);
            rCells.push_back(cell);
        }
    }
public:

    void Test2DCylindrical() throw (Exception)
    {        
        std::string output_directory = "Crypt2DCylindrical";
        
        unsigned cells_across = 6;
        unsigned cells_up = 12;
        double crypt_width = 5.0;
        unsigned thickness_of_ghost_layer = 0;
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        CryptHoneycombMeshGenerator generator(cells_across, cells_up, crypt_width,thickness_of_ghost_layer);
        Cylindrical2dMesh* p_mesh=generator.GetCylindricalMesh();
        std::set<unsigned> ghost_node_indices = generator.GetGhostNodeIndices();
        
        // Set up cells
        std::vector<MeinekeCryptCell> cells;
        CreateVectorOfCells(cells, *p_mesh, FIXED, true);// true = mature cells
               
        TissueSimulation<2> simulator(*p_mesh, cells);
        simulator.SetOutputDirectory(output_directory);
        
        /* 
         * Set length of simulation
         * and other options here.
         */
        simulator.SetEndTime(0.1);
        simulator.SetMaxCells(500);
        simulator.SetMaxElements(1000);
        simulator.SetGhostNodes(ghost_node_indices);
        simulator.SetWntGradient(OFFSET_LINEAR);
        
        // These are for coverage and use the defaults
        simulator.SetDt(1.0/120.0);
        simulator.SetReMeshRule(true);
        simulator.SetNoBirth(false);
        
        simulator.Solve();

        // test we have the same number of cells and nodes at the end of each time
        // (if we do then the boundaries are probably working!)
        std::vector<MeinekeCryptCell> result_cells = simulator.GetCells();
        std::vector<bool> ghost_cells = simulator.GetGhostNodes();
        unsigned number_of_cells = 0;
        unsigned number_of_nodes = result_cells.size();
        
        TS_ASSERT_EQUALS(result_cells.size(),ghost_cells.size());
        
        for (unsigned i=0 ; i<number_of_nodes ; i++)
        {
            if (!ghost_cells[i])
            {
                number_of_cells++;
            }
        }
        TS_ASSERT_EQUALS(number_of_cells, cells_across*cells_up+1u);  // 6 cells in a row*12 rows + 1 birth
        TS_ASSERT_EQUALS(number_of_nodes, number_of_cells+thickness_of_ghost_layer*2*cells_across); 

        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }

    // This is a rubbish test - all cells start at birthTime = 0.
    // So bizarrely the crypt shrinks as the rest lengths are shortened! 
    // But at least it uses Wnt cell cycle and runs reasonably quickly...
    // For a better test with more randomly distributed cell ages see the Nightly test pack.
    void TestWithWntDependentCells() throw (Exception)
    {
        CancerParameters *p_params = CancerParameters::Instance();
        // There is no limit on transit cells in Wnt simulation
        p_params->SetMaxTransitGenerations(1000);
        
        unsigned cells_across = 6;
        unsigned cells_up = 12;
        double crypt_width = 6.0;
        unsigned thickness_of_ghost_layer = 0;
        
        CryptHoneycombMeshGenerator generator(cells_across, cells_up, crypt_width, thickness_of_ghost_layer);
        Cylindrical2dMesh* p_mesh=generator.GetCylindricalMesh();
        std::set<unsigned> ghost_node_indices = generator.GetGhostNodeIndices();
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells 
        std::vector<MeinekeCryptCell> cells;        
        CreateVectorOfCells(cells, *p_mesh, WNT, false);
                
        TissueSimulation<2> simulator(*p_mesh, cells);
        simulator.SetOutputDirectory("Crypt2DPeriodicWnt");
        
        // Set length of simulation here
        simulator.SetEndTime(0.3);
        
        simulator.SetMaxCells(500);
        simulator.SetMaxElements(1000);
        
        simulator.SetWntGradient(LINEAR);
        
        simulator.SetGhostNodes(ghost_node_indices);
        
        simulator.Solve();
        
        std::vector<double> node_35_location = simulator.GetNodeLocation(35);
        //std::vector<double> node_100_location = simulator.GetNodeLocation(100);
        
        TS_ASSERT_DELTA(node_35_location[0], 5.5000 , 1e-4);
        TS_ASSERT_DELTA(node_35_location[1], 4.4000 , 1e-4);
//        TS_ASSERT_DELTA(node_100_location[0], 4.0000 , 1e-4);
//        TS_ASSERT_DELTA(node_100_location[1], 8.0945 , 1e-4);
//          
        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }
    

    // Testing Save (based on previous test)
    void TestSave() throw (Exception)
    {
        CancerParameters *p_params = CancerParameters::Instance();
        // There is no limit on transit cells in Wnt simulation
        p_params->SetMaxTransitGenerations(1000); 
        
        unsigned cells_across = 6;
        unsigned cells_up = 12;
        double crypt_width = 6.0;
        unsigned thickness_of_ghost_layer = 4;
        
        CryptHoneycombMeshGenerator generator(cells_across, cells_up, crypt_width,thickness_of_ghost_layer);
        Cylindrical2dMesh* p_mesh=generator.GetCylindricalMesh();
        std::set<unsigned> ghost_node_indices = generator.GetGhostNodeIndices();
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells
        std::vector<MeinekeCryptCell> cells;
        CreateVectorOfCells(cells, *p_mesh, WNT, false);
        
        TissueSimulation<2> simulator(*p_mesh, cells);
        simulator.SetOutputDirectory("Crypt2DPeriodicWntSaveAndLoad");
        
        // Our full end time is 0.2, here we run for half the time
        simulator.SetEndTime(0.1);
        
        simulator.SetMaxCells(500);
        simulator.SetMaxElements(1000);

        simulator.SetWntGradient(LINEAR);
        
        simulator.SetGhostNodes(ghost_node_indices);
        
        simulator.Solve();
        
        // save the results..
        simulator.Save();
        
        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }
    

    // Testing Load (based on previous two tests)
    void TestLoad() throw (Exception)
    {
        CancerParameters *p_params = CancerParameters::Instance();
        // There is no limit on transit cells in Wnt simulation
        p_params->SetMaxTransitGenerations(1000);
        RandomNumberGenerator::Instance();
        
        unsigned cells_across = 6;
        unsigned cells_up = 12;
        double crypt_width = 6.0;
        unsigned thickness_of_ghost_layer = 4;
        
        CryptHoneycombMeshGenerator generator(cells_across, cells_up, crypt_width,thickness_of_ghost_layer);
        Cylindrical2dMesh* p_mesh=generator.GetCylindricalMesh();
        std::set<unsigned> ghost_node_indices = generator.GetGhostNodeIndices();
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells by iterating through the mesh nodes
        unsigned num_cells = p_mesh->GetNumAllNodes();
        std::cout << "Num Cells = " << num_cells << std::endl;
        std::vector<MeinekeCryptCell> cells;
        for (unsigned i=0; i<num_cells; i++)
        {
            unsigned generation = 0;
            double wnt_level = 0;
            unsigned mutation_state = 0;
            MeinekeCryptCell cell(STEM, HEALTHY, generation, new WntCellCycleModel(wnt_level,mutation_state));
            cell.SetNodeIndex(i);
            cells.push_back(cell);
        }
        TissueSimulation<2> simulator(*p_mesh, cells);

        // Load the simulation from the TestSave method above and
        // run it from 0.1 to 0.2
        simulator.Load("Crypt2DPeriodicWntSaveAndLoad",0.1);
        
        simulator.SetEndTime(0.2);
        
        simulator.Solve();
        
        // save that then reload
        // and run from 0.2 to 0.3.
        
        simulator.Save();
        
        simulator.Load("Crypt2DPeriodicWntSaveAndLoad",0.2);
        
        simulator.SetEndTime(0.3);
        
        simulator.Solve();
        
        /* 
         * This checks that these two nodes are in exactly the same location 
         * (after a saved and loaded run) as after a single run
         */
        std::vector<double> node_35_location = simulator.GetNodeLocation(35);
        std::vector<double> node_100_location = simulator.GetNodeLocation(100);
        
        TS_ASSERT_DELTA(node_35_location[0], 5.5000 , 1e-4);
        TS_ASSERT_DELTA(node_35_location[1], 2.5104 , 1e-4);
        TS_ASSERT_DELTA(node_100_location[0], 4.0000 , 1e-4);
        TS_ASSERT_DELTA(node_100_location[1], 8.0945 , 1e-4);
        
        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
        
        // When the mesh is archived we need a good test here
        // to ensure these results are the same as the ones
        // from TestWithWntDependentCells().
    }
    
    
    
    // This is a rubbish test - all cells start at birthTime = 0.
    // So bizarrely the crypt shrinks as the rest lengths are shortened! But at least it uses Wnt
    // cell cycle and runs reasonably quickly...
    // For a better test with more randomly distributed cell ages see the Nightly test pack.
    void TestWithWntDependentCellsAndAMutation() throw (Exception)
    {
        CancerParameters *p_params = CancerParameters::Instance();
        // There is no limit on transit cells in Wnt simulation
        p_params->SetMaxTransitGenerations(1000);
        
        unsigned cells_across = 6;
        unsigned cells_up = 12;
        double crypt_width = 6.0;
        unsigned thickness_of_ghost_layer = 4;
        
        CryptHoneycombMeshGenerator generator(cells_across, cells_up, crypt_width,thickness_of_ghost_layer);
        Cylindrical2dMesh* p_mesh=generator.GetCylindricalMesh();
        std::set<unsigned> ghost_node_indices = generator.GetGhostNodeIndices();
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells
        std::vector<MeinekeCryptCell> cells;
        CreateVectorOfCells(cells, *p_mesh, WNT, false);

        // Set a stem cell to be an evil cancer cell and see what happens
        cells[27].SetMutationState(APC_TWO_HIT);
        
        TissueSimulation<2> simulator(*p_mesh, cells);
        simulator.SetOutputDirectory("Crypt2DMutation");
        
        simulator.SetMaxCells(500);
        simulator.SetMaxElements(1000);
        simulator.SetWntGradient(LINEAR);
        
        simulator.SetGhostNodes(ghost_node_indices);
                
        simulator.SetEndTime(0.05);
        
        simulator.Solve();
                
        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }

    
    // This is strange test -- all cells divide within a quick time, it gives
    // good testing of the periodic boundaries though...
    void TestWithTysonNovakCells() throw (Exception)
    {
        CancerParameters *p_params = CancerParameters::Instance();
        // There is no limit on transit cells in T&N
        p_params->SetMaxTransitGenerations(1000);
        
        unsigned cells_across = 6;
        unsigned cells_up = 12;
        double crypt_width = 6.0;
        unsigned thickness_of_ghost_layer = 4;
        
        CryptHoneycombMeshGenerator generator(cells_across, cells_up, crypt_width,thickness_of_ghost_layer);
        Cylindrical2dMesh* p_mesh=generator.GetCylindricalMesh();
        std::set<unsigned> ghost_node_indices = generator.GetGhostNodeIndices();
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells
        std::vector<MeinekeCryptCell> cells;
        CreateVectorOfCells(cells, *p_mesh, TYSONNOVAK, true);
        
        
        TissueSimulation<2> simulator(*p_mesh, cells);
        simulator.SetOutputDirectory("Crypt2DPeriodicTysonNovak");
        
        // Set length of simulation here
        simulator.SetEndTime(0.05);
        
        simulator.SetMaxCells(500);
        simulator.SetMaxElements(1000);
        
        simulator.SetGhostNodes(ghost_node_indices);
        
        simulator.SetDt(0.001);
        
        simulator.Solve();
        
        // test we have the same number of cells and nodes at the end of each time
        // (if we do then the boundaries are probably working!)
        std::vector<MeinekeCryptCell> result_cells = simulator.GetCells();
        std::vector<bool> ghost_cells = simulator.GetGhostNodes();
        unsigned number_of_cells = 0;
        unsigned number_of_nodes = result_cells.size();
        
        TS_ASSERT_EQUALS(result_cells.size(),ghost_cells.size());
        
        for (unsigned i=0 ; i<number_of_nodes ; i++)
        {
            if (!ghost_cells[i])
            {
                number_of_cells++;
            }
        }
        TS_ASSERT_EQUALS(number_of_cells, 113u);
        TS_ASSERT_EQUALS(number_of_nodes, 164u);

        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }
    
    /*
     * This test compares the visualizer output from the previous test with a known file.
     * 
     * Note - if the previous test is changed we need to update the file this test refers to. 
     */
    void TestVisualizerOutput() throw (Exception)
    {
        // work out where the previous test wrote its files
        OutputFileHandler handler("Crypt2DPeriodicTysonNovak",false);
        std::string results_dir = handler.GetTestOutputDirectory() + "results_from_time_0/vis_results";
        TS_ASSERT_EQUALS(system(("cmp " + results_dir + "/results.vizelements models/test/data/Crypt2DPeriodicTysonNovak_vis/results.vizelements").c_str()), 0);
        TS_ASSERT_EQUALS(system(("cmp " + results_dir + "/results.viznodes models/test/data/Crypt2DPeriodicTysonNovak_vis/results.viznodes").c_str()), 0);
        TS_ASSERT_EQUALS(system(("cmp " + results_dir + "/results.vizsetup models/test/data/Crypt2DPeriodicTysonNovak_vis/results.vizsetup").c_str()), 0);
    }
   
    
    
    void TestPrivateFunctionsOf2DCryptSimulation() throw (Exception)
    {
        CancerParameters *p_params = CancerParameters::Instance();
        RandomNumberGenerator::Instance();
        
        double crypt_length = 9.3;
        double crypt_width = 10.0;
        
        p_params->SetCryptLength(crypt_length);
        p_params->SetCryptWidth(crypt_width);        
        
        TrianglesMeshReader<2,2> mesh_reader("mesh/test/data/2D_0_to_100mm_200_elements");
        ConformingTetrahedralMesh<2,2> mesh;
        mesh.ConstructFromMeshReader(mesh_reader);
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells
        std::vector<MeinekeCryptCell> cells;
        CreateVectorOfCells(cells, mesh, FIXED, false, 0.0, 3.0, 6.5, 8.0);
        
        cells[60].SetBirthTime(-50.0);
        
        TissueSimulation<2> simulator(mesh,cells);
        
        simulator.SetFixedBoundaries();
        
        unsigned num_deaths = simulator.DoCellRemoval();
        unsigned num_births = simulator.DoCellBirth();
                                                                
        TS_ASSERT_EQUALS(num_births, 1u);
        TS_ASSERT_EQUALS(num_deaths,11u);
        
        p_params->SetCryptLength(10.1);
        TissueSimulation<2> simulator2(mesh,cells);
        
        simulator2.SetFixedBoundaries();
        
        num_deaths = simulator2.DoCellRemoval();
        TS_ASSERT_EQUALS(num_deaths,0u);
        
        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }
    
    
    void TestPrivateFunctionsOf2DCryptSimulationOnHoneycombMesh() throw (Exception)
    {
        /*
         ************************************************************************
         ************************************************************************ 
         *     Set up a simulation class to run the individual tests on.
         ************************************************************************
         ************************************************************************ 
         */
        unsigned cells_across2 = 6;
        unsigned cells_up2 = 5;
        double crypt_width2 = 6.0;
        unsigned thickness_of_ghost_layer2 = 3;
        
        CryptHoneycombMeshGenerator generator(cells_across2, cells_up2, crypt_width2,thickness_of_ghost_layer2,false);
        ConformingTetrahedralMesh<2,2>* p_mesh2=generator.GetMesh();
        std::set<unsigned> ghost_node_indices2 = generator.GetGhostNodeIndices();
        unsigned num_cells2 = p_mesh2->GetNumAllNodes();
        
        CancerParameters *p_params = CancerParameters::Instance();
        RandomNumberGenerator::Instance();
        
        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Set up cells by iterating through the mesh nodes
        std::vector<MeinekeCryptCell> cells2;
        for (unsigned i=0; i<num_cells2; i++)
        {
            double birth_time;
            CryptCellType cell_type;
            unsigned generation;
            double y = p_mesh2->GetNode(i)->GetPoint().rGetLocation()[1];
            if (y == 0.0)
            {
                cell_type = STEM;
                generation = 0;
                birth_time = -2.0; //hours - doesn't matter for stem cell;
            }
            else if (y < 3)
            {
                cell_type = TRANSIT;
                generation = 1;
                birth_time = -2.0; //hours
            }
            else if (y < 6.5)
            {
                cell_type = TRANSIT;
                generation = 2;
                birth_time = -2.0;  //hours
            }
            else if (y < 8)
            {
                cell_type = TRANSIT;
                generation = 3;
                birth_time = -2.0;  //hours
            }
            else
            {
                cell_type = DIFFERENTIATED;
                generation = 4;
                birth_time = -2.0;  //hours
            }
            
            CryptCellMutationState mutation_state;
            if(i!=60)
            {
                mutation_state = HEALTHY;
            }
            else
            {
                mutation_state = APC_TWO_HIT;
            }
            
            WntGradient wnt_gradient(LINEAR);
            double wnt = wnt_gradient.GetWntLevel(y);
            
            
            
            MeinekeCryptCell cell(cell_type, mutation_state, generation, new WntCellCycleModel(wnt,0));
            cell.SetNodeIndex(i);
            cell.SetBirthTime(birth_time);
            cells2.push_back(cell);
        }
        
        TissueSimulation<2> simulator3(*p_mesh2,cells2);
        simulator3.SetGhostNodes(ghost_node_indices2);
        
        simulator3.SetMaxCells(400);
        simulator3.SetMaxElements(400);

        /*
         ************************************************************************
         ************************************************************************ 
         *  Test Calculate Velocities on each node
         ************************************************************************
         ************************************************************************ 
         */
                
        std::vector<c_vector<double, 2> > velocities_on_each_node(p_mesh2->GetNumAllNodes());
        
        velocities_on_each_node = simulator3.CalculateVelocitiesOfEachNode();
 
        for (unsigned i=0; i<p_mesh2->GetNumAllNodes(); i++)
        {
            std::set<unsigned>::iterator iter = ghost_node_indices2.find(i);
            bool is_a_ghost_node = (iter!=ghost_node_indices2.end());

            if (!is_a_ghost_node)
            {
                TS_ASSERT_DELTA(velocities_on_each_node[i][0], 0.0, 1e-4);
                TS_ASSERT_DELTA(velocities_on_each_node[i][1], 0.0, 1e-4);
            }
        }
        
        // Move a node along the x-axis and calculate the force exerted on a neighbour
        c_vector<double,2> old_point = p_mesh2->GetNode(59)->rGetLocation();
        Point<2> new_point;
        new_point.rGetLocation()[0] = old_point[0]+0.5;
        new_point.rGetLocation()[1] = old_point[1];
        p_mesh2->SetNode(59, new_point, false);
        velocities_on_each_node = simulator3.CalculateVelocitiesOfEachNode();
        TS_ASSERT_DELTA(velocities_on_each_node[60][0], 0.5*p_params->GetSpringStiffness()/p_params->GetDampingConstantMutant(), 1e-4);
        TS_ASSERT_DELTA(velocities_on_each_node[60][1], 0.0, 1e-4);

        TS_ASSERT_DELTA(velocities_on_each_node[59][0], (-3+4.0/sqrt(7))*p_params->GetSpringStiffness()/p_params->GetDampingConstantNormal(), 1e-4);
        TS_ASSERT_DELTA(velocities_on_each_node[59][1], 0.0, 1e-4);

        TS_ASSERT_DELTA(velocities_on_each_node[58][0], 0.5*p_params->GetSpringStiffness()/p_params->GetDampingConstantNormal(), 1e-4);
        TS_ASSERT_DELTA(velocities_on_each_node[58][1], 0.0, 1e-4);
        /*
         ************************************************************************
         ************************************************************************ 
         *  Test Calculate force on a spring
         ************************************************************************
         ************************************************************************ 
         */
        
        c_vector<double,2> force_on_spring ; // between nodes 59 and 60
        
        // Find one of the elements that nodes 59 and 60 live on
        Point<2> new_point2;
        new_point2.rGetLocation()[0] = new_point[0] + 0.01;
        new_point2.rGetLocation()[1] = new_point[1] + 0.01 ;
        
        unsigned elem_index = p_mesh2->GetContainingElementIndex(new_point2,false);
        Element<2,2>* p_element = p_mesh2->GetElement(elem_index);
        
        force_on_spring = simulator3.CalculateForceInThisSpring(p_element,1,0);
        
        TS_ASSERT_DELTA(force_on_spring[0], 0.5*p_params->GetSpringStiffness(), 1e-4);
        TS_ASSERT_DELTA(force_on_spring[1], 0.0, 1e-4);
        
        
        /*
         ************************************************************************
         ************************************************************************ 
         *  Test UpdateNodePositions
         ************************************************************************
         ************************************************************************ 
         */
        
        Point<2> point_of_node60 = p_mesh2->GetNode(60)->rGetLocation();
        
        simulator3.SetDt(0.01);
        simulator3.UpdateNodePositions(velocities_on_each_node);
        
        TS_ASSERT_DELTA(p_mesh2->GetNode(60)->rGetLocation()[0],point_of_node60.rGetLocation()[0]+force_on_spring[0]/p_params->GetDampingConstantMutant() *0.01, 1e-4);
        TS_ASSERT_DELTA(p_mesh2->GetNode(60)->rGetLocation()[1],point_of_node60.rGetLocation()[1], 1e-4);
        
        /*
         ************************************************************************
         ************************************************************************ 
         * Test UpdateCellTypes 
         ************************************************************************
         ************************************************************************ 
         */
        
        std::vector<MeinekeCryptCell> cells3;
        simulator3.SetWntGradient(LINEAR);
        simulator3.UpdateCellTypes();
        cells3 = simulator3.GetCells();
        
        std::vector<bool> is_node_a_ghost = simulator3.GetGhostNodes();
        
        for (unsigned i=0; i<num_cells2; i++)
        {
            if (!is_node_a_ghost[i])
            {
                CryptCellType cell_type;
                cell_type = cells3[i].GetCellType();
                if (!cell_type==STEM)
                {
                    //std::cout << "Cell type = " << cell_type << std::endl;
                    WntCellCycleModel *p_this_model = static_cast<WntCellCycleModel*>(cells3[i].GetCellCycleModel());
                    double beta_cat_level = p_this_model->GetProteinConcentrations()[6]+ p_this_model->GetProteinConcentrations()[7];
                    //std::cout << "Cell " << i << ", beta-cat = " << beta_cat_level << std::endl;
                    if (beta_cat_level > 0.4127)
                    {
                        TS_ASSERT_EQUALS(cell_type,TRANSIT);
                    }
                    else
                    {
                        TS_ASSERT_EQUALS(cell_type,DIFFERENTIATED);
                    }
                }
            }
        }
        
        SimulationTime::Destroy();
        RandomNumberGenerator::Destroy();
    }
    
    void TestCalculateDividingCellCentreLocationsConfMesh() throw (Exception)
    {
        double separation = 0.1;

        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Make a parent node
        c_vector<double ,2> location;
        location[0]=1.0;
        location[1]=1.0;
        Node<2>* p_node = new Node<2>(0u,location, false);

        ConformingTetrahedralMesh<2,2> conf_mesh;
        conf_mesh.AddNode(p_node);
        
        // Set up cells
        std::vector<MeinekeCryptCell> conf_cells;
        CreateVectorOfCells(conf_cells, conf_mesh, TYSONNOVAK, true);        
        Crypt<2> conf_crypt(conf_mesh, conf_cells);

        Crypt<2>::Iterator conf_iter = conf_crypt.Begin();

        TissueSimulation<2> simulator(conf_mesh);                
        c_vector<double, 2> daughter_location = simulator.CalculateDividingCellCentreLocations(conf_iter);
        c_vector<double, 2> new_parent_location = conf_mesh.GetNode(0)->rGetLocation();
        c_vector<double, 2> parent_to_daughter = conf_mesh.GetVectorFromAtoB(new_parent_location, daughter_location);
        TS_ASSERT_DELTA(norm_2(parent_to_daughter), separation, 1e-7);
    }
    

    void TestCalculateDividingCellCentreLocationsConfMeshStemCell() throw (Exception)
    {
        double separation = 0.1;

        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Make a parent node
        c_vector<double ,2> location;
        location[0]=1.0;
        location[1]=0.0;                                    // <- y=0
        Node<2>* p_node = new Node<2>(0u,location, false);
        ConformingTetrahedralMesh<2,2> conf_mesh;
        conf_mesh.AddNode(p_node);
        
        // Set up cells
        std::vector<MeinekeCryptCell> conf_cells;
        CreateVectorOfCells(conf_cells, conf_mesh, TYSONNOVAK, true);        
        Crypt<2> conf_crypt(conf_mesh, conf_cells);

        Crypt<2>::Iterator conf_iter = conf_crypt.Begin();

        TissueSimulation<2> simulator(conf_mesh);                
        c_vector<double, 2> daughter_location = simulator.CalculateDividingCellCentreLocations(conf_iter);
        c_vector<double, 2> new_parent_location = conf_mesh.GetNode(0)->rGetLocation();
        c_vector<double, 2> parent_to_daughter = conf_mesh.GetVectorFromAtoB(new_parent_location, daughter_location);

        // The parent stem cell should stay where it is and the daughter be introduced at positive y.
        TS_ASSERT_DELTA(new_parent_location[0], location[0], 1e-7);
        TS_ASSERT_DELTA(new_parent_location[1], location[1], 1e-7);
        TS_ASSERT(daughter_location[1]>=location[1]);
        TS_ASSERT_DELTA(norm_2(parent_to_daughter), 0.5*separation, 1e-7);
    }

    void TestCalculateDividingCellCentreLocationsCylindricalMesh() throw (Exception)
    {
        double separation = 0.1;

        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Make a mesh
        c_vector<double ,2> location;
        location[0]=1.0;
        location[1]=1.0;
        Node<2>* p_node = new Node<2>(0u,location, false);
        Cylindrical2dMesh cyl_mesh(6.0);
        cyl_mesh.AddNode(p_node);
        
        // Set up cells
        std::vector<MeinekeCryptCell> cyl_cells;
        CreateVectorOfCells(cyl_cells, cyl_mesh, TYSONNOVAK, true);        
        Crypt<2> cyl_crypt(cyl_mesh, cyl_cells);

        Crypt<2>::Iterator cyl_iter = cyl_crypt.Begin();

        TissueSimulation<2> simulator(cyl_mesh);                
        c_vector<double, 2> daughter_location = simulator.CalculateDividingCellCentreLocations(cyl_iter);
        c_vector<double, 2> new_parent_location = cyl_mesh.GetNode(0)->rGetLocation();
        c_vector<double, 2> parent_to_daughter = cyl_mesh.GetVectorFromAtoB(new_parent_location, daughter_location);
        TS_ASSERT_DELTA(norm_2(parent_to_daughter), separation, 1e-7);
    }

    void TestCalculateDividingCellCentreLocationsCylindricalMeshStemCell() throw (Exception)
    {
        double separation = 0.1;

        SimulationTime* p_simulation_time = SimulationTime::Instance();
        p_simulation_time->SetStartTime(0.0);
        
        // Make a mesh
        c_vector<double ,2> location;
        location[0]=1.0;
        location[1]=0.0;                                    // <- y=0
        Node<2>* p_node = new Node<2>(0u,location, false);
        Cylindrical2dMesh cyl_mesh(6.0);
        cyl_mesh.AddNode(p_node);
        
        // Set up cells
        std::vector<MeinekeCryptCell> cyl_cells;
        CreateVectorOfCells(cyl_cells, cyl_mesh, TYSONNOVAK, true);        
        Crypt<2> cyl_crypt(cyl_mesh, cyl_cells);

        Crypt<2>::Iterator cyl_iter = cyl_crypt.Begin();

        TissueSimulation<2> simulator(cyl_mesh);                
        c_vector<double, 2> daughter_location = simulator.CalculateDividingCellCentreLocations(cyl_iter);
        c_vector<double, 2> new_parent_location = cyl_mesh.GetNode(0)->rGetLocation();
        c_vector<double, 2> parent_to_daughter = cyl_mesh.GetVectorFromAtoB(new_parent_location, daughter_location);
        
        // The parent stem cell should stay where it is and the daughter be introduced at positive y.
        TS_ASSERT_DELTA(new_parent_location[0], location[0], 1e-7);
        TS_ASSERT_DELTA(new_parent_location[1], location[1], 1e-7);
        TS_ASSERT(daughter_location[1]>=location[1]);
        TS_ASSERT_DELTA(norm_2(parent_to_daughter), 0.5*separation, 1e-7);
    }

   
};

#endif /*TESTCRYPTSIMULATION2DPERIODIC_HPP_*/
