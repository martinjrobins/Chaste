/*

Copyright (C) University of Oxford, 2005-2011

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

#ifndef ABSTRACTRUSHLARSENCARDIACCELL_HPP_
#define ABSTRACTRUSHLARSENCARDIACCELL_HPP_

#include "ChasteSerialization.hpp"
#include <boost/serialization/base_object.hpp>
#include "ClassIsAbstract.hpp"

#include "AbstractCardiacCell.hpp"
#include "PetscTools.hpp"

/**
 * This is the base class for cardiac cells solved using the Rush-Larsen method.
 * It is based on code contributed by Megan Lewis, University of Saskatchewan
 *
 * The basic approach to solving such models is:
 *  \li Compute alpha & beta values for gating variables, and derivatives for
 *      other state variables.
 *  \li Update the transmembrane potential, either from solving an external PDE,
 *      or using a forward Euler step.
 *  \li Update any elligible gating variables (or similar) with Rush-Larsen scheme.
 *  \li Update the remaining state variables using a forward Euler step.
 */
class AbstractRushLarsenCardiacCell : public AbstractCardiacCell
{
private:
    /** Needed for serialization. */
    friend class boost::serialization::access;
    /**
     * Archive the member variables.
     *
     * @param archive
     * @param version
     */
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        // This calls serialize on the base class.
        archive & boost::serialization::base_object<AbstractCardiacCell>(*this);
    }

public:
    /**
     * Standard constructor for a cell.
     *
     * @param numberOfStateVariables  the size of the ODE system
     * @param voltageIndex  the index of the variable representing the transmembrane
     *     potential within the state variable vector
     * @param pIntracellularStimulus  the intracellular stimulus function
     *
     * Some notes for future reference:
     *  \li It's a pity that inheriting from AbstractCardiacCell forces us to store a
     *      null pointer (for the unused ODE solver) in every instance.  We may want
     *      to revisit this design decision at a later date.
     */
    AbstractRushLarsenCardiacCell(
        unsigned numberOfStateVariables,
        unsigned voltageIndex,
        boost::shared_ptr<AbstractStimulusFunction> pIntracellularStimulus);

    /** Virtual destructor */
    virtual ~AbstractRushLarsenCardiacCell();

    /**
     * Simulates this cell's behaviour between the time interval [tStart, tEnd],
     * with timestep #mDt.  Uses a forward Euler step to update the transmembrane
     * potential at each timestep.
     *
     * The length of the time interval must be a multiple of the timestep.
     *
     * @param tStart  beginning of the time interval to simulate
     * @param tEnd  end of the time interval to simulate
     * @param tSamp  sampling interval for returned results (defaults to #mDt)
     * @return  the values of each state variable, at intervals of tSamp.
     */
    OdeSolution Compute(double tStart, double tEnd, double tSamp=0.0);

    /**
     * Simulates this cell's behaviour between the time interval [tStart, tEnd],
     * with timestep #mDt.  The transmembrane potential is kept fixed throughout,
     * but the other state variables are updated.
     *
     * The length of the time interval must be a multiple of the timestep.
     *
     * @param tStart  beginning of the time interval to simulate
     * @param tEnd  end of the time interval to simulate
     */
    void ComputeExceptVoltage(double tStart, double tEnd);

    /**
     * Simulate this cell's behaviour between the time interval [tStart, tEnd],
     * with timestemp #mDt, updating the internal state variable values.
     *
     * @param tStart  beginning of the time interval to simulate
     * @param tEnd  end of the time interval to simulate
     */
    void SolveAndUpdateState(double tStart, double tEnd);

private:
#define COVERAGE_IGNORE
    /**
     * This function should never be called - the cell class incorporates its own solver.
     *
     * @param time
     * @param rY
     * @param rDY
     */
    void EvaluateYDerivatives(double time, const std::vector<double> &rY, std::vector<double> &rDY)
    {
        NEVER_REACHED;
    }
#undef COVERAGE_IGNORE

protected:
    /**
     * Update the values of elligible gating variables using the Rush-Larsen method,
     * and of other non-V variables using forward Euler, for a single timestep.
     *
     * \note This method must be provided by subclasses.
     *
     * @param rDY  vector containing dy/dt values
     * @param rAlpha  vector containing alpha values
     * @param rBeta  vector containing beta values
     */
    virtual void ComputeOneStepExceptVoltage(const std::vector<double> &rDY,
                                             const std::vector<double> &rAlpha,
                                             const std::vector<double> &rBeta)=0;

    /**
     * Perform a forward Euler step to update the transmembrane potential.
     *
     * @param rDY  vector containing dy/dt values
     */
    void UpdateTransmembranePotential(const std::vector<double> &rDY);

     /**
     * Compute dy/dt and alpha and beta values.
     *
     * \note This method must be provided by subclasses.
     * @param time  start of this timestep
     * @param rDY  vector to fill in with dy/dt values
     * @param rAlpha  vector to fill in with alpha values
     * @param rBeta  vector to fill in with beta values
     */
    virtual void EvaluateEquations(double time,
                                   std::vector<double> &rDY,
                                   std::vector<double> &rAlpha,
                                   std::vector<double> &rBeta)=0;
};

CLASS_IS_ABSTRACT(AbstractRushLarsenCardiacCell)

#endif // ABSTRACTRUSHLARSENCARDIACCELL_HPP_