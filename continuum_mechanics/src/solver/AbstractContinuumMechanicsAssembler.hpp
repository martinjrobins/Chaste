/*

Copyright (C) University of Oxford, 2005-2011

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

#ifndef ABSTRACTCONTINUUMMECHANICSASSEMBLER_HPP_
#define ABSTRACTCONTINUUMMECHANICSASSEMBLER_HPP_

#include "QuadraticMesh.hpp"
#include "LinearBasisFunction.hpp"
#include "QuadraticBasisFunction.hpp"
#include "ReplicatableVector.hpp"
#include "DistributedVector.hpp"
#include "PetscTools.hpp"
#include "PetscVecTools.hpp"
#include "PetscMatTools.hpp"
#include "GaussianQuadratureRule.hpp"

// do mixed problem first, then allow for non-mixed. Needs template param? ought to check mat size is consistent with
// mixed or not


template<unsigned DIM, bool CAN_ASSEMBLE_VECTOR, bool CAN_ASSEMBLE_MATRIX>
class AbstractContinuumMechanicsAssembler : boost::noncopyable
{
    static const bool BLOCK_SYMMETRIC_MATRIX = true; //generalise to non-block symmetric matrices later (when needed maybe)

    /** Number of vertices per element. */
    static const unsigned NUM_VERTICES_PER_ELEMENT = DIM+1;

    /** Number of nodes per element. */
    static const unsigned NUM_NODES_PER_ELEMENT = (DIM+1)*(DIM+2)/2; // assuming quadratic

    static const unsigned SPATIAL_BLOCK_SIZE_ELEMENTAL = DIM*NUM_NODES_PER_ELEMENT;
    static const unsigned PRESSURE_BLOCK_SIZE_ELEMENTAL = NUM_VERTICES_PER_ELEMENT;

    /** Stencil size. */
    static const unsigned STENCIL_SIZE = DIM*NUM_NODES_PER_ELEMENT + NUM_VERTICES_PER_ELEMENT;

