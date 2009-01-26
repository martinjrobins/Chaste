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
#include "Cylindrical2dMesh.hpp"


Cylindrical2dMesh::Cylindrical2dMesh(double width)
  : MutableMesh<2,2>(),
    mWidth(width)
{
    assert(width > 0.0);
}


Cylindrical2dMesh::Cylindrical2dMesh(double width, std::vector<Node<2> *> nodes)
  : MutableMesh<2,2>(),
    mWidth(width)
{
    assert(width > 0.0);
    for (unsigned index=0; index<nodes.size(); index++)
    {
        Node<2>* temp_node = nodes[index];
        double x = temp_node->rGetLocation()[0];
        x=x; // Fix optimised build
        assert( 0 <= x && x < width);
        mNodes.push_back(temp_node);
    }

    NodeMap node_map(nodes.size());
    ReMesh(node_map);
}


void Cylindrical2dMesh::UpdateTopAndBottom()
{
    c_vector<double,2> extremes = GetWidthExtremes(1);
    mBottom = extremes[0];
    mTop = extremes[1];
}


void Cylindrical2dMesh::CreateMirrorNodes()
{
    unsigned num_nodes=GetNumAllNodes();
    double half_way = (mWidth)/2.0;

    mLeftOriginals.clear();
    mLeftImages.clear();
    mImageToLeftOriginalNodeMap.clear();
    mRightOriginals.clear();
    mRightImages.clear();
    mImageToRightOriginalNodeMap.clear();
    mLeftPeriodicBoundaryElementIndices.clear();
    mRightPeriodicBoundaryElementIndices.clear();

    for (unsigned i=0; i<num_nodes; i++)
    {
        if (!mNodes[i]->IsDeleted())
        {
            c_vector<double, 2> location = mNodes[i]->rGetLocation();
            unsigned this_node_index = mNodes[i]->GetIndex();
            double this_node_x_location = location[0];

            // Check the mesh currently conforms to the dimensions given
            assert(0.0<=location[0]);
            assert(location[0]<=mWidth);

            // Put the nodes which are to be mirrored in the relevant vectors
            if (this_node_x_location<half_way)
            {
                mLeftOriginals.push_back(this_node_index);
            }
            else
            {
                mRightOriginals.push_back(this_node_index);
            }
        }
    }

    // Go through the left original nodes and create an image node
    // recording its new index
    for (unsigned i=0; i<mLeftOriginals.size(); i++)
    {
        c_vector<double, 2> location = mNodes[mLeftOriginals[i]]->rGetLocation();
        location[0] = location[0] + mWidth;

        unsigned new_node_index = MutableMesh<2,2>::AddNode(new Node<2>(0u, location));
        mLeftImages.push_back(new_node_index);
        mImageToLeftOriginalNodeMap[new_node_index] = mLeftOriginals[i];
    }

    // Go through the right original nodes and create an image node
    // recording its new index
    for (unsigned i=0; i<mRightOriginals.size(); i++)
    {
        // Create new image nodes
        c_vector<double, 2> location = mNodes[mRightOriginals[i]]->rGetLocation();
        location[0] = location[0] - mWidth;

        unsigned new_node_index = MutableMesh<2,2>::AddNode(new Node<2>(0u, location));
        mRightImages.push_back(new_node_index);
        mImageToRightOriginalNodeMap[new_node_index] = mRightOriginals[i];
    }
}


