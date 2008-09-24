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

#ifndef ABSTRACTCACHEDMESHREADER_HPP_
#define ABSTRACTCACHEDMESHREADER_HPP_


/**
 * Abstract mesh reader class. Reads output generated by a mesh generator
 * and converts it to a standard format for use in constructing a finite
 * element mesh structure.
 *
 * A derived class TrianglesMeshReader exists for reading meshes generated
 * by Triangles (in 2-d) and TetGen (in 3-d).
 *
 * A derived class MemfemMeshReader reads 3D data from the Tulane University code
 *
 * A derived class FemlabMeshReader reads 2D data from Femlab or Matlab PDEToolbox
 *
 */


#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Exception.hpp"
#include "AbstractMeshReader.hpp"

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
class AbstractCachedMeshReader : public AbstractMeshReader<ELEMENT_DIM, SPACE_DIM> 
{
protected:
    unsigned mNumNodeAttributes; /**< Is the number of attributes stored at each node */
    unsigned mMaxNodeBdyMarker; /**< Is the maximum node boundary marker */
    unsigned mNumElementNodes; /** Is the number of nodes per element*/
    unsigned mNumElementAttributes; /**< Is the number of attributes stored for each element */
    unsigned mMaxFaceBdyMarker; /**< Is the maximum face (or edge) boundary marker */

    std::vector<std::string> mNodeRawData;  /**< Contents of node input file with comments removed */
    std::vector<std::string> mElementRawData;  /**< Contents of element input file with comments removed */
    std::vector<std::string> mFaceRawData;  /**< Contents of face (or edge) input file with comments removed */

    std::vector< std::vector<double> > mNodeData; /**< Is an array of node coordinates ((i,j)th entry is the jth coordinate of node i)*/
    std::vector< std::vector<unsigned> > mElementData; /**< Is an array of the nodes in each element ((i,j)th entry is the jth node of element i) */
    std::vector< std::vector<unsigned> > mFaceData; /**< Is an array of the nodes in each face ((i,j)th entry is the jth node of face i) */

    std::vector< std::vector<double> >::iterator mpNodeIterator; /**< Is an iterator for the node data */
    std::vector< std::vector<unsigned> >::iterator mpElementIterator; /**< Is an iterator for the element data */
    std::vector< std::vector<unsigned> >::iterator mpFaceIterator; /**< Is an iterator for the face data */

    bool mIndexFromZero; /**< True if input data is numbered from zero, false otherwise */

    std::vector<std::string> GetRawDataFromFile(std::string fileName); /**< Reads an input file fileName, removes comments (indicated by a #) and blank lines */


public:
    AbstractCachedMeshReader() /**< Constructor */
    {
        mNumNodeAttributes = 0;
        mMaxNodeBdyMarker = 0;
        mNumElementNodes = 0;
        mNumElementAttributes = 0;
        mMaxFaceBdyMarker = 0;

        // We have initialized all numeric variables to zero

        mIndexFromZero = false; // Initially assume that nodes are not numbered from zero
    }
    virtual ~AbstractCachedMeshReader()
    {}


    unsigned GetNumElements() const
    {
        return mElementData.size();
    } /**< Returns the number of elements in the mesh */
    unsigned GetNumNodes() const
    {
        return mNodeData.size();
    } /**< Returns the number of nodes in the mesh */
    unsigned GetNumFaces() const
    {
        return mFaceData.size();
    } /**< Returns the number of faces in the mesh (synonym of GetNumEdges()) */
    unsigned GetNumEdges() const
    {
        return mFaceData.size();
    }    /**< Returns the number of edges in the mesh (synonym of GetNumFaces()) */

    unsigned GetMaxNodeIndex(); /**< Returns the maximum node index */
    unsigned GetMinNodeIndex(); /**< Returns the minimum node index */

    std::vector<double> GetNextNode(); /**< Returns a vector of the coordinates of each node in turn */
    void Reset(); /**< Resets pointers to beginning*/
    std::vector<unsigned> GetNextElement(); /**< Returns a vector of the nodes of each element in turn */
    std::vector<unsigned> GetNextEdge(); /**< Returns a vector of the nodes of each edge in turn (synonym of GetNextFace()) */
    std::vector<unsigned> GetNextFace(); /**< Returns a vector of the nodes of each face in turn (synonym of GetNextEdge()) */
};