protected:
    /** The quadratic mesh */
    QuadraticMesh<DIM>* mpMesh;

    /** The vector to be assembled (only used if CAN_ASSEMBLE_VECTOR == true). */
    Vec mVectorToAssemble;

    /** The matrix to be assembled (only used if CAN_ASSEMBLE_MATRIX == true). */
    Mat mMatrixToAssemble;

    /**
     * Whether to assemble the matrix (an assembler may be able to assemble matrices
     * (CAN_ASSEMBLE_MATRIX==true), but may not want to do so each timestep, hence
     * this second boolean.
     */
    bool mAssembleMatrix;

    /** Whether to assemble the vector. */
    bool mAssembleVector;

    /** Whether to zero the given matrix before assembly, or just add to it. */
    bool mZeroMatrixBeforeAssembly;

    /** Whether to zero the given vector before assembly, or just add to it. */
    bool mZeroVectorBeforeAssembly;


    GaussianQuadratureRule<DIM>* mpQuadRule;

    /**
     * The main assembly method. Protected, should only be called through Assemble(),
     * AssembleMatrix() or AssembleVector() which set mAssembleMatrix, mAssembleVector
     * accordingly. Involves looping over elements, and computing
     * integrals and adding them to the vector or matrix
     */
    void DoAssemble();


    /**
     *  For a continuum mechanics problem in mixed form (displacement-pressure or velocity-pressure), the matrix
     *  has the form
     *  [A     B1]
     *  [B2^T  C ]
     *  (where often B1=B2 and C=0). The function is related to the spatial-spatial block, ie matrix A.
     *
     *  For the contribution to A from a given element, this method should return the INTEGRAND in the definition of A.
     *  See concrete classes for examples. Needed to be implemented (overridden) if the concrete class
     *  is going to assemble matrices (ie if CAN_ASSEMBLE_MATRIX is true).
     *
     *  @param rQuadPhi  All the quadratic basis functions on this element, evaluated at the current quad point
     *  @param rGradQuadPhi  Gradients of all the quadratic basis functions on this element, evaluated at the current quad point
     *  @param rX Current location (physical position corresponding to quad point
     *  @param pElement Current element
     */
    virtual c_matrix<double,SPATIAL_BLOCK_SIZE_ELEMENTAL,SPATIAL_BLOCK_SIZE_ELEMENTAL> ComputeSpatialSpatialMatrixTerm(
        c_vector<double, NUM_NODES_PER_ELEMENT>& rQuadPhi,
        c_matrix<double, DIM, NUM_NODES_PER_ELEMENT>& rGradQuadPhi,
        ChastePoint<DIM>& rX,
        Element<DIM,DIM>* pElement)
    {
        NEVER_REACHED;
        return zero_matrix<double>(SPATIAL_BLOCK_SIZE_ELEMENTAL,SPATIAL_BLOCK_SIZE_ELEMENTAL);
    }

    /**
     *  For a continuum mechanics problem in mixed form (displacement-pressure or velocity-pressure), the matrix
     *  has the form
     *  [A     B1]
     *  [B2^T  C ]
     *  (where often B1=B2 and C=0). The function is related to the spatial-pressure block, ie matrix B1. If
     *  BLOCK_SYMMETRIC_MATRIX is true, B1=B2 is assumed, so it also relates to B2.
     *
     *  For the contribution to A from a given element, this method should return the INTEGRAND in the definition of B.
     *  See concrete classes for examples. Needed to be implemented (overridden) if the concrete class
     *  is going to assemble matrices (ie if CAN_ASSEMBLE_MATRIX is true).
     *
     *  @param rQuadPhi  All the quadratic basis functions on this element, evaluated at the current quad point
     *  @param rGradQuadPhi  Gradients of all the quadratic basis functions on this element, evaluated at the current quad point
     *  @param rLinearPhi  All the linear basis functions on this element, evaluated at the current quad point
     *  @param rGradLinearPhi  Gradients of all the linear basis functions on this element, evaluated at the current quad point
     *  @param rX Current location (physical position corresponding to quad point
     *  @param pElement Current element
     */
    virtual c_matrix<double,SPATIAL_BLOCK_SIZE_ELEMENTAL,PRESSURE_BLOCK_SIZE_ELEMENTAL> ComputeSpatialPressureMatrixTerm(
        c_vector<double, NUM_NODES_PER_ELEMENT>& rQuadPhi,
        c_matrix<double, DIM, NUM_NODES_PER_ELEMENT>& rGradQuadPhi,
        c_vector<double, NUM_VERTICES_PER_ELEMENT>& rLinearPhi,
        c_matrix<double, DIM, NUM_VERTICES_PER_ELEMENT>& rGradLinearPhi,
        ChastePoint<DIM>& rX,
        Element<DIM,DIM>* pElement)
    {
        NEVER_REACHED;
        return zero_matrix<double>(SPATIAL_BLOCK_SIZE_ELEMENTAL,SPATIAL_BLOCK_SIZE_ELEMENTAL);
    }


    /**
     *  For a continuum mechanics problem in mixed form (displacement-pressure or velocity-pressure), the matrix
     *  has the form
     *  [A     B1]
     *  [B2^T  C ]
     *  (where often B1=B2 and C=0). The function is related to the pressure-pressure block, ie matrix C.
     *
     *  For the contribution to A from a given element, this method should return the INTEGRAND in the definition of C.
     *  See concrete classes for examples. Needed to be implemented (overridden) if the concrete class
     *  is going to assemble matrices (ie if CAN_ASSEMBLE_MATRIX is true).
     *
     *  @param rLinearPhi  All the linear basis functions on this element, evaluated at the current quad point
     *  @param rGradLinearPhi  Gradients of all the linear basis functions on this element, evaluated at the current quad point
     *  @param rX Current location (physical position corresponding to quad point
     *  @param pElement Current element
     */
    virtual c_matrix<double,PRESSURE_BLOCK_SIZE_ELEMENTAL,PRESSURE_BLOCK_SIZE_ELEMENTAL> ComputePressurePressureMatrixTerm(
        c_vector<double, NUM_VERTICES_PER_ELEMENT>& rLinearPhi,
        c_matrix<double, DIM, NUM_VERTICES_PER_ELEMENT>& rGradLinearPhi,
        ChastePoint<DIM>& rX,
        Element<DIM,DIM>* pElement)
    {
        NEVER_REACHED;
        return zero_matrix<double>(PRESSURE_BLOCK_SIZE_ELEMENTAL,PRESSURE_BLOCK_SIZE_ELEMENTAL);
    }


    /**
     *  For a continuum mechanics problem in mixed form (displacement-pressure or velocity-pressure), the matrix
     *  has the form
     *  [A     B1]
     *  [B2^T  C ]
     *  (where often B1=B2 and C=0) and the vector has the form
     *  [b1]
     *  [b2]
     *  The function is related to the spatial-block in the vector, ie b1.
     *
     *  For the contribution to b1 from a given element, this method should return the INTEGRAND in the definition of b1.
     *  See concrete classes for examples. Needed to be implemented (overridden) if the concrete class
     *  is going to assemble vectors (ie if CAN_ASSEMBLE_VECTOR is true).
     *
     *  @param rQuadPhi  All the quadratic basis functions on this element, evaluated at the current quad point
     *  @param rGradQuadPhi  Gradients of all the quadratic basis functions on this element, evaluated at the current quad point
     *  @param rX Current location (physical position corresponding to quad point
     *  @param pElement Current element
     */
    virtual c_vector<double,SPATIAL_BLOCK_SIZE_ELEMENTAL> ComputeSpatialVectorTerm(
        c_vector<double, NUM_NODES_PER_ELEMENT>& rQuadPhi,
        c_matrix<double, DIM, NUM_NODES_PER_ELEMENT>& rGradQuadPhi,
        ChastePoint<DIM>& rX,
        Element<DIM,DIM>* pElement)
    {
        NEVER_REACHED;
        return zero_vector<double>(SPATIAL_BLOCK_SIZE_ELEMENTAL);
    }


    /**
     *  For a continuum mechanics problem in mixed form (displacement-pressure or velocity-pressure), the matrix
     *  has the form
     *  [A     B1]
     *  [B2^T  C ]
     *  (where often B1=B2 and C=0) and the vector has the form
     *  [b1]
     *  [b2]
     *  The function is related to the pressure-block in the vector, ie b2.
     *
     *  For the contribution to b1 from a given element, this method should return the INTEGRAND in the definition of b2.
     *  See concrete classes for examples. Needed to be implemented (overridden) if the concrete class
     *  is going to assemble vectors (ie if CAN_ASSEMBLE_VECTOR is true).
     *
     *  @param rLinearPhi  All the linear basis functions on this element, evaluated at the current quad point
     *  @param rGradLinearPhi  Gradients of all the linear basis functions on this element, evaluated at the current quad point
     *  @param rX Current location (physical position corresponding to quad point
     *  @param pElement Current element
     */
    virtual c_vector<double,PRESSURE_BLOCK_SIZE_ELEMENTAL> ComputePressureVectorTerm(
            c_vector<double, NUM_VERTICES_PER_ELEMENT>& rLinearPhi,
            c_matrix<double, DIM, NUM_VERTICES_PER_ELEMENT>& rGradLinearPhi,
            ChastePoint<DIM>& rX,
            Element<DIM,DIM>* pElement)
    {
        NEVER_REACHED;
        return zero_vector<double>(PRESSURE_BLOCK_SIZE_ELEMENTAL);
    }



    /**
     * Calculate the contribution of a single element to the linear system.
     *
     * @param rElement The element to assemble on.
     * @param rAElem The element's contribution to the LHS matrix is returned in this
     *    n by n matrix, where n is the no. of nodes in this element. There is no
     *    need to zero this matrix before calling.
     * @param rBElem The element's contribution to the RHS vector is returned in this
     *    vector of length n, the no. of nodes in this element. There is no
     *    need to zero this vector before calling.
     */
    void AssembleOnElement(Element<DIM, DIM>& rElement,
                           c_matrix<double, STENCIL_SIZE, STENCIL_SIZE >& rAElem,
                           c_vector<double, STENCIL_SIZE>& rBElem);

