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
#include "CellwiseDataGradient.hpp"
#include "LinearBasisFunction.hpp"

template<unsigned DIM>
c_vector<double, DIM>& CellwiseDataGradient<DIM>::rGetGradient(unsigned nodeIndex)
{
    assert( !(CellwiseData<DIM>::Instance()->rGetTissue().IsGhostNode(nodeIndex)) );
    return mGradients[nodeIndex];
}


template<unsigned DIM>
void CellwiseDataGradient<DIM>::SetupGradients()
{
    MeshBasedTissue<DIM>& r_tissue = CellwiseData<DIM>::Instance()->rGetTissue();
    TetrahedralMesh<DIM,DIM>& r_mesh = r_tissue.rGetMesh();

    // Initialise gradients size
    unsigned num_nodes = r_tissue.rGetMesh().GetNumNodes();
    mGradients.resize(num_nodes, zero_vector<double>(DIM));

    // The constant gradients at each element
    std::vector<c_vector<double, DIM> > gradients_on_elements;
    unsigned num_elements = r_mesh.GetNumElements();
    gradients_on_elements.resize(num_elements, zero_vector<double>(DIM));

    // The number of elements containing a given node (excl ghost elements)
    std::vector<unsigned> num_real_elems_for_node(num_nodes, 0);

    for (unsigned elem_index=0; elem_index<num_elements; elem_index++)
    {
        Element<DIM,DIM>& r_elem = *(r_mesh.GetElement(elem_index));

        // Calculate the basis functions at any point (eg zero) in the element
        c_matrix<double, DIM, DIM> inverse_jacobian;
        r_mesh.GetInverseJacobianForElement(elem_index, inverse_jacobian);
        const ChastePoint<DIM> zero_point;
        c_matrix<double, DIM, DIM+1> grad_phi;
        LinearBasisFunction<DIM>::ComputeTransformedBasisFunctionDerivatives(zero_point, inverse_jacobian, grad_phi);

        bool is_ghost_element = false;

        for (unsigned node_index=0; node_index<DIM+1; node_index++)
        {
            unsigned node_global_index = r_elem.GetNodeGlobalIndex(node_index);

            // Check whether ghost element
            if ( r_tissue.IsGhostNode(node_global_index)==true )
            {
                is_ghost_element = true;
                break;
            }

            // If no ghost element, get nutrient conc
            TissueCell& r_cell = r_tissue.rGetCellUsingLocationIndex(node_global_index);
            double nutrient_concentration = CellwiseData<DIM>::Instance()->GetValue(&r_cell,0);

            // Interpolate gradient
            for (unsigned i=0; i<DIM; i++)
            {
                gradients_on_elements[elem_index](i) += nutrient_concentration* grad_phi(i, node_index);
            }
        }

        // Add gradient at element to gradient at node
        if (!is_ghost_element)
        {
            for (unsigned node_index=0; node_index<DIM+1; node_index++)
            {
                unsigned node_global_index = r_elem.GetNodeGlobalIndex(node_index);
                mGradients[node_global_index] += gradients_on_elements[elem_index];
                num_real_elems_for_node[node_global_index]++;
            }
        }
    }

    // Divide to obtain average gradient
    for (typename AbstractTissue<DIM>::Iterator cell_iter = r_tissue.Begin();
         cell_iter != r_tissue.End();
         ++cell_iter)
    {
        unsigned node_global_index = cell_iter->GetLocationIndex();

        if  (!num_real_elems_for_node[node_global_index]>0)
        {
            // The node is a real node which is not in any real element
            // but shoud be connected to some cells (if more than one cell in mesh)
            Node<DIM>& this_node = *(r_tissue.GetNodeCorrespondingToCell(*cell_iter));

            mGradients[node_global_index] = zero_vector<double>(DIM);
            unsigned num_real_adjacent_nodes = 0;

            // Get all the adjacent nodes which correspond to real cells
            std::set < Node<DIM>* > real_adjacent_nodes;
            real_adjacent_nodes.clear();

            // First loop over containing elements
            for (typename Node<DIM>::ContainingElementIterator element_iter = this_node.ContainingElementsBegin();
                 element_iter != this_node.ContainingElementsEnd();
                 ++element_iter)
            {
                // Then loop over nodes therein
                Element<DIM,DIM>& r_adjacent_elem = *(r_mesh.GetElement(*element_iter));
                for (unsigned local_node_index=0; local_node_index<DIM+1; local_node_index++)
                {
                    unsigned adjacent_node_global_index = r_adjacent_elem.GetNodeGlobalIndex(local_node_index);

                    // If not a ghost node and not the node we started with
                    if ( r_tissue.IsGhostNode(adjacent_node_global_index)==false && adjacent_node_global_index != node_global_index )
                    {

                        // Calculate the contribution of gradient from this node
                        Node<DIM> & adjacent_node= *(r_mesh.GetNode(adjacent_node_global_index));

                        double this_cell_concentration = CellwiseData<DIM>::Instance()->GetValue(&(*cell_iter),0);
                        TissueCell& adjacent_cell = r_tissue.rGetCellUsingLocationIndex(adjacent_node_global_index);
                        double adjacent_cell_concentration = CellwiseData<DIM>::Instance()->GetValue(&adjacent_cell,0);

                        c_vector<double, DIM> gradient_contribution=zero_vector<double>(DIM);

                        if (fabs(this_cell_concentration-adjacent_cell_concentration) > 100*DBL_EPSILON )
                        {
                            c_vector<double, DIM> edge_vector=r_mesh.GetVectorFromAtoB(this_node.rGetLocation(), adjacent_node.rGetLocation());
                            double norm_edge_vector = norm_2(edge_vector);
                            gradient_contribution = edge_vector
                                                        * (adjacent_cell_concentration - this_cell_concentration)
                                                        / (norm_edge_vector * norm_edge_vector);
                        }

                        mGradients[node_global_index]+=gradient_contribution;
                        num_real_adjacent_nodes++;
                    }

                }
            }
            mGradients[node_global_index] /= num_real_adjacent_nodes;
        }
        else
        {
            mGradients[node_global_index] /= num_real_elems_for_node[node_global_index];
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////

template class CellwiseDataGradient<1>;
template class CellwiseDataGradient<2>;
template class CellwiseDataGradient<3>;
