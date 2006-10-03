#ifndef _ABSTRACTASSEMBLER_HPP_
#define _ABSTRACTASSEMBLER_HPP_

#include "AbstractBasisFunction.hpp"
#include "LinearBasisFunction.cpp"
#include "GaussianQuadratureRule.hpp"
#include "ConformingTetrahedralMesh.hpp"
#include "BoundaryConditionsContainer.hpp"
#include "LinearSystem.hpp"
#include "GaussianQuadratureRule.hpp"
#include "AbstractBasisFunction.hpp"
#include "ReplicatableVector.hpp"



/**
 *  AbstractAssembler
 *
 *  Base class from which all solvers for linear and nonlinear PDEs inherit.
 *  Templated over the PROBLEM_DIM so also handles problems with more than one 
 *  unknown variable (ie those of the form u_xx + v = 0, v_xx + 2u = 1, where
 *  PROBLEM_DIM is equal to 2)
 *
 *  It defines a common interface and default code for AssembleSystem,
 *  AssembleOnElement and AssembleOnSurfaceElement. Each of these work
 *  for any PROBLEM_DIM>=1. Each of these methods work in both the 
 *  dynamic case (when there is a current solution available) and the static 
 *  case. The same code is used for the nonlinear and linear cases
 *
 *  user calls:
 *
 *  Solve() (in the linear case implemented in 
 *  AbsLin[Dynamic/Static]ProblemAssembler). In the linear case Solve() calls 
 *  AssembleSystem() directly, in the nonlinear case Solve() calls the PETSc nonlinear
 *  solver which then calls AssembleResidual or AssembleJacobian, both of which
 *  call AssembleSystem():
 *
 *  AssembleSystem() (implemented here, loops over elements and adds to the
 *  linear system) AssembleSystem() calls:
 *
 *  AssembleOnElement() and AssembleOnSurfaceElement() (implemented here. These
 *  loop over gauss points and create the element stiffness matrix and vector in 
 *  the linear case ). They call:
 *
 *  ComputeLhsTerm(), ComputeRhsTerm(), ComputeSurfaceRhsTerm() (implemented in
 *  the concrete assembler class (eg SimpleDg0ParabolicAssembler), which tells
 *  this assembler exactly what function of bases, position, pde constants etc
 *  to add to the element stiffness matrix/vector).
 * 
 */
template <int ELEMENT_DIM, int SPACE_DIM, int PROBLEM_DIM>
class AbstractAssembler
{
protected:
    bool mWeAllocatedBasisFunctionMemory;
    
    /*< Mesh to be solved on */
    ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>* mpMesh;
    
    /*< Boundary conditions to be applied */
    BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>* mpBoundaryConditions;
    
    /*< Basis function for use with normal elements */
    AbstractBasisFunction<ELEMENT_DIM> *mpBasisFunction;
    /*< Basis function for use with boundary elements */
    AbstractBasisFunction<ELEMENT_DIM-1> *mpSurfaceBasisFunction;
    
    /*< Quadrature rule for use on normal elements */
    GaussianQuadratureRule<ELEMENT_DIM> *mpQuadRule;
    /*< Quadrature rule for use on boundary elements */
    GaussianQuadratureRule<ELEMENT_DIM-1> *mpSurfaceQuadRule;

    /*< The current solution as a replicated vector. NULL for a static problem */
    ReplicatableVector mCurrentSolutionReplicated;
    
////////////// stuff for linear problems
    bool mProblemIsLinear; 

    /** 
     *  The linear system that is assembled in linear pde problems. Not used in
     *  nonlinear problems
     */
    LinearSystem *mpLinearSystem;
    
    /**
     * mMatrixIsConstant is a flag to say whether the matrix of the system
     * needs to be assembled at each time step.
     */
    bool mMatrixIsConstant;
    
    /**
     * mMatrixIsAssembled is a flag to say whether the matrix has been assembled 
     * for the current time step.
     */
    bool mMatrixIsAssembled;
    