public:
    AbstractContinuumMechanicsAssembler(QuadraticMesh<DIM>* pMesh, unsigned numQuadPoints = 3)
        : mpMesh(pMesh),
          mVectorToAssemble(NULL),
          mMatrixToAssemble(NULL),
          mZeroMatrixBeforeAssembly(true),
          mZeroVectorBeforeAssembly(true)
    {
        assert(pMesh);
        mpQuadRule = new GaussianQuadratureRule<DIM>(numQuadPoints);
    }

    /**
     * Set the matrix that needs to be assembled. Requires CAN_ASSEMBLE_MATRIX==true.
     *
     * @param rMatToAssemble Reference to the matrix
     * @param zeroMatrixBeforeAssembly Whether to zero the matrix before assembling
     *  (otherwise it is just added to)
     */
    void SetMatrixToAssemble(Mat& rMatToAssemble, bool zeroMatrixBeforeAssembly=true);

    /**
     * Set the vector that needs to be assembled. Requires CAN_ASSEMBLE_VECTOR==true.
     *
     * @param rVecToAssemble Reference to the vector
     * @param zeroVectorBeforeAssembly Whether to zero the vector before assembling
     *  (otherwise it is just added to)
     */
    void SetVectorToAssemble(Vec& rVecToAssemble, bool zeroVectorBeforeAssembly);

    /**
     * Set a current solution vector that will be used in AssembleOnElement and can passed
     * up to ComputeMatrixTerm() or ComputeVectorTerm().
     *
     * @param currentSolution Current solution vector.
     */
    void SetCurrentSolution(Vec currentSolution);

    /**
     * Assemble everything that the class can assemble.
     */
    void Assemble()
    {
        mAssembleMatrix = CAN_ASSEMBLE_MATRIX;
        mAssembleVector = CAN_ASSEMBLE_VECTOR;
        DoAssemble();
    }

    /**
     * Assemble the matrix. Requires CAN_ASSEMBLE_MATRIX==true and ComputeMatrixTerm() to be implemented.
     */
    void AssembleMatrix()
    {
        assert(CAN_ASSEMBLE_MATRIX);
        mAssembleMatrix = true;
        mAssembleVector = false;
        DoAssemble();
    }

    /**
     * Assemble the vector. Requires CAN_ASSEMBLE_VECTOR==true and ComputeVectorTerm() to be implemented.
     */
    void AssembleVector()
    {
        assert(CAN_ASSEMBLE_VECTOR);
        mAssembleMatrix = false;
        mAssembleVector = true;
        DoAssemble();
    }

    /**
     * Destructor.
     */
    virtual ~AbstractContinuumMechanicsAssembler()
    {
        delete mpQuadRule;
    }
};

