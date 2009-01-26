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
#ifndef TYSONNOVAKCELLCYCLEMODEL_HPP_
#define TYSONNOVAKCELLCYCLEMODEL_HPP_

#include <boost/serialization/access.hpp>

#include <iostream>

#include "AbstractOdeBasedCellCycleModel.hpp"
#include "TysonNovak2001OdeSystem.hpp"
#include "BackwardEulerIvpOdeSolver.hpp"
#include "CvodeAdaptor.hpp"
#include "Exception.hpp"

// Needs to be included last
#include <boost/serialization/export.hpp>

/**
 *  Tyson-Novak 2001 cell cycle model, taken from the version at  doi:10.1006/jtbi.2001.2293
 *
 *  Note that this is not a model for murine or human colonic-cell cycling, but is
 *  included in chaste as one of the most commonly known ODE based cell cycle models.
 *
 *  Time taken to progress through the cycle is deterministic and given by
 *  an ODE system independent of external factors.
 *
 *  Note that this class uses C++'s default copying semantics, and so doesn't implement a copy constructor
 *  or operator=.
 */
class TysonNovakCellCycleModel : public AbstractOdeBasedCellCycleModel
{
private:

#ifdef CHASTE_CVODE
    static CvodeAdaptor msSolver;
#else
    static BackwardEulerIvpOdeSolver msSolver;
#endif  //CHASTE_CVODE

    /**
    * A private constructor for daughter cells called only by the CreateDaughterCellCycleModel function
     *
    * @param parentProteinConcentrations a std::vector of doubles of the protein concentrations
    * @param birthTime the SimulationTime when the cell divided (birth time of parent cell)
    * @param generation the cell's generation
    */
    TysonNovakCellCycleModel(std::vector<double> parentProteinConcentrations,
                             double divideTime,
                             unsigned generation);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & boost::serialization::base_object<AbstractOdeBasedCellCycleModel>(*this);
    }

public:

    /**
     * Default constructor.
     */
    TysonNovakCellCycleModel();

    /**
     * Reset cell cycle model by calling AbstractOdeBasedCellCycleModel::ResetForDivision() 
     * and setting initial conditions for protein concentrations.
     */
    void ResetForDivision();

    /**
     * Returns a new TysonNovakCellCycleModel, created with the correct 
     * initial conditions.
     *
     * This method should be called just after the parent cell cycle model 
     * has been reset.
     * 
     * @return pointer to the daughter cell cycle model
     */
    AbstractCellCycleModel* CreateDaughterCellCycleModel();

    /**
     * Solve the ODEs up to the current time and return whether a stopping event occurred.
     * 
     * @param currentTime the current time
     * @return whether a stopping event occured
     */
    bool SolveOdeToTime(double currentTime);

    /**
     * Get the time at which the ODE stopping event occured.
     * 
     * @return the stopping event time
     */
    double GetOdeStopTime();

    /**
     * Get the duration of the cell's S phase.
     */
    double GetSDuration();

    /**
     * Get the duration of the cell's G2 phase.
     */
    double GetG2Duration();
    
    /**
     * Get the duration of the cell's M phase.
     */
    double GetMDuration();

    /**
     * If the daughter cell type is stem, change it to transit.
     */
    void InitialiseDaughterCell();

};


// Declare identifier for the serializer
BOOST_CLASS_EXPORT(TysonNovakCellCycleModel)


#endif /*TYSONNOVAKCELLCYCLEMODEL_HPP_*/
