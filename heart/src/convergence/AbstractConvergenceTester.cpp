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

#include "AbstractConvergenceTester.hpp"
#include "Exception.hpp"


AbstractUntemplatedConvergenceTester::AbstractUntemplatedConvergenceTester()
    : mMeshWidth(0.2),//cm
      mKspTolerance(2e-4),//Justification from overlayed 1D time/space convergence plots with varied KSP tolerances
      mUseKspAbsoluteTolerance(true),
      OdeTimeStep(0.0025),//Justification from 1D test with this->PdeTimeStep held at 0.01 (allowing two hits at convergence)
      PdeTimeStep(0.005),//Justification from 1D test with this->OdeTimeStep held at 0.0025
      MeshNum(5u),//Justification from 1D test
      RelativeConvergenceCriterion(1e-4),
      LastDifference(1),
      AbsoluteStimulus(-1e7),
      PopulatedResult(false),
      FixedResult(false),
      UseAbsoluteStimulus(false),
      //UseNeumannStimulus(false),
      Converged(false),
      //StimulateRegion(false)
      Stimulus(PLANE),
      NeumannStimulus(4000)
{
}

void AbstractUntemplatedConvergenceTester::SetKspRelativeTolerance(const double relativeTolerance)
{
   mKspTolerance = relativeTolerance;
   mUseKspAbsoluteTolerance = false;
}

void AbstractUntemplatedConvergenceTester::SetKspAbsoluteTolerance(const double absoluteTolerance)
{
   mKspTolerance = absoluteTolerance;
   mUseKspAbsoluteTolerance = true;
}

double AbstractUntemplatedConvergenceTester::GetKspAbsoluteTolerance()
{
    if (!mUseKspAbsoluteTolerance)
    {
        EXCEPTION("Currently using relative tolerance");
    }
    return mKspTolerance;
}

double AbstractUntemplatedConvergenceTester::GetKspRelativeTolerance()
{
    if (mUseKspAbsoluteTolerance)
    {
        EXCEPTION("Currently using absolute tolerance");
    }
    return mKspTolerance;
}

AbstractUntemplatedConvergenceTester::~AbstractUntemplatedConvergenceTester()
{
}