template<unsigned DIM, bool CAN_ASSEMBLE_VECTOR, bool CAN_ASSEMBLE_MATRIX>
void AbstractContinuumMechanicsAssembler<DIM,CAN_ASSEMBLE_VECTOR,CAN_ASSEMBLE_MATRIX>::SetMatrixToAssemble(Mat& rMatToAssemble, bool zeroMatrixBeforeAssembly)
{
//    assert(rMatToAssemble);
//    MatGetOwnershipRange(rMatToAssemble, &mOwnershipRangeLo, &mOwnershipRangeHi);

    assert( PetscMatTools::GetSize(rMatToAssemble) == DIM*mpMesh->GetNumNodes()+mpMesh->GetNumVertices() );

    mMatrixToAssemble = rMatToAssemble;
    mZeroMatrixBeforeAssembly = zeroMatrixBeforeAssembly;
}

template<unsigned DIM, bool CAN_ASSEMBLE_VECTOR, bool CAN_ASSEMBLE_MATRIX>
void AbstractContinuumMechanicsAssembler<DIM,CAN_ASSEMBLE_VECTOR,CAN_ASSEMBLE_MATRIX>::SetVectorToAssemble(Vec& rVecToAssemble, bool zeroVectorBeforeAssembly)
{
//    assert(rVecToAssemble);
//    VecGetOwnershipRange(rVecToAssemble, &mOwnershipRangeLo, &mOwnershipRangeHi);

    assert( PetscVecTools::GetSize(rVecToAssemble) == DIM*mpMesh->GetNumNodes()+mpMesh->GetNumVertices() );

    mVectorToAssemble = rVecToAssemble;
    mZeroVectorBeforeAssembly = zeroVectorBeforeAssembly;
}

