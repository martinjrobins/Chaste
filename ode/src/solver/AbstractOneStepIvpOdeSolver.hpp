/**
 * Abstract One Step Ode Solver class. Sets up variables and functions for all the ODE solvers
 * that only have one timestep.
*/
#ifndef _ABSTRACTONESTEPIVPODESOLVER_HPP_
#define _ABSTRACTONESTEPIVPODESOLVER_HPP_

#include "AbstractIvpOdeSolver.hpp"

class AbstractOneStepIvpOdeSolver : public AbstractIvpOdeSolver
{
protected:
    /**
     * Method that actually performs the solving on behalf of the public Solve methods.
     * 
     * @param pAbstractOdeSystem  the system to solve
     * @param rCurrentYValues  the current (initial) state; results will also be returned
     *     in here
     * @param rWorkingMemory  working memory; same size as rCurrentYValues
     * @param startTime  initial time
     * @param endTime  time to solve to
     * @param timeStep  dt
     */
    virtual void InternalSolve(AbstractOdeSystem* pAbstractOdeSystem,
                               std::vector<double>& rCurrentYValues,
                               std::vector<double>& rWorkingMemory,
                               double startTime,
                               double endTime,
                               double timeStep);
                               
    /**
     * Calculate the next time step.  Concrete subclasses should provide this method.
     */
    virtual void CalculateNextYValue(AbstractOdeSystem* pAbstractOdeSystem,
                                     double timeStep,
                                     double time,
                                     std::vector<double>& currentYValues,
                                     std::vector<double>& nextYValues)=0;
                                     
public:
    /**
     * Solves a system of ODEs using a specified one-step ODE solver
     * 
     * @param pAbstractOdeSystem points to the concrete ODE system to be solved
     * 
     * @param rYValues a standard vector specifying the intial condition 
     * of each solution variable in the system (this can be the initial
     * conditions vector stored in the ode system)
     * 
     * @param startTime the time at which the initial conditions are specified
     * 
     * @param endTime the time to which the system should be solved and the solution 
     * returned
     * 
     * @param timeStep the time interval to be used by the solver
     * 
     * @param timeSampling the times at which the solution is returned
     * 
     * @return OdeSolution is an object containing an integer of the number of 
     * equations, a std::vector of times and a std::vector of std::vectors where 
     * each of those vectors contains the solution for one variable of the ODE 
     * system at those times.
     * 
     * EXAMPLE, which returns the solution every 0.1 seconds using dt=0.01:
     * 
     * MyOdeSystem ode;
     * 
     * std::vector<double> init_cond = ode_system.GetInitialConditions();
     * 
     * OdeSolution solution = solver.Solve(&ode, init_cond, 0, 1, 0.01, 0.1);
     *   
     */
    virtual OdeSolution Solve(AbstractOdeSystem* pAbstractOdeSystem,
                              std::vector<double>& rYValues,
                              double startTime,
                              double endTime,
                              double timeStep,
                              double timeSampling);
                              
                              
    /**
     * Second version of Solve. See comments for the first version of Solve. 
     * This method does not return the solution and therefore does not take 
     * in a sampling time. Instead, the mStateVariables component in the 
     * ode object is updated.
     * 
     * @param pAbstractOdeSystem points to the concrete ODE system to be solved
     * 
     * @param rYValues a standard vector specifying the intial condition 
     * of each solution variable in the system (this can be the initial
     * conditions vector stored in the ode system)
     * 
     * @param startTime the time at which the initial conditions are specified
     * 
     * @param endTime the time to which the system should be solved and the solution 
     * returned
     * 
     * @param timeStep the time interval to be used by the solver
     * 
     * EXAMPLE:
     *    
     * std::vector<double> init_cond = ode_system.GetInitialConditions();
     * 
     * solver.Solve(&ode, init_cond, 0, 1, 0.01);
     * 
     * state_variables = ode_system.rGetStateVariables(); // solution at t=1 found here
     * 
     */
    virtual void Solve(AbstractOdeSystem* pAbstractOdeSystem,
                       std::vector<double>& rYValues,
                       double startTime,
                       double endTime,
                       double timeStep);
                       
    virtual ~AbstractOneStepIvpOdeSolver()
    {}
};

#endif //_ABSTRACTONESTEPIVPODESOLVER_HPP_
