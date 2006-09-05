#ifndef _MONODOMAINDG0ASSEMBLER_HPP_
#define _MONODOMAINDG0ASSEMBLER_HPP_


//#include <iostream>
#include <vector>
#include <petscvec.h>

#include "ConformingTetrahedralMesh.cpp"
#include "AbstractAssembler.hpp"
#include "AbstractLinearAssembler.hpp"
#include "SimpleDg0ParabolicAssembler.hpp"
#include "AbstractBasisFunction.hpp"
#include "GaussianQuadratureRule.hpp"
#include "MonodomainPde.hpp"


/**
 *  MonodomainDg0Assembler
 * 
 *  This is essentially the same as the SimpleDg0ParabolicAssembler (which it inherits from),
 *  except that the source term (ie ionic current + stimulus) is interpolated from
 *  their nodal values, instead of computed at the gauss point, since they are only
 *  known at the nodes.
 *  
 *  Also, the MonodomainAssembler automatically creates zero neumann boundary conditions
 *  when constructed and therefore does not need to take in a BoundaryConditionsContainer.
 * 
 *  The user should call Solve() from the superclass AbstractLinearDynamicProblemAssembler.
 */
template<int ELEMENT_DIM, int SPACE_DIM>
class MonodomainDg0Assembler : public SimpleDg0ParabolicAssembler<ELEMENT_DIM, SPACE_DIM>
{
private:
    double mSourceTerm;
    
protected:

    /** 
     *  ComputeRhsTerm()
     * 
     *  This method is called by AssembleOnElement() and tells the assembler
     *  the contribution to add to the element stiffness vector.
     * 
     *  Here, the SimpleDg0ParabolicAssembler version of this method is 
     *  overloaded using the interpolated source term
     */
    virtual c_vector<double,1*(ELEMENT_DIM+1)> ComputeRhsTerm(
        const c_vector<double, ELEMENT_DIM+1> &rPhi,
        const Point<SPACE_DIM> &rX,
        const c_vector<double,1> &u)
    {
        AbstractLinearParabolicPde<SPACE_DIM>* pde = dynamic_cast<AbstractLinearParabolicPde<SPACE_DIM>*> (this->mpPde);
        
        return  rPhi * (mSourceTerm + this->mDtInverse *
                        pde->ComputeDuDtCoefficientFunction(rX) * u(0));
    }    
    
    
    void ResetInterpolatedQuantities( void )
    {
        mSourceTerm=0;
    }
    
    
    void IncrementInterpolatedQuantities(double phi_i, const Node<SPACE_DIM> *pNode)
    {
        AbstractLinearParabolicPde<SPACE_DIM>* pde = dynamic_cast<AbstractLinearParabolicPde<SPACE_DIM>*> (this->mpPde);

        mSourceTerm += phi_i*pde->ComputeNonlinearSourceTermAtNode(*pNode, this->mCurrentSolutionReplicated[ pNode->GetIndex() ] );
    }
    
    
public:
    /**
     * Constructor stores the mesh and pde and sets up boundary conditions.
     */
    MonodomainDg0Assembler(ConformingTetrahedralMesh<ELEMENT_DIM,SPACE_DIM>* pMesh,
                           AbstractLinearParabolicPde<SPACE_DIM>* pPde,
                           int numQuadPoints = 2) :
            SimpleDg0ParabolicAssembler<ELEMENT_DIM,SPACE_DIM>(pMesh, pPde, NULL /*bcs - set below*/, numQuadPoints)
    {
        this->mpMesh = pMesh;
        this->mpPde = pPde;

        // set up boundary conditions
        this->mpBoundaryConditions = new BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM, 1>( this->mpMesh->GetNumNodes() );
        this->mpBoundaryConditions->DefineZeroNeumannOnMeshBoundary(this->mpMesh);
        
        this->SetMatrixIsConstant();
    }

    /**
     *  Alternative constructor which stores the mesh and pde, sets up 
     *  boundary conditions, and also takes in basis functions.
     */
    MonodomainDg0Assembler(ConformingTetrahedralMesh<ELEMENT_DIM,SPACE_DIM>* pMesh,
                           AbstractLinearParabolicPde<SPACE_DIM>* pPde,
                           AbstractBasisFunction<ELEMENT_DIM> *pBasisFunction,
                           AbstractBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction,
                           int numQuadPoints = 2) :
            SimpleDg0ParabolicAssembler<ELEMENT_DIM,SPACE_DIM>(pMesh, pPde, NULL /*bcs - set below*/, pBasisFunction, pSurfaceBasisFunction, numQuadPoints)
    {
        this->mpMesh = pMesh;
        this->mpPde = pPde;

        // set up boundary conditions
        this->mpBoundaryConditions = new BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM, 2>( this->mpMesh->GetNumNodes() );
        this->mpBoundaryConditions->DefineZeroNeumannOnMeshBoundary(this->mpMesh);
        
        this->SetMatrixIsConstant();
    }

    ~MonodomainDg0Assembler()
    {
        delete this->mpBoundaryConditions;
    }
};

#endif //_MONODOMAINDG0ASSEMBLER_HPP_
