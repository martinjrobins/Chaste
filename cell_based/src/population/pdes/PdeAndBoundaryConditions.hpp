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

#ifndef PDEANDBOUNDARYCONDITIONS_HPP_
#define PDEANDBOUNDARYCONDITIONS_HPP_

#include "AbstractLinearEllipticPde.hpp"
#include "AveragedSourcePde.hpp"
#include "AbstractBoundaryCondition.hpp"
#include "PetscTools.hpp"

/**
 * A helper class for use in CellBasedSimulationWithPdes. The class
 * contains a pointer to a linear elliptic PDE, which is to be solved
 * on the domain defined by the cell population. The class also contains
 * information describing the boundary condition that is to be imposed
 * when solving the PDE. Currently we allow Neumann (imposed flux) or
 * Dirichlet (imposed value) boundary conditions. The boundary condition
 * may be constant on the boundary or vary spatially and/or temporally.
 */
template<unsigned DIM>
class PdeAndBoundaryConditions
{
private:

    /**
     * Pointer to a linear elliptic PDE object.
     */
    AbstractLinearEllipticPde<DIM,DIM>* mpPde;

    /**
     * AbstractBoundaryCondition<SPACE_DIM>
     */
    AbstractBoundaryCondition<DIM>* mpBoundaryCondition;

    /**
     * Whether the boundary condition is Neumann (false
     * corresponds to a Dirichlet boundary condition).
     */
    bool mIsNeumannBoundaryCondition;

    /**
     * Current solution to the PDE problem, for use as an initial guess
     * when solving at the next time step.
     */
    Vec mCurrentSolution;

public:

    /**
     * Constructor.
     *
     * @param pPde A pointer to a linear elliptic PDE object
     * @param pBoundaryCondition A pointer to an abstract boundary condition
     *                           (defaults to NULL, corresponding to a constant boundary condition with value zero)
     * @param isNeumannBoundaryCondition Whether the boundary condition is Neumann (defaults to true)
     */
    PdeAndBoundaryConditions(AbstractLinearEllipticPde<DIM,DIM>* pPde,
                             AbstractBoundaryCondition<DIM>* pBoundaryCondition=NULL,
                             bool isNeumannBoundaryCondition=true);

    /**
     * Destructor.
     */
    ~PdeAndBoundaryConditions();

    /**
     * @return mpPde
     */
    AbstractLinearEllipticPde<DIM,DIM>* GetPde();

    /**
     * @return mpBoundaryCondition
     */
    AbstractBoundaryCondition<DIM>* GetBoundaryCondition() const;

    /**
     * @return mCurrentSolution
     */
    Vec GetSolution();

    /**
     * Set mCurrentSolution.
     *
     * @param solution the current solution
     */
    void SetSolution(Vec solution);

    /**
     * @return mIsNeumannBoundaryCondition
     */
    bool IsNeumannBoundaryCondition();

    /**
     * @return whether the PDE is of type AveragedSourcePde
     */
    bool HasAveragedSourcePde();

    /**
     * Call VecDestroy on mCurrentSolution.
     */
    void DestroySolution();

    /**
     * In the case where mpPde is of type AveragedSourcePde, set the source terms
     * using the information in the given mesh.
     *
     * @param pMesh Pointer to a tetrahedral mesh
     */
    void SetUpSourceTermsForAveragedSourcePde(TetrahedralMesh<DIM,DIM>* pMesh);
};

#endif /* PDEANDBOUNDARYCONDITIONS_HPP_ */
