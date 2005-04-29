#ifndef _SIMPLENONLINEARELLIPTICASSEMBLER_HPP_
#define _SIMPLENONLINEARELLIPTICASSEMBLER_HPP_

 /* What we need to do:
     * 
     * 1. declare members of class - mPDE, mMesh, mBC, residual and jacobian
     * 2. method - assembleandsolve(pSolver,PDE,Mesh,BC,basis function, quad)
     * 3. in the method - AssembleSystem set up pesky vectors and call solver->solve(PDE,jacobian,*this)
     * 					  AssembleSystem is a method of the abstract class
     * 4. other methods - compute residual, compute jacobiananalytically, compute jacobiannumapprox
     *
     *  
    */	

  
#include <vector>
#include "AbstractNonlinearEllipticPde.hpp"
#include "ConformingTetrahedralMesh.hpp"
#include "BoundaryConditionsContainer.hpp"
#include "AbstractNonlinearSolver.hpp"
#include "AbstractNonlinearEllipticAssembler.hpp"
#include "GaussianQuadratureRule.hpp"
#include "petscsnes.h"
#include "petscvec.h"
#include "petscmat.h"  
#include "NonlinearEllipticEquation.hpp"

/*
 * Since we need to pass function pointers to the PETSc SNES routines, we can't
 * make these functions below methods. This is a pain, since it also means we
 * need to pass round a pointer to our assembler object as the void *pContext,
 * and cast it within the function to access data members.
 */
template<int ELEMENT_DIM, int SPACE_DIM>
PetscErrorCode ComputeResidual(SNES snes, Vec CurrentSolution, Vec res_vector,
								void *pContext);
template<int ELEMENT_DIM, int SPACE_DIM>
PetscErrorCode ComputeJacobianAnalytically(SNES snes, Vec CurrentSolution,
								Mat *pGlobal_jacobian, Mat *pPreconditioner,
								MatStructure *pMatStructure, void *pContext);
template<int ELEMENT_DIM, int SPACE_DIM>
PetscErrorCode ComputeJacobianNumerically(SNES snes, Vec input, Mat *pJacobian, 
    								     	  Mat *pPreconditioner, MatStructure *pMatStructure, 
    										  void *pContext);

/**
 * Concrete simple class that assembles and solves the nonlinear system
 * for a nonlinear elliptic PDE.
 */ 
template<int ELEMENT_DIM, int SPACE_DIM>
class SimpleNonlinearEllipticAssembler: public AbstractNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM>
{
public:
	ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM> *mpMesh;
  	AbstractNonlinearEllipticPde<SPACE_DIM> *mpPde;
   	BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM> *mpBoundaryConditions;
   	AbstractNonlinearSolver *mpSolver;
   	AbstractBasisFunction<SPACE_DIM> *mpBasisFunction;
   	GaussianQuadratureRule<ELEMENT_DIM> *mpGaussianQuadratureRule;
   	

	//Constructor - does nothing
	SimpleNonlinearEllipticAssembler() {};
	//Destructor - does nothing
	~SimpleNonlinearEllipticAssembler() {};
		
    Vec AssembleSystem(ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM> *pMesh,
                       AbstractNonlinearEllipticPde<SPACE_DIM> *pPde,
                       BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM> *pBoundaryConditions,
                       AbstractNonlinearSolver *pSolver,
                       AbstractBasisFunction<SPACE_DIM> *pBasisFunction,
                       GaussianQuadratureRule<ELEMENT_DIM> *pGaussianQuadratureRule,
                       Vec initialGuess);

};

/**
 * Assemble and solve the system for a nonlinear elliptic PDE.
 * 
 * @param pMesh Pointer to the mesh to solve on
 * @param pPde Pointer to the object specifying the equation to solve
 * @param pBoundaryConditions Pointer to the container object for our boundary conditions
 * @param pSolver Pointer to the nonlinear solver object
 * @param pBasisFunction Pointer to object for computing basis functions
 * @param pGaussianQuadratureRule Pointer to database object for Gaussian quadrature
 * @param initialGuess An initial guess for the iterative solver
 * @return A PETSc vector giving the solution at each mesh node.
 */
