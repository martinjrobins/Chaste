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
#ifndef MEINEKEINTERACTIONWITHVARIABLESPRINGCONSTANTSFORCE_HPP_
#define MEINEKEINTERACTIONWITHVARIABLESPRINGCONSTANTSFORCE_HPP_

#include "MeinekeInteractionForce.hpp"
#include "MeshBasedTissue.hpp"
#include "IngeWntSwatCellCycleModel.hpp"
#include "VoronoiTessellation.hpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

template<unsigned DIM>
class MeinekeInteractionWithVariableSpringConstantsForce : public MeinekeInteractionForce<DIM>
{
    friend class TestForces;
    
private :

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        // If Archive is an output archive, then '&' resolves to '<<'
        // If Archive is an input archive, then '&' resolves to '>>'
        archive & boost::serialization::base_object<MeinekeInteractionForce<DIM> >(*this);
        archive & mUseEdgeBasedSpringConstant;
        archive & mUseMutantSprings;
        archive & mMutantMutantMultiplier;
        archive & mNormalMutantMultiplier;
        archive & mUseBCatSprings;
        archive & mUseApoptoticSprings;
    }

protected :
    
    /** Whether to use spring constant proportional to cell-cell contact length/area (defaults to false) */
    bool mUseEdgeBasedSpringConstant;

    /** Whether to use different stiffnesses depending on whether either cell is a mutant */
    bool mUseMutantSprings;

    /** Multiplier for spring stiffness if mutant */
    double mMutantMutantMultiplier;

    /** Multiplier for spring stiffness if mutant */
    double mNormalMutantMultiplier;

    /** Use springs which are dependent on beta-catenin levels */
    bool mUseBCatSprings;

    /** Use springs which are dependent on whether cells are apoptotic */
    bool mUseApoptoticSprings;

public :

    MeinekeInteractionWithVariableSpringConstantsForce();
    
    ~MeinekeInteractionWithVariableSpringConstantsForce();

    /**
     * Use an edge-based spring constant
     */
    void SetEdgeBasedSpringConstant(bool useEdgeBasedSpringConstant);

    /**
     * Use Different spring strengths depending on two cells:
     * Normal-normal, Normal-mutant, mutant-mutant
     */
    void SetMutantSprings(bool useMutantSprings, double mutantMutantMultiplier=2, double normalMutantMultiplier=1.5);

    /**
     * Use the amount of B-Catenin on an edge to find spring constant.
     */
    void SetBCatSprings(bool useBCatSprings);

    /**
     * Set spring stiffness to be dependent on whether cells are necrotic
     */
    void SetApoptoticSprings(bool useApoptoticSprings);    
        
    double VariableSpringConstantMultiplicationFactor(unsigned nodeAGlobalIndex, unsigned nodeBGlobalIndex, AbstractTissue<DIM>& rTissue, double distanceBetweenNodes, double restLength);

    /// \todo eventually this should be a force contribution (see #627)
    void AddForceContribution(std::vector<c_vector<double, DIM> >& rForces,
                                 AbstractTissue<DIM>& rTissue);
 
};

template<unsigned DIM>
MeinekeInteractionWithVariableSpringConstantsForce<DIM>::MeinekeInteractionWithVariableSpringConstantsForce()
   : MeinekeInteractionForce<DIM>()
{    
    // Edge-based springs
    mUseEdgeBasedSpringConstant = false;

    // Cell-type dependent springs
    mUseMutantSprings = false;
    mMutantMutantMultiplier = DOUBLE_UNSET;
    mNormalMutantMultiplier = DOUBLE_UNSET;

    // Beta-cat springs
    mUseBCatSprings = false;

    // Apoptotic springs
    mUseApoptoticSprings = false;
}

template<unsigned DIM>
MeinekeInteractionWithVariableSpringConstantsForce<DIM>::~MeinekeInteractionWithVariableSpringConstantsForce()
{
}