    /**
     *  This method returns the matrix to be added to element stiffness matrix
     *  for a given gauss point. The arguments are the bases, bases gradients, 
     *  x and current solution computed at the Gauss point. The returned matrix
     *  will be multiplied by the gauss weight and jacobian determinent and 
     *  added to the element stiffness matrix (see AssembleOnElement()).
     * 
     *    --This method has to be implemented in the concrete class--
     */
    virtual c_matrix<double,PROBLEM_DIM*(ELEMENT_DIM+1),PROBLEM_DIM*(ELEMENT_DIM+1)> ComputeLhsTerm(
        const c_vector<double, ELEMENT_DIM+1> &rPhi,
        const c_matrix<double, ELEMENT_DIM, ELEMENT_DIM+1> &rGradPhi,
        const Point<SPACE_DIM> &rX,
        const c_vector<double,PROBLEM_DIM> &u,
        const c_vector<double, SPACE_DIM> &rGradU)=0;

        
    /**
     *  This method returns the vector to be added to element stiffness vector
     *  for a given gauss point. The arguments are the bases, 
     *  x and current solution computed at the Gauss point. The returned vector
     *  will be multiplied by the gauss weight and jacobian determinent and 
     *  added to the element stiffness matrix (see AssembleOnElement()).
     * 
     *     --This method has to be implemented in the concrete class--
     */        
    virtual c_vector<double,PROBLEM_DIM*(ELEMENT_DIM+1)> ComputeRhsTerm(
        const c_vector<double, ELEMENT_DIM+1> &rPhi,
        const c_matrix<double, ELEMENT_DIM, ELEMENT_DIM+1> &rGradPhi,
        const Point<SPACE_DIM> &rX,
        const c_vector<double,PROBLEM_DIM> &u,
        const c_vector<double, SPACE_DIM> &rGradU)=0;         // this should be a matrix really PROBLEM_DIM by SPACE_DI, rGradU(i,j) = d u_i/ d x_j

        
        
