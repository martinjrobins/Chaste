#ifndef TESTREFINEDTETRAHEDRALMESH_HPP_
#define TESTREFINEDTETRAHEDRALMESH_HPP_

#include <cxxtest/TestSuite.h>

#include "RefinedTetrahedralMesh.cpp"
#include "TrianglesMeshReader.cpp"


class TestRefinedTetrahedralMesh : public CxxTest::TestSuite
{
public:

//   void TestMeshConstructionFromMeshReader(void)
// is not yet implemented
    void TestConstructionFromCuboidMeshes3D()
    {
        // create fine mesh as CTM
        
        ConformingTetrahedralMesh<3,3> fine_mesh;
        
        fine_mesh.ConstructCuboid(6, 6, 6);
        double sixth=1.0L/6.0L;
        fine_mesh.Scale(sixth, sixth, sixth);
        
        // create coarse mesh as RTM
        RefinedTetrahedralMesh<3,3> coarse_mesh;
        
        coarse_mesh.ConstructCuboid(3, 3, 3);
        double third=1.0L/3.0L;
        coarse_mesh.Scale(third, third, third);
        
        // give fine mesh to coarse mesh and calculate node map
        coarse_mesh.SetFineMesh(&fine_mesh);
        
        const NodeMap &node_map = coarse_mesh.rGetCoarseFineNodeMap();
        
        TS_ASSERT_EQUALS(node_map.GetNewIndex(0), 0u);
        TS_ASSERT_EQUALS(node_map.GetNewIndex(1), 2u);
        TS_ASSERT_EQUALS(node_map.GetNewIndex(4), 14u);
        
        TS_ASSERT_EQUALS(node_map.GetNewIndex(63), 342u);
        //Top node is 4^3-1 and 7^3-1 respectively
        
        // We're not allowed to call SetFineMesh twice
        TS_ASSERT_THROWS_ANYTHING(coarse_mesh.SetFineMesh(&fine_mesh));
    }
    
    void TestCoarseFineElementsMap2D(void)
    {
        ConformingTetrahedralMesh<2,2> fine_mesh;
        fine_mesh.ConstructRectangularMesh(2, 2, false);
        double half=1.0L/2.0L;
        fine_mesh.Scale(half, half, 0.0);
        
        // create coarse mesh as RTM
        RefinedTetrahedralMesh<2,2> coarse_mesh;
        coarse_mesh.ConstructRectangularMesh(1, 1, false);
        
        // give fine mesh to coarse mesh and calculate node map
        coarse_mesh.SetFineMesh(&fine_mesh);
        
        std::set<Element<2,2>*> expected_elements;
        expected_elements.insert(fine_mesh.GetElement(6));
        expected_elements.insert(fine_mesh.GetElement(0));
        expected_elements.insert(fine_mesh.GetElement(3));
        expected_elements.insert(fine_mesh.GetElement(2));
        // Elements 0,2,3 and 6 form the upper right half of the space
        // (added in a funny order since set equality ought to cope with this)
        
        TS_ASSERT(expected_elements ==
                  coarse_mesh.GetFineElementsForCoarseElementIndex(0));
    }
    
    void TestTransferFlags()
    {
        ConformingTetrahedralMesh<2,2> fine_mesh;
        fine_mesh.ConstructRectangularMesh(4, 4, false);
        double half=1.0L/2.0L;
        fine_mesh.Scale(half, half, 0.0);
        
        // create coarse mesh as RTM
        RefinedTetrahedralMesh<2,2> coarse_mesh;
        coarse_mesh.ConstructRectangularMesh(2, 2, false);
        
        // give fine mesh to coarse mesh and calculate node map
        coarse_mesh.SetFineMesh(&fine_mesh);
        
        // Flag the right half of the coarse mesh
        ConformingTetrahedralMesh<2, 2>::ElementIterator i_coarse_element;
        for (i_coarse_element = coarse_mesh.GetElementIteratorBegin();
             i_coarse_element != coarse_mesh.GetElementIteratorEnd();
             i_coarse_element++)
        {
            Element<2,2> &element = **i_coarse_element;
            Point<2> centroid = Point<2>(element.CalculateCentroid());
            if (centroid[0] > 1.0)
            {
                element.Flag();
            }
            else
            {
                element.Unflag();
            }
        }
        
        // Flag the corresponding region of the fine mesh
        coarse_mesh.TransferFlags();
        
        // Check
        ConformingTetrahedralMesh<2, 2>::ElementIterator i_fine_element;
        for (i_fine_element = fine_mesh.GetElementIteratorBegin();
             i_fine_element != fine_mesh.GetElementIteratorEnd();
             i_fine_element++)
        {
            Element<2,2> &element = **i_fine_element;
            Point<2> centroid = Point<2>(element.CalculateCentroid());
            if (centroid[0] > 1.0)
            {
                TS_ASSERT(element.IsFlagged());
            }
            else
            {
                TS_ASSERT(!element.IsFlagged());
            }
        }
    }
    
