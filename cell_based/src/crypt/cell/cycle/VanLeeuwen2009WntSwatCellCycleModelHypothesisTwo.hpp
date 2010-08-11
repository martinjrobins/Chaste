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

#ifndef VANLEEUWEN2009WNTSWATCELLCYCLEMODELHYPOTHESISTWO_HPP_
#define VANLEEUWEN2009WNTSWATCELLCYCLEMODELHYPOTHESISTWO_HPP_

#include "AbstractVanLeeuwen2009WntSwatCellCycleModel.hpp"

/**
 * Concrete Van Leeuwen 2009 cell cycle model, using hypothesis two (see paper).
 */
class VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo
    : public AbstractVanLeeuwen2009WntSwatCellCycleModel
{
private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & boost::serialization::base_object<AbstractVanLeeuwen2009WntSwatCellCycleModel>(*this);
    }

public:

    /**
     * Default constructor calls base class.
     * 
     * @param pOdeSolver An optional pointer to a cell cycle model ODE solver object (allows the use of different ODE solvers)
     */
    VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo(boost::shared_ptr<AbstractCellCycleModelOdeSolver> pOdeSolver = boost::shared_ptr<AbstractCellCycleModelOdeSolver>());

    /**
     *  Overloaded method which allocates the ODE system using HYPOTHESIS TWO.
     *
     *  @param wntConcentration Wnt concentration
     *  @param pMutationState Mutation state
     */
    void InitialiseOdeSystem(double wntConcentration, boost::shared_ptr<AbstractCellMutationState> pMutationState);

    /**
     * Overridden builder method to create new copies of
     * this cell cycle model.
     */
    AbstractCellCycleModel* CreateCellCycleModel();
};

// Declare identifier for the serializer
#include "SerializationExportWrapper.hpp"
CHASTE_CLASS_EXPORT(VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo)

namespace boost
{
namespace serialization
{
/**
 * Allow us to not need a default constructor, by specifying how Boost should
 * instantiate a VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo instance.
 */
template<class Archive>
inline void save_construct_data(
    Archive & ar, const VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo * t, const unsigned int file_version)
{
    const boost::shared_ptr<AbstractCellCycleModelOdeSolver> p_ode_solver = t->GetOdeSolver();
    ar & p_ode_solver;
}

/**
 * Allow us to not need a default constructor, by specifying how Boost should
 * instantiate a VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo instance.
 */
template<class Archive>
inline void load_construct_data(
    Archive & ar, VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo * t, const unsigned int file_version)
{
    boost::shared_ptr<AbstractCellCycleModelOdeSolver> p_ode_solver;
    ar & p_ode_solver;

    ::new(t)VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo(p_ode_solver);
}
}
} // namespace

#include "CellCycleModelOdeSolverExportWrapper.hpp"
EXPORT_CELL_CYCLE_MODEL_ODE_SOLVER(VanLeeuwen2009WntSwatCellCycleModelHypothesisTwo)

#endif /* VANLEEUWEN2009WNTSWATCELLCYCLEMODELHYPOTHESISTWO_HPP_ */
