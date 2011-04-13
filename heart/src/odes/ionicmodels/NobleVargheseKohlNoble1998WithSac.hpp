#ifndef _CML_noble_varghese_kohl_noble_1998_basic_with_sac_
#define _CML_noble_varghese_kohl_noble_1998_basic_with_sac_

#include "ChasteSerialization.hpp"
#include <boost/serialization/base_object.hpp>

#include <cmath>
#include <cassert>
#include "AbstractCardiacCell.hpp"
#include "Exception.hpp"
#include "AbstractStimulusFunction.hpp"
#include "OdeSystemInformation.hpp"


/**
 *  The Noble98 'Basic' model, but hand-altered to add a stretch activation channel ionic
 *  current, which is dependent on the stretch the cell is under (an additional member variable
 *  which can be set be the user/mechanics solver
 */ 
class CML_noble_varghese_kohl_noble_1998_basic_with_sac : public AbstractCardiacCell
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & mStretch;
        archive & boost::serialization::base_object<AbstractCardiacCell>(*this);
    }
private:

    /** The stretch the cell is under - affects the stretch-activated-channel ionic current */
    double mStretch;
    
public:
    CML_noble_varghese_kohl_noble_1998_basic_with_sac(boost::shared_ptr<AbstractIvpOdeSolver> pSolver,
                                             boost::shared_ptr<AbstractStimulusFunction> pIntracellularStimulus);

    ~CML_noble_varghese_kohl_noble_1998_basic_with_sac(void);

    double GetIIonic(const std::vector<double>* pStateVariables=NULL);

    void EvaluateYDerivatives (
            double var_environment__time,
            const std::vector<double> &rY,
            std::vector<double> &rDY);

    /** 
     *  Set the stretch (overloaded)
     *  @param stretch stretch
     */            
    void SetStretch(double stretch)
    {
        assert(stretch > 0.0);
        mStretch = stretch;
    }
    
    /** 
     *  Get the stretch
     */
    double GetStretch()
    {
        return mStretch;
    }
    
    /**
     * Get the intracellular calcium concentration
     *
     * @return the intracellular calcium concentration
     */
    double GetIntracellularCalciumConcentration()
    {
        return mStateVariables[16];
    }
};


#include "SerializationExportWrapper.hpp"
CHASTE_CLASS_EXPORT(CML_noble_varghese_kohl_noble_1998_basic_with_sac)
namespace boost
{
    namespace serialization
    {
        template<class Archive>
        inline void save_construct_data(
            Archive & ar, const CML_noble_varghese_kohl_noble_1998_basic_with_sac * t, const unsigned int fileVersion)
        {
            const boost::shared_ptr<AbstractIvpOdeSolver> p_solver = t->GetSolver();
            const boost::shared_ptr<AbstractStimulusFunction> p_stimulus = t->GetStimulusFunction();
            ar << p_solver;
            ar << p_stimulus;
        }

        template<class Archive>
        inline void load_construct_data(
            Archive & ar, CML_noble_varghese_kohl_noble_1998_basic_with_sac * t, const unsigned int fileVersion)
        {
            boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
            boost::shared_ptr<AbstractStimulusFunction> p_stimulus;
            ar >> p_solver;
            ar >> p_stimulus;
            ::new(t)CML_noble_varghese_kohl_noble_1998_basic_with_sac(p_solver, p_stimulus);
        }

    }

}

#endif
