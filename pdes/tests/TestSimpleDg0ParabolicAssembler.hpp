#ifndef _TESTSIMPLEDG0PARABOLICASSEMBLER_HPP_
#define _TESTSIMPLEDG0PARABOLICASSEMBLER_HPP_

#include <cxxtest/TestSuite.h>
#include "petscvec.h"
#include "TimeDependentDiffusionEquationPde.hpp"
#include "SimpleLinearSolver.hpp"
#include "ConformingTetrahedralMesh.cpp"
#include <vector>
#include <iostream>
#include "Node.hpp"
#include "Element.hpp"
#include "BoundaryConditionsContainer.hpp"
#include "SimpleDg0ParabolicAssembler.hpp" 
#include "TrianglesMeshReader.hpp"
#include "TimeDependentDiffusionEquationWithSourceTermPde.hpp"

#include "math.h"

// temporary with non-zero source term for use by testing methods
//template <int SPACE_DIM>
//class TempPdeWithSourceToTestSimpleDg0 : public AbstractLinearParabolicPde<SPACE_DIM>
//{
//public:
//	double ComputeLinearSourceTerm(Point<SPACE_DIM> x)
//	{
//		return 0.0;
//	}
//    
//    double ComputeNonlinearSourceTerm(Point<SPACE_DIM> x, double u)
//    {
//    	return 1.0;
//    }
//
//    MatrixDouble ComputeDiffusionTerm(Point<SPACE_DIM> x)
//    {
//    	return MatrixDouble::Identity(SPACE_DIM);
//    }
//    
//	double ComputeDuDtCoefficientFunction(Point<SPACE_DIM> x)
//    {
//    	return 1;
//    }
//    
//};
	

