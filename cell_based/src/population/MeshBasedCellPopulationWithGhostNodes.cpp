/*

Copyright (c) 2005-2012, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "MeshBasedCellPopulationWithGhostNodes.hpp"
#include "CellwiseData.hpp"

template<unsigned DIM>
MeshBasedCellPopulationWithGhostNodes<DIM>::MeshBasedCellPopulationWithGhostNodes(
     MutableMesh<DIM, DIM>& rMesh,
     std::vector<CellPtr>& rCells,
     const std::vector<unsigned> locationIndices,
     bool deleteMesh,
     double ghostSpringStiffness)
             : MeshBasedCellPopulation<DIM>(rMesh, rCells, locationIndices, deleteMesh, false), // do not call the base class Validate()
               mGhostSpringStiffness(ghostSpringStiffness)
{
    if (!locationIndices.empty())
    {
        // Create a set of node indices corresponding to ghost nodes
        std::set<unsigned> node_indices;
        std::set<unsigned> location_indices;
        std::set<unsigned> ghost_node_indices;

        for (unsigned i=0; i<this->GetNumNodes(); i++)
        {
            node_indices.insert(this->GetNode(i)->GetIndex());
        }
        for (unsigned i=0; i<locationIndices.size(); i++)
        {
            location_indices.insert(locationIndices[i]);
        }

        std::set_difference(node_indices.begin(), node_indices.end(),
                            location_indices.begin(), location_indices.end(),
                            std::inserter(ghost_node_indices, ghost_node_indices.begin()));

        // This method finishes and then calls Validate()
        SetGhostNodes(ghost_node_indices);
    }
    else
    {
        this->mIsGhostNode = std::vector<bool>(this->GetNumNodes(), false);
        Validate();
    }
}

template<unsigned DIM>
MeshBasedCellPopulationWithGhostNodes<DIM>::MeshBasedCellPopulationWithGhostNodes(MutableMesh<DIM, DIM>& rMesh,
                                                                  double ghostSpringStiffness)
             : MeshBasedCellPopulation<DIM>(rMesh),
               mGhostSpringStiffness(ghostSpringStiffness)
{
}

template<unsigned DIM>
std::vector<bool>& MeshBasedCellPopulationWithGhostNodes<DIM>::rGetGhostNodes()
{
    return this->mIsGhostNode;
}

template<unsigned DIM>
bool MeshBasedCellPopulationWithGhostNodes<DIM>::IsGhostNode(unsigned index)
{
    return this->mIsGhostNode[index];
}

template<unsigned DIM>
std::set<unsigned> MeshBasedCellPopulationWithGhostNodes<DIM>::GetGhostNodeIndices()
{
    std::set<unsigned> ghost_node_indices;
    for (unsigned i=0; i<this->mIsGhostNode.size(); i++)
    {
        if (this->mIsGhostNode[i])
        {
            ghost_node_indices.insert(i);
        }
    }
    return ghost_node_indices;
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::SetGhostNodes(const std::set<unsigned>& rGhostNodeIndices)
{
    // Reinitialise all entries of mIsGhostNode to false
    this->mIsGhostNode = std::vector<bool>(this->mrMesh.GetNumNodes(), false);

    // Update mIsGhostNode
    for (std::set<unsigned>::iterator iter=rGhostNodeIndices.begin(); iter!=rGhostNodeIndices.end(); ++iter)
    {
        this->mIsGhostNode[*iter] = true;
    }

    Validate();
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::UpdateGhostPositions(double dt)
{
    // Initialise vector of forces on ghost nodes
    std::vector<c_vector<double, DIM> > drdt(this->GetNumNodes());
    for (unsigned i=0; i<drdt.size(); i++)
    {
        drdt[i] = zero_vector<double>(DIM);
    }

    // Calculate forces on ghost nodes
    for (typename MutableMesh<DIM, DIM>::EdgeIterator edge_iterator = static_cast<MutableMesh<DIM, DIM>&>((this->mrMesh)).EdgesBegin();
        edge_iterator != static_cast<MutableMesh<DIM, DIM>&>((this->mrMesh)).EdgesEnd();
        ++edge_iterator)
    {
        unsigned nodeA_global_index = edge_iterator.GetNodeA()->GetIndex();
        unsigned nodeB_global_index = edge_iterator.GetNodeB()->GetIndex();

        c_vector<double, DIM> force = CalculateForceBetweenGhostNodes(nodeA_global_index, nodeB_global_index);

        double damping_constant = this->GetDampingConstantNormal();

        if (!this->mIsGhostNode[nodeA_global_index])
        {
            drdt[nodeB_global_index] -= force / damping_constant;
        }
        else
        {
            drdt[nodeA_global_index] += force / damping_constant;

            if (this->mIsGhostNode[nodeB_global_index])
            {
                drdt[nodeB_global_index] -= force / damping_constant;
            }
        }
    }

    for (typename AbstractMesh<DIM,DIM>::NodeIterator node_iter = this->mrMesh.GetNodeIteratorBegin();
         node_iter != this->mrMesh.GetNodeIteratorEnd();
         ++node_iter)
    {
        unsigned node_index = node_iter->GetIndex();
        if (this->mIsGhostNode[node_index])
        {
            ChastePoint<DIM> new_point(node_iter->rGetLocation() + dt*drdt[node_index]);
            static_cast<MutableMesh<DIM, DIM>&>((this->mrMesh)).SetNode(node_index, new_point, false);
        }
    }
}

template<unsigned DIM>
c_vector<double, DIM> MeshBasedCellPopulationWithGhostNodes<DIM>::CalculateForceBetweenGhostNodes(const unsigned& rNodeAGlobalIndex, const unsigned& rNodeBGlobalIndex)
{
    assert(rNodeAGlobalIndex != rNodeBGlobalIndex);
    c_vector<double, DIM> unit_difference;
    c_vector<double, DIM> node_a_location = this->GetNode(rNodeAGlobalIndex)->rGetLocation();
    c_vector<double, DIM> node_b_location = this->GetNode(rNodeBGlobalIndex)->rGetLocation();

    // There is reason not to subtract one position from the other (cylindrical meshes)
    unit_difference = this->mrMesh.GetVectorFromAtoB(node_a_location, node_b_location);

    double distance_between_nodes = norm_2(unit_difference);

    unit_difference /= distance_between_nodes;

    double rest_length = 1.0;

    return mGhostSpringStiffness * unit_difference * (distance_between_nodes - rest_length);
}

template<unsigned DIM>
CellPtr MeshBasedCellPopulationWithGhostNodes<DIM>::AddCell(CellPtr pNewCell, const c_vector<double,DIM>& rCellDivisionVector, CellPtr pParentCell)
{
    // Add new cell to cell population
    CellPtr p_created_cell = MeshBasedCellPopulation<DIM>::AddCell(pNewCell, rCellDivisionVector, pParentCell);
    assert(p_created_cell == pNewCell);

    // Update size of mIsGhostNode if necessary
    unsigned new_node_index = this->mCellLocationMap[p_created_cell.get()];

    if (this->GetNumNodes() > this->mIsGhostNode.size())
    {
        this->mIsGhostNode.resize(this->GetNumNodes());
        this->mIsGhostNode[new_node_index] = false;
    }

    // Return pointer to new cell
    return p_created_cell;
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::Validate()
{
    // Get a list of all the nodes that are ghosts
    std::vector<bool> validated_node = mIsGhostNode;
    assert(mIsGhostNode.size()==this->GetNumNodes());

    // Look through all of the cells and record what node they are associated with.
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter=this->Begin(); cell_iter!=this->End(); ++cell_iter)
    {
        unsigned node_index = this->mCellLocationMap[(*cell_iter).get()];

        // If the node attached to this cell is labelled as a ghost node, then throw an error
        if (mIsGhostNode[node_index])
        {
            EXCEPTION("Node " << node_index << " is labelled as a ghost node and has a cell attached");
        }
        validated_node[node_index] = true;
    }

    for (unsigned i=0; i<validated_node.size(); i++)
    {
        if (!validated_node[i])
        {
            EXCEPTION("Node " << i << " does not appear to be a ghost node or have a cell associated with it");
        }
    }
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::UpdateGhostNodesAfterReMesh(NodeMap& rMap)
{
    // Copy mIsGhostNode to a temporary vector
    std::vector<bool> ghost_nodes_before_remesh = mIsGhostNode;

    // Reinitialise mIsGhostNode
    mIsGhostNode.clear();
    mIsGhostNode.resize(this->GetNumNodes());

    // Update mIsGhostNode using the node map
    for (unsigned old_index=0; old_index<rMap.Size(); old_index++)
    {
        if (!rMap.IsDeleted(old_index))
        {
            unsigned new_index = rMap.GetNewIndex(old_index);
            mIsGhostNode[new_index] = ghost_nodes_before_remesh[old_index];
        }
    }
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::UpdateNodeLocations(const std::vector< c_vector<double, DIM> >& rNodeForces, double dt)
{
    // First update ghost positions first because they do not affect the real cells
    UpdateGhostPositions(dt);

    // Then call the base class method
    AbstractCentreBasedCellPopulation<DIM>::UpdateNodeLocations(rNodeForces, dt);
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::WriteVtkResultsToFile()
{
#ifdef CHASTE_VTK
    if (this->mpVoronoiTessellation != NULL)
    {
        VertexMeshWriter<DIM, DIM> mesh_writer(this->mDirPath, "results", false);
        std::stringstream time;
        time << SimulationTime::Instance()->GetTimeStepsElapsed();

        unsigned num_elements = this->mpVoronoiTessellation->GetNumElements();
        std::vector<double> ghosts(num_elements);
        std::vector<double> cell_types(num_elements);
        std::vector<double> cell_ancestors(num_elements);
        std::vector<double> cell_mutation_states(num_elements);
        std::vector<double> cell_ages(num_elements);
        std::vector<double> cell_cycle_phases(num_elements);
        std::vector<double> cell_volumes(num_elements);
        std::vector<std::vector<double> > cellwise_data;

        // This code is commented code is because Cellwise Data can't deal with ghost nodes see #1975
        assert(!CellwiseData<DIM>::Instance()->IsSetUp());
        //if (CellwiseData<DIM>::Instance()->IsSetUp())
        //{
        //    CellwiseData<DIM>* p_data = CellwiseData<DIM>::Instance();
        //    unsigned num_variables = p_data->GetNumVariables();
        //    for (unsigned var=0; var<num_variables; var++)
        //    {
        //        std::vector<double> cellwise_data_var(num_elements);
        //        cellwise_data.push_back(cellwise_data_var);
        //    }
        //}

        // Loop over Voronoi elements
        for (typename VertexMesh<DIM,DIM>::VertexElementIterator elem_iter = this->mpVoronoiTessellation->GetElementIteratorBegin();
             elem_iter != this->mpVoronoiTessellation->GetElementIteratorEnd();
             ++elem_iter)
        {
            // Get index of this element in the Voronoi tessellation mesh
            unsigned elem_index = elem_iter->GetIndex();

            unsigned node_index = this->mpVoronoiTessellation->GetDelaunayNodeIndexCorrespondingToVoronoiElementIndex(elem_index);

            ghosts[elem_index] = (double)(this->IsGhostNode(node_index));

            if (!this->IsGhostNode(node_index))
            {
                // Get the cell corresponding to this element
                CellPtr p_cell = this->GetCellUsingLocationIndex(node_index);

                if (this->mOutputCellAncestors)
                {
                    double ancestor_index = (p_cell->GetAncestor() == UNSIGNED_UNSET) ? (-1.0) : (double)p_cell->GetAncestor();
                    cell_ancestors[elem_index] = ancestor_index;
                }
                if (this->mOutputCellProliferativeTypes)
                {
                    double cell_type = p_cell->GetCellCycleModel()->GetCellProliferativeType();
                    cell_types[elem_index] = cell_type;
                }
                if (this->mOutputCellMutationStates)
                {
                    double mutation_state = p_cell->GetMutationState()->GetColour();
                    cell_mutation_states[elem_index] = mutation_state;
                }
                if (this->mOutputCellAges)
                {
                    double age = p_cell->GetAge();
                    cell_ages[elem_index] = age;
                }
                if (this->mOutputCellCyclePhases)
                {
                    double cycle_phase = p_cell->GetCellCycleModel()->GetCurrentCellCyclePhase();
                    cell_cycle_phases[elem_index] = cycle_phase;
                }
                if (this->mOutputCellVolumes)
                {
                    double cell_volume = this->mpVoronoiTessellation->GetVolumeOfElement(elem_index);
                    cell_volumes[elem_index] = cell_volume;
                }
                // This code is commented  because Cellwise Data can't deal with ghost nodes see #1975
                assert(!CellwiseData<DIM>::Instance()->IsSetUp());
                //if (CellwiseData<DIM>::Instance()->IsSetUp())
                //{
                //    CellwiseData<DIM>* p_data = CellwiseData<DIM>::Instance();
                //    unsigned num_variables = p_data->GetNumVariables();
                //    for (unsigned var=0; var<num_variables; var++)
                //    {
                //        cellwise_data[var][elem_index] = p_data->GetValue(p_cell, var);
                //    }
                //}
            }
            else
            {
                if (this->mOutputCellAncestors)
                {
                    cell_ancestors[elem_index] = -1.0;
                }
                if (this->mOutputCellProliferativeTypes)
                {
                    cell_types[elem_index] = -1.0;
                }
                if (this->mOutputCellMutationStates)
                {
                    cell_mutation_states[elem_index] = -1.0;
                }
                if (this->mOutputCellAges)
                {
                    cell_ages[elem_index] = -1.0;
                }
                if (this->mOutputCellCyclePhases)
                {
                    cell_cycle_phases[elem_index] = -1.0;
                }
                if (this->mOutputCellVolumes)
                {
                    cell_volumes[elem_index] = -1.0;
                }
            }
        }

        mesh_writer.AddCellData("Non-ghosts", ghosts);
        if (this->mOutputCellProliferativeTypes)
        {
            mesh_writer.AddCellData("Cell types", cell_types);
        }
        if (this->mOutputCellAncestors)
        {
            mesh_writer.AddCellData("Ancestors", cell_ancestors);
        }
        if (this->mOutputCellMutationStates)
        {
            mesh_writer.AddCellData("Mutation states", cell_mutation_states);
        }
        if (this->mOutputCellAges)
        {
            mesh_writer.AddCellData("Ages", cell_ages);
        }
        if (this->mOutputCellCyclePhases)
        {
            mesh_writer.AddCellData("Cycle phases", cell_cycle_phases);
        }
        if (this->mOutputCellVolumes)
        {
            mesh_writer.AddCellData("Cell volumes", cell_volumes);
        }
        // This code is commented code is because Cellwise Data can't deal with ghost nodes see #1975
        assert(!CellwiseData<DIM>::Instance()->IsSetUp());
//if (CellwiseData<DIM>::Instance()->IsSetUp())
//{
//	for (unsigned var=0; var<cellwise_data.size(); var++)
//	{
//		std::stringstream data_name;
//		data_name << "Cellwise data " << var;
//		std::vector<double> cellwise_data_var = cellwise_data[var];
//		mesh_writer.AddCellData(data_name.str(), cellwise_data_var);
//	}
//}

        mesh_writer.WriteVtkUsingMesh(*(this->mpVoronoiTessellation), time.str());
        *(this->mpVtkMetaFile) << "        <DataSet timestep=\"";
        *(this->mpVtkMetaFile) << SimulationTime::Instance()->GetTimeStepsElapsed();
        *(this->mpVtkMetaFile) << "\" group=\"\" part=\"0\" file=\"results_";
        *(this->mpVtkMetaFile) << SimulationTime::Instance()->GetTimeStepsElapsed();
        *(this->mpVtkMetaFile) << ".vtu\"/>\n";
    }
#endif //CHASTE_VTK
}

template<unsigned DIM>
void MeshBasedCellPopulationWithGhostNodes<DIM>::OutputCellPopulationParameters(out_stream& rParamsFile)
{
    *rParamsFile << "\t\t<GhostSpringStiffness>" << mGhostSpringStiffness << "</GhostSpringStiffness>\n";

    // Call method on direct parent class
    MeshBasedCellPopulation<DIM>::OutputCellPopulationParameters(rParamsFile);
}

/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////

template class MeshBasedCellPopulationWithGhostNodes<1>;
template class MeshBasedCellPopulationWithGhostNodes<2>;
template class MeshBasedCellPopulationWithGhostNodes<3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(MeshBasedCellPopulationWithGhostNodes)
