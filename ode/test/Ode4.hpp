/*
 * Concrete Ode4 class
 */
#ifndef _ODE4_HPP_
#define _ODE4_HPP_
#include "AbstractOdeSystem.hpp"


class Ode4 : public AbstractOdeSystem
{
public :
    Ode4() : AbstractOdeSystem(1)  // 1 here is the number of unknowns
    {
        mInitialConditions.push_back(0.5);
    }
    
    void EvaluateYDerivatives(double time, const std::vector<double> &rY, std::vector<double>& rDY)
    {
        double alpha = 100;
        rDY[0]=alpha*rY[0]*(1-rY[0])*time;
    }
    
};

#endif //_ODE4_HPP_