class TestSimpleDg0ParabolicAssembler : public CxxTest::TestSuite 
{	
public:
	void setUp()
    {
		int FakeArgc=0;
		char *FakeArgv0="testrunner";
		char **FakeArgv=&FakeArgv0;
    	
		PetscInitialize(&FakeArgc, &FakeArgv, PETSC_NULL, 0);
    }	
    
    
    // test 1D problem
	void testSimpleDg0ParabolicAssembler1DZeroDirich( void )
	{		
        // Create mesh from mesh reader
		TrianglesMeshReader mesh_reader("pdes/tests/meshdata/1D_0_to_1_10_elements");
		ConformingTetrahedralMesh<1,1> mesh;
		mesh.ConstructFromMeshReader(mesh_reader);
		
		// Instantiate PDE object
		TimeDependentDiffusionEquationPde<1> pde;  		
	
		// Boundary conditions - zero dirichlet at first and last node;
	    BoundaryConditionsContainer<1,1> bcc;
        ConstBoundaryCondition<1>* pBoundaryCondition1 = new ConstBoundaryCondition<1>(0.0);
        bcc.AddDirichletBoundaryCondition(mesh.GetNodeAt(0), pBoundaryCondition1);

        ConstBoundaryCondition<1>* pBoundaryCondition2 = new ConstBoundaryCondition<1>(0.0);
        bcc.AddDirichletBoundaryCondition(mesh.GetNodeAt( mesh.GetNumNodes()-1 ), pBoundaryCondition2);
   
   		// Linear solver
		SimpleLinearSolver linearSolver;
	
		// Assembler
		SimpleDg0ParabolicAssembler<1,1> fullSolver;
		
		// initial condition, u(0,x) = sin(x*pi);
		Vec initialCondition;
		VecCreate(PETSC_COMM_WORLD, &initialCondition);
    	VecSetSizes(initialCondition, PETSC_DECIDE, mesh.GetNumNodes() );
	    VecSetType(initialCondition, VECSEQ);
  
  		double* initialConditionArray;
 		int ierr = VecGetArray(initialCondition, &initialConditionArray);
		
		const double PI = 3.1415926535;
		for(int i=0; i<mesh.GetNumNodes(); i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			initialConditionArray[i] = sin(x*PI);
		}
		VecRestoreArray(initialCondition, &initialConditionArray);
		
		double t_end = 0.1;	
		fullSolver.SetTimes(0, 0.1, 0.01);
		fullSolver.SetInitialCondition(initialCondition);
		Vec result = fullSolver.Solve(mesh, &pde, bcc, &linearSolver);
		
		// Check result 
		double *res;
	    ierr = VecGetArray(result, &res);

		// Solution should be u = e^{-t*pi*pi} sin(x*pi), t=1
		for (int i=0; i < mesh.GetNumNodes() ; i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			double u = exp(-0.1*PI*PI)*sin(x*PI); //std::cout << i << " " << res[i] << " " << u << "\n";
			TS_ASSERT_DELTA(res[i], u, 0.1);
		}
		VecRestoreArray(result, &res);	
	}	
	
	
	    // test 1D problem
	void testSimpleDg0ParabolicAssembler1DZeroDirichWithSourceTerm( void )
	{		
        // Create mesh from mesh reader
		TrianglesMeshReader mesh_reader("pdes/tests/meshdata/1D_0_to_1_10_elements");
		ConformingTetrahedralMesh<1,1> mesh;
		mesh.ConstructFromMeshReader(mesh_reader);
		
		// Instantiate PDE object
		TimeDependentDiffusionEquationWithSourceTermPde<1> pde;  		
	
		// Boundary conditions - zero dirichlet at first and last node;
	    BoundaryConditionsContainer<1,1> bcc;
        ConstBoundaryCondition<1>* pBoundaryCondition1 = new ConstBoundaryCondition<1>(0.0);
        bcc.AddDirichletBoundaryCondition(mesh.GetNodeAt(0), pBoundaryCondition1);

        ConstBoundaryCondition<1>* pBoundaryCondition2 = new ConstBoundaryCondition<1>(-0.5);
        bcc.AddDirichletBoundaryCondition(mesh.GetNodeAt( mesh.GetNumNodes()-1 ), pBoundaryCondition2);
   
   		// Linear solver
		SimpleLinearSolver linearSolver;
	
		// Assembler
		SimpleDg0ParabolicAssembler<1,1> fullSolver;
		
		// initial condition, u(0,x) = sin(x*pi)+0.5*x*x;
		Vec initialCondition;
		VecCreate(PETSC_COMM_WORLD, &initialCondition);
    	VecSetSizes(initialCondition, PETSC_DECIDE, mesh.GetNumNodes() );
	    VecSetType(initialCondition, VECSEQ);
  
  		double* initialConditionArray;
 		int ierr = VecGetArray(initialCondition, &initialConditionArray);
		
		const double PI = 3.1415926535;
		for(int i=0; i<mesh.GetNumNodes(); i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			initialConditionArray[i] = sin(x*PI)-0.5*x*x;
		}
		VecRestoreArray(initialCondition, &initialConditionArray);
		
		double t_end = 0.1;	
		fullSolver.SetTimes(0, 0.1, 0.01);
		fullSolver.SetInitialCondition(initialCondition);
		Vec result = fullSolver.Solve(mesh, &pde, bcc, &linearSolver);
		
		// Check result 
		double *res;
	    ierr = VecGetArray(result, &res);

		// Solution should be u = e^{-t*pi*pi} sin(x*pi) + 0.5*x^2, t=1
		for (int i=0; i < mesh.GetNumNodes() ; i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			double u = exp(-0.1*PI*PI)*sin(x*PI)-0.5*x*x; //std::cout << i << " " << res[i] << " " << u << "\n";
			TS_ASSERT_DELTA(res[i], u, 0.1);
		}
		VecRestoreArray(result, &res);	
	}	
	
	
	void testSimpleDg0ParabolicAssemblerNonzeroNeumannCondition()
    {
        // Create mesh from mesh reader
		TrianglesMeshReader mesh_reader("pdes/tests/meshdata/1D_0_to_1_10_elements");
		ConformingTetrahedralMesh<1,1> mesh;
		mesh.ConstructFromMeshReader(mesh_reader);
        
    	// Instantiate PDE object
		TimeDependentDiffusionEquationPde<1> pde;  
	    
        // Boundary conditions
        // u(0)=0 u'(1)=1 
        BoundaryConditionsContainer<1,1> bcc;
        ConstBoundaryCondition<1>* pBoundaryCondition = new ConstBoundaryCondition<1>(0);
        bcc.AddDirichletBoundaryCondition(mesh.GetNodeAt(0), pBoundaryCondition);  

        ConstBoundaryCondition<1>* pNeumannBoundaryCondition = new ConstBoundaryCondition<1>(1.0);
        ConformingTetrahedralMesh<1,1>::BoundaryElementIterator iter = mesh.GetLastBoundaryElement();
        iter--;
        bcc.AddNeumannBoundaryCondition(*iter, pNeumannBoundaryCondition);
        
    	// Linear solver
		SimpleLinearSolver linearSolver;
	
		// Assembler
		SimpleDg0ParabolicAssembler<1,1> fullSolver;
		
		// initial condition;   
		Vec initialCondition;
		VecCreate(PETSC_COMM_WORLD, &initialCondition);
    	VecSetSizes(initialCondition, PETSC_DECIDE, mesh.GetNumNodes() );
	    VecSetType(initialCondition, VECSEQ);
  
  		double* initialConditionArray;
 		int ierr = VecGetArray(initialCondition, &initialConditionArray);
		
		const double PI_over_2 = 3.1415926535/2.0;
		for(int i=0; i<mesh.GetNumNodes(); i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			initialConditionArray[i] = x + sin(PI_over_2 * x);
		}
		
		VecRestoreArray(initialCondition, &initialConditionArray);
		fullSolver.SetTimes(0, 0.5, 0.01);
		fullSolver.SetInitialCondition(initialCondition);
		Vec result = fullSolver.Solve(mesh, &pde, bcc, &linearSolver);
		
		// Check result 
		double *res;
	    ierr = VecGetArray(result, &res);

		for (int i=0; i < mesh.GetNumNodes() ; i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			double u = x + exp(-0.5*PI_over_2*PI_over_2)*sin(x*PI_over_2); 
			//std::cout << i << " " << x << " " << res[i] << " " << u << "\n";
			TS_ASSERT_DELTA(res[i], u, 0.01);
		} 
		VecRestoreArray(result, &res);	
    }
	
	
	void testSimpleDg0ParabolicAssembler2DZeroDirich( void )
	{	
		// read mesh on [0,1]x[0,1]
		TrianglesMeshReader mesh_reader("pdes/tests/meshdata/square_128_elements");
		ConformingTetrahedralMesh<2,2> mesh;
		mesh.ConstructFromMeshReader(mesh_reader);
		
		// Instantiate PDE object
		TimeDependentDiffusionEquationPde<2> pde;  		

		// Boundary conditions - zero dirichlet everywhere on boundary
        BoundaryConditionsContainer<2,2> bcc;
        bcc.DefineZeroDirichletOnMeshBoundary(&mesh);

   		// Linear solver
		SimpleLinearSolver linearSolver;
		
		// Assembler
		SimpleDg0ParabolicAssembler<2,2> fullSolver;
		
		// initial condition;
		Vec initialCondition;
		VecCreate(PETSC_COMM_WORLD, &initialCondition);
    	VecSetSizes(initialCondition, PETSC_DECIDE, mesh.GetNumNodes() );
	    VecSetType(initialCondition, VECSEQ);
  
  		double* initialConditionArray;
 		int ierr = VecGetArray(initialCondition, &initialConditionArray);
		
		// choose initial condition sin(x*pi)*sin(y*pi) as this is an eigenfunction of
		// the heat equation.
		const double PI = 3.1415926535;
		for(int i=0; i<mesh.GetNumNodes(); i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			double y = mesh.GetNodeAt(i)->GetPoint()[1];
			initialConditionArray[i] = sin(x*PI)*sin(y*PI);
		}

		VecRestoreArray(initialCondition, &initialConditionArray);
		
		double t_end = 0.1;
		fullSolver.SetTimes(0, t_end, 0.001);
		fullSolver.SetInitialCondition(initialCondition);
		Vec result = fullSolver.Solve(mesh, &pde, bcc, &linearSolver);
		
		// Check result 
		double *res;
	    ierr = VecGetArray(result, &res);

		// Solution should be u = e^{-2*t*pi*pi} sin(x*pi)*sin(y*pi), t=1
		for (int i=0; i < mesh.GetNumNodes(); i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			double y = mesh.GetNodeAt(i)->GetPoint()[1];
			double u = exp(-2*t_end*PI*PI)*sin(x*PI)*sin(y*PI);
			TS_ASSERT_DELTA(res[i], u, 0.01);
		}
		VecRestoreArray(result, &res);	
	}
	