void Cylindrical2dMesh::CreateHaloNodes()
{
    UpdateTopAndBottom();

    mTopHaloNodes.clear();
    mBottomHaloNodes.clear();

    unsigned num_halo_nodes = (unsigned)(floor(mWidth*2.0));
    double halo_node_separation = mWidth/((double)(num_halo_nodes));
    double y_top_coordinate = mTop + halo_node_separation;
    double y_bottom_coordinate = mBottom - halo_node_separation;

    c_vector<double, 2> location;
    for (unsigned i=0; i< num_halo_nodes; i++)
    {
       double x_coordinate = 0.5*halo_node_separation + (double)(i)*halo_node_separation;

       // Inserting top halo node in mesh
       location[0] = x_coordinate;
       location[1] = y_top_coordinate;
       unsigned new_node_index = MutableMesh<2,2>::AddNode(new Node<2>(0u, location));
       mTopHaloNodes.push_back(new_node_index);

       location[1] = y_bottom_coordinate;
       new_node_index = MutableMesh<2,2>::AddNode(new Node<2>(0u, location));
       mBottomHaloNodes.push_back(new_node_index);
    }
}


void Cylindrical2dMesh::ReMesh(NodeMap &map)
{
    unsigned old_num_all_nodes = GetNumAllNodes();

    map.Resize(old_num_all_nodes);
    map.ResetToIdentity();

    // Flag the deleted nodes as deleted in the map
    for (unsigned i=0; i<old_num_all_nodes; i++)
    {
        if (mNodes[i]->IsDeleted())
        {
            map.SetDeleted(i);
        }
    }

    CreateHaloNodes();

    // Create a mirrored load of nodes for the normal remesher to work with.
    CreateMirrorNodes();

    // The mesh now has messed up boundary elements, but this
    // doesn't matter as the ReMesh below doesn't read them in
    // and reconstructs the boundary elements.

    // Call the normal re-mesh. Note that the mesh now has lots
    // of extra nodes which will be deleted, hence the name 'big_map'
    NodeMap big_map(GetNumAllNodes());
    MutableMesh<2,2>::ReMesh(big_map);

    // If the big_map isn't the identity map, the little map ('map') needs to be
    // altered accordingly before being passed to the user. not sure how this all works,
    // so deal with this bridge when we get to it
    assert(big_map.IsIdentityMap());

    // Re-index the vectors according to the big nodemap, and set up the maps.
    mImageToLeftOriginalNodeMap.clear();
    mImageToRightOriginalNodeMap.clear();

    for (unsigned i=0; i<mLeftOriginals.size(); i++)
    {
        mLeftOriginals[i] = big_map.GetNewIndex(mLeftOriginals[i]);
        mLeftImages[i] = big_map.GetNewIndex(mLeftImages[i]);
        mImageToLeftOriginalNodeMap[mLeftImages[i]] = mLeftOriginals[i];
    }

    for (unsigned i=0; i<mRightOriginals.size(); i++)
    {
        mRightOriginals[i] = big_map.GetNewIndex(mRightOriginals[i]);
        mRightImages[i] = big_map.GetNewIndex(mRightImages[i]);
        mImageToRightOriginalNodeMap[mRightImages[i]] = mRightOriginals[i];
    }

    for (unsigned i=0; i<mTopHaloNodes.size(); i++)
    {
        mTopHaloNodes[i] = big_map.GetNewIndex(mTopHaloNodes[i]);
        mBottomHaloNodes[i] = big_map.GetNewIndex(mBottomHaloNodes[i]);
    }

    // This method checks elements crossing the periodic boundary have been meshed in the same way at each side.
    CorrectNonPeriodicMesh();

    // This method takes in the double sized mesh,
    // with its new boundary elements,
    // and removes the relevant nodes, elements and boundary elements
    // to leave a proper periodic mesh.
    ReconstructCylindricalMesh();

    DeleteHaloNodes();

    // Create a random boundary element between two nodes of the first element if it is not deleted.
    // This is a temporary measure to get around reindex crashing when there are no boundary elements ( J. Coopers idea )
    bool boundary_element_made = false;
    unsigned elem_index = 0;
    while (elem_index<GetNumAllElements() && !boundary_element_made)
    {
        Element<2,2>* p_element = GetElement(elem_index);
        if (!p_element->IsDeleted())
        {
            boundary_element_made = true;
            std::vector<Node<2>*> nodes;
            nodes.push_back(p_element->GetNode(0));
            nodes.push_back(p_element->GetNode(1));
            BoundaryElement<1,2>* p_boundary_element = new BoundaryElement<1,2>(0, nodes);
            p_boundary_element->RegisterWithNodes();
            mBoundaryElements.push_back(p_boundary_element);

        }
        elem_index++;
    }

    // Now call ReIndex to remove the temporary nodes which are marked as deleted.
    NodeMap reindex_map(GetNumAllNodes());
    ReIndex(reindex_map);
    assert(!reindex_map.IsIdentityMap());  // maybe don't need this

    // Go through the reindex map and use it to populate the original NodeMap
    // (the one that is returned to the user)
    for (unsigned i=0; i<map.Size(); i++) // only going up to be size of map, not size of reindex_map
    {
        if (reindex_map.IsDeleted(i))
        {
            // i < num_original_nodes and node is deleted, this should correspond to
            // a node that was labelled as before the remeshing, so should have already
            // been set as deleted in the map above
            assert(map.IsDeleted(i));
        }
        else
        {
            map.SetNewIndex(i, reindex_map.GetNewIndex(i) );
        }
    }

    // We can now clear the index vectors & maps; they are only used for remeshing
    mLeftOriginals.clear();
    mLeftImages.clear();
    mImageToLeftOriginalNodeMap.clear();
    mRightOriginals.clear();
    mRightImages.clear();
    mImageToRightOriginalNodeMap.clear();
    mLeftPeriodicBoundaryElementIndices.clear();
    mRightPeriodicBoundaryElementIndices.clear();
}


