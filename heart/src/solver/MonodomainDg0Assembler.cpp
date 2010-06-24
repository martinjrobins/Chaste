/*

Copyright (C) University of Oxford, 2005-2010

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

#include "MonodomainDg0Assembler.hpp"
#include "GaussianQuadratureRule.hpp"
#include "HeartConfig.hpp"


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
c_matrix<double,1*(ELEMENT_DIM+1),1*(ELEMENT_DIM+1)>
    MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::ComputeMatrixTerm(
            c_vector<double, ELEMENT_DIM+1> &rPhi,
            c_matrix<double, SPACE_DIM, ELEMENT_DIM+1> &rGradPhi,
            ChastePoint<SPACE_DIM> &rX,
            c_vector<double,1> &rU,
            c_matrix<double, 1, SPACE_DIM> &rGradU /* not used */,
            Element<ELEMENT_DIM,SPACE_DIM>* pElement)
{
    // get bidomain parameters
    double Am = mpConfig->GetSurfaceAreaToVolumeRatio();
    double Cm = mpConfig->GetCapacitance();

    const c_matrix<double, SPACE_DIM, SPACE_DIM>& sigma_i = mpMonodomainPde->rGetIntracellularConductivityTensor(pElement->GetIndex());

    c_matrix<double, SPACE_DIM, ELEMENT_DIM+1> temp = prod(sigma_i, rGradPhi);
    c_matrix<double, ELEMENT_DIM+1, ELEMENT_DIM+1> grad_phi_sigma_i_grad_phi =
        prod(trans(rGradPhi), temp);

    c_matrix<double, ELEMENT_DIM+1, ELEMENT_DIM+1> basis_outer_prod =
        outer_prod(rPhi, rPhi);

    return (Am*Cm/this->mDt)*basis_outer_prod + grad_phi_sigma_i_grad_phi;
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
c_vector<double,1*(ELEMENT_DIM+1)> MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::ComputeVectorTerm(
    c_vector<double, ELEMENT_DIM+1> &rPhi,
    c_matrix<double, SPACE_DIM, ELEMENT_DIM+1> &rGradPhi /* not used */,
    ChastePoint<SPACE_DIM> &rX /* not used */,
    c_vector<double,1> &rU,
    c_matrix<double, 1, SPACE_DIM> &rGradU /* not used */,
    Element<ELEMENT_DIM,SPACE_DIM>* pElement /* not used */)
{
    double Am = mpConfig->GetSurfaceAreaToVolumeRatio();
    double Cm = mpConfig->GetCapacitance();
    
//    //#1429
//    if(mpTheCell)
//    {
//        for(unsigned i=0; i<mpTheCell->rGetStateVariables().size(); i++)
//        {
//            mpTheCell->rGetStateVariables()[i] = mStateVariablesAtQuadPoint[i];
//        }
//        mIionic = mpTheCell->GetIIonic();
//    }

    return  rPhi * (this->mDtInverse * Am * Cm * rU(0) - Am*mIionic - mIIntracellularStimulus);
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
c_vector<double, 1*ELEMENT_DIM> MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::ComputeVectorSurfaceTerm(
    const BoundaryElement<ELEMENT_DIM-1,SPACE_DIM> &rSurfaceElement,
    c_vector<double,ELEMENT_DIM> &rPhi,
    ChastePoint<SPACE_DIM> &rX)
{
    // D_times_gradu_dot_n = [D grad(u)].n, D=diffusion matrix
    double sigma_i_times_grad_phi_i_dot_n = this->mpBoundaryConditions->GetNeumannBCValue(&rSurfaceElement, rX, 0);

    return rPhi*sigma_i_times_grad_phi_i_dot_n;
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::ResetInterpolatedQuantities( void )
{
    mIionic=0;
    mIIntracellularStimulus=0;
    
//    //#1429
//    if(mpTheCell)
//    {
//        for(unsigned i=0; i<mStateVariablesAtQuadPoint.size(); i++)
//        {
//            mStateVariablesAtQuadPoint[i] = 0;
//        }
//    }
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::IncrementInterpolatedQuantities(
            double phiI, const Node<SPACE_DIM>* pNode)
{
    unsigned node_global_index = pNode->GetIndex();

    mIionic                 += phiI * mpMonodomainPde->rGetIionicCacheReplicated()[ node_global_index ];
    mIIntracellularStimulus += phiI * mpMonodomainPde->rGetIntracellularStimulusCacheReplicated()[ node_global_index ];

//    //#1429 (note: could put the 'mIionic +=' line above in an else clause below)
//    if(mpTheCell)
//    {
//        for(unsigned i=0; i<mStateVariablesAtQuadPoint.size(); i++)
//        {
//            mStateVariablesAtQuadPoint[i] += phiI * mpMonodomainPde->GetCardiacCell(node_global_index)->rGetStateVariables()[i];
//        }
//    }   
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::PrepareForAssembleSystem(
            Vec existingSolution, double time)
{
    mpMonodomainPde->SolveCellSystems(existingSolution, time, time+this->mDt);
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::InitialiseForSolve(Vec initialSolution)
{
    if (this->mpLinearSystem != NULL)
    {
        return;
    }

    // linear system created here
    BaseClassType::InitialiseForSolve(initialSolution);

    if(HeartConfig::Instance()->GetUseAbsoluteTolerance())
    {
        this->mpLinearSystem->SetAbsoluteTolerance(HeartConfig::Instance()->GetAbsoluteTolerance());
    }
    else
    {
        this->mpLinearSystem->SetRelativeTolerance(HeartConfig::Instance()->GetRelativeTolerance());
    }

    this->mpLinearSystem->SetKspType(HeartConfig::Instance()->GetKSPSolver());
    this->mpLinearSystem->SetPcType(HeartConfig::Instance()->GetKSPPreconditioner());
    this->mpLinearSystem->SetMatrixIsSymmetric(true);
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::MonodomainDg0Assembler(
            AbstractTetrahedralMesh<ELEMENT_DIM,SPACE_DIM>* pMesh,
            MonodomainPde<ELEMENT_DIM, SPACE_DIM>* pPde,
            BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM, 1>* pBcc,
            unsigned numQuadPoints)
    : AbstractAssembler<ELEMENT_DIM,SPACE_DIM,1>(),
      BaseClassType(numQuadPoints),
      AbstractDynamicAssemblerMixin<ELEMENT_DIM,SPACE_DIM,1>()
{
    mpMonodomainPde = pPde;

    this->mpBoundaryConditions = pBcc;

    this->SetMesh(pMesh);

    this->SetMatrixIsConstant();

    mpConfig = HeartConfig::Instance();
    
//    //#1429
//    mpTheCell = NULL;
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::~MonodomainDg0Assembler()
{
}


////#1429
//template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
//void MonodomainDg0Assembler<ELEMENT_DIM,SPACE_DIM>::InterpolateCellStateVariablesNotIonicCurrent(AbstractCardiacCell* pCell)
//{
//    mpTheCell = pCell;
//    mStateVariablesAtQuadPoint.resize(mpTheCell->rGetStateVariables().size());
//}


/////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////

template class MonodomainDg0Assembler<1,1>;
template class MonodomainDg0Assembler<1,2>;
template class MonodomainDg0Assembler<1,3>;
template class MonodomainDg0Assembler<2,2>;
template class MonodomainDg0Assembler<3,3>;

