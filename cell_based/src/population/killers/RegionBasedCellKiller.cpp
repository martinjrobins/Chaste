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
#include "RegionBasedCellKiller.hpp"
#include "PetscTools.hpp"

template <unsigned DIM>
RegionBasedCellKiller<DIM>::RegionBasedCellKiller(AbstractCellPopulation<DIM>* pCellPopulation,
                                                  c_vector<double, DIM> point,
                                                  c_vector<double, DIM> normal)
        : AbstractCellKiller<DIM>(pCellPopulation),
          mPointOnPlane(point)
{
    assert(norm_2(normal)>0.0);
    mNormalToPlane = normal/norm_2(normal);
}

template <unsigned DIM>
c_vector<double, DIM> RegionBasedCellKiller<DIM>::GetPointOnPlane() const
{
    return mPointOnPlane;
}

template <unsigned DIM>
c_vector<double, DIM> RegionBasedCellKiller<DIM>::GetNormalToPlane() const
{
    return mNormalToPlane;
}

template <unsigned DIM>
void RegionBasedCellKiller<DIM>::TestAndLabelCellsForApoptosisOrDeath()
{
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mpCellPopulation->Begin();
         cell_iter != this->mpCellPopulation->End();
         ++cell_iter)
    {
        c_vector<double, DIM> cell_location= this->mpCellPopulation->GetLocationOfCellCentre(*cell_iter);

        if (inner_prod(cell_location - mPointOnPlane,mNormalToPlane) > 0.0 )
        {
            cell_iter->Kill();
        }
    }
}

template<unsigned DIM>
void RegionBasedCellKiller<DIM>::OutputCellKillerParameters(out_stream& rParamsFile)
{
    *rParamsFile << "\t\t\t<PointOnPlane>";
    for (unsigned index=0; index<DIM-1u; index++)
    {
        *rParamsFile << mPointOnPlane[0] << ",";
    }
    *rParamsFile << mPointOnPlane[DIM-1u] <<  "</PointOnPlane> \n";

    *rParamsFile << "\t\t\t<NormalToPlane>";
     for (unsigned index=0; index<DIM-1u; index++)
     {
         *rParamsFile << mNormalToPlane[0] << ",";
     }
     *rParamsFile << mNormalToPlane[DIM-1u] <<  "</NormalToPlane> \n";

    // Call direct parent class
    AbstractCellKiller<DIM>::OutputCellKillerParameters(rParamsFile);
}



/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////

template class RegionBasedCellKiller<1>;
template class RegionBasedCellKiller<2>;
template class RegionBasedCellKiller<3>;


// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(RegionBasedCellKiller)