	// test 2D problem
	void testSimpleDg0ParabolicAssembler2DZeroDirichWithSourceTerm( void )
	{		
        // Create mesh from mesh reader
		TrianglesMeshReader mesh_reader("pdes/tests/meshdata/square_128_elements");
		ConformingTetrahedralMesh<2,2> mesh;
		mesh.ConstructFromMeshReader(mesh_reader);
		
		// Instantiate PDE object
		TimeDependentDiffusionEquationWithSourceTermPde<2> pde;  		
	
		// Boundary conditions - zero dirichlet on boundary;
	    BoundaryConditionsContainer<2,2> bcc;
	    ConformingTetrahedralMesh<2,2>::BoundaryNodeIterator iter = mesh.GetFirstBoundaryNode();
        
	    while(iter < mesh.GetLastBoundaryNode())
		{
			double x = (*iter)->GetPoint()[0];
			double y = (*iter)->GetPoint()[1];
			ConstBoundaryCondition<2>* pDirichletBoundaryCondition = new ConstBoundaryCondition<2>(-0.25*(x*x+y*y));
			bcc.AddDirichletBoundaryCondition(*iter, pDirichletBoundaryCondition);
			iter++;
		}
	               
   		// Linear solver
		SimpleLinearSolver linearSolver;
	
		// Assembler
		SimpleDg0ParabolicAssembler<2,2> fullSolver;
		
		// initial condition, u(0,x) = sin(x*pi)*sin(y*pi)-0.25*(x^2+y^2);
		Vec initialCondition;
		VecCreate(PETSC_COMM_WORLD, &initialCondition);
    	VecSetSizes(initialCondition, PETSC_DECIDE, mesh.GetNumNodes() );
	    VecSetType(initialCondition, VECSEQ);
  
  		double* initialConditionArray;
 		int ierr = VecGetArray(initialCondition, &initialConditionArray);
		
		const double PI = 3.1415926535;
		for(int i=0; i<mesh.GetNumNodes(); i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			
			double y = mesh.GetNodeAt(i)->GetPoint()[1];
			initialConditionArray[i] = sin(x*PI)*sin(y*PI)-0.25*(x*x+y*y);
		}
		VecRestoreArray(initialCondition, &initialConditionArray);
		
		double t_end = 0.1;	
		fullSolver.SetTimes(0, 0.1, 0.01);
		fullSolver.SetInitialCondition(initialCondition);
		Vec result = fullSolver.Solve(mesh, &pde, bcc, &linearSolver);
		
		// Check result 
		double *res;
	    ierr = VecGetArray(result, &res);

		// Solution should be u = e^{-t*2*pi*pi} sin(x*pi) sin(y*pi) - 0.25(x^2+y^2), t=0.1
		for (int i=0; i < mesh.GetNumNodes() ; i++)
		{
			double x = mesh.GetNodeAt(i)->GetPoint()[0];
			double y = mesh.GetNodeAt(i)->GetPoint()[1];
			double u = exp(-0.1*2*PI*PI)*sin(x*PI)*sin(y*PI)-0.25*(x*x+y*y); //std::cout << i << " " << res[i] << " " << u << "\n";
			TS_ASSERT_DELTA(res[i], u, 0.1);
		}
		VecRestoreArray(result, &res);	
	}	
	
