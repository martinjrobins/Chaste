#ifndef _ABSTRACTASSEMBLER_HPP_
#define _ABSTRACTASSEMBLER_HPP_

#include "AbstractBasisFunction.hpp"
#include "LinearBasisFunction.hpp"
#include "GaussianQuadratureRule.hpp"


/**
 * Abstract base class for all the PDE assemblers.
 * 
 * Currently this provides methods for selecting what type of basis function to
 * use, and how many quadrature points per dimension.
 */
template <int ELEMENT_DIM, int SPACE_DIM>
class AbstractAssembler
{
protected:
	bool mWeAllocatedBasisFunctionMemory;

	/*< Basis function for use with normal elements */
	AbstractBasisFunction<ELEMENT_DIM> *mpBasisFunction;
	/*< Basis function for use with boundary elements */
	AbstractBasisFunction<ELEMENT_DIM-1> *mpSurfaceBasisFunction;
	
	/*< Quadrature rule for use on normal elements */
	GaussianQuadratureRule<ELEMENT_DIM> *mpQuadRule;
	/*< Quadrature rule for use on boundary elements */
	GaussianQuadratureRule<ELEMENT_DIM-1> *mpSurfaceQuadRule;
	
public:
	/**
	 * Default constructor. Uses linear basis functions.
	 * 
	 * @param numPoints Number of quadrature points to use per dimension.
	 */
	AbstractAssembler(int numPoints = 2)
	{
		mWeAllocatedBasisFunctionMemory = false;
		LinearBasisFunction<ELEMENT_DIM> *pBasisFunction = new LinearBasisFunction<ELEMENT_DIM>();
		LinearBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction = new LinearBasisFunction<ELEMENT_DIM-1>();
		SetBasisFunctions(pBasisFunction, pSurfaceBasisFunction);
		mWeAllocatedBasisFunctionMemory = true;
		
		mpQuadRule = NULL; mpSurfaceQuadRule = NULL;
		SetNumberOfQuadraturePointsPerDimension(numPoints);
	}
	
	/**
	 * Constructor allowing specification of the type of basis function to use.
	 * 
	 * @param pBasisFunction Basis function to use for normal elements.
	 * @param pSurfaceBasisFunction Basis function to use for boundary elements.
	 * @param numPoints Number of quadrature points to use per dimension.
	 */
	AbstractAssembler(AbstractBasisFunction<ELEMENT_DIM> *pBasisFunction,
					  AbstractBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction,
					  int numPoints = 2)
	{
		mWeAllocatedBasisFunctionMemory = false;
		SetBasisFunctions(pBasisFunction, pSurfaceBasisFunction);
		
		mpQuadRule = NULL; mpSurfaceQuadRule = NULL;
		SetNumberOfQuadraturePointsPerDimension(numPoints);
	}
	
	/**
	 * Specify what type of basis functions to use.
	 * 
	 * @param pBasisFunction Basis function to use for normal elements.
	 * @param pSurfaceBasisFunction Basis function to use for boundary elements.
	 */
	void SetBasisFunctions(AbstractBasisFunction<ELEMENT_DIM> *pBasisFunction,
						   AbstractBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction)
	{
		if (mWeAllocatedBasisFunctionMemory)
		{
			delete mpBasisFunction;
			delete mpSurfaceBasisFunction;
			mWeAllocatedBasisFunctionMemory = false;
		}
		mpBasisFunction = pBasisFunction;
		mpSurfaceBasisFunction = pSurfaceBasisFunction;
	}
	
	/**
	 * Set the number of quadrature points to use, per dimension.
	 * 
	 * This method will throw an exception if the requested number of quadrature
	 * points is not supported. (TODO: There may be a small memory leak if this
	 * occurs.)
	 * 
	 * @param numPoints Number of quadrature points to use per dimension.
	 */
	void SetNumberOfQuadraturePointsPerDimension(int numPoints)
	{
		if (mpQuadRule) delete mpQuadRule;
		mpQuadRule = new GaussianQuadratureRule<ELEMENT_DIM>(numPoints);
		if (mpSurfaceQuadRule) delete mpSurfaceQuadRule;
		mpSurfaceQuadRule = new GaussianQuadratureRule<ELEMENT_DIM-1>(numPoints);
	}
	
	/**
	 * Delete any memory allocated by this class.
	 */
	virtual ~AbstractAssembler()
	{
		// Basis functions, if we used the default.
		if (mWeAllocatedBasisFunctionMemory)
		{
			delete mpBasisFunction;
			delete mpSurfaceBasisFunction;
			mWeAllocatedBasisFunctionMemory = false;
		}
		
		// Quadrature rules
		if (mpQuadRule) delete mpQuadRule;
		if (mpSurfaceQuadRule) delete mpSurfaceQuadRule;
	}

};

#endif //_ABSTRACTASSEMBLER_HPP_