template <int ELEMENT_DIM, int SPACE_DIM>
Vec SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM>::AssembleSystem(
						ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM> *pMesh,
						AbstractNonlinearEllipticPde<SPACE_DIM> *pPde, 
						BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM> *pBoundaryConditions,
						AbstractNonlinearSolver *pSolver,
						AbstractBasisFunction<SPACE_DIM> *pBasisFunction,
						GaussianQuadratureRule<ELEMENT_DIM> *pGaussianQuadratureRule,
						Vec initialGuess)
{
	// Store data structures as public members
	mpMesh = pMesh;
	mpPde = pPde;
	mpBoundaryConditions=pBoundaryConditions;
	mpBasisFunction=pBasisFunction;
	mpGaussianQuadratureRule=pGaussianQuadratureRule;
	
    Vec residual;
 	VecDuplicate(initialGuess, &residual);

//	return pSolver->Solve(&ComputeResidual<ELEMENT_DIM, SPACE_DIM>,
//			&ComputeJacobianAnalytically<ELEMENT_DIM, SPACE_DIM>, residual, initialGuess, this);
	return pSolver->Solve(&ComputeResidual<ELEMENT_DIM, SPACE_DIM>,		
		&ComputeJacobianNumerically<ELEMENT_DIM, SPACE_DIM>, residual, initialGuess, this);
}



/*
 * ComputeResidual, ComputeJacobianAnalytically and ComputeJacobianNumerically need
 * to be placed beneath, but separate from, this class.
 * 
 */
 
/**
/**
* Implementation of Nonlinear system
*===============================================================================
* ------------------------------------------------------------------------------
*/


/**
 * Compute Residual on Surface Elements
 * 
 */
 template<int ELEMENT_DIM, int SPACE_DIM>
 void ComputeResidualOnSurfaceElement(const Element<ELEMENT_DIM-1,SPACE_DIM> &rSurfaceElement,
								 VectorDouble &rBsubElem,
								 AbstractLinearEllipticPde<SPACE_DIM> *pPde,
								 AbstractBasisFunction<ELEMENT_DIM-1> &rBasisFunction,
								 BoundaryConditionsContainer<ELEMENT_DIM,SPACE_DIM> &rBoundaryConditions,
								 VectorDouble Ui)
{
	    static int NUM_GAUSS_POINTS_PER_DIMENSION=2;
		static GaussianQuadratureRule<ELEMENT_DIM-1> quad_rule(NUM_GAUSS_POINTS_PER_DIMENSION);
		double jacobian_determinant = rSurfaceElement.GetJacobianDeterminant();
		
		const int num_nodes = rSurfaceElement.GetNumNodes();

		for(int quad_index=0; quad_index<quad_rule.GetNumQuadPoints(); quad_index++)
		{
			Point<ELEMENT_DIM-1> quad_point=quad_rule.GetQuadPoint(quad_index);

			std::vector<double>  phi = rBasisFunction.ComputeBasisFunctions(quad_point);

            // location of the gauss point in the original element will be stored in x
			Point<SPACE_DIM> x(0,0,0);
			
			double U = 0;  
			
			for(int i=0; i<rSurfaceElement.GetNumNodes(); i++)
			{
				U+= phi[i]*Ui(i);
							
				for(int j=0; j<SPACE_DIM; j++)
				{
					x.SetCoordinate(j, x[j] + phi[i]*rSurfaceElement.GetNodeLocation(i,j));
				}
			}
				
				
			 /**
			  * \todo
			  *  In the nonlinear case of Practical 1: when solving d/dx u(du/dx) = -1
			  * f(U) = U whereas in general case need to calculate fofU
			  */
			double FOfU = U;
					
			// TODO: horrendously inefficient!!!
			double Dgradu_dot_n = rBoundaryConditions.GetNeumannBCValue(&rSurfaceElement, x);

			for (int row=0; row < num_nodes; row++)
			{
				double integrand_value =  FOfU * phi[row] * Dgradu_dot_n;
				rBsubElem(row) += integrand_value * jacobian_determinant * quad_rule.GetWeight(quad_index);
			}
		}		
}
 

/**
 * Compute Residual on Element
 * 
 */