    /**
     *  This method returns the vector to be added to element stiffness vector
     *  for a given gauss point in BoundaryElement. The arguments are the bases, 
     *  x and current solution computed at the Gauss point. The returned vector
     *  will be multiplied by the gauss weight and jacobian determinent and 
     *  added to the element stiffness matrix (see AssembleOnElement()).
     * 
     *     --This method has to be implemented in the concrete class--
     */
    virtual c_vector<double, PROBLEM_DIM*ELEMENT_DIM> ComputeSurfaceRhsTerm(
        const BoundaryElement<ELEMENT_DIM-1,SPACE_DIM> &rSurfaceElement,
        const c_vector<double, ELEMENT_DIM> &rPhi,
        const Point<SPACE_DIM> &rX )=0; 
        
        
    /**
     *  Calculate the contribution of a single element to the linear system.
     * 
     *  @param rElement The element to assemble on.
     *  @param rAElem The element's contribution to the LHS matrix is returned in this
     *     n by n matrix, where n is the no. of nodes in this element. There is no
     *     need to zero this matrix before calling.
     *  @param rBElem The element's contribution to the RHS vector is returned in this
     *     vector of length n, the no. of nodes in this element. There is no
     *     need to zero this vector before calling.
     *  @param currentSolution For the parabolic linear case, the solution at the current 
     *     timestep. NULL for the static linear case. In the nonlinear case, the current
     *     guess.
     *  @param assembleVector a bool stating whether to assemble the load vector (in the 
     *     linear case) or the residual vector (in the nonlinear case)
     *  @param assembleMatrix a bool stating whether to assemble the stiffness matrix (in 
     *     the linear case) or the Jacobian matrix (in the nonlinear case)
     * 
     *  Called by AssembleSystem()
     *  Calls ComputeLhsTerm() etc
     */
    virtual void AssembleOnElement( Element<ELEMENT_DIM,SPACE_DIM> &rElement,
                                    c_matrix<double, PROBLEM_DIM*(ELEMENT_DIM+1), PROBLEM_DIM*(ELEMENT_DIM+1) > &rAElem,
                                    c_vector<double, PROBLEM_DIM*(ELEMENT_DIM+1)> &rBElem,
                                    Vec currentSolution,
                                    bool assembleVector,
                                    bool assembleMatrix)
    {
        GaussianQuadratureRule<ELEMENT_DIM> &quad_rule =
            *(AbstractAssembler<ELEMENT_DIM,SPACE_DIM,PROBLEM_DIM>::mpQuadRule);
        AbstractBasisFunction<ELEMENT_DIM> &rBasisFunction =
            *(AbstractAssembler<ELEMENT_DIM,SPACE_DIM,PROBLEM_DIM>::mpBasisFunction);
            
            
        /**
         * \todo This assumes that the Jacobian is constant on an element.
         * This is true for linear basis functions, but not for any other type of
         * basis function.
         */
        const c_matrix<double, SPACE_DIM, SPACE_DIM> *p_inverse_jacobian = NULL;
        double jacobian_determinant = rElement.GetJacobianDeterminant();
        
        // Initialise element contributions to zero
//        if (!this->mMatrixIsAssembled)
        {
            p_inverse_jacobian = rElement.GetInverseJacobian();
            rAElem.clear();
        }
        
        rBElem.clear();
        
        
        const int num_nodes = rElement.GetNumNodes();
        
        // loop over Gauss points
        for (int quad_index=0; quad_index < quad_rule.GetNumQuadPoints(); quad_index++)
        {
            Point<ELEMENT_DIM> quad_point = quad_rule.GetQuadPoint(quad_index);
            
            c_vector<double, ELEMENT_DIM+1> phi = rBasisFunction.ComputeBasisFunctions(quad_point);
            c_matrix<double, ELEMENT_DIM, ELEMENT_DIM+1> grad_phi;

// loss of efficiency            
//            if (!this->mMatrixIsAssembled)  
            {
                grad_phi = rBasisFunction.ComputeTransformedBasisFunctionDerivatives
                           (quad_point, *p_inverse_jacobian);
            }
            
            // Location of the gauss point in the original element will be stored in x
            // Where applicable, u will be set to the value of the current solution at x
            Point<SPACE_DIM> x(0,0,0);
            
            c_vector<double,PROBLEM_DIM> u = zero_vector<double>(PROBLEM_DIM);
            c_vector<double,SPACE_DIM>   grad_u = zero_vector<double>(SPACE_DIM);
            
            // allow the concrete version of the assembler to interpolate any
            // desired quantities
            ResetInterpolatedQuantities();
            
            
            /////////////////////////////////////////////////////////////
            // interpolation
            /////////////////////////////////////////////////////////////
            for (int i=0; i<num_nodes; i++)
            {
                const Node<SPACE_DIM> *p_node = rElement.GetNode(i);
                const Point<SPACE_DIM> node_loc = p_node->rGetPoint();
                
                // interpolate x
                for (int j=0; j<SPACE_DIM; j++)
                {
                    x.rGetLocation()[j] += phi(i)*node_loc[j];
                }
                
                // interpolate u
                int node_global_index = rElement.GetNodeGlobalIndex(i);
                if (currentSolution)
                {
                    for (unsigned index_of_unknown=0; index_of_unknown<PROBLEM_DIM; index_of_unknown++)
                    {
                        // If we have a current solution (e.g. this is a dynamic problem)
                        // get the value in a usable form.
                        
                        // NOTE - currentSolution input is actually now redundant at this point -
                        
                        // NOTE - following assumes that, if say there are two unknowns u and v, they
                        // are stored in the curren solution vector as
                        // [U1 V1 U2 V2 ... U_n V_n]
                        u(index_of_unknown)  += phi(i)*this->mCurrentSolutionReplicated[ PROBLEM_DIM*node_global_index + index_of_unknown];
                    }
                    
                    for(unsigned j=0; j<SPACE_DIM; j++)
                    {
//                        grad_u(j) += grad_phi(i,j)*this->mCurrentSolutionReplicated[ PROBLEM_DIM*node_global_index + index_of_unknown];
                        grad_u(j) += grad_phi(j,i)*this->mCurrentSolutionReplicated[ node_global_index ];
                    }
                        
                }
                
                // allow the concrete version of the assembler to interpolate any
                // desired quantities
                IncrementInterpolatedQuantities(phi(i), p_node);
            }
            
            double wJ = jacobian_determinant * quad_rule.GetWeight(quad_index);
            
            ////////////////////////////////////////////////////////////
            // create rAElem and rBElem
            ////////////////////////////////////////////////////////////
            if(assembleMatrix) //(!this->mMatrixIsAssembled && mProblemIsLinear)
            {
                noalias(rAElem) += ComputeLhsTerm(phi, grad_phi, x, u, grad_u) * wJ;
            }
            
            if(assembleVector)
            {
                noalias(rBElem) += ComputeRhsTerm(phi, grad_phi, x, u, grad_u) * wJ;
            }
        }
    }
    
    
    
