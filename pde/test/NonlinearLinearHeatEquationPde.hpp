#ifndef _NONLINEARLINEARHEATEQUATIONPDE_HPP_
#define _NONLINEARLINEARHEATEQUATIONPDE_HPP_


#include "AbstractNonlinearEllipticPde.hpp"

/**
 * Linear heat equation, using the nonlinear class format.
 */
template <int SPACE_DIM>
class NonlinearLinearHeatEquationPde : public AbstractNonlinearEllipticPde<SPACE_DIM>
{
public:

    double ComputeLinearSourceTerm(ChastePoint<SPACE_DIM> )
    {
        return 1.0;
    }
    
    double ComputeNonlinearSourceTerm(ChastePoint<SPACE_DIM> , double )
    {
        return 0.0;
    }
    
    c_matrix<double, SPACE_DIM, SPACE_DIM> ComputeDiffusionTerm(ChastePoint<SPACE_DIM> , double u)
    {
        return identity_matrix<double>(SPACE_DIM);
    }
    
    c_matrix<double, SPACE_DIM, SPACE_DIM> ComputeDiffusionTermPrime(ChastePoint<SPACE_DIM> , double )
    {
        return identity_matrix<double>(SPACE_DIM)*0.0;
    }
    
    double ComputeNonlinearSourceTermPrime(ChastePoint<SPACE_DIM> , double )
    {
        return 0.0;
    }
    
};


#endif //_NONLINEARLINEARHEATEQUATIONPDE_HPP_