template<unsigned DIM>
void MeinekeInteractionWithVariableSpringConstantsForce<DIM>::SetEdgeBasedSpringConstant(bool useEdgeBasedSpringConstant)
{
    assert(DIM == 2);
    mUseEdgeBasedSpringConstant = useEdgeBasedSpringConstant;
}

template<unsigned DIM>
void MeinekeInteractionWithVariableSpringConstantsForce<DIM>::SetMutantSprings(bool useMutantSprings, double mutantMutantMultiplier, double normalMutantMultiplier)
{
    mUseMutantSprings = useMutantSprings;
    mMutantMutantMultiplier = mutantMutantMultiplier;
    mNormalMutantMultiplier = normalMutantMultiplier;
}

template<unsigned DIM>
void MeinekeInteractionWithVariableSpringConstantsForce<DIM>::SetBCatSprings(bool useBCatSprings)
{
    mUseBCatSprings = useBCatSprings;
}

template<unsigned DIM>
void MeinekeInteractionWithVariableSpringConstantsForce<DIM>::SetApoptoticSprings(bool useApoptoticSprings)
{
    mUseApoptoticSprings = useApoptoticSprings;
}

template<unsigned DIM>
double MeinekeInteractionWithVariableSpringConstantsForce<DIM>::VariableSpringConstantMultiplicationFactor(unsigned nodeAGlobalIndex, unsigned nodeBGlobalIndex,
                                                           AbstractTissue<DIM>& rTissue, double distanceBetweenNodes, double restLength)
{
    double multiplication_factor = 1.0;
    
    TissueCell& r_cell_A = rTissue.rGetCellUsingLocationIndex(nodeAGlobalIndex);
    TissueCell& r_cell_B = rTissue.rGetCellUsingLocationIndex(nodeBGlobalIndex);
    
    if (mUseEdgeBasedSpringConstant)
    {
        assert(rTissue.HasMesh());
        assert(!mUseBCatSprings);   // don't want to do both (both account for edge length)

        VoronoiTessellation<DIM>& tess = (static_cast<MeshBasedTissue<DIM>*>(&rTissue))->rGetVoronoiTessellation();

        multiplication_factor = tess.GetEdgeLength(nodeAGlobalIndex, nodeBGlobalIndex)*sqrt(3);
    }

    if (mUseMutantSprings)
    {
        unsigned number_of_mutants=0;

        if (r_cell_A.GetMutationState() == APC_TWO_HIT || r_cell_A.GetMutationState() == BETA_CATENIN_ONE_HIT)
        {
            // If cell A is mutant
            number_of_mutants++;
        }

        if (r_cell_B.GetMutationState() == APC_TWO_HIT || r_cell_B.GetMutationState() == BETA_CATENIN_ONE_HIT)
        {
            // If cell B is mutant
            number_of_mutants++;
        }

        switch (number_of_mutants)
        {
            case 1u:
            {
                multiplication_factor *= mNormalMutantMultiplier;
                break;
            }
            case 2u:
            {
                multiplication_factor *= mMutantMutantMultiplier;
                break;
            }
        }
    }

    if (mUseBCatSprings)
    {
        assert(rTissue.HasMesh());
        // If using beta-cat dependent springs, both cell-cycle models has better be IngeWntSwatCellCycleModel
        IngeWntSwatCellCycleModel* p_model_A = dynamic_cast<IngeWntSwatCellCycleModel*>(r_cell_A.GetCellCycleModel());
        IngeWntSwatCellCycleModel* p_model_B = dynamic_cast<IngeWntSwatCellCycleModel*>(r_cell_B.GetCellCycleModel());

        assert(!mUseEdgeBasedSpringConstant);   // This already adapts for edge lengths - don't want to do it twice.
        double beta_cat_cell_1 = p_model_A->GetMembraneBoundBetaCateninLevel();
        double beta_cat_cell_2 = p_model_B->GetMembraneBoundBetaCateninLevel();

        VoronoiTessellation<DIM>& tess = (static_cast<MeshBasedTissue<DIM>*>(&rTissue))->rGetVoronoiTessellation();

        double perim_cell_1 = tess.GetFacePerimeter(nodeAGlobalIndex);
        double perim_cell_2 = tess.GetFacePerimeter(nodeBGlobalIndex);
        double edge_length_between_1_and_2 = tess.GetEdgeLength(nodeAGlobalIndex, nodeBGlobalIndex);

        double beta_cat_on_cell_1_edge = beta_cat_cell_1 *  edge_length_between_1_and_2 / perim_cell_1;
        double beta_cat_on_cell_2_edge = beta_cat_cell_2 *  edge_length_between_1_and_2 / perim_cell_2;

        double min_beta_Cat_of_two_cells = std::min(beta_cat_on_cell_1_edge, beta_cat_on_cell_2_edge);

        double beta_cat_scaling_factor = CancerParameters::Instance()->GetBetaCatSpringScaler();
        multiplication_factor *= min_beta_Cat_of_two_cells / beta_cat_scaling_factor;
    }

    if (mUseApoptoticSprings)
    {
        if (r_cell_A.GetCellType()==APOPTOTIC || r_cell_B.GetCellType()==APOPTOTIC)
        {
            double spring_a_stiffness = 2.0*CancerParameters::Instance()->GetSpringStiffness();
            double spring_b_stiffness = 2.0*CancerParameters::Instance()->GetSpringStiffness();

            if (r_cell_A.GetCellType()==APOPTOTIC)
            {
                if (distanceBetweenNodes - restLength > 0) // if under tension
                {
                    spring_a_stiffness = CancerParameters::Instance()->GetApoptoticSpringTensionStiffness();
                }
                else // if under compression
                {
                    spring_a_stiffness = CancerParameters::Instance()->GetApoptoticSpringCompressionStiffness();
                }
            }
            if (r_cell_B.GetCellType()==APOPTOTIC)
            {
                if (distanceBetweenNodes - restLength > 0) // if under tension
                {
                    spring_b_stiffness = CancerParameters::Instance()->GetApoptoticSpringTensionStiffness();
                }
                else // if under compression
                {
                    spring_b_stiffness = CancerParameters::Instance()->GetApoptoticSpringCompressionStiffness();
                }
            }

            multiplication_factor *= 1.0 / (( 1.0/spring_a_stiffness + 1.0/spring_b_stiffness)*CancerParameters::Instance()->GetSpringStiffness());
        }
    }
    
    return multiplication_factor;
}