template<int ELEMENT_DIM, int SPACE_DIM>
void ComputeResidualOnElement(const Element<ELEMENT_DIM,SPACE_DIM> &rElement,
							VectorDouble &rBElem,
							AbstractBasisFunction<ELEMENT_DIM> &rBasisFunction,
							VectorDouble Ui
                       		/*GaussianQuadratureRule<ELEMENT_DIM> *pGaussianQuadratureRule*/)
{
		static int NUM_GAUSS_POINTS_PER_DIMENSION=2;
		static GaussianQuadratureRule<ELEMENT_DIM> pGaussianQuadratureRule(NUM_GAUSS_POINTS_PER_DIMENSION);
		
		const MatrixDouble *inverseJacobian = rElement.GetInverseJacobian();
		double jacobian_determinant = rElement.GetJacobianDeterminant();
		
		// Initialise element contributions to zero
		const int num_nodes = rElement.GetNumNodes();
		
		for(int quad_index=0; quad_index<pGaussianQuadratureRule.GetNumQuadPoints(); quad_index++)
		{
			Point<ELEMENT_DIM> quad_point=pGaussianQuadratureRule.GetQuadPoint(quad_index);

			std::vector<double>       phi     = rBasisFunction.ComputeBasisFunctions(quad_point);
			std::vector<VectorDouble> gradPhi = rBasisFunction.ComputeTransformedBasisFunctionDerivatives
			                                    (quad_point, *inverseJacobian);

			Point<SPACE_DIM> x(0,0,0);
			double U = 0;
			VectorDouble gradU(SPACE_DIM);
			gradU.ResetToZero();
			
			for(int i=0; i<num_nodes; i++)
			{
				//Need to compute add U as double and gradU as vector double
				// get U =sum(Ui phi_i)
				U += phi[i]*Ui(i);
				
				for(int j=0; j<SPACE_DIM; j++)
				{
					x.SetCoordinate(j, x[j] + phi[i]*rElement.GetNodeLocation(i,j));
					
					gradU(j)+= gradPhi[i](j)*Ui(i);//might have to do as line above
					
				}
				
				//std::cout << "phi[" << i << "]=" << phi[i] << std::endl;
			}
			
			
			//std::cout << "u'" << ": gradU(" << 1 << ")=" << gradU(0) << std::endl;
			//std::cout << "U=" << U << std::endl;
					
			double integrand_value3=0;
			for (int i=0; i < num_nodes; i++)
			{
				// RHS  need to change for Practical 1
				double integrand_value1 = U*(gradU.dot(gradPhi[i]));
				//std::cout << "i_v1 at " << i << " is " << integrand_value1 << std::endl;																		
				double integrand_value2 = phi[i];
							
				integrand_value3 += gradU(0)* jacobian_determinant 
				               * pGaussianQuadratureRule.GetWeight(quad_index);
				               
               	
				// For solving NonlinearEllipticEquation 
				// which should be defined in/by NonlinearEllipticEquation.hpp:
				// d/dx [f(U,x) du/dx ] = -g
				// where g(x,U) is the forcing term
				// !! to be modified
				// MatrixDouble FOfU = pPde->ComputeDiffusionTerm(x,U); 
				// double  integrand_value1 = FOfU*(gradU.dot(gradPhi[i]));	
				// make RHS general: consists of linear and nonlinear source terms
				// double ForcingTerm = pPde->ComputeLinearSourceTerm(x);
				// ForcingTerm += pPde->ComputeNonlinearSourceTerm(x, U);
				//double integrand_value2 = ForcingTerm * phi[i];
				
				
				rBElem(i) += integrand_value1 * jacobian_determinant 
				               * pGaussianQuadratureRule.GetWeight(quad_index)
				               - integrand_value2 * jacobian_determinant 
				               * pGaussianQuadratureRule.GetWeight(quad_index);
			}
			//std::cout << "i_v3 is " << integrand_value3 << std::endl;
		}
}
//------------------------------------------------------------------------------
template<int ELEMENT_DIM, int SPACE_DIM>
PetscErrorCode ComputeResidual(SNES snes,Vec CurrentSolution,Vec res_vector,void *pContext)
{
	//std::cout << "In ComputeResidual()" << std::endl << std::flush;
	
	SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM> *pAssembler =
    ((SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM>*)pContext);
    
	//Set residual vector to zero
	PetscScalar zero = 0.0;
	VecSet(&zero, res_vector);

	//NEED A pde OBJECT!!!
	LinearHeatEquationPde<1> *pPde; 

    LinearBasisFunction<ELEMENT_DIM> basis_function;
    
    // Get an iterator over the elements of the mesh
    typename ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>::MeshIterator iter = pAssembler->mpMesh->GetFirstElement();
 
	// Assume all elements have the same number of nodes...
 	const int num_nodes = iter->GetNumNodes();
    VectorDouble b_elem(num_nodes);
 
    while (iter !=  pAssembler->mpMesh->GetLastElement())
    {
        const Element<ELEMENT_DIM, SPACE_DIM> &element = *iter;
                    
        b_elem.ResetToZero();            
                      
        //get relevant entries for the nodes from CurrentSolution and put into Ui
        VectorDouble Ui(num_nodes);
        double *answerElements;
        VecGetArray(CurrentSolution, &answerElements);
        for (int i=0; i<num_nodes; i++)
        {
        	int node = element.GetNodeGlobalIndex(i);

			double value = answerElements[node];
			
        	Ui(i) = value;
        }
        VecRestoreArray(CurrentSolution,&answerElements);
        
        //GaussianQuadratureRule(2);//(int numPointsInEachDimension)
        ComputeResidualOnElement(element, b_elem, /*pPde,*/ 
        							basis_function, Ui/*, pGaussianQuadratureRule*/);
        
        for (int i=0; i<num_nodes; i++)
        {
        	int node = element.GetNodeGlobalIndex(i);
        	
        	PetscScalar value = b_elem(i);
        	VecSetValue(res_vector,node,value,ADD_VALUES); /* update residual vector*/
        	
        }
        iter++;
    }
    
    /*
     * 
     * BOUNDARY CONDITIONS
     * 
     */
    // add the integrals associated with Neumann boundary conditions to the linear system
	LinearBasisFunction<ELEMENT_DIM-1> surf_basis_function;
	typename ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>::BoundaryElementIterator surf_iter = pAssembler->mpMesh->GetFirstBoundaryElement();
	
	if (surf_iter != pAssembler->mpMesh->GetLastBoundaryElement())
	{					
		const int num_surf_nodes = (*surf_iter)->GetNumNodes();
		VectorDouble b_surf_elem(num_surf_nodes);

		while (surf_iter != pAssembler->mpMesh->GetLastBoundaryElement())
		{
			const Element<ELEMENT_DIM-1,SPACE_DIM>& surf_element = **surf_iter;
			
			//get relevant entries for the nodes from CurrentSolution and put into Ui
			VectorDouble UiSurf(num_surf_nodes);
			PetscScalar *answerElements;
			for (int i=0; i<num_surf_nodes; i++)
            {
            	int node = surf_element.GetNodeGlobalIndex(i);
				VecGetArray(CurrentSolution, &answerElements);
				double value = answerElements[node];
				VecRestoreArray(CurrentSolution,&answerElements);
            	UiSurf(i) = value;
            }
        
			/**
			 * \todo
			 * Check surf_element is in the Neumann surface in an efficient manner.
			 */
			if (pAssembler->mpBoundaryConditions->HasNeumannBoundaryCondition(&surf_element))
			{
				b_surf_elem.ResetToZero();
				ComputeResidualOnSurfaceElement(surf_element, b_surf_elem, pPde, surf_basis_function, *(pAssembler->mpBoundaryConditions), UiSurf);

			    for (int i=0; i<num_surf_nodes; i++)
		            {
		            	int node1 = surf_element.GetNodeGlobalIndex(i);
		            	
		            	PetscScalar value1 = b_surf_elem(i);
		            	VecSetValue(res_vector,node1,value1,ADD_VALUES); 	
		            }

//					for (int i=0; i<num_surf_nodes; i++)
//		            {
//		            	int node1 = surf_element.GetNodeGlobalIndex(i);
//		            	mpAssembledLinearSystem->AddToRhsVectorElement(node1,b_surf_elem(i));
//		            }
			}
			surf_iter++;
		}
	}

	// Apply Dirichlet boundary conditions for nonlinear problem
    pAssembler->mpBoundaryConditions->ApplyDirichletToNonlinearProblem(CurrentSolution, res_vector);
    
    //std::cout << "Residual:" << std::endl;
    //VecView(res_vector, 0); std::cout << std::endl;
    //std::cout << "Current solution:" << std::endl;
    //VecView(CurrentSolution, 0);
    
    return 0;
}



