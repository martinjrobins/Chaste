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


#ifndef _ODETHIRDORDERWITHEVENTS_HPP
#define _ODETHIRDORDERWITHEVENTS_HPP

#include "AbstractOdeSystem.hpp"
#include "OdeSystemInformation.hpp"

/**
  * Concrete OdeThirdOrder class with events
  */
class OdeThirdOrderWithEvents : public AbstractOdeSystem
{
public :
    OdeThirdOrderWithEvents()
            : AbstractOdeSystem(3) // 3 here is the number of unknowns
    {
        mpSystemInfo = OdeSystemInformation<OdeThirdOrderWithEvents>::Instance();
    }

    void EvaluateYDerivatives(double time, const std::vector<double> &rY, std::vector<double>& rDY)
    {
        rDY[0]=rY[0]-rY[1]+rY[2];
        rDY[1]=rY[1]-rY[2];
        rDY[2]=2*rY[1]-rY[2];
    }

    bool CalculateStoppingEvent(double time, const std::vector<double> &rY)
    {
        return (rY[0]<-0.5);
    }
};

template<>
void OdeSystemInformation<OdeThirdOrderWithEvents>::Initialise()
{
    this->mVariableNames.push_back("Variable 1");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.0);
    
    this->mVariableNames.push_back("Variable 2");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(1.0);
    
    this->mVariableNames.push_back("Variable 3");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.0);
    
    this->mInitialised = true;
}

#endif //_ODETHIRDORDERWITHEVENTS_HPP
