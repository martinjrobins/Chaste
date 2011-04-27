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

#ifndef MIXEDDIMENSIONMESH_HPP_
#define MIXEDDIMENSIONMESH_HPP_

#include "AbstractTetrahedralMesh.hpp"
#include "AbstractMeshReader.hpp"


/**
 * A tetrahedral mesh that also supports embedded 1D cable elements.
 * 
 * Could be used for Purkinje or blood vessels, etc.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
class MixedDimensionMesh : public AbstractTetrahedralMesh< ELEMENT_DIM, SPACE_DIM>
{
public:
    /**
     * Construct the mesh using a MeshReader.
     *
     * @param rMeshReader the mesh reader
     */
    void ConstructFromMeshReader(AbstractMeshReader<ELEMENT_DIM,SPACE_DIM>& rMeshReader);
    
    /**
     * Get the number of cable elements that are actually in use.
     */
    unsigned GetNumCableElements() const;
    
    /**
     * Get the cable element with a given index in the mesh.
     *
     * @param index the global index of the cable element
     * @return a pointer to the cable element.
     */
    Element<1u, SPACE_DIM>* GetCableElement(unsigned index) const;


protected:

    /**
     * Overridden solve node mapping method.
     *
     * @param index the global index of the node
     */
    unsigned SolveNodeMapping(unsigned index) const;

    /**
     * Overridden solve element mapping method.
     *
     * @param index the global index of the element
     */
    unsigned SolveElementMapping(unsigned index) const;

    /**
     * Overridden solve boundary element mapping method.
     *
     * @param index the global index of the boundary element
     */
    unsigned SolveBoundaryElementMapping(unsigned index) const;

private:
    /** The elements making up the 1D cables */
    std::vector<Element<1u, SPACE_DIM>*> mCableElements;
};

#endif /*MIXEDDIMENSIONMESH_HPP_*/