void Cylindrical2dMesh::ReconstructCylindricalMesh()
{
    // Figure out which elements have real nodes and image nodes in them
    // and replace image nodes with corresponding real ones.
    for (unsigned elem_index = 0; elem_index<GetNumAllElements(); elem_index++)
    {
        Element<2,2>* p_element = GetElement(elem_index);
        if (!p_element->IsDeleted())
        {
            // Left images are on the right of the mesh
            unsigned number_of_left_image_nodes = 0u;
            unsigned number_of_right_image_nodes = 0u;
            for (unsigned i=0; i<3; i++)
            {
                unsigned this_node_index = p_element->GetNodeGlobalIndex(i);

                if (mImageToLeftOriginalNodeMap.find(this_node_index)
                   	!= mImageToLeftOriginalNodeMap.end())
                {
                    number_of_left_image_nodes++;
                }
                else if (mImageToRightOriginalNodeMap.find(this_node_index)
                    != mImageToRightOriginalNodeMap.end())
                {
                    number_of_right_image_nodes++;
                }
            }

            // Delete all the elements on the left hand side (images of right)...
            if (number_of_right_image_nodes>=1u)
            {
                p_element->MarkAsDeleted();
                mDeletedElementIndices.push_back(p_element->GetIndex());
            }

            // Delete only purely imaginary elements on the right (images of left nodes)
            if (number_of_left_image_nodes==3u)
            {
                p_element->MarkAsDeleted();
                mDeletedElementIndices.push_back(p_element->GetIndex());
            }

            // If some are images then replace them with the real nodes.
            // There can be elements with either two image nodes on the
            // right (and one real) or one image node on the right
            // (and two real).
            if (number_of_left_image_nodes==1u || number_of_left_image_nodes==2u )
            {
                for (unsigned i=0; i<3; i++)
                {
                    unsigned this_node_index = p_element->GetNodeGlobalIndex(i);
                    std::map<unsigned, unsigned>::iterator it = mImageToLeftOriginalNodeMap.find(this_node_index);
                    if (it != mImageToLeftOriginalNodeMap.end())
                    {
                    	p_element->ReplaceNode(mNodes[this_node_index], mNodes[it->second]);
                    }
                }
            }
        }
    } // end of loop over elements

    // Figure out which boundary elements have real nodes and image nodes in them
    // and replace image nodes with corresponding real ones.
    for (unsigned elem_index = 0; elem_index<GetNumAllBoundaryElements(); elem_index++)
    {
        BoundaryElement<1,2>* p_boundary_element = GetBoundaryElement(elem_index);
        if (!p_boundary_element->IsDeleted())
        {
            unsigned number_of_image_nodes = 0;
            for (unsigned i=0; i<2; i++)
            {
                unsigned this_node_index = p_boundary_element->GetNodeGlobalIndex(i);

                if (mImageToLeftOriginalNodeMap.find(this_node_index)
                    != mImageToLeftOriginalNodeMap.end())
                {
                    number_of_image_nodes++;
                }
                else if (mImageToRightOriginalNodeMap.find(this_node_index)
                    != mImageToRightOriginalNodeMap.end())
                {
                    number_of_image_nodes++;
                }
            }

            if (number_of_image_nodes==2 )
            {
                p_boundary_element->MarkAsDeleted();
                mDeletedBoundaryElementIndices.push_back(p_boundary_element->GetIndex());
            }

            // To avoid having two copies of the boundary elements on the periodic
            // boundaries we only deal with the elements on the left image and
            // delete the ones on the right image.

            if (number_of_image_nodes==1 )
            {

                for (unsigned i=0; i<2; i++)
                {
                    unsigned this_node_index = p_boundary_element->GetNodeGlobalIndex(i);
                    std::map<unsigned, unsigned>::iterator it = mImageToLeftOriginalNodeMap.find(this_node_index);
                    if (it != mImageToLeftOriginalNodeMap.end())
                    {
                        //std::cout << "PERIODIC \n" << std::flush;
                        p_boundary_element->ReplaceNode(mNodes[this_node_index], mNodes[it->second]);
                        //std::cout << "Node " << this_node_index << " swapped for node " << it->second << "\n" << std::flush;
                    }
                    else
                    {
	                    it = mImageToRightOriginalNodeMap.find(this_node_index);
	                    if (it != mImageToRightOriginalNodeMap.end())
	                    {
	                        //std::cout << "IMAGE\n" << std::flush;
	                        p_boundary_element->MarkAsDeleted();
	                        mDeletedBoundaryElementIndices.push_back(p_boundary_element->GetIndex());
	                    }
                    }
                }
            }
        }

    }

    // Delete all image nodes unless they have already gone (halo nodes)
    for (unsigned i=0; i<mLeftImages.size(); i++)
    {
        mNodes[mLeftImages[i]]->MarkAsDeleted();
        mDeletedNodeIndices.push_back(mLeftImages[i]);
    }

    for (unsigned i=0; i<mRightImages.size(); i++)
    {
        mNodes[mRightImages[i]]->MarkAsDeleted();
        mDeletedNodeIndices.push_back(mRightImages[i]);
    }
}


