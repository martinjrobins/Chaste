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
#include "VertexMeshWriter.hpp"


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
VertexMeshWriter<ELEMENT_DIM, SPACE_DIM>::VertexMeshWriter(const std::string& rDirectory,
                                                           const std::string& rBaseName,
                                                           const bool clearOutputDir)
{
     mpOutputFileHandler = new OutputFileHandler(rDirectory, clearOutputDir);
     mBaseName = rBaseName; 
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
VertexMeshWriter<ELEMENT_DIM, SPACE_DIM>::~VertexMeshWriter()
{
    delete mpOutputFileHandler; 
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void VertexMeshWriter<ELEMENT_DIM, SPACE_DIM>::WriteFilesUsingMesh(VertexMesh<ELEMENT_DIM, SPACE_DIM>& rMesh)
{
    std::string comment = "#Generated by Chaste vertex mesh file writer";

    // Write node file
    std::string node_file_name = mBaseName + ".node";
    out_stream p_node_file = mpOutputFileHandler->OpenOutputFile(node_file_name);

    // Write the node header
    unsigned num_attr = 0;
    unsigned max_bdy_marker = 0;
    unsigned num_nodes = rMesh.GetNumNodes();
    
    *p_node_file << num_nodes << "\t";
    *p_node_file << SPACE_DIM << "\t";
    *p_node_file << num_attr << "\t";
    *p_node_file << max_bdy_marker << "\n";
    *p_node_file << std::setprecision(6);

    // Write each node's data
    unsigned default_marker = 0;
    for (unsigned node_num=0; node_num<num_nodes; node_num++)
    {
        unsigned global_node_index = rMesh.GetNode(node_num)->GetIndex();
        c_vector<double,2> position = rMesh.GetNode(node_num)->rGetLocation(); //this->mNodeData[item_num];
        *p_node_file << global_node_index;
        
        for (unsigned i=0; i<SPACE_DIM; i++)
        {
            *p_node_file << "\t" << position(i);
        }
        *p_node_file << "\t" << default_marker << "\n";

    }
    *p_node_file << comment << "\n";
    p_node_file->close();

    // Write element file
    std::string element_file_name = mBaseName + ".cell";
    out_stream p_element_file = mpOutputFileHandler->OpenOutputFile(element_file_name);

    // Write the element header
    unsigned num_elements = rMesh.GetNumElements();

    *p_element_file << num_elements << "\t";
    *p_element_file << num_attr << "\n";

    // Write each element's data 
    /// \todo need to think about how best to do this in 3D (see #866)
    for (unsigned element_num=0; element_num<num_elements; element_num++)
    {
        *p_element_file << element_num;
        *p_element_file << "\t" << rMesh.GetElement(element_num)->GetNumNodes();
        
        for (unsigned i=0; i<rMesh.GetElement(element_num)->GetNumNodes(); i++)
        {
            *p_element_file << "\t" << rMesh.GetElement(element_num)->GetNodeGlobalIndex(i);
        }
        *p_element_file << "\n";

    }
    *p_element_file << comment << "\n";
    p_element_file->close();
}


/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////

template class VertexMeshWriter<1,1>;
template class VertexMeshWriter<2,2>;
template class VertexMeshWriter<3,3>;