//// add this method when needed..
//template<unsigned DIM, bool CAN_ASSEMBLE_VECTOR, bool CAN_ASSEMBLE_MATRIX>
//void AbstractContinuumMechanicsAssembler<DIM,CAN_ASSEMBLE_VECTOR,CAN_ASSEMBLE_MATRIX>::SetCurrentSolution(Vec currentSolution)
//{
//    assert(currentSolution != NULL);
//
//    // Replicate the current solution and store so can be used in AssembleOnElement
//    HeartEventHandler::BeginEvent(HeartEventHandler::COMMUNICATION);
//    mCurrentSolutionOrGuessReplicated.ReplicatePetscVector(currentSolution);
//    HeartEventHandler::EndEvent(HeartEventHandler::COMMUNICATION);
//
//    // The AssembleOnElement type methods will determine if a current solution or
//    // current guess exists by looking at the size of the replicated vector, so
//    // check the size is zero if there isn't a current solution.
//    assert(mCurrentSolutionOrGuessReplicated.GetSize() > 0);
//}

template<unsigned DIM, bool CAN_ASSEMBLE_VECTOR, bool CAN_ASSEMBLE_MATRIX>
void AbstractContinuumMechanicsAssembler<DIM,CAN_ASSEMBLE_VECTOR,CAN_ASSEMBLE_MATRIX>::DoAssemble()
{
    assert(mAssembleMatrix || mAssembleVector);
    if (mAssembleMatrix && mMatrixToAssemble==NULL)
    {
        EXCEPTION("Matrix to be assembled has not been set");
    }
    if (mAssembleVector && mVectorToAssemble==NULL)
    {
        EXCEPTION("Vector to be assembled has not been set");
    }

    // Zero the matrix/vector if it is to be assembled
    if (mAssembleVector && mZeroVectorBeforeAssembly)
    {
        PetscVecTools::Zero(mVectorToAssemble);
    }
    if (mAssembleMatrix && mZeroMatrixBeforeAssembly)
    {
        PetscMatTools::Zero(mMatrixToAssemble);
    }

//todo!
//mpPreconditionMatrixLinearSystem->ZeroLhsMatrix();

    c_matrix<double, STENCIL_SIZE, STENCIL_SIZE> a_elem = zero_matrix<double>(STENCIL_SIZE,STENCIL_SIZE);
    c_vector<double, STENCIL_SIZE> b_elem = zero_vector<double>(STENCIL_SIZE);


    // Loop over elements
    for (typename AbstractTetrahedralMesh<DIM, DIM>::ElementIterator iter = mpMesh->GetElementIteratorBegin();
         iter != mpMesh->GetElementIteratorEnd();
         ++iter)
    {
        Element<DIM, DIM>& r_element = *iter;

        // Test for ownership first, since it's pointless to test the criterion on something which we might know nothing about.
        if ( r_element.GetOwnership() == true  /*&& ElementAssemblyCriterion(r_element)==true*/ )
        {
            AssembleOnElement(r_element, a_elem, b_elem);

            unsigned p_indices[STENCIL_SIZE];
            for (unsigned i=0; i<NUM_NODES_PER_ELEMENT; i++)
            {
                for (unsigned j=0; j<DIM; j++)
                {
                    p_indices[DIM*i+j] = DIM*r_element.GetNodeGlobalIndex(i) + j;
                }
            }

            for (unsigned i=0; i<NUM_VERTICES_PER_ELEMENT; i++)
            {
                p_indices[DIM*NUM_NODES_PER_ELEMENT + i] = DIM*mpMesh->GetNumNodes() + r_element.GetNodeGlobalIndex(i);
            }

            if (mMatrixToAssemble)
            {
                PetscMatTools::AddMultipleValues<STENCIL_SIZE>(mMatrixToAssemble, p_indices, a_elem);
            }

//mpPreconditionMatrixLinearSystem->AddLhsMultipleValues(p_indices, a_elem_precond);

            if (mAssembleVector)
            {
                PetscVecTools::AddMultipleValues<STENCIL_SIZE>(mVectorToAssemble, p_indices, b_elem);
            }
        }
    }
}

