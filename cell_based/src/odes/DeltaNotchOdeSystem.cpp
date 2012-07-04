/*

Copyright (c) 2005-2012, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "DeltaNotchOdeSystem.hpp"
#include "CellwiseOdeSystemInformation.hpp"

DeltaNotchOdeSystem::DeltaNotchOdeSystem(double meanDelta, std::vector<double> stateVariables)
    : AbstractOdeSystem(3)
{
    mpSystemInfo.reset(new CellwiseOdeSystemInformation<DeltaNotchOdeSystem>);

    /**
     * The state variables are as follows:
     *
     * 0 - Notch concentration for this cell
     * 1 - Delta concentration for this cell
     * 2 - average Delta concentration for this cell's immediate neighbours
     *
     * We store the last state variable so that it can be written
     * to file at each time step alongside the others, and visualized.
     */

    SetDefaultInitialCondition(0, 1.0); // soon overwritten
    SetDefaultInitialCondition(1, 1.0); // soon overwritten
    SetDefaultInitialCondition(2, meanDelta);

    if (stateVariables != std::vector<double>())
    {
        SetStateVariables(stateVariables);
    }
}

DeltaNotchOdeSystem::~DeltaNotchOdeSystem()
{
}

void DeltaNotchOdeSystem::EvaluateYDerivatives(double time, const std::vector<double>& rY, std::vector<double>& rDY)
{
    double notch = rY[0];
    double delta = rY[1];
    double mean_delta = rY[2];

    // The next two lines define the ODE system by Collier et al. (1996)
    double dx1 = mean_delta*mean_delta/(0.01 + mean_delta*mean_delta) - notch;
    double dx2 = 1.0/(1.0 + 100.0*notch*notch) - delta;

    rDY[0] = dx1;
    rDY[1] = dx2;
    rDY[2] = 0.0; // don't change the mean Delta level over the course of the mechanics time step
}

template<>
void CellwiseOdeSystemInformation<DeltaNotchOdeSystem>::Initialise()
{
    this->mVariableNames.push_back("Notch");
    this->mVariableUnits.push_back("non-dim");
    this->mInitialConditions.push_back(0.0); // will be filled in later

    this->mVariableNames.push_back("Delta");
    this->mVariableUnits.push_back("non-dim");
    this->mInitialConditions.push_back(0.0); // will be filled in later

    this->mVariableNames.push_back("Mean Delta");
    this->mVariableUnits.push_back("non-dim");
    this->mInitialConditions.push_back(0.0); // will be filled in later

    this->mInitialised = true;
}

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
CHASTE_CLASS_EXPORT(DeltaNotchOdeSystem)