    /**
     * Calculate the contribution of a single surface element with Neumann
     * boundary condition to the linear system.
     * 
     * @param rSurfaceElement The element to assemble on.
     * @param rBSurfElem The element's contribution to the RHS vector is returned in this
     *     vector of length n, the no. of nodes in this element. There is no
     *     need to zero this vector before calling.
     */
    virtual void AssembleOnSurfaceElement(const BoundaryElement<ELEMENT_DIM-1,SPACE_DIM> &rSurfaceElement,
                                          c_vector<double, PROBLEM_DIM*ELEMENT_DIM> &rBSurfElem)
    {
        GaussianQuadratureRule<ELEMENT_DIM-1> &quad_rule =
            *(AbstractAssembler<ELEMENT_DIM,SPACE_DIM,PROBLEM_DIM>::mpSurfaceQuadRule);
        AbstractBasisFunction<ELEMENT_DIM-1> &rBasisFunction =
            *(AbstractAssembler<ELEMENT_DIM,SPACE_DIM,PROBLEM_DIM>::mpSurfaceBasisFunction);
            
        double jacobian_determinant = rSurfaceElement.GetJacobianDeterminant();
        
        rBSurfElem.clear();
        
        // loop over Gauss points
        for (int quad_index=0; quad_index<quad_rule.GetNumQuadPoints(); quad_index++)
        {
            Point<ELEMENT_DIM-1> quad_point=quad_rule.GetQuadPoint(quad_index);
            
            c_vector<double, ELEMENT_DIM>  phi = rBasisFunction.ComputeBasisFunctions(quad_point);
            
            
            /////////////////////////////////////////////////////////////
            // interpolation
            /////////////////////////////////////////////////////////////
            
            // Location of the gauss point in the original element will be
            // stored in x
            Point<SPACE_DIM> x(0,0,0);
            
            ResetInterpolatedQuantities();
            for (int i=0; i<rSurfaceElement.GetNumNodes(); i++)
            {
                const Point<SPACE_DIM> node_loc = rSurfaceElement.GetNode(i)->rGetPoint();
                for (int j=0; j<SPACE_DIM; j++)
                {
                    x.rGetLocation()[j] += phi(i)*node_loc[j];
                }
                
                // allow the concrete version of the assembler to interpolate any
                // desired quantities
                IncrementInterpolatedQuantities(phi(i), rSurfaceElement.GetNode(i));
                
                ///\todo: add interpolation of u as well
            }
            
            double wJ = jacobian_determinant * quad_rule.GetWeight(quad_index);
            
            ////////////////////////////////////////////////////////////
            // create rAElem and rBElem
            ////////////////////////////////////////////////////////////
            ///\todo Improve efficiency of Neumann BC implementation.
            noalias(rBSurfElem) += ComputeSurfaceRhsTerm(rSurfaceElement, phi, x) * wJ;
        }
    }
    
    /**
     *  The concrete subclass can overload this and IncrementInterpolatedQuantities()
     *  if there are some quantities which need to be computed at each Gauss point. 
     *  They are called in AssembleOnElement()
     */
    virtual void ResetInterpolatedQuantities( void )
    {}
    
