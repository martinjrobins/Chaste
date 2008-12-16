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
#include "SimpleTissue.hpp"

template<unsigned DIM>
SimpleTissue<DIM>::SimpleTissue(const std::vector<Node<DIM> >& rNodes,
                                const std::vector<TissueCell>& rCells)
        : AbstractTissue<DIM>(rCells),
          mNodes(rNodes)
{
    Validate();
}


template<unsigned DIM>
SimpleTissue<DIM>::SimpleTissue(const std::vector<Node<DIM> >& rNodes)
        : AbstractTissue<DIM>(),
          mNodes(rNodes)
{
}


template<unsigned DIM>
SimpleTissue<DIM>::SimpleTissue(const AbstractMesh<DIM,DIM>& rMesh,
                                const std::vector<TissueCell>& rCells)
        : AbstractTissue<DIM>(rCells)
{
    for(unsigned i=0; i<rMesh.GetNumNodes(); i++)
    {
        mNodes.push_back(*(rMesh.GetNode(i)));
    }
    
    Validate();
}

template<unsigned DIM>
void SimpleTissue<DIM>::Validate()
{
    std::vector<bool> validated_node(GetNumNodes());

    for (typename AbstractTissue<DIM>::Iterator cell_iter=this->Begin(); cell_iter!=this->End(); ++cell_iter)
    {
        unsigned node_index = cell_iter->GetLocationIndex();
        validated_node[node_index] = true;
    }

    for (unsigned i=0; i<validated_node.size(); i++)
    {
        if (!validated_node[i])
        {
            std::stringstream ss;
            ss << "Node " << i << " does not appear to have a cell associated with it";
            EXCEPTION(ss.str());
        }
    }
}

template<unsigned DIM>
std::vector<Node<DIM> >& SimpleTissue<DIM>::rGetNodes()
{
    return mNodes;
}


template<unsigned DIM>
const std::vector<Node<DIM> >& SimpleTissue<DIM>::rGetNodes() const
{
    return mNodes;
}


template<unsigned DIM>
Node<DIM>* SimpleTissue<DIM>::GetNode(unsigned index)
{
    return &(mNodes[index]);
}


template<unsigned DIM>
void SimpleTissue<DIM>::SetNode(unsigned index, ChastePoint<DIM> point)
{
    mNodes[index].SetPoint(point);
}


template<unsigned DIM>
void SimpleTissue<DIM>::MoveCell(typename AbstractTissue<DIM>::Iterator iter, ChastePoint<DIM>& rNewLocation)
{
    unsigned index = iter.GetNode()->GetIndex();
    SetNode(index, rNewLocation);
}


template<unsigned DIM>
TissueCell* SimpleTissue<DIM>::AddCell(TissueCell newCell, c_vector<double,DIM> newLocation)
{
    // Create a new node
    Node<DIM> new_node(GetNumNodes(), newLocation, false); // never on boundary

    unsigned new_node_index = AddNode(&new_node); //Uses copy constructor (so it doesn't matter that new_node goes out of scope)

    // Associate the new cell with the node
    newCell.SetLocationIndex(new_node_index);
    this->mCells.push_back(newCell);

    TissueCell *p_created_cell = &(this->mCells.back());
    this->mLocationCellMap[new_node_index] = p_created_cell;

    return p_created_cell;
}


template<unsigned DIM>
void SimpleTissue<DIM>::ReMesh()
{
    // Create and reserve space for a temporary vector
    std::vector<Node<DIM> > old_nodes;
    old_nodes.reserve(mNodes.size());

    // Store all non-deleted nodes in the temporary vector
    for (unsigned i=0; i<mNodes.size(); i++)
    {
        if (mNodes[i].IsDeleted()==false)
        {
            old_nodes.push_back(mNodes[i]);
        }
    }

    // Update mNodes
    mNodes = old_nodes;

    // Update the correspondence between nodes and cells.
    // We expect the node indices to be {0,1,...,num_nodes}
    std::set<unsigned> expected_node_indices;
    for (unsigned i=0; i<GetNumNodes(); i++)
    {
        expected_node_indices.insert(i);
    }

    // Get the actual set of node indices
    std::set<unsigned> node_indices;
    for (typename AbstractTissue<DIM>::Iterator cell_iter=this->Begin(); cell_iter!=this->End(); ++cell_iter)
    {
        unsigned node_index = cell_iter->GetLocationIndex();
        node_indices.insert(node_index);
    }

    // If necessary, update the node cell map
    if (node_indices != expected_node_indices)
    {
        // Fix up the mappings between cells and nodes
        this->mLocationCellMap.clear();
        unsigned new_node_index = 0;
        for (typename AbstractTissue<DIM>::Iterator cell_iter=this->Begin(); cell_iter!=this->End(); ++cell_iter)
        {
            cell_iter->SetLocationIndex(new_node_index);
            this->mLocationCellMap[new_node_index] = &(*cell_iter);
            new_node_index++;
        }
    }

    Validate();
}


template<unsigned DIM>
unsigned SimpleTissue<DIM>::RemoveDeadCells()
{
    unsigned num_removed = 0;

    for (std::list<TissueCell>::iterator cell_iter = this->mCells.begin();
         cell_iter != this->mCells.end();
         ++cell_iter)
    {
        if (cell_iter->IsDead())
        {
            // Remove the node from the mesh
            num_removed++;
            this->GetNodeCorrespondingToCell(*cell_iter)->MarkAsDeleted();
            cell_iter = this->mCells.erase(cell_iter);
            --cell_iter;
        }
    }
    return num_removed;
}


template<unsigned DIM>
unsigned SimpleTissue<DIM>::AddNode(Node<DIM> *pNewNode)
{
    /// \todo employ a std::vector of deleted node indices to re-use indices? (see #841)
    pNewNode->SetIndex(mNodes.size());
    mNodes.push_back(*pNewNode);
    return pNewNode->GetIndex();
}


template<unsigned DIM>
unsigned SimpleTissue<DIM>::GetNumNodes()
{
    return mNodes.size();
}


/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////


template class SimpleTissue<1>;
template class SimpleTissue<2>;
template class SimpleTissue<3>;