void Cylindrical2dMesh::DeleteHaloNodes()
{
    assert(mTopHaloNodes.size()==mBottomHaloNodes.size());
    for (unsigned i=0; i<mTopHaloNodes.size(); i++)
    {
        DeleteBoundaryNodeAt(mTopHaloNodes[i]);
        DeleteBoundaryNodeAt(mBottomHaloNodes[i]);
    }
}


c_vector<double, 2> Cylindrical2dMesh::GetVectorFromAtoB(const c_vector<double, 2>& rLocation1, const c_vector<double, 2>& rLocation2)
{
    assert(mWidth>0.0);

    c_vector<double, 2> location1 = rLocation1;
    c_vector<double, 2> location2 = rLocation2;

    location1[0] = fmod(location1[0], mWidth);
    location2[0] = fmod(location2[0], mWidth);

    c_vector<double, 2> vector = location2 - location1;

    // Handle the cylindrical condition here
    // if the points are more than halfway around the cylinder apart
    // measure the other way.
    if ( vector[0] > (mWidth / 2.0) )
    {
        vector[0] -= mWidth;
    }
    if ( vector[0] < -(mWidth / 2.0))
    {
        vector[0] += mWidth;
    }
    return vector;
}


void Cylindrical2dMesh::SetNode(unsigned index, ChastePoint<2> point, bool concreteMove)
{
    // Perform a periodic movement if necessary
    if (point.rGetLocation()[0] >= mWidth)
    {
        // Move point to the left
        point.SetCoordinate(0u, point.rGetLocation()[0]-mWidth);
    }
    if (point.rGetLocation()[0] < 0.0)
    {
        // Move point to the right
        point.SetCoordinate(0u, point.rGetLocation()[0]+mWidth);
    }

    // Update the node's location
    MutableMesh<2,2>::SetNode(index, point, concreteMove);
}