/*
 * Compute Jacobian Analytically 
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * ======================================================================
 * 
 * ______________________________________________________________________
 * 
 * 
 *  */


template<int ELEMENT_DIM, int SPACE_DIM>

void ComputeJacobianOnElement(const Element<ELEMENT_DIM,SPACE_DIM> &rElement,
							MatrixDouble &rAElem,
							AbstractBasisFunction<ELEMENT_DIM> &rBasisFunction,
							VectorDouble Ui
                       		/*GaussianQuadratureRule<ELEMENT_DIM> *pGaussianQuadratureRule*/)
{
		static int NUM_GAUSS_POINTS_PER_DIMENSION=2;
		static GaussianQuadratureRule<ELEMENT_DIM> pGaussianQuadratureRule(NUM_GAUSS_POINTS_PER_DIMENSION);
		
		const MatrixDouble *inverseJacobian = rElement.GetInverseJacobian();
		double jacobian_determinant = rElement.GetJacobianDeterminant();
		
		// Initialise element contributions to zero
		const int num_nodes = rElement.GetNumNodes();
		
		for(int quad_index=0; quad_index<pGaussianQuadratureRule.GetNumQuadPoints(); quad_index++)
		{
			Point<ELEMENT_DIM> quad_point=pGaussianQuadratureRule.GetQuadPoint(quad_index);

			std::vector<double>       phi     = rBasisFunction.ComputeBasisFunctions(quad_point);
			std::vector<VectorDouble> gradPhi = rBasisFunction.ComputeTransformedBasisFunctionDerivatives
			                                    (quad_point, *inverseJacobian);

			Point<SPACE_DIM> x(0,0,0);
			double U = 0;
			VectorDouble gradU(SPACE_DIM);
			gradU.ResetToZero();
			
			for(int i=0; i<num_nodes; i++)
			{
				//Need to compute add U as double and gradU as vector double
				// get U =sum(Ui phi_i)
				U+= phi[i]*Ui(i);
				
				for(int j=0; j<SPACE_DIM; j++)
				{
					x.SetCoordinate(j, x[j] + phi[i]*rElement.GetNodeLocation(i,j));
					
					gradU(j)+= gradPhi[i](j)*Ui(i);//might have to do as line above
				}
				
				
			}
			
					
			for (int i=0; i < num_nodes; i++)
			{
				for (int j=0; j< num_nodes; j++)
				{
					// RHS  need to change for Practical 1
					double integrand_value1 = (gradPhi[j]*U+phi[j]*gradU).dot(gradPhi[i]);																		
					
					// For solving NonlinearEllipticEquation 
					// which should be defined in/by NonlinearEllipticEquation.hpp:
					// d/dx [f(U,x) du/dx ] = -g
					// where g(x,U) is the forcing term
					// !! to be modified
					// MatrixDouble FOfU = pPde->ComputeDiffusionTerm(x,U); 
					// double  integrand_value1 = FOfU*(gradU.dot(gradPhi[i]));	
					// make RHS general: consists of linear and nonlinear source terms
					// double ForcingTerm = pPde->ComputeLinearSourceTerm(x);
					// ForcingTerm += pPde->ComputeNonlinearSourceTerm(x, U);
					//double integrand_value2 = ForcingTerm * phi[i];
					
					
					rAElem(i,j) += integrand_value1 * jacobian_determinant 
					               * pGaussianQuadratureRule.GetWeight(quad_index);
				}
			}
		}
}

