/*

Copyright (C) University of Oxford, 2005-2009

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

#include "ImplicitCardiacMechanicsAssembler.hpp"

template<unsigned DIM>
ImplicitCardiacMechanicsAssembler<DIM>::ImplicitCardiacMechanicsAssembler(
                                  QuadraticMesh<DIM>* pQuadMesh,
                                  std::string outputDirectory,
                                  std::vector<unsigned>& rFixedNodes,
                                  AbstractIncompressibleMaterialLaw<DIM>* pMaterialLaw)
    : AbstractCardiacMechanicsAssembler<DIM>(pQuadMesh,
                                             outputDirectory,
                                             rFixedNodes,
                                             pMaterialLaw)
{
    // initialise stores
    mLambda.resize(this->mTotalQuadPoints, 1.0);
    mLambdaLastTimeStep.resize(this->mTotalQuadPoints, 1.0);
    mCellMechSystems.resize(this->mTotalQuadPoints);
}

template<unsigned DIM>
ImplicitCardiacMechanicsAssembler<DIM>::~ImplicitCardiacMechanicsAssembler()
{
}


template<unsigned DIM>
void ImplicitCardiacMechanicsAssembler<DIM>::SetCalciumAndVoltage(std::vector<double>& rCalciumConcentrations, 
                                                                  std::vector<double>& rVoltages)
                                        
{
    assert(rCalciumConcentrations.size() == this->mTotalQuadPoints);
    assert(rVoltages.size() == this->mTotalQuadPoints);

    ContractionModelInputParameters input_parameters;
    
    for(unsigned i=0; i<rCalciumConcentrations.size(); i++)
    {
        input_parameters.intracellularCalciumConcentration = rCalciumConcentrations[i];
        input_parameters.voltage = rVoltages[i];
        
        mCellMechSystems[i].SetInputParameters(input_parameters);
    }
}

template<unsigned DIM>
std::vector<double>& ImplicitCardiacMechanicsAssembler<DIM>::rGetLambda()
{
    return mLambda;
}


template<unsigned DIM>
void ImplicitCardiacMechanicsAssembler<DIM>::Solve(double time, double nextTime, double odeTimestep)
{
    // set the times, which are used in AssembleOnElement
    assert(time < nextTime);
    this->mCurrentTime = time;
    this->mNextTime = nextTime;
    this->mOdeTimestep = odeTimestep;

    // solve
    NonlinearElasticityAssembler<DIM>::Solve();

    // assemble residual again (to solve the cell models implicitly again
    // using the correct value of the deformation x (in case this wasn't the
    // last thing that was done
    this->AssembleSystem(true,false);

    // now update state variables, and set lambda at last timestep. Note
    // lambda was set in AssembleOnElement
    for(unsigned i=0; i<mCellMechSystems.size(); i++)
    {
         mCellMechSystems[i].UpdateStateVariables();
         mLambdaLastTimeStep[i] = mCellMechSystems[i].GetLambda();
    }
}



template<unsigned DIM>
void ImplicitCardiacMechanicsAssembler<DIM>::GetActiveTensionAndTensionDerivs(double currentFibreStretch, 
                                                                              unsigned currentQuadPointGlobalIndex,
                                                                              bool assembleJacobian,
                                                                              double& rActiveTension,
                                                                              double& rDerivActiveTensionWrtLambda,
                                                                              double& rDerivActiveTensionWrtDLambdaDt)
{
/////
// EMTODO: better comments and tidy
/////
    mLambda[currentQuadPointGlobalIndex] = currentFibreStretch;

    double dlam_dt = (currentFibreStretch-mLambdaLastTimeStep[currentQuadPointGlobalIndex])/(this->mNextTime-this->mCurrentTime);

    NhsSystemWithImplicitSolver& r_system = mCellMechSystems[currentQuadPointGlobalIndex];

    // get proper active tension
    // see NOTE below
    r_system.SetStretchAndStretchRate(currentFibreStretch, dlam_dt);

    try
    {
        r_system.RunDoNotUpdate(this->mCurrentTime,this->mNextTime,this->mOdeTimestep);
        rActiveTension = r_system.GetNextActiveTension();
    }
    catch (Exception& e)
    {
        #define COVERAGE_IGNORE
        if(assembleJacobian)
        {
            EXCEPTION("Failed in solving NHS systems when assembling Jacobian");
        }
        #undef COVERAGE_IGNORE
    }


    if(assembleJacobian)
    {
        // get active tension for (lam+h,dlamdt)
        double h1 = std::max(1e-6, currentFibreStretch/100);
        r_system.SetStretchAndStretchRate(currentFibreStretch+h1, dlam_dt);
        r_system.RunDoNotUpdate(this->mCurrentTime,this->mNextTime,this->mOdeTimestep);
        double active_tension_at_lam_plus_h = r_system.GetNextActiveTension();

        // get active tension for (lam,dlamdt+h)
        double h2 = std::max(1e-6, dlam_dt/100);
        r_system.SetStretchAndStretchRate(currentFibreStretch, dlam_dt+h2);
        r_system.RunDoNotUpdate(this->mCurrentTime,this->mNextTime,this->mOdeTimestep);
        double active_tension_at_dlamdt_plus_h = r_system.GetNextActiveTension();

        rDerivActiveTensionWrtLambda = (active_tension_at_lam_plus_h - rActiveTension)/h1;
        rDerivActiveTensionWrtDLambdaDt = (active_tension_at_dlamdt_plus_h - rActiveTension)/h2;
    }

    // NOTE - have to get the active tension again, this must be done last!!
    // As if this turns out to be the correct solution, the state vars will be updated!
    /// \todo: sort out this inefficiency
    r_system.SetStretchAndStretchRate(currentFibreStretch, dlam_dt);
    r_system.SetActiveTensionInitialGuess(rActiveTension);

    try
    {
        r_system.RunDoNotUpdate(this->mCurrentTime,this->mNextTime,this->mOdeTimestep);
        assert( fabs(r_system.GetNextActiveTension()-rActiveTension)<1e-8);
    }
    catch (Exception& e)
    {
        #define COVERAGE_IGNORE
        LOG(2, "WARNING in ImplicitCardiacMechanicsAssembler!\n");
//todo: huh?!
        assert(0);
        //rActiveTension = 1e10;
        //// should have done something above..
        #undef COVERAGE_IGNORE
    }
}    



template class ImplicitCardiacMechanicsAssembler<2>;
template class ImplicitCardiacMechanicsAssembler<3>;