    /**
     *  The concrete subclass can overload this and ResetInterpolatedQuantities()
     *  if there are some quantities which need to be computed at each Gauss point. 
     *  They are called in AssembleOnElement()
     */
    virtual void IncrementInterpolatedQuantities(double phi_i, const Node<SPACE_DIM> *pNode)
    {}




    /**
     *  AssembleSystem
     * 
     *  Assemble the linear system for a linear PDE, or the residual or Jacobian for
     *  nonlinear PDEs Loops over each element (and each each surface element if 
     * there are non-zero Neumann boundary conditions and 
     *  calls AssembleOnElement() and adds the contribution to the linear system.
     * 
     *  Takes in current solution and time if necessary but only used if the problem 
     *  is a dynamic one. This method uses PROBLEM_DIM and can assemble linear systems 
     *  for any number of unknown variables.
     * 
     *  Called by Solve()
     *  Calls AssembleOnElement()
     */
    virtual void AssembleSystem(Vec currentSolution=NULL, double currentTime=0.0, Vec residualVector=NULL, Mat* pJacobian=NULL)
    {
        // Replicate the current solution and store so can be used in
        // AssembleOnElement
        if (currentSolution != NULL)
        {
            this->mCurrentSolutionReplicated.ReplicatePetscVector(currentSolution);
        }
        
        PrepareForAssembleSystem(currentSolution, currentTime);
        

        int lo, hi;
        
        if(mProblemIsLinear)
        {
            if (mpLinearSystem == NULL)
            {
                if (currentSolution == NULL)
                {
                    // static problem, create linear system using the size
                    unsigned size = PROBLEM_DIM * this->mpMesh->GetNumNodes();
                    mpLinearSystem = new LinearSystem(size);
                }
                else
                {
                    // use the currrent solution (ie the initial solution)
                    // as the template in the alternative constructor of
                    // LinearSystem. This appears to avoid problems with
                    // VecScatter.
                    mpLinearSystem = new LinearSystem(currentSolution);
                }
            }
            else
            {
                if (mMatrixIsConstant && mMatrixIsAssembled)
                {
                    mpLinearSystem->ZeroRhsVector();
                }
                else
                {
                    mpLinearSystem->ZeroLinearSystem();
                    mMatrixIsAssembled = false;
                }
            }
        }
        else
        {
            assert(residualVector || pJacobian);
        
            if(residualVector)
            {
                // Set residual vector to zero
                PetscScalar zero = 0.0;
#if (PETSC_VERSION_MINOR == 2) //Old API
                PETSCEXCEPT( VecSet(&zero, residualVector) );
#else
                PETSCEXCEPT( VecSet(residualVector, zero) );
#endif
            }
            else if(pJacobian)
            {
                // Set all entries of jacobian to 0
                MatZeroEntries(*pJacobian);
            }
            else
            {
                assert(0);
            }
                
   
            // Replicate the currentGuess data
            //ReplicatableVector current_guess_replicated_array;
            //current_guess_replicated_array.ReplicatePetscVector(currentSolution);
        
            // Get our ownership range
            VecGetOwnershipRange(currentSolution, &lo, &hi);
        }
        
                 
      
   
        // Get an iterator over the elements of the mesh
        typename ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>::ElementIterator
            iter = this->mpMesh->GetElementIteratorBegin();
        
        // Assume all elements have the same number of nodes...
        const int num_elem_nodes = (*iter)->GetNumNodes();
        c_matrix<double, PROBLEM_DIM*(ELEMENT_DIM+1), PROBLEM_DIM*(ELEMENT_DIM+1)> a_elem;
        c_vector<double, PROBLEM_DIM*(ELEMENT_DIM+1)> b_elem;
        
        bool assemble_vector = ((mProblemIsLinear) || ((!mProblemIsLinear) && (residualVector!=NULL)));
        bool assemble_matrix = ( (mProblemIsLinear && !mMatrixIsAssembled) || ((!mProblemIsLinear) && (pJacobian!=NULL)) );
       
        
        while (iter != this->mpMesh->GetElementIteratorEnd())
        {
            Element<ELEMENT_DIM, SPACE_DIM>& element = **iter;
            
            AssembleOnElement(element, a_elem, b_elem, currentSolution, assemble_vector, assemble_matrix);
            
            for (int i=0; i<num_elem_nodes; i++)
            {
                int node1 = element.GetNodeGlobalIndex(i);
                
                if (!mMatrixIsAssembled || pJacobian)
                {
                    for (int j=0; j<num_elem_nodes; j++)
                    {
                        int node2 = element.GetNodeGlobalIndex(j);
                        
                        for (int k=0; k<PROBLEM_DIM; k++)
                        {
                            for (int m=0; m<PROBLEM_DIM; m++)
                            {
                                if(mProblemIsLinear && !mMatrixIsAssembled)
                                {  
                                    // the following expands to, for (eg) the case of two unknowns:
                                    // mpLinearSystem->AddToMatrixElement(2*node1,   2*node2,   a_elem(2*i,   2*j));
                                    // mpLinearSystem->AddToMatrixElement(2*node1+1, 2*node2,   a_elem(2*i+1, 2*j));
                                    // mpLinearSystem->AddToMatrixElement(2*node1,   2*node2+1, a_elem(2*i,   2*j+1));
                                    // mpLinearSystem->AddToMatrixElement(2*node1+1, 2*node2+1, a_elem(2*i+1, 2*j+1));
                                    mpLinearSystem->AddToMatrixElement( PROBLEM_DIM*node1+k,
                                                                                 PROBLEM_DIM*node2+m,
                                                                                 a_elem(PROBLEM_DIM*i+k,PROBLEM_DIM*j+m) );
                                }
                                else if(!mProblemIsLinear && pJacobian!=NULL)
                                {
                                    assert(PROBLEM_DIM==1);
                                    if (lo<=node1 && node1<hi)
                                    {
                                        PetscScalar value = a_elem(i,j);
                                        
                                        //std::cout << value << "\n";
                                        
                                        
                                        MatSetValue(*pJacobian, node1, node2, value, ADD_VALUES);                                
                                    }
                                }
                            }
                        }
                    }
                }


                for (int k=0; k<PROBLEM_DIM; k++)
                {
                    if(mProblemIsLinear)
                    {
                        mpLinearSystem->AddToRhsVectorElement(PROBLEM_DIM*node1+k,b_elem(PROBLEM_DIM*i+k));
                    }
                    else if (residualVector!=NULL)
                    {
                        assert(PROBLEM_DIM==1);
                        //Make sure it's only done once
                        if (lo<=node1 && node1<hi)
                        {
                            PetscScalar value = b_elem(i);
                            PETSCEXCEPT( VecSetValue(residualVector,node1,value,ADD_VALUES) );
                        }
                    }
                }
            }
            iter++;
        }
        
        // add the integrals associated with Neumann boundary conditions to the linear system
        typename ConformingTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>::BoundaryElementIterator
        surf_iter = this->mpMesh->GetBoundaryElementIteratorBegin();
        
        // the following is not true of Bidomain or Monodomain
        if (this->mpBoundaryConditions->AnyNonZeroNeumannConditions()==true)
        {
            if (surf_iter != this->mpMesh->GetBoundaryElementIteratorEnd())
            {
                const int num_surf_nodes = (*surf_iter)->GetNumNodes();
                c_vector<double, PROBLEM_DIM*ELEMENT_DIM> b_surf_elem;
                
                while (surf_iter != this->mpMesh->GetBoundaryElementIteratorEnd())
                {
                    const BoundaryElement<ELEMENT_DIM-1,SPACE_DIM>& surf_element = **surf_iter;
                    
                    ///\todo Check surf_element is in the Neumann surface in an efficient manner
                    if (this->mpBoundaryConditions->HasNeumannBoundaryCondition(&surf_element))
                    {
                        AssembleOnSurfaceElement(surf_element, b_surf_elem);
                        for (int i=0; i<num_surf_nodes; i++)
                        {
                            int node_index = surf_element.GetNodeGlobalIndex(i);
                            
                            for (int k=0; k<PROBLEM_DIM; k++)
                            {
                                if(mProblemIsLinear)
                                {
                                    mpLinearSystem->AddToRhsVectorElement(PROBLEM_DIM*node_index + k, b_surf_elem(PROBLEM_DIM*i+k));
                                }
                                else if(residualVector!=NULL)
                                {
                                    assert(PROBLEM_DIM==1);
                                    PetscScalar value = b_surf_elem(i);
                                    if (lo<=node_index && node_index<hi)
                                    {
                                        PETSCEXCEPT( VecSetValue(residualVector, node_index, value, ADD_VALUES) );
                                    }
                                }
                            }
                        }
                    }
                    surf_iter++;
                }
            }
        }
        
        if(mProblemIsLinear)
        {
            if (mMatrixIsAssembled)
            {
                mpLinearSystem->AssembleRhsVector();
            }
            else
            {
                mpLinearSystem->AssembleIntermediateLinearSystem();
            }
        }
        else if(pJacobian)
        {
            MatAssemblyBegin(*pJacobian, MAT_FLUSH_ASSEMBLY);
            MatAssemblyEnd(*pJacobian, MAT_FLUSH_ASSEMBLY);
        }
        
        
        // the assembler do anything else required like setting up a null basis
        // (see BidomainDg0Assembler) in this function
        FinaliseAssembleSystem(currentSolution, currentTime);
        
        
        // Apply dirichlet boundary conditions
        if(mProblemIsLinear)
        {
            this->mpBoundaryConditions->ApplyDirichletToLinearProblem(*mpLinearSystem, mMatrixIsAssembled);
        }
        else if(residualVector)
        {
            this->mpBoundaryConditions->ApplyDirichletToNonlinearResidual(currentSolution, residualVector);
        }        
        else if(pJacobian)
        {
            this->mpBoundaryConditions->ApplyDirichletToNonlinearJacobian(*pJacobian);
        }
        
            
        if(mProblemIsLinear)
        {        
            if (mMatrixIsAssembled)
            {
                mpLinearSystem->AssembleRhsVector();
            }
            else
            {
                mpLinearSystem->AssembleFinalLinearSystem();
            }
        }
        else if(residualVector)
        {
            VecAssemblyBegin(residualVector);
            VecAssemblyEnd(residualVector);
        }
        else if(pJacobian)
        {
            MatAssemblyBegin(*pJacobian, MAT_FINAL_ASSEMBLY);
            MatAssemblyEnd(*pJacobian, MAT_FINAL_ASSEMBLY);
        }        
        mMatrixIsAssembled = true;
    }
    
    
    
