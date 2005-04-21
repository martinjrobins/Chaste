#ifndef _ABSTRACTLINEARELLIPTICPDE_HPP_
#define _ABSTRACTLINEARELLIPTICPDE_HPP_


#include "MatrixDouble.hpp"
#include "Point.hpp"

template <int SPACE_DIM>
class AbstractLinearEllipticPde
{
public:
    virtual double ComputeLinearSourceTerm(Point<SPACE_DIM> x)=0;
    
    virtual double ComputeNonlinearSourceTerm(Point<SPACE_DIM> x,
                                              double u)=0;

    virtual MatrixDouble ComputeDiffusionTerm(Point<SPACE_DIM> x)=0;
};

#endif //_ABSTRACTLINEARELLIPTICPDE_HPP_
