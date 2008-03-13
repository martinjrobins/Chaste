#ifndef _TESTMONODOMAINHEART_HPP_
#define _TESTMONODOMAINHEART_HPP_

#include <cxxtest/TestSuite.h>
#include "MonodomainProblem.hpp"
#include <petscvec.h>
#include <vector>

#include "PetscSetupAndFinalize.hpp"
#include "AbstractCardiacCellFactory.hpp"
#include "LuoRudyIModel1991OdeSystem.hpp"
#include "Hdf5DataReader.hpp"


class PointStimulusHeartCellFactory : public AbstractCardiacCellFactory<3>
{
private:
    InitialStimulus *mpStimulus;
public:
    PointStimulusHeartCellFactory(double timeStep) : AbstractCardiacCellFactory<3>(timeStep)
    {
        mpStimulus = new InitialStimulus(-1000*1000, 0.5);
    }
    
    AbstractCardiacCell* CreateCardiacCellForNode(unsigned node)
    {
        return new LuoRudyIModel1991OdeSystem(mpSolver, mTimeStep, mpZeroStimulus);
    }
    
    void FinaliseCellCreation(std::vector<AbstractCardiacCell* >* pCellsDistributed, unsigned lo, unsigned hi)
    {
        /* Here's the list of stimulated cells from the mesh file with tetgen numbering:
37483   1.95075 0.02458 0.007709
37498   1.974   0.0669055       0.0212167
37776   1.92132 -0.0185282      0.0264612
37778   1.90362 -0.0457586      0.0653502
38007   1.93232 -0.0006106      0.0718023
38331   1.9199  -0.0110326      -0.0303119
38586   1.90586 -0.0489975      -0.0275832
38587   1.90054 -0.0704444      0.0187846
39311   1.95105 0.0306952       -0.0127931
39313   1.97209 0.072277        -0.0302588
39642   1.93571 0.0272909       -0.0672191
40587   1.95069 0.0286633       -0.0049338
40589   1.97168 0.0738751       -0.0122153
63884   1.93084 0       0
        */        
        int stimulated_cells[] = {
                                     37484-1,
                                     37499-1,
                                     37777-1,
                                     37779-1,
                                     38008-1,
                                     38332-1,
                                     38587-1,
                                     38588-1,
                                     39312-1,
                                     39314-1,
                                     39643-1,
                                     40588-1,
                                     40590-1,
                                     63885-1
                                 };
                                    
                                 
        for (unsigned i=0; i<14; i++)
        {
            int global_index = stimulated_cells[i];
            if ((global_index>=(int)lo) && (global_index<(int)hi))
            {
                int local_index = global_index - lo;
                (*pCellsDistributed)[ local_index ]->SetStimulusFunction(mpStimulus);
            }
        }
    }
    
    ~PointStimulusHeartCellFactory(void)
    {
        delete mpStimulus;
    }
};

class TestMonodomainHeart : public CxxTest::TestSuite
{

public:
    void TestMonodomainDg0Heart() throw(Exception)
    {
        ///////////////////////////////////////////////////////////////////////
        // Solve
        ///////////////////////////////////////////////////////////////////////
        double pde_time_step = 0.01;  // ms
        double ode_time_step = pde_time_step/4.0; // ms
        double end_time = 100;        // ms
        
        double printing_time_step = end_time/100;
        
        PointStimulusHeartCellFactory cell_factory(ode_time_step);
        MonodomainProblem<3> monodomain_problem(&cell_factory);
        
        monodomain_problem.SetMeshFilename("heart/test/data/heart"); // note that this is the full heart mesh (not fifthheart)
        monodomain_problem.SetOutputDirectory("MonoDg0Heart");
        monodomain_problem.SetOutputFilenamePrefix("MonodomainLR91_Heart");
        
        monodomain_problem.SetEndTime(end_time);
        monodomain_problem.SetPdeAndPrintingTimeSteps(pde_time_step, printing_time_step);
        
        monodomain_problem.SetWriteInfo();

        monodomain_problem.SetIntracellularConductivities(Create_c_vector(1.75, 1.75, 1.75));
        
        monodomain_problem.Initialise();
        monodomain_problem.Solve();
        
        ///////////////////////////////////////////////////////////////////////
        // now reread the data and check verify that one of the stimulated
        // nodes was actually stimulated, and that the propagation spread to
        // a nearby node
        ///////////////////////////////////////////////////////////////////////
        Hdf5DataReader data_reader("MonoDg0Heart","MonodomainLR91_Heart");
        
        // get the voltage values at stimulated node
        std::vector<double> voltage_values_at_node_37483 = data_reader.GetVariableOverTime("V", 37484-1);
        // get the voltage values at a nearby unstimulated node
        std::vector<double> voltage_values_at_node_500 = data_reader.GetVariableOverTime("V", 501-1);
        bool stimulated_node_was_excited = false;
        bool unstimulated_node_was_excited = false;
        
        for (unsigned i=0; i<voltage_values_at_node_37483.size(); i++)
        {
            if (voltage_values_at_node_37483[i] > 0)
            {
                stimulated_node_was_excited = true;
            }
            if (voltage_values_at_node_500[i] > 0)
            {
                unstimulated_node_was_excited = true;
            }
        }
        TS_ASSERT(stimulated_node_was_excited);
        TS_ASSERT(unstimulated_node_was_excited);
    }
};

#endif //_TESTMONODOMAINHEART_HPP_
