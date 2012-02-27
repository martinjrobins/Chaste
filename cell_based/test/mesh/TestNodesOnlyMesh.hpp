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

#ifndef TESTNODESONLYMESH_HPP_
#define TESTNODESONLYMESH_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <algorithm>

#include "UblasCustomFunctions.hpp"
#include "NodesOnlyMesh.hpp"
#include "VtkMeshWriter.hpp"
#include "ArchiveOpener.hpp"

class TestNodesOnlyMesh : public CxxTest::TestSuite
{
public:

    void TestConstructNodesWithoutMesh()
    {
        std::vector<Node<3>*> nodes;
        nodes.push_back(new Node<3>(0, true,  0.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(1, false, 1.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(2, false, 0.0, 1.0, 0.0));
        nodes.push_back(new Node<3>(3, false, 1.0, 1.0, 0.0));
        nodes.push_back(new Node<3>(4, false, 0.0, 0.0, 1.0));
        nodes.push_back(new Node<3>(5, false, 1.0, 0.0, 1.0));
        nodes.push_back(new Node<3>(6, false, 0.0, 1.0, 1.0));
        nodes.push_back(new Node<3>(7, false, 1.0, 1.0, 1.0));

        NodesOnlyMesh<3> mesh;
        mesh.ConstructNodesWithoutMesh(nodes);

        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 8u);
        TS_ASSERT_EQUALS(mesh.GetNumElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumBoundaryElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumAllNodes(), 8u);

        // When the mesh goes out of scope, then it's a different set of nodes that get destroyed
        for (unsigned i=0; i<nodes.size(); i++)
        {
            delete nodes[i];
        }
    }

    void TestClearingNodesOnlyMesh()
    {
        std::vector<Node<3>*> nodes;
        nodes.push_back(new Node<3>(0, true,  0.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(1, false, 1.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(2, false, 0.0, 1.0, 0.0));
        nodes.push_back(new Node<3>(3, false, 1.0, 1.0, 0.0));
        nodes.push_back(new Node<3>(4, false, 0.0, 0.0, 1.0));
        nodes.push_back(new Node<3>(5, false, 1.0, 0.0, 1.0));
        nodes.push_back(new Node<3>(6, false, 0.0, 1.0, 1.0));
        nodes.push_back(new Node<3>(7, false, 1.0, 1.0, 1.0));

        NodesOnlyMesh<3> mesh;
        mesh.ConstructNodesWithoutMesh(nodes);

        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 8u);

        mesh.Clear();

        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumAllNodes(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumAllElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumBoundaryElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumAllBoundaryElements(), 0u);
        TS_ASSERT_EQUALS(mesh.mCellRadii.size(),0u);

        // When the mesh goes out of scope, then it's a different set of nodes that get destroyed
        for (unsigned i=0; i<nodes.size(); i++)
        {
            delete nodes[i];
        }
    }

    void TestConstructNodesWithoutMeshUsingMesh()
    {
        // Create a simple mesh
        TrianglesMeshReader<2,2> mesh_reader("mesh/test/data/square_4_elements");
        TetrahedralMesh<2,2> generating_mesh;
        generating_mesh.ConstructFromMeshReader(mesh_reader);

        NodesOnlyMesh<2> mesh;
        mesh.ConstructNodesWithoutMesh(generating_mesh);

        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 5u);
        TS_ASSERT_EQUALS(mesh.GetNumElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumBoundaryElements(), 0u);
        TS_ASSERT_EQUALS(mesh.GetNumAllNodes(), 5u);
    }

    void TestWriteNodesWithoutMeshUsingVtk()
    {
 #ifdef CHASTE_VTK
// Requires  "sudo aptitude install libvtk5-dev" or similar
        std::vector<Node<3>*> nodes;
        nodes.push_back(new Node<3>(0, true,  0.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(1, false, 1.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(2, false, 0.0, 1.0, 0.0));
        nodes.push_back(new Node<3>(3, false, 1.0, 1.0, 0.0));
        nodes.push_back(new Node<3>(4, false, 0.0, 0.0, 1.0));
        nodes.push_back(new Node<3>(5, false, 1.0, 0.0, 1.0));
        nodes.push_back(new Node<3>(6, false, 0.0, 1.0, 1.0));
        nodes.push_back(new Node<3>(7, false, 1.0, 1.0, 1.0));

        NodesOnlyMesh<3> mesh;
        mesh.ConstructNodesWithoutMesh(nodes);

        // Note in my version of Paraview, you need data on points before you can view with Glyphs
        VtkMeshWriter<3,3> writer("TestVtkMeshWriter", "just_nodes", false);

        // Add distance from origin into the node "point" data
        std::vector<double> distance;
        for (unsigned i=0; i<mesh.GetNumNodes(); i++)
        {
            distance.push_back(norm_2(mesh.GetNode(i)->rGetLocation()));
        }
        writer.AddPointData("Distance from origin", distance);

        // Add boundary node "point" data
        std::vector<double> boundary;
        for (unsigned i=0; i<mesh.GetNumNodes(); i++)
        {
            boundary.push_back(mesh.GetNode(i)->IsBoundaryNode());
        }
        writer.AddPointData("Boundary", boundary);

        // Add fibre type to "point" data
        std::vector< c_vector<double, 3> > location;
        for (unsigned i=0; i<mesh.GetNumNodes(); i++)
        {
            location.push_back(mesh.GetNode(i)->rGetLocation());
        }
        writer.AddPointData("Location", location);

        writer.WriteFilesUsingMesh(mesh);

        // When the mesh goes out of scope, then it's a different set of nodes that get destroyed
        for (unsigned i=0; i<nodes.size(); i++)
        {
            delete nodes[i];
        }
#endif //CHASTE_VTK
    }

    void TestWriteNodesWithoutMesh()
    {
         std::vector<Node<3>*> nodes;
         nodes.push_back(new Node<3>(0, true,  0.0, 0.0, 0.0));
         nodes.push_back(new Node<3>(1, false, 1.0, 0.0, 0.0));
         nodes.push_back(new Node<3>(2, false, 0.0, 1.0, 0.0));
         nodes.push_back(new Node<3>(3, false, 1.0, 1.0, 0.0));
         nodes.push_back(new Node<3>(4, false, 0.0, 0.0, 1.0));
         nodes.push_back(new Node<3>(5, false, 1.0, 0.0, 1.0));
         nodes.push_back(new Node<3>(6, false, 0.0, 1.0, 1.0));
         nodes.push_back(new Node<3>(7, false, 1.0, 1.0, 1.0));

         NodesOnlyMesh<3> mesh;
         mesh.ConstructNodesWithoutMesh(nodes);

         TrianglesMeshWriter<3,3> writer("TestMeshWriter", "3dNodesOnlyMesh");
         TS_ASSERT_THROWS_NOTHING(writer.WriteFilesUsingMesh(mesh));

         // When the mesh goes out of scope, then it's a different set of nodes that get destroyed
         for (unsigned i=0; i<nodes.size(); i++)
         {
             delete nodes[i];
         }
     }

    void TestGetSetMethods()
    {
        std::vector<Node<3>*> nodes;
        nodes.push_back(new Node<3>(0, true,  0.0, 0.0, 0.0));
        nodes.push_back(new Node<3>(1, false, 1.0, 0.0, 0.0));

        NodesOnlyMesh<3> mesh;
        mesh.ConstructNodesWithoutMesh(nodes);

        mesh.SetCellRadius(0, 1.0);
        mesh.SetCellRadius(1, 2.0);

        // Coverage
        mesh.SetMaximumInteractionDistance(1.0);

        TS_ASSERT_DELTA(mesh.GetCellRadius(0), 1.0, 1e-6);
        TS_ASSERT_DELTA(mesh.GetCellRadius(1), 2.0, 1e-6);

        // When the mesh goes out of scope, then it's a different set of nodes that get destroyed
        for (unsigned i=0; i<nodes.size(); i++)
        {
            delete nodes[i];
        }
    }

    void TestAddNode() throw (Exception)
    {
        std::vector<Node<2>*> nodes;
        Node<2> node0(0, true, 0.0, 0.0);
        nodes.push_back(&node0);
        Node<2> node1(1, true, 0.0, 0.5);
        nodes.push_back(&node1);

        NodesOnlyMesh<2> mesh;
        mesh.ConstructNodesWithoutMesh(nodes);

        // Test add node
        mesh.AddNode(new Node<2>(2, true, 0.0, 1.0));//This node pointer is added to the mesh and deleted by the destructor

        unsigned num_nodes = 3;
        TS_ASSERT_EQUALS(mesh.GetNumNodes(), num_nodes);
        TS_ASSERT_DELTA(mesh.GetCellRadius(2), 0.5, 1e-4);
    }

    void TestDeleteNodesAndRemesh() throw (Exception)
    {
        std::vector<Node<2>*> nodes;
        nodes.push_back(new Node<2>(0, true, 0.0, 0.0));
        nodes.push_back(new Node<2>(1, true, 0.0, 0.5));
        nodes.push_back(new Node<2>(2, true, 1.0, 0.0));
        nodes.push_back(new Node<2>(3, true, 1.0, 1.0));
        nodes.push_back(new Node<2>(4, true, 0.0, 1.0));
        nodes.push_back(new Node<2>(5, false, 0.4, 0.5));
        nodes.push_back(new Node<2>(6, false, 0.6, 0.5));
        nodes.push_back(new Node<2>(7, false, 0.0, 0.5));

        NodesOnlyMesh<2> mesh;
        mesh.ConstructNodesWithoutMesh(nodes);

        // Test that there are never any boundary nodes
        TS_ASSERT_EQUALS(mesh.GetNumBoundaryNodes(), 0u);
        NodeMap node_map(8);
        mesh.ReMesh(node_map);
        TS_ASSERT_EQUALS(mesh.GetNumBoundaryNodes(), 0u);

        // Free memory - the constructor does a deep copy of its input
        for (unsigned i=0; i<nodes.size(); i++)
        {
            delete nodes[i];
        }

        // Set radii of cells from 1 to 8
        for (unsigned i=0; i<nodes.size(); i++)
        {
            mesh.SetCellRadius(i, i+1);
        }

        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 8u);

        // Delete from interior
        TS_ASSERT_DELTA(mesh.GetCellRadius(6), 7.0, 1e-4);
        mesh.DeleteNode(6);
        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 7u);
        TS_ASSERT_EQUALS(mesh.GetCellRadius(6), DOUBLE_UNSET);

        // Delete from edge
        mesh.DeleteNode(1);
        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 6u);

        // Delete from corner
        TS_ASSERT_DELTA(mesh.GetCellRadius(3), 4.0, 1e-4);
        mesh.DeleteNode(3);
        TS_ASSERT_EQUALS(mesh.GetNumNodes(), 5u);

        // Deleting a deleted node should throw an exception
        TS_ASSERT_THROWS_THIS(mesh.DeleteNode(3),"Trying to delete a deleted node");

        /*
         * Check that mCellRadii is updated correctly when a new cell
         * is added using the most recently deleted index.
         * (Index 3 is at the back of the deleted nodes list and is thus the one to be reused.)
         */
        mesh.AddNode(new Node<2>(0, true, 6.0, 6.0)); //This node pointer is added to the mesh and deleted by the destructor

        // Check the most recently deleted node now has the correct cell radius
        TS_ASSERT_DELTA(mesh.GetCellRadius(3), 0.5, 1e-4);

        // Now we have deleted/reused 3, and deleted 1 and 6.
        // The new nodes are:
        // New:     0     1     2     3     4     5
        // Old:     0     2 (new3)    4     5     7
        // Radius:  1     3     0.5     5     6     8

        NodeMap map(8);
        mesh.ReMesh(map);
        TS_ASSERT_EQUALS(map.GetNewIndex(0), 0u);
        TS_ASSERT(map.IsDeleted(1));
        TS_ASSERT_EQUALS(map.GetNewIndex(2), 1u);
        TS_ASSERT_EQUALS(map.GetNewIndex(3), 2u);
        TS_ASSERT_EQUALS(map.GetNewIndex(4), 3u);
        TS_ASSERT_EQUALS(map.GetNewIndex(5), 4u);
        TS_ASSERT(map.IsDeleted(6));
        TS_ASSERT_EQUALS(map.GetNewIndex(7), 5u);

        TS_ASSERT_DELTA(mesh.GetCellRadius(0), 1.0, 1e-4);
        TS_ASSERT_DELTA(mesh.GetCellRadius(1), 3.0, 1e-4);
        TS_ASSERT_DELTA(mesh.GetCellRadius(2), 0.5, 1e-4);
        TS_ASSERT_DELTA(mesh.GetCellRadius(3), 5.0, 1e-4);
        TS_ASSERT_DELTA(mesh.GetCellRadius(4), 6.0, 1e-4);
        TS_ASSERT_DELTA(mesh.GetCellRadius(5), 8.0, 1e-4);
    }

