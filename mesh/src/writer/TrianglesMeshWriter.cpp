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

#include "TrianglesMeshWriter.hpp"
#include <cassert>

///////////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////////

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
TrianglesMeshWriter<ELEMENT_DIM, SPACE_DIM>::TrianglesMeshWriter(
    const std::string &rDirectory,
    const std::string &rBaseName,
    const bool clearOutputDir)
        : AbstractTetrahedralMeshWriter<ELEMENT_DIM, SPACE_DIM>(rDirectory, rBaseName, clearOutputDir)
{
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
TrianglesMeshWriter<ELEMENT_DIM, SPACE_DIM>::~TrianglesMeshWriter()
{
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void TrianglesMeshWriter<ELEMENT_DIM, SPACE_DIM>::WriteFiles()
{
    std::string comment = "#Generated by Chaste mesh file writer";

    // Write node file
    std::string node_file_name = this->mBaseName + ".node";
    out_stream p_node_file = this->mpOutputFileHandler->OpenOutputFile(node_file_name);

    // Write the node header
    unsigned num_attr = 0;
    unsigned max_bdy_marker = 0;
    unsigned num_nodes = this->GetNumNodes();

    *p_node_file << num_nodes << "\t";
    *p_node_file << SPACE_DIM << "\t";
    *p_node_file << num_attr << "\t";
    *p_node_file << max_bdy_marker << "\n";
    *p_node_file << std::setprecision(20);

    // Write each node's data
    unsigned default_marker = 0;
    for (unsigned item_num=0; item_num<num_nodes; item_num++)
    {
        std::vector<double> current_item = this->GetNextNode();
        *p_node_file << item_num;
        for (unsigned i=0; i<SPACE_DIM; i++)
        {
            *p_node_file << "\t" << current_item[i];
        }
        *p_node_file << "\t" << default_marker << "\n";

    }
    *p_node_file << comment << "\n";
    p_node_file->close();

    if (ELEMENT_DIM < SPACE_DIM)
    {
        WriteElementsAsFaces();
        WriteFacesAsEdges();
        return;
    }

    // Write element file
    std::string element_file_name = this->mBaseName + ".ele";
    out_stream p_element_file = this->mpOutputFileHandler->OpenOutputFile(element_file_name);

    // Write the element header
    unsigned num_elements = this->GetNumElements();
    
 //   assert( this->mElementData.size()>0 );//Read element size from the data we're given

    std::vector<unsigned> element_data = this->GetNextElement().NodeIndices;

    unsigned nodes_per_element = element_data.size();
    if(nodes_per_element != ELEMENT_DIM+1)
    {
        // Check that this is a quadratic mesh
        assert(ELEMENT_DIM == SPACE_DIM);
        assert(nodes_per_element == (ELEMENT_DIM+1)*(ELEMENT_DIM+2)/2);
     }
        
    *p_element_file << num_elements << "\t";
    *p_element_file << nodes_per_element << "\t";
    *p_element_file << num_attr << "\n";

    // Write each element's data
    for (unsigned item_num=0; item_num<num_elements; item_num++)
    {
        // if item_num==0 we will already got the element above (in order to 
        // get the number of nodes per element
        if(item_num>0)
        {
            element_data = this->GetNextElement().NodeIndices;
        }
        
        *p_element_file << item_num;
        for (unsigned i=0; i<nodes_per_element; i++)
        {
            *p_element_file << "\t" << element_data[i];
        }
        *p_element_file << "\n";
    }
    *p_element_file << comment << "\n";
    p_element_file->close();

    // Write boundary face file
    std::string face_file_name = this->mBaseName;

    if (SPACE_DIM == 1)
    {
        // In 1-D there is no boundary file.  It's trivial to calculate
        return;
    }
    else if (SPACE_DIM == 2)
    {
        face_file_name = face_file_name + ".edge";
    }
    else
    {
        face_file_name = face_file_name + ".face";
    }
    out_stream p_face_file = this->mpOutputFileHandler->OpenOutputFile(face_file_name);

    // Write the boundary face header
    unsigned num_faces = this->GetNumBoundaryFaces();

    *p_face_file << num_faces << "\t";
    *p_face_file << max_bdy_marker << "\n";

    // Write each face's data
    for (unsigned item_num=0; item_num<num_faces; item_num++)
    {
        std::vector<unsigned> current_item = this->mBoundaryFaceData[item_num];
        *p_face_file << item_num;
        for (unsigned i=0; i<current_item.size(); i++)
        {
            *p_face_file << "\t" << current_item[i];
        }
        *p_face_file << "\t" << default_marker << "\n";

    }
    *p_face_file << comment << "\n";
    p_face_file->close();
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void TrianglesMeshWriter<ELEMENT_DIM, SPACE_DIM>::WriteElementsAsFaces()
{
    std::string comment = "#Generated by Chaste mesh file writer";

    std::string element_file_name = this->mBaseName;
    if (ELEMENT_DIM == 1 && (SPACE_DIM == 2 || SPACE_DIM == 3))
    {
        element_file_name = element_file_name + ".edge";
    }
    else if (ELEMENT_DIM == 2 && SPACE_DIM == 3)
    {
        element_file_name = element_file_name + ".face";
    }
/*    else
    {
        // This is ignored in coverage:
        // Since we can't yet read line element meshes in 3D, we won't have anything to write
#define COVERAGE_IGNORE
        EXCEPTION("Can only write 1D/2D elements in 2D/3D space.");
#undef COVERAGE_IGNORE
    }*/

    out_stream p_element_file = this->mpOutputFileHandler->OpenOutputFile(element_file_name);

    // Write the element header
    unsigned num_elements = this->GetNumElements();
    assert(SPACE_DIM != ELEMENT_DIM);
    unsigned nodes_per_element = ELEMENT_DIM+1;
    unsigned num_attr = 0;

    *p_element_file << num_elements << "\t";
    //*p_element_file << nodes_per_element << "\t";
    *p_element_file << num_attr << "\n";

    // Write each element's data
    for (unsigned item_num=0; item_num<num_elements; item_num++)
    {
        std::vector<unsigned> current_item = this->GetNextElement().NodeIndices;
        *p_element_file << item_num;
        for (unsigned i=0; i<nodes_per_element; i++)
        {
            *p_element_file << "\t" << current_item[i];
        }
        *p_element_file << "\n";

    }
    *p_element_file << comment << "\n";
    p_element_file->close();

}
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void TrianglesMeshWriter<ELEMENT_DIM, SPACE_DIM>::WriteFacesAsEdges()
{
    std::string comment = "#Generated by Chaste mesh file writer";

    if (ELEMENT_DIM == 1 && (SPACE_DIM == 2 || SPACE_DIM == 3))
    {
        return;
    }

    //Gcov is confused by this assertion
#define COVERAGE_IGNORE
    assert(SPACE_DIM == 3 && ELEMENT_DIM == 2);
#undef COVERAGE_IGNORE

    std::string face_file_name = this->mBaseName;
    face_file_name = face_file_name + ".edge";

    out_stream p_face_file = this->mpOutputFileHandler->OpenOutputFile(face_file_name);

    // Write the boundary face header
    unsigned num_faces = this->GetNumBoundaryFaces();

    unsigned max_bdy_marker = 0;
    unsigned default_marker = 0;

    *p_face_file << num_faces << "\t";
    *p_face_file << max_bdy_marker << "\n";

    // Write each face's data
    for (unsigned item_num=0; item_num<num_faces; item_num++)
    {
        std::vector<unsigned> current_item = this->mBoundaryFaceData[item_num];
        *p_face_file << item_num;
        for (unsigned i=0; i<current_item.size(); i++)
        {
            *p_face_file << "\t" << current_item[i];
        }
        *p_face_file << "\t" << default_marker << "\n";

    }
    *p_face_file << comment << "\n";
    p_face_file->close();
}

/////////////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////////////

template class TrianglesMeshWriter<1,1>;
template class TrianglesMeshWriter<1,2>;
template class TrianglesMeshWriter<1,3>;
template class TrianglesMeshWriter<2,2>;
template class TrianglesMeshWriter<2,3>;
template class TrianglesMeshWriter<3,3>;