    /**
     *  This method is called at the beginning of Solve(). Subclass assemblers can 
     *  use it to check everything has been set up correctly
     */
    virtual void PrepareForSolve()
    {}
    
    
    /**
     *  This method is called at the beginning of AssembleSystem() and should be 
     *  overloaded in the concrete assembler class if there is any work to be done
     *  before assembling, for example integrating ODEs such as in the Monodomain
     *  assembler.
     */
    virtual void PrepareForAssembleSystem(Vec currentSolution, double currentTime)
    {}
    
    /**
     *  This method is called at the end of AssembleSystem() and should be overloaded
     *  in the concrete assembler class if there is any further work to be done
     */
    virtual void FinaliseAssembleSystem(Vec currentSolution, double currentTime)
    {}


    
public:
    /**
     * Default constructor. Uses linear basis functions.
     * 
     * @param numQuadPoints Number of quadrature points to use per dimension.
     */
    AbstractAssembler(int numQuadPoints = 2)
    {
        // Initialise mesh and bcs to null, so we can check they
        // have been set before attempting to solve
        mpMesh = NULL;
        mpBoundaryConditions = NULL;
        
        mWeAllocatedBasisFunctionMemory = false; // sic
        LinearBasisFunction<ELEMENT_DIM> *pBasisFunction = new LinearBasisFunction<ELEMENT_DIM>();
        LinearBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction = new LinearBasisFunction<ELEMENT_DIM-1>();
        SetBasisFunctions(pBasisFunction, pSurfaceBasisFunction);
        mWeAllocatedBasisFunctionMemory = true;
        
        mpQuadRule = NULL;
        mpSurfaceQuadRule = NULL;
        SetNumberOfQuadraturePointsPerDimension(numQuadPoints);
    }
    
