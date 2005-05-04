/**
 * Concrete EulerIvpOdeSolver class. 
*/
#ifndef _EULERIVPODESOLVER_HPP_
#define _EULERIVPODESOLVER_HPP_

#include "AbstractOneStepIvpOdeSolver.hpp"
#include "AbstractOdeSystem.hpp"
#include "OdeSolution.hpp"

#include <vector>

class EulerIvpOdeSolver : public AbstractOneStepIvpOdeSolver
{
	public:
	EulerIvpOdeSolver() {}; //do nothing
				      
	std::vector<double> CalculateNextYValue(AbstractOdeSystem* pAbstractOdeSystem,
											double timeStep,
											double time,
											std::vector<double> currentYValue);
	
};

#endif //_EULERIVPODESOLVER_HPP_