template<unsigned DIM>
void MeinekeInteractionWithVariableSpringConstantsForce<DIM>::AddForceContribution(std::vector<c_vector<double, DIM> >& rForces,
                                                           AbstractTissue<DIM>& rTissue)
{
    for (typename MeshBasedTissue<DIM>::SpringIterator spring_iterator=(static_cast<MeshBasedTissue<DIM>*>(&rTissue))->SpringsBegin();
        spring_iterator!=(static_cast<MeshBasedTissue<DIM>*>(&rTissue))->SpringsEnd();
        ++spring_iterator)
    {
        unsigned nodeA_global_index = spring_iterator.GetNodeA()->GetIndex();
        unsigned nodeB_global_index = spring_iterator.GetNodeB()->GetIndex();

        c_vector<double, DIM> force = CalculateForceBetweenNodes(nodeA_global_index, nodeB_global_index, rTissue);

        rForces[nodeB_global_index] -= force;
        rForces[nodeA_global_index] += force;
    }
}

#include "TemplatedExport.hpp"

EXPORT_TEMPLATE_CLASS_SAME_DIMS(MeinekeInteractionWithVariableSpringConstantsForce)


#endif /*MEINEKEINTERACTIONWITHVARIABLESPRINGCONSTANTSFORCE_HPP_*/
