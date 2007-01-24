#ifndef TEST3DBIDOMAINFOREFFICIENCY_HPP_
#define TEST3DBIDOMAINFOREFFICIENCY_HPP_




#include <cxxtest/TestSuite.h>
#include "MonodomainProblem.hpp"
#include "BidomainProblem.hpp"
#include <petscvec.h>
#include <vector>
#include "PetscSetupAndFinalize.hpp"
#include "AbstractCardiacCellFactory.hpp"
#include "LuoRudyIModel1991OdeSystem.hpp"
#include "RegularStimulus.hpp"
#include "RandomNumberGenerator.hpp"

class BidomainFaceStimulusCellFactory : public AbstractCardiacCellFactory<3>
{
private:
    InitialStimulus *mpStimulus;
    RegularStimulus *mpRegStimulus;
    
public:
    //Pdetime step is (by default) 0.01
    //Odetime step set below to 0.001 (10:1)
    BidomainFaceStimulusCellFactory() : AbstractCardiacCellFactory<3>(0.001)
    {
        mpStimulus = new InitialStimulus(-900.0*1000, 0.5);
        mpRegStimulus = new RegularStimulus(-900.0*1000, 0.5, 1.0/100.0, 0.0);//Same as above, but every 100ms
    }
    
    AbstractCardiacCell* CreateCardiacCellForNode(unsigned node)
    {
        if (mpMesh->GetNode(node)->GetPoint()[0] == 0.0)
        {
            //std::cout << node+1 << "\n";
            return new LuoRudyIModel1991OdeSystem(mpSolver, mTimeStep, mpRegStimulus, mpZeroStimulus);
        }
        else
        {
            return new LuoRudyIModel1991OdeSystem(mpSolver, mTimeStep, mpZeroStimulus, mpZeroStimulus);
        }
    }
    
    ~BidomainFaceStimulusCellFactory(void)
    {
        delete mpStimulus;
        delete mpRegStimulus;
    }
};

class Test3dBidomainProblemForEfficiency :  public CxxTest::TestSuite
{
public:

    void TestBidomain3d() throw (Exception)
    {
        BidomainFaceStimulusCellFactory bidomain_cell_factory;
        
        BidomainProblem<3> bidomain_problem( &bidomain_cell_factory );
        
        bidomain_problem.SetMeshFilename("mesh/test/data/3D_0_to_.5mm_1889_elements_irregular");
        bidomain_problem.SetEndTime(150);   // ms
        bidomain_problem.SetOutputDirectory("");
        bidomain_problem.SetOutputFilenamePrefix("");
        bidomain_problem.PrintOutput(false);
        bidomain_problem.SetLinearSolverRelativeTolerance(1e-6);
        
        PetscOptionsSetValue("-ksp_type", "symmlq");
        PetscOptionsSetValue("-pc_type", "bjacobi");
        PetscOptionsSetValue("-options_table", "");
        PetscOptionsSetValue("-log_summary", "");
        
        
        bidomain_problem.Initialise();
        
        bidomain_problem.Solve();
        
        Vec voltage=bidomain_problem.GetVoltage();
        ReplicatableVector voltage_replicated;
        voltage_replicated.ReplicatePetscVector(voltage);
        
        /*
         * Test the top right node against the right one in the 1D case, 
         * comparing voltage, and then test all the nodes on the right hand 
         * face of the cube against the top right one, comparing voltage.
         */
        bool need_initialisation = true;
        double probe_voltage=-9999.;
        
        need_initialisation = true;
        
        // Test the RHF of the mesh
        for (int i = 0; i < bidomain_problem.rGetMesh().GetNumNodes(); i++)
        {
            if (bidomain_problem.rGetMesh().GetNode(i)->GetPoint()[0] == 0.05)
            {
                // x = 0 is where the stimulus has been applied
                // x = 0.05cm is the other end of the mesh and where we want to
                //       to test the value of the nodes
                
                if (need_initialisation)
                {
                    probe_voltage = voltage_replicated[2*i];
                    need_initialisation = false;
                }
                else
                {
                    // the voltage at the end face varies a little because
                    // of drift due to the orientation of the tets in the mesh,
                    // hence the tolerance of 0.02
                    TS_ASSERT_DELTA(voltage_replicated[2*i], probe_voltage, 0.02);
                }
                
                // Check against hard coded value
                // For 50 ms test TS_ASSERT_DELTA(voltage_replicated[2*i],  7.3, 0.2);
                // For 150 ms test 
                TS_ASSERT_DELTA(voltage_replicated[2*i],  -1.735, 0.001);
                
            }
        }
        
    }
    
    
    
    
    
};


#endif /*TEST3DBIDOMAINFOREFFICIENCY_HPP_*/