    void TestArchiving() throw(Exception)
    {
        FileFinder archive_dir("archive", RelativeTo::ChasteTestOutput);
        std::string archive_file = "nodes_only_mesh.arch";
        ArchiveLocationInfo::SetMeshFilename("nodes_only_mesh");

        {
            TrianglesMeshReader<2,2> mesh_reader("mesh/test/data/disk_984_elements");
            TetrahedralMesh<2,2> generating_mesh;
            generating_mesh.ConstructFromMeshReader(mesh_reader);

            // Convert this to a NodesOnlyMesh
            NodesOnlyMesh<2> nodes_only_mesh;
            nodes_only_mesh.ConstructNodesWithoutMesh(generating_mesh);

            nodes_only_mesh.SetCellRadius(0, 1.12);
            nodes_only_mesh.SetCellRadius(1, 2.34);

            TS_ASSERT_EQUALS(nodes_only_mesh.mCellRadii.size(), 543u);
            TS_ASSERT_DELTA(nodes_only_mesh.GetCellRadius(0), 1.12, 1e-6);
            TS_ASSERT_DELTA(nodes_only_mesh.GetCellRadius(1), 2.34, 1e-6);

            AbstractTetrahedralMesh<2,2>* const p_mesh = &nodes_only_mesh;

            // Create output archive
            ArchiveOpener<boost::archive::text_oarchive, std::ofstream> arch_opener(archive_dir, archive_file);
            boost::archive::text_oarchive* p_arch = arch_opener.GetCommonArchive();

            (*p_arch) << p_mesh;
        }

        {
            /*
             * Should archive the most abstract class possible to check that
             * boost knows what individual classes are (but here AbstractMesh
             * doesn't have the methods below).
             */
            AbstractTetrahedralMesh<2,2>* p_mesh2;

            // Create an input archive
            ArchiveOpener<boost::archive::text_iarchive, std::ifstream> arch_opener(archive_dir, archive_file);
            boost::archive::text_iarchive* p_arch = arch_opener.GetCommonArchive();

            // Restore from the archive
            (*p_arch) >> p_mesh2;

            NodesOnlyMesh<2>* p_nodes_only_mesh = dynamic_cast<NodesOnlyMesh<2>*>(p_mesh2);

            // Check we have the right number of nodes & elements
            TS_ASSERT_EQUALS(p_nodes_only_mesh->GetNumNodes(), 543u);
            TS_ASSERT_EQUALS(p_nodes_only_mesh->GetNumElements(), 0u);

            // Check some node co-ordinates
            TS_ASSERT_DELTA(p_nodes_only_mesh->GetNode(0)->GetPoint()[0],  0.9980267283, 1e-6);
            TS_ASSERT_DELTA(p_nodes_only_mesh->GetNode(0)->GetPoint()[1], -0.0627905195, 1e-6);
            TS_ASSERT_DELTA(p_nodes_only_mesh->GetNode(1)->GetPoint()[0], 1.0, 1e-6);
            TS_ASSERT_DELTA(p_nodes_only_mesh->GetNode(1)->GetPoint()[1], 0.0, 1e-6);

            // Check some cell radii
            TS_ASSERT_EQUALS(p_nodes_only_mesh->mCellRadii.size(), 543u);
            TS_ASSERT_DELTA(p_nodes_only_mesh->GetCellRadius(0), 1.12, 1e-6);
            TS_ASSERT_DELTA(p_nodes_only_mesh->GetCellRadius(1), 2.34, 1e-6);

            // Tidy up
            delete p_mesh2;
        }
    }
};

#endif /*TESTNODESONLYMESH_HPP_*/
