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

#ifndef ABSTRACTCELLCYCLEMODELODESOLVER_HPP_
#define ABSTRACTCELLCYCLEMODELODESOLVER_HPP_

#include <boost/shared_ptr.hpp>
#include "AbstractIvpOdeSolver.hpp"

/**
 * An abstract class which provides access to information about a particular
 * cell cycle model ODE solver *class* (as opposed to an instance).
 *
 * \todo list the information available
 *
 * This class requires a subclass defining the Initialise method in order to set
 * up the information.  Developers may do this by defining their own subclass, but
 * the most convenient method is likely to be to use the CellCycleModelOdeSolver
 * class, which is a templated singleton subclass of this AbstractCellCycleModelOdeSolver
 * class.  See its documentation for details of how to use it.
 * 
 * \todo implement archiving (see #1427)
 */
class AbstractCellCycleModelOdeSolver
{
protected:

    /** The ODE solver. */
    boost::shared_ptr<AbstractIvpOdeSolver> mpOdeSolver;

    /** The size of the ODE system to be solved. */
    unsigned mSizeOfOdeSystem;

public:

    /**
     * Constructor.
     */
    AbstractCellCycleModelOdeSolver();

    /**
     * Virtual destructor since we have virtual methods.
     */
    virtual ~AbstractCellCycleModelOdeSolver();

    /**
     * Is the instance in existence and fully set up.
     */
    virtual bool IsSetUp()=0;

    /**
     * Reset the instance.
     */
    virtual void Reset();

    /**
     * Call mpOdeSolver->SolveAndUpdateStateVariable.
     *
     * @param pAbstractOdeSystem  pointer to the concrete ODE system to be solved
     * @param startTime  the time at which the initial conditions are specified
     * @param endTime  the time to which the system should be solved and the solution
     *                 returned
     * @param timeStep  the time interval to be used by the solver
     */
    void SolveAndUpdateStateVariable(AbstractOdeSystem* pAbstractOdeSystem,
                                     double startTime,
                                     double endTime,
                                     double timeStep);

    /**
     * Determine whether the solver quit due to the ODE's stopping event
     * triggering
     */
    bool StoppingEventOccurred();

    /**
     * Call mpOdeSolver->GetStoppingTime.
     *
     * @return mStoppingTime.
     */
    double GetStoppingTime();

    /**
     * Set method for mSizeOfOdeSystem.
     * 
     * @param sizeOfOdeSystem the new value of mSizeOfOdeSystem
     */
    void SetSizeOfOdeSystem(unsigned sizeOfOdeSystem);

    /**
     * If using CVODE, make the solver check for stopping events using CVODE's rootfinding functionality
     * (by default we do not check).
     */
    void CheckForStoppingEvents();

    /**
     * If using CVODE, change the maximum number of steps to be taken by the solver in its attempt to reach
     * the next output time (default is 500).
     *
     * @param numSteps the new maximum number of steps
     */
    void SetMaxSteps(long int numSteps);

    /**
     * If using CVODE, set relative and absolute tolerances; both scalars.
     * If no parameters are given, tolerances will be reset to default values.
     *
     * @param relTol the relative tolerance for the solver
     * @param absTol the absolute tolerance for the solver
     */
    void SetTolerances(double relTol=1e-4, double absTol=1e-6);
};

#endif /*ABSTRACTCELLCYCLEMODELODESOLVER_HPP_*/