bool Cylindrical2dMesh::IsThisIndexInList(const unsigned& rNodeIndex, const std::vector<unsigned>& rListOfNodes)
{
    for (unsigned i=0; i<rListOfNodes.size(); i++)
    {
        if (rNodeIndex==rListOfNodes[i])
        {
            return true;
        }
    }
    return false;
}


double Cylindrical2dMesh::GetWidth(const unsigned& rDimension) const
{
    double width=0.0;
    assert(rDimension==0 || rDimension==1);
    if (rDimension==0)
    {
        width = mWidth;
    }
    else
    {
        width = MutableMesh<2,2>::GetWidth(rDimension);
    }
    return width;
}


unsigned Cylindrical2dMesh::AddNode(Node<2> *pNewNode)
{
    unsigned node_index = MutableMesh<2,2>::AddNode(pNewNode);

    // If necessary move it to be back on the cylinder
    ChastePoint<2> new_node_point = pNewNode->GetPoint();
    SetNode(node_index, new_node_point, false);

    return node_index;
}


void Cylindrical2dMesh::CorrectNonPeriodicMesh()
{
    GenerateVectorsOfElementsStraddlingPeriodicBoundaries();
    // Copy the member variables into new vectors, which we modify
    // by knocking out elements which pair up on each side
    std::set<unsigned> temp_left_hand_side_elements = mLeftPeriodicBoundaryElementIndices;
    std::set<unsigned> temp_right_hand_side_elements = mRightPeriodicBoundaryElementIndices;

    // Go through all of the elements on the left periodic boundary
    for (std::set<unsigned>::iterator left_iter = mLeftPeriodicBoundaryElementIndices.begin();
         left_iter != mLeftPeriodicBoundaryElementIndices.end();
         ++left_iter)
    {
        unsigned elem_index = *left_iter;
        Element<2,2>* p_element = GetElement(elem_index);

        // Make lists of the nodes which the elements on the left contain
        // and the nodes which should be in a corresponding element on the right.
        c_vector<unsigned,3> original_element_node_indices;
        c_vector<unsigned,3> corresponding_element_node_indices;
        for (unsigned i=0; i<3; i++)
        {
            original_element_node_indices[i] = p_element->GetNodeGlobalIndex(i);
            corresponding_element_node_indices[i] = GetCorrespondingNodeIndex(original_element_node_indices[i]);
        }

        // Search the right hand side elements for the corresponding element
        for (std::set<unsigned>::iterator right_iter = mRightPeriodicBoundaryElementIndices.begin();
             right_iter != mRightPeriodicBoundaryElementIndices.end();
             ++right_iter)
        {
            unsigned corresponding_elem_index = *right_iter;
            Element<2,2>* p_corresponding_element = GetElement(corresponding_elem_index);

            bool is_coresponding_node = true;

            for (unsigned i=0; i<3; i++)
            {
                if ( !(corresponding_element_node_indices[i] == p_corresponding_element->GetNodeGlobalIndex(0)) &&
                    !(corresponding_element_node_indices[i] == p_corresponding_element->GetNodeGlobalIndex(1)) &&
                    !(corresponding_element_node_indices[i] == p_corresponding_element->GetNodeGlobalIndex(2)) )
                {
                    is_coresponding_node = false;
                }
            }

            if (is_coresponding_node)
            {
                // Remove original and corresponding element from sets
                temp_left_hand_side_elements.erase(elem_index);
                temp_right_hand_side_elements.erase(corresponding_elem_index);
            }
        }
    }

    /*
     * If either of these ever throw you have more than one situation where the mesher has an option
     * of how to mesh. If it does ever throw you need to be cleverer and match up the
     * elements into as many pairs as possible on the left hand and right hand sides.
     */
    assert(temp_left_hand_side_elements.size()<=2u);
    assert(temp_right_hand_side_elements.size()<=2u);

    /*
     * Now we just have to use the first pair of elements and copy their info over to the other side.
     *
     * First we need to get hold of both elements on either side.
     */
    if (temp_left_hand_side_elements.size()==0u || temp_right_hand_side_elements.size()==0u)
    {
        assert(temp_right_hand_side_elements.size()==0u);
        assert(temp_left_hand_side_elements.size()==0u);
    }
    else
    {
        assert(temp_right_hand_side_elements.size()==2u && temp_left_hand_side_elements.size()==2u);
        if (temp_right_hand_side_elements.size()==2u)
        {
            // Use the right hand side meshing and map to left
            UseTheseElementsToDecideMeshing(temp_right_hand_side_elements);
        }
        else
        {
            // If you get here there are more than two mixed up elements on the periodic edge.
            // We need to knock the pair out and then rerun this function,
            // shouldn't be too hard to do but as yet unnecessary.
            NEVER_REACHED;
        }
    }
}

