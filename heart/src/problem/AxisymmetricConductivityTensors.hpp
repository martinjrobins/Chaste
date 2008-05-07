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
#ifndef AXISYMMETRICCONDUCTIVITYTENSORS_HPP_
#define AXISYMMETRICCONDUCTIVITYTENSORS_HPP_

#include "AbstractConductivityTensors.hpp"

// Axisymmetric anisotropic conductivity only makes sense in 3D
class AxisymmetricConductivityTensors : public AbstractConductivityTensors<3>
{

private:

    void ReadOrientationVectorFromFile (c_vector<double,3u>& orientVector)
    {
        std::vector<double> tokens;       
        
        unsigned num_elems = this->GetTokensAtNextLine(tokens);

        if (num_elems != 3u)
        {
            this->CloseFibreOrientationFile();
            EXCEPTION("ttlon file should contain 3 values per element");                
        }
        
        for (unsigned i=0; i<3u; i++)
        {
            orientVector[i] = tokens[i];
        }
    }
    
public:

    void Init() throw (Exception)
    {
        c_matrix<double, 3u, 3u> conductivity_matrix(zero_matrix<double>(3u,3u));
        
        for (unsigned dim=0; dim<3u; dim++)
        {
            conductivity_matrix(dim,dim) = this->mConstantConductivities(dim);     
        }
        
        if (!this->mUseNonConstantConductivities && !this->mUseFibreOrientation)
        {
            // Constant tensor for every element
            this->mTensors.push_back(conductivity_matrix);
        }
        else
        {
            c_vector<double,3u> fibre_vector((zero_vector<double>(3u)));
            fibre_vector[0]=1.0;
                        
            if (this->mUseFibreOrientation)
            {
                this->OpenFibreOrientationFile();
                this->mNumElements = this->GetNumElementsFromFile();
            }
            else
            {
                this->mNumElements = this->mpNonConstantConductivities->size();
            }
                
            // reserve() allocates all the memory at once, more efficient than relying 
            // on the automatic reallocation scheme.
            this->mTensors.reserve(this->mNumElements);
                           
            for (unsigned element_index=0; element_index<this->mNumElements; element_index++)
            {
                /*
                 *  For every element of the mesh we compute its tensor like (from 
                 * "Laminar Arrangement of VentricularMyocites Influences Electrical 
                 * Behavior of the Heart", Darren et al. 2007):
                 * 
                 *                         [g_f  0   0 ] [a_f']
                 *  tensor = [a_f a_l a_n] [ 0  g_l  0 ] [a_l']
                 *                         [ 0   0  g_n] [a_n']
                 * 
                 *              [x_i]
                 *  where a_i = [y_i], i={f,l,n} are read from the fibre orientation file and
                 *              [z_i]
                 * 
                 *  g_f = fibre/longitudinal conductivity (constant or element specific)
                 *  g_l = laminar/transverse conductivity (constant or element specific)
                 *  g_n = normal conductivity (constant or element specific) 
                 * 
                 */
                
                if (this->mUseNonConstantConductivities)
                {
                    for (unsigned dim=0; dim<3u; dim++)
                    {
                        conductivity_matrix(dim,dim) = (*this->mpNonConstantConductivities)[element_index][dim];     
                    }                    
                }      
                
                if (this->mUseFibreOrientation)
                {
                    ReadOrientationVectorFromFile(fibre_vector);                    
                }                    
                              
                this->mTensors.push_back( conductivity_matrix(1,1) * identity_matrix<double>(3u) +
                                          (conductivity_matrix(0,0) - conductivity_matrix(1,1)) * outer_prod(fibre_vector,fibre_vector));
            }
            
            if (this->mUseFibreOrientation)
            {
                this->CloseFibreOrientationFile();
            }
        }
        
        this->mInitialised = true;        
    }
    
};


#endif /*AXISYMMETRICCONDUCTIVITYTENSORS_HPP_*/
