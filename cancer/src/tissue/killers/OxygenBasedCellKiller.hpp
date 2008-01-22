#ifndef OXYGENBASEDCELLKILLER_HPP_
#define OXYGENBASEDCELLKILLER_HPP_

#include "AbstractCellKiller.hpp"
#include "CancerParameters.hpp"
#include "SimulationTime.hpp"
#include "Tissue.cpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

/** 
 *  Kills cells that have experienced a continuous period of hypoxia.
 * 
 *  The non-dimensionalised oxygen concentration at which cells become 
 *  hypoxic is optionally passed into the constructor. 
 * 
 */

template <unsigned SPACE_DIM>
class OxygenBasedCellKiller : public AbstractCellKiller<SPACE_DIM>
{
private: 
    double mHypoxicConcentration;
     
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & boost::serialization::base_object<AbstractCellKiller<SPACE_DIM> >(*this);        
    }
    
public:
    OxygenBasedCellKiller(Tissue<SPACE_DIM>* pTissue, double concentration=CancerParameters::Instance()->GetHepaOneCellHypoxicConcentration())
        : AbstractCellKiller<SPACE_DIM>(pTissue),
          mHypoxicConcentration(concentration)
    {
    }
    
    void SetHypoxicConcentration(double hypoxicConcentration)
    {
        mHypoxicConcentration = hypoxicConcentration;    
    }
   
    double GetHypoxicConcentration() const
    {
        return mHypoxicConcentration;
    }
    
    /**
     *  Starts apoptosis if the cell has has been hypoxic for longer than 
     *  some critical period, and  it is currently hypoxic, and a random number 
     *  is less than some probability of death (which scales linearly with the 
     *  local oxygen concentration).
     */  
    void TestAndLabelSingleCellForApoptosis(TissueCell& rCell)
    {   
        if (rCell.GetCellType()==NECROTIC && !(rCell.HasApoptosisBegun()))
        {
            rCell.StartApoptosis();
        }          
    }
    
    /**
     * Loops over cells and starts apoptosis if the cell satisfies certain
     * conditions 
     */
     virtual void TestAndLabelCellsForApoptosisOrDeath()
    {      
        for( typename Tissue<SPACE_DIM>::Iterator cell_iter = this->mpTissue->Begin();
            cell_iter != this->mpTissue->End();
            ++cell_iter)
        {               
            TestAndLabelSingleCellForApoptosis(*cell_iter);
        }
    }     
};

#include "TemplatedExport.hpp"

EXPORT_TEMPLATE_CLASS_SAME_DIMS(OxygenBasedCellKiller)

namespace boost
{
namespace serialization
{
/**
 * Serialize information required to construct a TissueSimulation.
 */
template<class Archive, unsigned DIM>
inline void save_construct_data(
    Archive & ar, const OxygenBasedCellKiller<DIM> * t, const BOOST_PFTO unsigned int file_version)
{
    // save data required to construct instance
    const Tissue<DIM>* const p_tissue = t->GetTissue();
    ar << p_tissue;
    double conc = t->GetHypoxicConcentration();
    ar << conc;
}

/**
 * De-serialize constructor parameters and initialise Tissue.
 */
template<class Archive, unsigned DIM>
inline void load_construct_data(
    Archive & ar, OxygenBasedCellKiller<DIM> * t, const unsigned int file_version)
{
    // retrieve data from archive required to construct new instance
    Tissue<DIM>* p_tissue;
    ar >> p_tissue;
    double conc;
    ar >> conc;
    // invoke inplace constructor to initialize instance
    ::new(t)OxygenBasedCellKiller<DIM>(p_tissue, conc);
}
}
} // namespace ...

#endif /*OXYGENBASEDCELLKILLER_HPP_*/