void Cylindrical2dMesh::UseTheseElementsToDecideMeshing(std::set<unsigned> mainSideElements)
{
    assert(mainSideElements.size()==2u);

    // We find the four nodes surrounding the dodgy meshing, on each side.
    std::set<unsigned> main_four_nodes;
    for (std::set<unsigned>::iterator left_iter = mainSideElements.begin();
         left_iter != mainSideElements.end();
         ++left_iter)
    {
        unsigned elem_index = *left_iter;
        Element<2,2>* p_element = GetElement(elem_index);
        for (unsigned i=0; i<3; i++)
        {
            unsigned index = p_element->GetNodeGlobalIndex(i);
            main_four_nodes.insert(index);
        }
    }
    assert(main_four_nodes.size()==4u);

    std::set<unsigned> other_four_nodes;
    for (std::set<unsigned>::iterator iter = main_four_nodes.begin();
         iter != main_four_nodes.end();
         ++iter)
    {
        other_four_nodes.insert(GetCorrespondingNodeIndex(*iter));
    }
    assert(other_four_nodes.size()==4u);

    // Find the elements surrounded by the nodes on the right
    // and change them to match the elements on the left
    std::vector<unsigned> corresponding_elements;

    // Loop over all elements
    for (unsigned elem_index = 0; elem_index<GetNumAllElements(); elem_index++)
    {
        Element<2,2>* p_element = GetElement(elem_index);
        if (!p_element->IsDeleted())
        {
            // Loop over the nodes of the element
            if (!(other_four_nodes.find(p_element->GetNodeGlobalIndex(0))==other_four_nodes.end()) &&
                !(other_four_nodes.find(p_element->GetNodeGlobalIndex(1))==other_four_nodes.end()) &&
                !(other_four_nodes.find(p_element->GetNodeGlobalIndex(2))==other_four_nodes.end()) )
            {
                corresponding_elements.push_back(elem_index);
                p_element->MarkAsDeleted();
                mDeletedElementIndices.push_back(p_element->GetIndex());
            }
        }
    }
    assert(corresponding_elements.size()==2u);

    // Now corresponding_elements contains the two elements which are going to be replaced by mainSideElements
    for (std::set<unsigned>::iterator iter = mainSideElements.begin();
         iter != mainSideElements.end();
         ++iter)
    {
        Element<2,2>* p_main_element = GetElement(*iter);
        std::vector<Node<2>*> nodes;

        // Put corresponding nodes into a std::vector
        for (unsigned i=0; i<3; i++)
        {
            unsigned main_node = p_main_element->GetNodeGlobalIndex(i);
            nodes.push_back(this->GetNode(GetCorrespondingNodeIndex(main_node)));
        }

        // Make a new element.
        Element<2,2>* p_new_element = new Element<2,2>(GetNumAllElements(), nodes);
        this->mElements.push_back(p_new_element);
    }

    // Reindex to get rid of extra elements indices
    NodeMap map(GetNumAllNodes());
    this->ReIndex(map);
}