template<unsigned DIM, bool CAN_ASSEMBLE_VECTOR, bool CAN_ASSEMBLE_MATRIX>
void AbstractContinuumMechanicsAssembler<DIM,CAN_ASSEMBLE_VECTOR,CAN_ASSEMBLE_MATRIX>::AssembleOnElement(Element<DIM, DIM>& rElement,
                                                                                                         c_matrix<double, STENCIL_SIZE, STENCIL_SIZE >& rAElem,
                                                                                                         c_vector<double, STENCIL_SIZE>& rBElem)
{
    static c_matrix<double,DIM,DIM> jacobian;
    static c_matrix<double,DIM,DIM> inverse_jacobian;
    double jacobian_determinant;

    mpMesh->GetInverseJacobianForElement(rElement.GetIndex(), jacobian, jacobian_determinant, inverse_jacobian);

    if (mAssembleMatrix)
    {
        rAElem.clear();
    }

    if (mAssembleVector)
    {
        rBElem.clear();
    }


    // Allocate memory for the basis functions values and derivative values
    static c_vector<double, NUM_VERTICES_PER_ELEMENT> linear_phi;
    static c_vector<double, NUM_NODES_PER_ELEMENT> quad_phi;
    static c_matrix<double, DIM, NUM_NODES_PER_ELEMENT> grad_quad_phi;
    static c_matrix<double, DIM, NUM_VERTICES_PER_ELEMENT> grad_linear_phi;

    c_vector<double,DIM> body_force;

    // Loop over Gauss points
    for (unsigned quadrature_index=0; quadrature_index < mpQuadRule->GetNumQuadPoints(); quadrature_index++)
    {
        double wJ = jacobian_determinant * mpQuadRule->GetWeight(quadrature_index);
        const ChastePoint<DIM>& quadrature_point = mpQuadRule->rGetQuadPoint(quadrature_index);

        // Set up basis function info
        LinearBasisFunction<DIM>::ComputeBasisFunctions(quadrature_point, linear_phi);
        QuadraticBasisFunction<DIM>::ComputeBasisFunctions(quadrature_point, quad_phi);
        QuadraticBasisFunction<DIM>::ComputeTransformedBasisFunctionDerivatives(quadrature_point, inverse_jacobian, grad_quad_phi);
        LinearBasisFunction<DIM>::ComputeTransformedBasisFunctionDerivatives(quadrature_point, inverse_jacobian, grad_linear_phi);

        // interpolate X (ie physical location of this quad point).
        ChastePoint<DIM> X;
        for (unsigned vertex_index=0; vertex_index<NUM_VERTICES_PER_ELEMENT; vertex_index++)
        {
            X.rGetLocation() += linear_phi(vertex_index)*rElement.GetNode(vertex_index)->rGetLocation();
        }

        if(mAssembleVector)
        {
            c_vector<double,SPATIAL_BLOCK_SIZE_ELEMENTAL> b_spatial
                = ComputeSpatialVectorTerm(quad_phi, grad_quad_phi, X, &rElement);
            c_vector<double,PRESSURE_BLOCK_SIZE_ELEMENTAL> b_pressure = ComputePressureVectorTerm(linear_phi, grad_linear_phi, X, &rElement);

            for (unsigned i=0; i<SPATIAL_BLOCK_SIZE_ELEMENTAL; i++)
            {
                rBElem(i) += b_spatial(i)*wJ;
            }


            for (unsigned i=0; i<PRESSURE_BLOCK_SIZE_ELEMENTAL; i++)
            {
                rBElem(SPATIAL_BLOCK_SIZE_ELEMENTAL + i) += b_pressure(i)*wJ;
            }
        }


        if(mAssembleMatrix)
        {
            c_matrix<double,SPATIAL_BLOCK_SIZE_ELEMENTAL,SPATIAL_BLOCK_SIZE_ELEMENTAL> a_spatial_spatial
                = ComputeSpatialSpatialMatrixTerm(quad_phi, grad_quad_phi, X, &rElement);

            c_matrix<double,SPATIAL_BLOCK_SIZE_ELEMENTAL,PRESSURE_BLOCK_SIZE_ELEMENTAL> a_spatial_pressure
                = ComputeSpatialPressureMatrixTerm(quad_phi, grad_quad_phi, linear_phi, grad_linear_phi, X, &rElement);

            c_matrix<double,PRESSURE_BLOCK_SIZE_ELEMENTAL,SPATIAL_BLOCK_SIZE_ELEMENTAL> a_pressure_spatial;
            if(!BLOCK_SYMMETRIC_MATRIX)
            {
                NEVER_REACHED; // to-come: non-mixed problems
                //a_pressure_spatial = ComputeSpatialPressureMatrixTerm(quad_phi, grad_quad_phi, lin_phi, grad_lin_phi, x, &rElement);
            }

            c_matrix<double,PRESSURE_BLOCK_SIZE_ELEMENTAL,PRESSURE_BLOCK_SIZE_ELEMENTAL> a_pressure_pressure
                = ComputePressurePressureMatrixTerm(linear_phi, grad_linear_phi, X, &rElement);

            for (unsigned i=0; i<SPATIAL_BLOCK_SIZE_ELEMENTAL; i++)
            {
                for(unsigned j=0; j<SPATIAL_BLOCK_SIZE_ELEMENTAL; j++)
                {
                    rAElem(i,j) += a_spatial_spatial(i,j)*wJ;
                }

                for(unsigned j=0; j<PRESSURE_BLOCK_SIZE_ELEMENTAL; j++)
                {
                    rAElem(i, SPATIAL_BLOCK_SIZE_ELEMENTAL + j) += a_spatial_pressure(i,j)*wJ;
                }
            }

            for(unsigned i=0; i<PRESSURE_BLOCK_SIZE_ELEMENTAL; i++)
            {
                if(BLOCK_SYMMETRIC_MATRIX)
                {
                    for(unsigned j=0; j<SPATIAL_BLOCK_SIZE_ELEMENTAL; j++)
                    {
                        rAElem(SPATIAL_BLOCK_SIZE_ELEMENTAL + i, j) += a_spatial_pressure(j,i)*wJ;
                    }
                }
                else
                {
                    NEVER_REACHED; // to-come: non-mixed problems
                }

                for(unsigned j=0; j<PRESSURE_BLOCK_SIZE_ELEMENTAL; j++)
                {
                    rAElem(SPATIAL_BLOCK_SIZE_ELEMENTAL + i, SPATIAL_BLOCK_SIZE_ELEMENTAL + j) += a_pressure_pressure(i,j)*wJ;
                }
            }
        }
    }
}


#endif // ABSTRACTCONTINUUMMECHANICSASSEMBLER_HPP_