//------------------------------------------------------------------------------
template<int ELEMENT_DIM, int SPACE_DIM>
PetscErrorCode ComputeJacobianAnalytically(SNES snes, Vec CurrentSolution,
								Mat *pGlobal_jacobian, Mat *pPreconditioner,
								MatStructure *pMatStructure, void *pContext)
{
	//std::cout << "In ComputeJacobianAnalytically()" << std::endl << std::flush;

	// Extract an assembler from the void*
    SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM> *pAssembler =
    ((SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM>*)pContext);
    
    LinearBasisFunction<ELEMENT_DIM> basis_function;
    
    // Get an iterator over the elements of the mesh
    typename ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>::MeshIterator iter = pAssembler->mpMesh->GetFirstElement();

	// Assume all elements have the same number of nodes...
	const int num_nodes = iter->GetNumNodes();
	MatrixDouble a_elem(num_nodes,num_nodes);

	while (iter != pAssembler->mpMesh->GetLastElement())
	{
		//std::cout << "Beginning iteration" << std::endl << std::flush;
		const Element<ELEMENT_DIM, SPACE_DIM> &element = *iter;

		a_elem.ResetToZero();

		//get relevant entries for the nodes from CurrentSolution and put into Ui
		//std::cout << "Building Ui" << std::endl << std::flush;
        VectorDouble Ui(num_nodes);
        double *answerElements;
        VecGetArray(CurrentSolution, &answerElements);
        for (int i=0; i<num_nodes; i++)
        {
        	int node = element.GetNodeGlobalIndex(i);
        	//Ui(i) = CurrentSolution(node); /// >?!??! current solution is Petski vector!!!!
			
			double value = answerElements[node];
        	
        	Ui(i) = value;
        }
        VecRestoreArray(CurrentSolution,&answerElements);
        
        //GaussianQuadratureRule(2);//(int numPointsInEachDimension)
		//std::cout << "ComputeJacobianOnElement" << std::endl << std::flush;
        ComputeJacobianOnElement(element, a_elem, /*pPde,*/ 
        							basis_function, Ui/*, pGaussianQuadratureRule*/);
        
 		//std::cout << "Putting values in global jacobian" << std::endl << std::flush;
 		
 		// Possibly inefficient?
        std::vector<int> nodes_vec(num_nodes);
    	for (int i=0; i<num_nodes; i++)
    	{
    		nodes_vec[i] = element.GetNodeGlobalIndex(i);
    	}
        
        for (int i=0; i<num_nodes; i++)
        {
        	PetscInt nodes_i=nodes_vec[i];
        	for (int j=0; j<num_nodes; j++)
        	{
        		PetscInt nodes_j=nodes_vec[j];
        		PetscScalar value = a_elem(i,j);
				//std::cout << "(" << nodes_i << "," << nodes_j << ")=" << value << std::endl;
        		MatSetValue(*pGlobal_jacobian,nodes_i,nodes_j,value,ADD_VALUES); /* update global jacobian matrix*/
        	}
        }
        
        iter++;
    }

	MatAssemblyBegin(*pGlobal_jacobian,MAT_FINAL_ASSEMBLY);
	MatAssemblyEnd(*pGlobal_jacobian,MAT_FINAL_ASSEMBLY);
	
	//MatView(*pGlobal_jacobian, 0);
    
    /**
     * \todo Do we need to do anything with boundary conditions here?
     */
    // add the integrals associated with Neumann boundary conditions to the linear system
	/*LinearBasisFunction<ELEMENT_DIM-1> surf_basis_function;
	typename ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>::BoundaryElementIterator surf_iter = rMesh.GetFirstBoundaryElement();
	
	if (surf_iter != rMesh.GetLastBoundaryElement())
	{					
		const int num_surf_nodes = (*surf_iter)->GetNumNodes();
		VectorDouble b_surf_elem(num_surf_nodes);

		while (surf_iter != rMesh.GetLastBoundaryElement())
		{
			const Element<ELEMENT_DIM-1,SPACE_DIM>& surf_element = **surf_iter;
			
			*
			 * \todo
			 * Check surf_element is in the Neumann surface in an efficient manner.
			 
			if (rBoundaryConditions.HasNeumannBoundaryCondition(&surf_element))
			{
				b_surf_elem.ResetToZero();
				AssembleOnSurfaceElement(surf_element, b_surf_elem, pPde, surf_basis_function, rBoundaryConditions);

				for (int i=0; i<num_surf_nodes; i++)
	            {
	            	int node1 = surf_element.GetNodeGlobalIndex(i);
	            	mpAssembledLinearSystem->AddToRhsVectorElement(node1,b_surf_elem(i));
	            }
			}
			surf_iter++;
		}
	}*/

	// apply dirichlet boundary conditions 
    //rBoundaryConditions.ApplyDirichletToNonlinearProblem(res_vector);   
    return 0;
}