	// test 2D problem - this test currently fails!!
	// because the neumann B.C. for our pde is time-dependent and we cannot specify that!!
//	void testSimpleDg0ParabolicAssembler2DNeumannWithSourceTerm( void )
//	{		
//        // Create mesh from mesh reader
//		TrianglesMeshReader mesh_reader("pdes/tests/meshdata/annulus_256_elements");
//		ConformingTetrahedralMesh<2,2> mesh;
//		mesh.ConstructFromMeshReader(mesh_reader);
//		
//		// Instantiate PDE object
//		TimeDependentDiffusionEquationPde<2> pde;  		
//	
//		// Boundary conditions - zero dirichlet on boundary;
//	    BoundaryConditionsContainer<2,2> bcc;
//	    ConformingTetrahedralMesh<2,2>::BoundaryNodeIterator iter = mesh.GetFirstBoundaryNode();
//        
//        ConstBoundaryCondition<2>* pDirichletBoundaryCondition = new ConstBoundaryCondition<2>(0.0);
//		ConstBoundaryCondition<2>* pNeumannBoundaryCondition = new ConstBoundaryCondition<2>(1.0);
//        
//	    while(iter < mesh.GetLastBoundaryNode())
//		{
//			double x = (*iter)->GetPoint()[0];
//			double y = (*iter)->GetPoint()[1];
//			double r = sqrt(x*x+y*y);
//			
//			if (fabs(r - 1.0) < 0.01)
//			{
//				bcc.AddDirichletBoundaryCondition(*iter, pDirichletBoundaryCondition);
//			}
//			
//			iter++;
//		}
//	    
//	    ConformingTetrahedralMesh<2,2>::BoundaryElementIterator surf_iter = mesh.GetFirstBoundaryElement();
//        
//        while(surf_iter < mesh.GetLastBoundaryElement())
//		{
//			int node = (*surf_iter)->GetNodeGlobalIndex(0);
//			double x = mesh.GetNodeAt(node)->GetPoint()[0];
//			double y = mesh.GetNodeAt(node)->GetPoint()[1];
//			double r = sqrt(x*x+y*y);
//			
//			if (fabs(r - 3.0) < 0.01)
//			{
//				bcc.AddNeumannBoundaryCondition(*surf_iter, pNeumannBoundaryCondition);
//			}
//			
//			surf_iter++;
//		}
//	           
//   		// Linear solver
//		SimpleLinearSolver linearSolver;
//	
//		// Assembler
//		SimpleDg0ParabolicAssembler<2,2> fullSolver;
//		
//		// initial condition, u(0,r) = 3 * log (r);
//		Vec initialCondition;
//		VecCreate(PETSC_COMM_WORLD, &initialCondition);
//    	VecSetSizes(initialCondition, PETSC_DECIDE, mesh.GetNumNodes() );
//	    VecSetType(initialCondition, VECSEQ);
//  
//  		double* initialConditionArray;
// 		int ierr = VecGetArray(initialCondition, &initialConditionArray);
//		
//		const double PI = 3.1415926535;
//		for(int i=0; i<mesh.GetNumNodes(); i++)
//		{
//			double x = mesh.GetNodeAt(i)->GetPoint()[0];
//			
//			double y = mesh.GetNodeAt(i)->GetPoint()[1];
//			
//			double r = sqrt(x*x+y*y);
//			
//			initialConditionArray[i] = 3*log(r);
//		}
//		VecRestoreArray(initialCondition, &initialConditionArray);
//		
//		double t_end = 0.1;	
//		fullSolver.SetTimes(0, 0.1, 0.01);
//		fullSolver.SetInitialCondition(initialCondition);
//		Vec result = fullSolver.Solve(mesh, &pde, bcc, &linearSolver);
//		
//		// Check result 
//		double *res;
//	    ierr = VecGetArray(result, &res);
//
//		// Solution should be u = 3 e^{-t} log(r), t=0.1
//		for (int i=0; i < mesh.GetNumNodes() ; i++)
//		{
//			double x = mesh.GetNodeAt(i)->GetPoint()[0];
//			double y = mesh.GetNodeAt(i)->GetPoint()[1];
//			double r = sqrt(x*x+y*y);
//			double u = 3*exp(-0.1)*log(r); //std::cout << i << " " << res[i] << " " << u << "\n";
//			TS_ASSERT_DELTA(res[i], u, 0.1);
//		}
//		VecRestoreArray(result, &res);	
//	}
};

#endif //_TESTSIMPLEDG0PARABOLICASSEMBLER_HPP_