/**
 * Reads an input file fileName, removes comments (indicated by a #) and blank
 * lines and returns a vector of strings. Each string corresponds to one line
 * of the input file.
 *
 *
 */


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
std::vector<std::string> AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetRawDataFromFile(std::string fileName)
{
    // Open raw data file

    std::vector<std::string> RawDataFromFile;
    std::ifstream dataFile(fileName.c_str());

    // Checks that input file has been opened correctly. If not throws an
    // exception that should be caught by the user.
    if (!dataFile.is_open())
    {
        EXCEPTION("Could not open data file "+fileName+" .");
    }

    // Read each line in turn
    std::string RawLineFromFile;
    getline(dataFile, RawLineFromFile);

    while (dataFile)
    {
        //Remove comments (everything from a hash to the end of the line)
        //If there is no hash, then hashLocation = string::npos = -1 = 4294967295 = UINT_MAX
        //(so it works with unsigneds but is a little nasty)
        long hash_location=RawLineFromFile.find('#',0);
        if (hash_location >= 0)
        {
            RawLineFromFile=RawLineFromFile.substr(0,hash_location);
        }
        //Remove blank lines.  This is unnecessary, since the tokenizer will
        //ignore blank lines anyway.
        long not_blank_location=RawLineFromFile.find_first_not_of(" \t",0);
        if (not_blank_location >= 0)
        {
            RawDataFromFile.push_back(RawLineFromFile);
        }

        // Move onto next line
        getline(dataFile, RawLineFromFile);
    }

    dataFile.close(); // Closes the data file
    return(RawDataFromFile);
}



/**
 *  Returns the maximum node index. Used in testing to check that output nodes
 *  are always indexed from zero even if they are input indexed from one.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
unsigned AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetMaxNodeIndex()
{
    //Initialize an interator for the vector of nodes
    std::vector<std::vector<unsigned> >::iterator the_iterator;

    unsigned max_node_index = 0; // Nice if it were negative

    for (the_iterator = mElementData.begin(); the_iterator < mElementData.end(); the_iterator++)
    {
        std::vector<unsigned> indices = *the_iterator; // the_iterator points at each line in turn

        for (unsigned i = 0; i < ELEMENT_DIM+1; i++)
        {
            if ( indices[i] >  max_node_index)
            {
                max_node_index = indices[i];
            }
        }
    }

    return max_node_index;
}



/**
 *  Returns the minimum node index. Used in testing to check that output nodes
 *  are always indexed from zero even if they are input indexed from one.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
unsigned AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetMinNodeIndex()
{
    //Initialize an interator for the vector of nodes
    std::vector<std::vector<unsigned> >::iterator the_iterator;

    unsigned min_node_index = UINT_MAX; // A large integer

    for (the_iterator = mElementData.begin(); the_iterator < mElementData.end(); the_iterator++)
    {
        std::vector<unsigned> indices = *the_iterator; // the_iterator points at each line in turn

        for (unsigned i = 0; i < ELEMENT_DIM+1; i++)
        {
            if (indices[i] < min_node_index)
            {
                min_node_index = indices[i];
            }
        }
    }

    return min_node_index;
}


/**
 *  Returns a vector of the coordinates of each node in turn, starting with
 *  node 0 the first time it is called followed by nodes 1, 2, ... , mNumNodes-1.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
std::vector<double> AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetNextNode()
{
    // Checks that there are still some nodes left to read. If not throws an
    // exception that must be caught by the user.
    if (mpNodeIterator == mNodeData.end())
    {
        EXCEPTION("All nodes already got");
    }

    std::vector<double> next_node = *mpNodeIterator;

    mpNodeIterator++;

    return next_node;
}



/**
 *  Returns a vector of the nodes of each element in turn, starting with
 *  element 0 the first time it is called followed by elements 1, 2, ... ,
 *  mNumElements-1.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
std::vector<unsigned> AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetNextElement()
{
    // Checks that there are still some elements left to read. If not throws an
    // exception that must be caught by the user.
    if (mpElementIterator == mElementData.end())
    {
        EXCEPTION("All elements already got");
    }

    std::vector<unsigned> next_element = *mpElementIterator;

    mpElementIterator++;

    return next_element;
}


template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
void AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::Reset()
{
    mpElementIterator = mElementData.begin();
    mpFaceIterator = mFaceData.begin();
    mpNodeIterator = mNodeData.begin();
}


/**
 *  Returns a vector of the nodes of each face in turn, starting with face 0 the
 *  first time it is called followed by faces 1, 2, ... , mNumFaces-1.
 *
 *  Is a synonum of GetNextEdge(). The two functions can be used interchangeably,
 *  i.e. they use the same iterator.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
std::vector<unsigned> AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetNextFace()
{
    // Checks that there are still some faces left to read. If not throws an
    // exception that must be caught by the user.
    if (mpFaceIterator == mFaceData.end())
    {
        EXCEPTION("All faces (or edges) already got");
    }

    std::vector<unsigned> next_face = *mpFaceIterator;

    mpFaceIterator++;

    return next_face;
}



/**
 *  Returns a vector of the nodes of each edge in turn, starting with edge 0 the
 *  first time it is called followed by edges 1, 2, ... , mNumFaces-1.
 *
 *  Is a synonym of GetNextFace(). The two functions can be used interchangeably,
 *  i.e. they use the same iterator.
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM>
std::vector<unsigned> AbstractCachedMeshReader<ELEMENT_DIM, SPACE_DIM>::GetNextEdge()
{
    return GetNextFace();
}


#endif /*ABSTRACTCACHEDMESHREADER_HPP_*/