/**
 * Computes the Jacobian numerically i.e. an approximation, using partial derivatives.
 * 
 * @param snes A PETSc nonlinear solver object
 * @param input Indepedent variable, u in f(u), for example
 * @param *pJacobian A pointer to the Jacobian matrix
 * @param *pPreconditioner A pointer to a preconditioner matrix
 * @param *pMatStructure A pointer to the PETSc matrix type e.g. AIJ
 * @param *pContext A pointer to anything else that needs to be passed
 * 
 * @return PetscErrorCode Petsc Error Code
 */
template <int ELEMENT_DIM, int SPACE_DIM>
PetscErrorCode ComputeJacobianNumerically(SNES snes, Vec input, Mat *pJacobian, 
    								     	  Mat *pPreconditioner, MatStructure *pMatStructure, 
    										  void *pContext)
{
	int ierr;
    Vec residual;
    Vec perturbedResidual;
    Vec result;
    
    SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM> *integrator =
	    ((SimpleNonlinearEllipticAssembler<ELEMENT_DIM, SPACE_DIM>*)pContext);    
    
    int num_nodes = integrator->mpMesh->GetNumNodes();

    VecCreate(PETSC_COMM_WORLD, &residual);    
    VecCreate(PETSC_COMM_WORLD, &result);    
    VecCreate(PETSC_COMM_WORLD, &perturbedResidual);    
    
    VecSetSizes(residual,PETSC_DECIDE,num_nodes);
    VecSetSizes(result,PETSC_DECIDE,num_nodes);
    VecSetSizes(perturbedResidual,PETSC_DECIDE,num_nodes);
    
    //VecSetType(residual, VECSEQ);
    //VecSetType(result, VECSEQ);
    //VecSetType(perturbedResidual, VECSEQ);
    VecSetFromOptions(residual);
    VecSetFromOptions(result);
    VecSetFromOptions(perturbedResidual);
    
    Vec inputcopy;

    ierr = VecDuplicate(input,&inputcopy); CHKERRQ(ierr);
    ierr = VecCopy(input, inputcopy);CHKERRQ(ierr);
    
    // Hard coding residual and perturbedResidual to test since ComputeResidual() function
    // not complete!
    ComputeResidual<ELEMENT_DIM, SPACE_DIM>(snes,input,residual,pContext);
    //***************************************************
//    for (int row=0;row<num_nodes;row++)
//    {
//    	PetscScalar value = 1;
//    	VecSetValue(residual, row, value, INSERT_VALUES);
//    }
    //***************************************************
    
    double h = 0.00001;    
    PetscScalar subtract = -1;
    PetscScalar oneOverH = 1.0/h;
    
    
    for(int j = 0; j < num_nodes; j++)
    {
		PetscScalar *resultElements;
        ierr = VecSetValue(inputcopy,j,h, ADD_VALUES);CHKERRQ(ierr);
        
        ComputeResidual<ELEMENT_DIM, SPACE_DIM>(snes, inputcopy, perturbedResidual,pContext);
        //*************************************************************
//        for (int row=0;row<num_nodes;row++)
//	   	{
//    		int temp = 1;
//    		if (row==j) temp += 1;
//    		PetscScalar value2 = temp;
//    		VecSetValue(perturbedResidual, row, value2, INSERT_VALUES);
//    	}
        //*************************************************************
        
        
        ierr = VecWAXPY(&subtract,residual,perturbedResidual,result);CHKERRQ(ierr);
        ierr = VecScale(&oneOverH, result);CHKERRQ(ierr);
        
        ierr = VecGetArray(result,&resultElements);CHKERRQ(ierr);

        for (int i=0; i < num_nodes; i++)
        {
            ierr = MatSetValue(*pJacobian,i,j,resultElements[i],INSERT_VALUES);CHKERRQ(ierr);
        }
        ierr = VecRestoreArray(result,&resultElements); CHKERRQ(ierr);
        
        ierr = VecSetValue(inputcopy,j,-h, ADD_VALUES); CHKERRQ(ierr);
    }
    
    VecDestroy(residual);
    VecDestroy(perturbedResidual);
    VecDestroy(result);
    VecDestroy(inputcopy);
 
    MatAssemblyBegin(*pJacobian,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(*pJacobian,MAT_FINAL_ASSEMBLY);
    return 0;
}
 
 
#endif  // _SIMPLENONLINEARELLIPTICASSEMBLER_HPP_