void Cylindrical2dMesh::GenerateVectorsOfElementsStraddlingPeriodicBoundaries()
{
    mLeftPeriodicBoundaryElementIndices.clear();
    mRightPeriodicBoundaryElementIndices.clear();

    for (unsigned elem_index = 0; elem_index<GetNumAllElements(); elem_index++)
    {
        Element<2,2>* p_element = GetElement(elem_index);
        if (!p_element->IsDeleted())
        {
            // Left images are on the right of the mesh
            unsigned number_of_left_image_nodes = 0u;
            unsigned number_of_right_image_nodes = 0u;
            for (unsigned i=0; i<3; i++)
            {
                unsigned this_node_index = p_element->GetNodeGlobalIndex(i);

                if (mImageToLeftOriginalNodeMap.find(this_node_index)
                	!= mImageToLeftOriginalNodeMap.end())
                {
                    number_of_left_image_nodes++;
                }
                else if (mImageToRightOriginalNodeMap.find(this_node_index)
                    	!= mImageToRightOriginalNodeMap.end())
                {
                    number_of_right_image_nodes++;
                }
            }

            // Elements on the left hand side (images of right)...
            if (number_of_right_image_nodes==1u || number_of_right_image_nodes==2u)
            {
                mLeftPeriodicBoundaryElementIndices.insert(elem_index);
            }

            // Elements on the right (images of left nodes)
            if (number_of_left_image_nodes==1u|| number_of_left_image_nodes==2u)
            {
                mRightPeriodicBoundaryElementIndices.insert(elem_index);
            }
        }
    }
}


unsigned Cylindrical2dMesh::GetCorrespondingNodeIndex(unsigned nodeIndex)
{
    unsigned corresponding_node_index = UINT_MAX;
    bool found = false;

    if (IsThisIndexInList(nodeIndex, mRightOriginals))
    {
        for (unsigned i=0; i<mRightOriginals.size(); i++)
        {
            if (mRightOriginals[i]==nodeIndex)
            {
                corresponding_node_index = mRightImages[i];
                found = true;
            }
        }
    }
    if (IsThisIndexInList(nodeIndex, mRightImages))
    {
        for (unsigned i=0; i<mRightImages.size(); i++)
        {
            if (mRightImages[i]==nodeIndex)
            {
                corresponding_node_index = mRightOriginals[i];
                found = true;
            }
        }
    }
    if (IsThisIndexInList(nodeIndex, mLeftOriginals))
    {
        for (unsigned i=0; i<mLeftOriginals.size(); i++)
        {
            if (mLeftOriginals[i]==nodeIndex)
            {
                corresponding_node_index = mLeftImages[i];
                found = true;
            }
        }
    }
    if (IsThisIndexInList(nodeIndex, mLeftImages))
    {
        for (unsigned i=0; i<mLeftImages.size(); i++)
        {
            if (mLeftImages[i]==nodeIndex)
            {
                corresponding_node_index = mLeftOriginals[i];
                found = true;
            }
        }
    }

    assert(found);
    return corresponding_node_index;
}