    void TestFineNodesCoarseElementsMap2D(void)
    {
        ConformingTetrahedralMesh<2,2> fine_mesh;
        fine_mesh.ConstructRectangularMesh(2, 2, false);
        double half=1.0L/2.0L;
        fine_mesh.Scale(half, half, 0.0);
        
        // create coarse mesh as RTM
        RefinedTetrahedralMesh<2,2> coarse_mesh;
        coarse_mesh.ConstructRectangularMesh(1, 1, false);
        
        // give fine mesh to coarse mesh and calculate node map
        coarse_mesh.SetFineMesh(&fine_mesh);
        
        //node 1 is on the top edge of the fine mesh
        TS_ASSERT(coarse_mesh.GetElement(0) ==
                  coarse_mesh.GetACoarseElementForFineNodeIndex(1));
        //node 3 is on the left edge of the fine mesh
        TS_ASSERT(coarse_mesh.GetElement(1) ==
                  coarse_mesh.GetACoarseElementForFineNodeIndex(3));
    }
    
    void TestFineMeshIncorrect3D(void)
    {
        ConformingTetrahedralMesh<3,3> fine_mesh;
        
        fine_mesh.ConstructCuboid(5, 5, 5);
        double fifth=1.0L/5.0L;
        fine_mesh.Scale(fifth, fifth, fifth);
        
        // create coarse mesh as RTM
        RefinedTetrahedralMesh<3,3> coarse_mesh;
        
        coarse_mesh.ConstructCuboid(3, 3, 3);
        double third=1.0L/3.0L;
        coarse_mesh.Scale(third, third, third);
        
        // give fine mesh to coarse mesh and calculate node map
        // should throw because not every coarse node has a coincident fine node
        TS_ASSERT_THROWS_ANYTHING(coarse_mesh.SetFineMesh(&fine_mesh));
    }
    
    void TestFineAndCoarseDisc(void)
    {
        TrianglesMeshReader<2,2> fine_mesh_reader("mesh/test/data/disk_984_elements");
        ConformingTetrahedralMesh<2,2> fine_mesh;
        fine_mesh.ConstructFromMeshReader(fine_mesh_reader,1);
        
        TrianglesMeshReader<2,2> coarse_mesh_reader("mesh/test/data/DecimatedDisk");
        RefinedTetrahedralMesh<2,2> coarse_mesh;
        coarse_mesh.ConstructFromMeshReader(coarse_mesh_reader,1);
        
        TS_ASSERT_THROWS_ANYTHING(coarse_mesh.TransferFlags());
        
        coarse_mesh.SetFineMesh(&fine_mesh);
        
        // Flag the right semicircle of the coarse mesh
        ConformingTetrahedralMesh<2, 2>::ElementIterator i_coarse_element;
        for (i_coarse_element = coarse_mesh.GetElementIteratorBegin();
             i_coarse_element != coarse_mesh.GetElementIteratorEnd();
             i_coarse_element++)
        {
            Element<2,2> &element = **i_coarse_element;
            Point<2> centroid = Point<2>(element.CalculateCentroid());
            if (centroid[0] > 0)
            {
                element.Flag();
            }
            else
            {
                element.Unflag();
            }
        }
        
        // Flag the corresponding region of the fine mesh
        coarse_mesh.TransferFlags();
        
        // Check - flagged fine elements must have at least one node in a flagged coarse element
        ConformingTetrahedralMesh<2, 2>::ElementIterator i_fine_element;
        for (i_fine_element = fine_mesh.GetElementIteratorBegin();
             i_fine_element != fine_mesh.GetElementIteratorEnd();
             i_fine_element++)
        {
            Element<2,2> &fine_element = **i_fine_element;
            
            unsigned count = 0;
            for (unsigned i=0; i<fine_element.GetNumNodes(); i++)
            {
                unsigned node_index = fine_element.GetNodeGlobalIndex(i);
                const Element<2,2> *p_coarse_element = coarse_mesh.GetACoarseElementForFineNodeIndex(node_index);
                if (p_coarse_element->IsFlagged())
                {
                    count++;
                }
            }
            if (count == 0)
            {
                TS_ASSERT(!fine_element.IsFlagged());
            }
            else if (count > 1)
            {
                if (fine_element.GetIndex() != 421)
                {
                    TS_ASSERT(fine_element.IsFlagged());
                }
            }
            // Some fine elements just have 1 node touching the edge of the flagged
            // region in the coarse mesh, so are not flagged.
            // Element 421 is a special case - one edge lies on the border of the
            // flagged region.
        }
    }
};





#endif /*TESTREFINEDTETRAHEDRALMESH_HPP_*/