    /**
     * Constructor allowing specification of the type of basis function to use.
     * 
     * @param pBasisFunction Basis function to use for normal elements.
     * @param pSurfaceBasisFunction Basis function to use for boundary elements.
     * @param numQuadPoints Number of quadrature points to use per dimension.
     */
    AbstractAssembler(AbstractBasisFunction<ELEMENT_DIM> *pBasisFunction,
                      AbstractBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction,
                      int numQuadPoints = 2)
    {
        // Initialise mesh and bcs to null, so we can check they
        // have been set before attempting to solve
        mpMesh = NULL;
        mpBoundaryConditions = NULL;
       
        mWeAllocatedBasisFunctionMemory = false;
        SetBasisFunctions(pBasisFunction, pSurfaceBasisFunction);
        
        mpQuadRule = NULL;
        mpSurfaceQuadRule = NULL;
        SetNumberOfQuadraturePointsPerDimension(numQuadPoints);
    }
    
    /**
     * Specify what type of basis functions to use.
     * 
     * @param pBasisFunction Basis function to use for normal elements.
     * @param pSurfaceBasisFunction Basis function to use for boundary elements.
     */
    void SetBasisFunctions(AbstractBasisFunction<ELEMENT_DIM> *pBasisFunction,
                           AbstractBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction)
    {
        if (mWeAllocatedBasisFunctionMemory)
        {
            delete mpBasisFunction;
            delete mpSurfaceBasisFunction;
            mWeAllocatedBasisFunctionMemory = false;
        }
        mpBasisFunction = pBasisFunction;
        mpSurfaceBasisFunction = pSurfaceBasisFunction;
    }
    
