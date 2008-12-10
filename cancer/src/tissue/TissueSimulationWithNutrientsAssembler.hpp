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
#ifndef _TISSUESIMULATIONWITHNUTRIENTSASSEMBLER_HPP_
#define _TISSUESIMULATIONWITHNUTRIENTSASSEMBLER_HPP_

#include <vector>
#include <petscvec.h>

#include "TetrahedralMesh.hpp"
#include "SimpleLinearEllipticAssembler.hpp"
#include "GaussianQuadratureRule.hpp"


/**
 *  TissueSimulationWithNutrientsAssembler
 *
 *  This is a purpose made elliptic assembler that interpolates the source terms
 *  from node onto gauss points, as for a nutrients simulation the source will only
 *  be known at the cells (nodes), not the gauss points.
 */
template<unsigned DIM>
class TissueSimulationWithNutrientsAssembler
    : public SimpleLinearEllipticAssembler<DIM, DIM, TissueSimulationWithNutrientsAssembler<DIM> >
{
    typedef SimpleLinearEllipticAssembler<DIM, DIM, TissueSimulationWithNutrientsAssembler<DIM> > BaseClassType;
    friend class AbstractStaticAssembler<DIM, DIM, 1u, true, BaseClassType>;

private:

    /** The constant in u part of the source term, interpolated onto
     *  the current point
     */
    double mConstantInUSourceTerm;

    /** The linear in u part of the source term, interpolated onto
     *  the current point
     */
    double mLinearInUCoeffInSourceTerm;

protected:

    /**
     *  The SimpleLinearEllipticAssembler version of this method is
     *  overloaded using the interpolated source term
     */
    virtual c_vector<double,1*(DIM+1)> ComputeVectorTerm(
        c_vector<double, DIM+1> &rPhi,
        c_matrix<double, DIM, DIM+1> &rGradPhi,
        ChastePoint<DIM> &rX,
        c_vector<double,1>& rU,
        c_matrix<double, 1, DIM> &rGradU /* not used */,
        Element<DIM,DIM>* pElement);

    /**
     *  The SimpleLinearEllipticAssembler version of this method is
     *  overloaded using the interpolated source term
     */
    virtual c_matrix<double,1*(DIM+1),1*(DIM+1)> ComputeMatrixTerm(
        c_vector<double, DIM+1> &rPhi,
        c_matrix<double, DIM, DIM+1> &rGradPhi,
        ChastePoint<DIM> &rX,
        c_vector<double,1> &u,
        c_matrix<double,1,DIM> &rGradU,
        Element<DIM,DIM>* pElement);

    void ResetInterpolatedQuantities();

    void IncrementInterpolatedQuantities(double phiI, const Node<DIM> *pNode);

public:

    /**
     * Constructor stores the mesh and pde and boundary conditions.
     */
    TissueSimulationWithNutrientsAssembler(TetrahedralMesh<DIM,DIM>* pMesh,
                                  AbstractLinearEllipticPde<DIM>* pPde,
                                  BoundaryConditionsContainer<DIM,DIM,1>* pBoundaryConditions,
                                  unsigned numQuadPoints = 2);

    /**
     *  Destructor
     */
    ~TissueSimulationWithNutrientsAssembler();

};

template<unsigned DIM>
TissueSimulationWithNutrientsAssembler<DIM>::TissueSimulationWithNutrientsAssembler(TetrahedralMesh<DIM,DIM>* pMesh,
                              AbstractLinearEllipticPde<DIM>* pPde,
                              BoundaryConditionsContainer<DIM,DIM,1>* pBoundaryConditions,
                              unsigned numQuadPoints) :
        BaseClassType(pMesh, pPde, pBoundaryConditions, numQuadPoints)
{
}

template<unsigned DIM>
TissueSimulationWithNutrientsAssembler<DIM>::~TissueSimulationWithNutrientsAssembler()
{
}

template<unsigned DIM>
c_vector<double,1*(DIM+1)> TissueSimulationWithNutrientsAssembler<DIM>::ComputeVectorTerm(
        c_vector<double, DIM+1> &rPhi,
        c_matrix<double, DIM, DIM+1> &rGradPhi,
        ChastePoint<DIM> &rX,
        c_vector<double,1>& rU,
        c_matrix<double, 1, DIM> &rGradU /* not used */,
        Element<DIM,DIM>* pElement)
{
    return mConstantInUSourceTerm * rPhi;
}

template<unsigned DIM>
c_matrix<double,1*(DIM+1),1*(DIM+1)> TissueSimulationWithNutrientsAssembler<DIM>::ComputeMatrixTerm(
        c_vector<double, DIM+1> &rPhi,
        c_matrix<double, DIM, DIM+1> &rGradPhi,
        ChastePoint<DIM> &rX,
        c_vector<double,1> &u,
        c_matrix<double,1,DIM> &rGradU,
        Element<DIM,DIM>* pElement)
{
    c_matrix<double, DIM, DIM> pde_diffusion_term = this->mpEllipticPde->ComputeDiffusionTerm(rX);

    // if statement just saves computing phi*phi^T if it is to be multiplied by zero
    if (mLinearInUCoeffInSourceTerm!=0)
    {
        return   prod( trans(rGradPhi), c_matrix<double, DIM, DIM+1>(prod(pde_diffusion_term, rGradPhi)) )
               - mLinearInUCoeffInSourceTerm * outer_prod(rPhi,rPhi);
    }
    else
    {
        return   prod( trans(rGradPhi), c_matrix<double, DIM, DIM+1>(prod(pde_diffusion_term, rGradPhi)) );
    }
}

template<unsigned DIM>
void TissueSimulationWithNutrientsAssembler<DIM>::ResetInterpolatedQuantities()
{
    mConstantInUSourceTerm = 0;
    mLinearInUCoeffInSourceTerm = 0;
}

template<unsigned DIM>
void TissueSimulationWithNutrientsAssembler<DIM>::IncrementInterpolatedQuantities(double phiI, const Node<DIM> *pNode)
{
    mConstantInUSourceTerm += phiI * this->mpEllipticPde->ComputeConstantInUSourceTermAtNode(*pNode);
    mLinearInUCoeffInSourceTerm += phiI * this->mpEllipticPde->ComputeLinearInUCoeffInSourceTermAtNode(*pNode);
}

template<unsigned DIM>
struct AssemblerTraits<TissueSimulationWithNutrientsAssembler<DIM> >
{
    typedef TissueSimulationWithNutrientsAssembler<DIM> CVT_CLS;
    typedef TissueSimulationWithNutrientsAssembler<DIM> CMT_CLS;
    typedef TissueSimulationWithNutrientsAssembler<DIM> INTERPOLATE_CLS;
};

#endif //_TISSUESIMULATIONWITHNUTRIENTSASSEMBLER_HPP_
