/*

Copyright (C) University of Oxford, 2008

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
#ifndef ABSTRACTCELLCENTREBASEDTISSUE_HPP_
#define ABSTRACTCELLCENTREBASEDTISSUE_HPP_


#include "AbstractTissue.hpp"

template<unsigned DIM>
class AbstractCellCentreBasedTissue : public AbstractTissue<DIM>
{
private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        // If Archive is an output archive, then '&' resolves to '<<'
        // If Archive is an input archive, then '&' resolves to '>>'
        archive & boost::serialization::base_object<AbstractTissue<DIM> >(*this);
    }

public:

    /**
     * Default constructor.
     */
    AbstractCellCentreBasedTissue(const std::vector<TissueCell>& rCells);

    /**
     * Constructor for use by archiving - doesn't take in cells, since these are dealt
     * with by the serialize method.
     */
    AbstractCellCentreBasedTissue();

    /**
     * Find where the given cell is in space.
     */
    c_vector<double, DIM> GetLocationOfCell(const TissueCell& rCell);

    /**
     * Get a pointer to the node corresponding to a given TissueCell.
     */
    Node<DIM>* GetNodeCorrespondingToCell(const TissueCell& rCell);
    
    /**
     * Add a new cell to the tissue.
     * 
     * @param cell  the cell to add
     * @param newLocation  the position in space at which to put it
     * @returns address of cell as it appears in the cell list
     */
    TissueCell* AddCell(TissueCell cell, c_vector<double,DIM> newLocation);
    
    /**
     * Overridden GetDampingConstant() method.
     *  
     * Get the damping constant for the cell associated with this node,
     * i.e. d in drdt = F/d. This depends on whether using area-based 
     * viscosity has been switched on, and on whether the cell is a mutant 
     * or not.
     * 
     * @param nodeIndex the global index of this node
     * @return the damping constant at the TissueCell associated with this node
     */
    virtual double GetDampingConstant(unsigned nodeIndex);

    /**
     * Write results from the current tissue state to output files.
     * 
     * @param outputCellMutationStates  whether to output cell mutation state results
     * @param outputCellTypes  whether to output cell type results
     * @param outputCellVariables  whether to output cell-cycle variable results
     * @param outputCellCyclePhases  whether to output cell-cycle phase results
     * @param outputCellAncestors  whether to output cell ancestor results
     */
    virtual void WriteResultsToFiles(bool outputCellMutationStates,
                                     bool outputCellTypes,
                                     bool outputCellVariables,
                                     bool outputCellCyclePhases,
                                     bool outputCellAncestors);

};

template<unsigned DIM>
AbstractCellCentreBasedTissue<DIM>::AbstractCellCentreBasedTissue(const std::vector<TissueCell>& rCells)
    : AbstractTissue<DIM>(rCells)
{
}

template<unsigned DIM>
AbstractCellCentreBasedTissue<DIM>::AbstractCellCentreBasedTissue()
    : AbstractTissue<DIM>()
{
}

template<unsigned DIM>
c_vector<double, DIM> AbstractCellCentreBasedTissue<DIM>::GetLocationOfCell(const TissueCell& rCell)
{
    return GetNodeCorrespondingToCell(rCell)->rGetLocation();
}

template<unsigned DIM>
Node<DIM>* AbstractCellCentreBasedTissue<DIM>::GetNodeCorrespondingToCell(const TissueCell& rCell)
{
    unsigned node_index = rCell.GetLocationIndex();
    return this->GetNode(node_index);
}

template<unsigned DIM>
TissueCell* AbstractCellCentreBasedTissue<DIM>::AddCell(TissueCell newCell, c_vector<double,DIM> newLocation)
{
    // Create a new node
    Node<DIM>* p_new_node = new Node<DIM>(this->GetNumNodes(), newLocation, false);   // never on boundary
    unsigned new_node_index = AddNode(p_new_node); // use copy constructor so it doesn't matter that new_node goes out of scope

    // Associate the new cell with the node
    newCell.SetLocationIndex(new_node_index);
    this->mCells.push_back(newCell);

    TissueCell *p_created_cell = &(this->mCells.back());
    this->mLocationCellMap[new_node_index] = p_created_cell;

    return p_created_cell;
}

template<unsigned DIM>
double AbstractCellCentreBasedTissue<DIM>::GetDampingConstant(unsigned nodeIndex)
{
    double damping_multiplier = 1.0;

    if (   (this->rGetCellUsingLocationIndex(nodeIndex).GetMutationState() != HEALTHY) 
        && (this->rGetCellUsingLocationIndex(nodeIndex).GetMutationState() != APC_ONE_HIT) )
    {
        return CancerParameters::Instance()->GetDampingConstantMutant()*damping_multiplier;
    }
    else
    {
        return CancerParameters::Instance()->GetDampingConstantNormal()*damping_multiplier;
    }        
}

template<unsigned DIM>
void AbstractCellCentreBasedTissue<DIM>::WriteResultsToFiles(bool outputCellMutationStates,
                                                             bool outputCellTypes,
                                                             bool outputCellVariables,
                                                             bool outputCellCyclePhases,
                                                             bool outputCellAncestors)
{
    std::vector<unsigned> cell_type_counter, cell_mutation_state_counter, cell_cycle_phase_counter;

    this->WriteTimeAndNodeResultsToFiles(outputCellMutationStates,
                                         outputCellTypes,
                                         outputCellVariables,
                                         outputCellCyclePhases,
                                         outputCellAncestors,
                                         cell_type_counter,
                                         cell_mutation_state_counter,
                                         cell_cycle_phase_counter);

    for (unsigned node_index=0; node_index<this->GetNumNodes(); node_index++)
    {
        if ( !(this->GetNode(node_index)->IsDeleted()) )
        {
            this->GenerateCellResults(node_index,
                                      outputCellMutationStates,
                                      outputCellTypes,
                                      outputCellVariables,
                                      outputCellCyclePhases,
                                      outputCellAncestors,
                                      cell_type_counter,
                                      cell_mutation_state_counter,
                                      cell_cycle_phase_counter);
        }        
    }
    
    this->WriteCellResultsToFiles(outputCellMutationStates,
                                  outputCellTypes,
                                  outputCellVariables,
                                  outputCellCyclePhases,
                                  outputCellAncestors,
                                  cell_type_counter,
                                  cell_mutation_state_counter,
                                  cell_cycle_phase_counter);
}


#endif /*ABSTRACTCELLCENTREBASEDTISSUE_HPP_*/
