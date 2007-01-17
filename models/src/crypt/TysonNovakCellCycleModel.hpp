#ifndef TYSONNOVAKCELLCYCLEMODEL_HPP_
#define TYSONNOVAKCELLCYCLEMODEL_HPP_
#include "AbstractCellCycleModel.hpp"
#include "TysonNovak2001OdeSystem.hpp"
#include "BackwardEulerIvpOdeSolver.hpp"
#include "CancerParameters.hpp"
#include "SimulationTime.hpp"
/**
 *  Tyson Novak cell cycle model
 *
 *  Time taken to progress through the cycle is actually deterministic as ODE system
 *  independent of external factors.
 */
class TysonNovakCellCycleModel : public AbstractCellCycleModel
{
private:    
    TysonNovak2001OdeSystem mOdeSystem;
    BackwardEulerIvpOdeSolver mSolver;
    double mLastTime;
    std::vector <double> mProteinConcentrations;
  
    
public:
    
    TysonNovakCellCycleModel();

    virtual bool ReadyToDivide(std::vector<double> cellCycleInfluences = std::vector<double>());
    
    virtual void ResetModel();
    
    std::vector< double > GetProteinConcentrations();
    
    AbstractCellCycleModel *CreateCellCycleModel();
    
};





#endif /*TYSONNOVAKCELLCYCLEMODEL_HPP_*/
