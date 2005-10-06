#ifndef _MONODOMAINPROBLEM_HPP_
#define _MONODOMAINPROBLEM_HPP_

#include <iostream>

#include "MonodomainProblem.hpp"

#include "SimpleLinearSolver.hpp"
#include "ConformingTetrahedralMesh.cpp"
#include "Node.hpp"
#include "Element.hpp"
#include "BoundaryConditionsContainer.hpp"
#include "SimpleDg0ParabolicAssembler.hpp"  
#include "MonodomainDg0Assembler.hpp"
#include "TrianglesMeshReader.hpp"
#include "ColumnDataWriter.hpp"

#include "MonodomainPde.hpp"
#include "MockEulerIvpOdeSolver.hpp"

#include "MonodomainProblem.hpp"
#include "MonodomainProblemStimulus.hpp"

template<int SPACE_DIM>
class MonodomainProblem
{
private:
    double mEndTime;
    MonodomainProblemStimulus<SPACE_DIM> *mpStimulus;
    ConformingTetrahedralMesh<SPACE_DIM,SPACE_DIM> mMesh;
    std::string mMeshFilename, mOutputDirectory, mOutputFilenamePrefix;

public:
    Vec mCurrentVoltage; // Current solution
    int mLo, mHi;
    MonodomainPde<SPACE_DIM> *mMonodomainPde;
    
    /**
     * Constructor
     */
     
    MonodomainProblem(const std::string &rMeshFilename,
                      const double &rEndTime,
                      const std::string &rOutputDirectory,
                      const std::string &rOutputFilenamePrefix,
                      MonodomainProblemStimulus<SPACE_DIM> *rStimulus)
    : mMeshFilename(rMeshFilename),
      mEndTime(rEndTime),
      mOutputDirectory(rOutputDirectory),
      mOutputFilenamePrefix(rOutputFilenamePrefix),
      mpStimulus(rStimulus),
      mMonodomainPde(NULL)
    {
    }

    /**
     * Destructor
     */
     
    ~MonodomainProblem()
    { 
        if (mMonodomainPde != NULL)
        {
            delete mMonodomainPde;
        }
    }

    /**
     * Solve the problem
     */
    void Solve(void)
    {
        double start_time = 0.0;
    
        double big_time_step = 0.01; 
        double small_time_step = 0.01;
    
        // Read mMesh
        TrianglesMeshReader mesh_reader(mMeshFilename);
        mMesh.ConstructFromMeshReader(mesh_reader);
    
        // Instantiate PDE object
        MockEulerIvpOdeSolver ode_solver;
        mMonodomainPde = new MonodomainPde<SPACE_DIM>(mMesh.GetNumNodes(), &ode_solver, start_time, big_time_step, small_time_step);
    
        // Add initial stim       
        mpStimulus->Apply(mMonodomainPde);    
    
        // Boundary conditions, zero neumann everywhere
        BoundaryConditionsContainer<SPACE_DIM,SPACE_DIM> bcc(1, mMesh.GetNumNodes());
       
        // The 'typename' keyword is required otherwise the compiler complains
        // Not totally sure why!
        typename ConformingTetrahedralMesh<SPACE_DIM,SPACE_DIM>::BoundaryElementIterator iter = mMesh.GetFirstBoundaryElement();
        ConstBoundaryCondition<SPACE_DIM>* p_neumann_boundary_condition = new ConstBoundaryCondition<SPACE_DIM>(0.0);
        
        while(iter < mMesh.GetLastBoundaryElement())
        {
            bcc.AddNeumannBoundaryCondition(*iter, p_neumann_boundary_condition);
            iter++;
        }
        
        // Linear solver
        SimpleLinearSolver linear_solver;
    
        // Assembler
        MonodomainDg0Assembler<SPACE_DIM,SPACE_DIM> monodomain_assembler;
        
        // initial condition;   
        Vec initial_condition;
        VecCreate(PETSC_COMM_WORLD, &initial_condition);
        VecSetSizes(initial_condition, PETSC_DECIDE, mMesh.GetNumNodes() );
        VecSetFromOptions(initial_condition);
  
        double* initial_condition_array;
        VecGetArray(initial_condition, &initial_condition_array); 
        
        VecGetOwnershipRange(initial_condition, &mLo, &mHi);
        
        // Set a constant initial voltage throughout the mMesh
        for(int global_index=mLo; global_index<mHi; global_index++)
        {
            initial_condition_array[global_index-mLo] = -84.5;
        }
        VecRestoreArray(initial_condition, &initial_condition_array);      
        VecAssemblyBegin(initial_condition);
        VecAssemblyEnd(initial_condition);
    
        /*
         *  Write data to a file <mOutputFilenamePrefix>_xx.dat, 'xx' refers to 
         *  'xx'th time step using ColumnDataWriter 
         */         
        
        mkdir(mOutputDirectory.c_str(), 0777);
                 
        ColumnDataWriter *p_test_writer;
        p_test_writer = new ColumnDataWriter(mOutputDirectory,mOutputFilenamePrefix);
       
        int time_var_id = 0;
        int voltage_var_id = 0;
    
        p_test_writer->DefineFixedDimension("Node", "dimensionless", mMesh.GetNumNodes() );
        time_var_id = p_test_writer->DefineUnlimitedDimension("Time","msecs");
    
        voltage_var_id = p_test_writer->DefineVariable("V","mV");
        p_test_writer->EndDefineMode();
        
        double* p_current_voltage_array;
        
        double current_time = start_time;        

        int big_steps = 0;
        
        while( current_time < mEndTime )
        {
            monodomain_assembler.SetTimes(current_time, current_time+big_time_step, big_time_step);
            monodomain_assembler.SetInitialCondition( initial_condition );
            
            mCurrentVoltage = monodomain_assembler.Solve(mMesh, mMonodomainPde, bcc, &linear_solver);
            
            // Free old initial condition
            VecDestroy(initial_condition);
            // Initial condition for next loop is current solution
            initial_condition = mCurrentVoltage;
            
            // Writing data out to the file <mOutputFilenamePrefix>.dat
             
            VecGetArray(mCurrentVoltage, &p_current_voltage_array);

            p_test_writer->PutVariable(time_var_id, current_time); 
            for(int j=0; j<mMesh.GetNumNodes(); j++) 
            {
                p_test_writer->PutVariable(voltage_var_id, p_current_voltage_array[j], j);    
            }
  
            VecRestoreArray(mCurrentVoltage, &p_current_voltage_array); 
             p_test_writer->AdvanceAlongUnlimitedDimension();
     
            mMonodomainPde->ResetAsUnsolvedOdeSystem();
            current_time += big_time_step;
                
            big_steps++;
        }

        TS_ASSERT_EQUALS(ode_solver.GetCallCount(), (mHi-mLo)*big_steps);

            // close the file that stores voltage values
        p_test_writer->Close();
        delete p_test_writer;
    }
};
#endif //_MONODOMAINPROBLEM_HPP_