    /**
     * Set the number of quadrature points to use, per dimension.
     * 
     * This method will throw an exception if the requested number of quadrature
     * points is not supported. (///\todo: There may be a small memory leak if this
     * occurs.)
     * 
     * @param numQuadPoints Number of quadrature points to use per dimension.
     */
    void SetNumberOfQuadraturePointsPerDimension(int numQuadPoints)
    {
        if (mpQuadRule) delete mpQuadRule;
        mpQuadRule = new GaussianQuadratureRule<ELEMENT_DIM>(numQuadPoints);
        if (mpSurfaceQuadRule) delete mpSurfaceQuadRule;
        mpSurfaceQuadRule = new GaussianQuadratureRule<ELEMENT_DIM-1>(numQuadPoints);
    }
    
    /**
     * Set the mesh.
     */
    void SetMesh(ConformingTetrahedralMesh<ELEMENT_DIM,SPACE_DIM>* pMesh)
    {
        mpMesh = pMesh;
    }
    
    /**
     * Set the boundary conditions.
     */
    void SetBoundaryConditionsContainer(BoundaryConditionsContainer<ELEMENT_DIM,SPACE_DIM,PROBLEM_DIM>* pBoundaryConditions)
    {
        mpBoundaryConditions = pBoundaryConditions;
    }
    
    /**
     * Delete any memory allocated by this class.
     */
    virtual ~AbstractAssembler()
    {
        // Basis functions, if we used the default.
        if (mWeAllocatedBasisFunctionMemory)
        {
            delete mpBasisFunction;
            delete mpSurfaceBasisFunction;
            mWeAllocatedBasisFunctionMemory = false;
        }
        
        // Quadrature rules
        if (mpQuadRule) delete mpQuadRule;
        if (mpSurfaceQuadRule) delete mpSurfaceQuadRule;
    }
};

#endif //_ABSTRACTASSEMBLER_HPP_
