#ifndef _PARALLELMONODOMAINDG0ASSEMBLER_HPP_
#define _PARALLELMONODOMAINDG0ASSEMBLER_HPP_

#include  <vector>
#include "petscvec.h"

#include "MatrixDouble.hpp"
#include "VectorDouble.hpp"
#include "Point.hpp"
#include "Element.hpp"
#include "SimpleDg0ParabolicAssembler.hpp"
#include "ParallelMonodomainPde.hpp"
#include "AbstractBasisFunction.hpp"
#include "GaussianQuadratureRule.hpp"

#include <iostream>

template<int ELEMENT_DIM, int SPACE_DIM>
class ParallelMonodomainDg0Assembler : public SimpleDg0ParabolicAssembler<ELEMENT_DIM, SPACE_DIM>
{
protected:
	/**
	 * We override this method in order to compute the source term by interpolating
	 * the values of the source term at the nodes on this element, rather than
	 * computing the source term directly at a point.
	 */ 
    void AssembleOnElement(const Element<ELEMENT_DIM,SPACE_DIM> &rElement,
                           MatrixDouble &rAElem,
                           VectorDouble &rBElem,
                           AbstractLinearPde<SPACE_DIM> *pAbstractPde,
                           Vec currentSolution)
    {
		GaussianQuadratureRule<ELEMENT_DIM> &quad_rule =
			*(AbstractAssembler<ELEMENT_DIM,SPACE_DIM>::mpQuadRule);
		AbstractBasisFunction<ELEMENT_DIM> &rBasisFunction =
			*(AbstractAssembler<ELEMENT_DIM,SPACE_DIM>::mpBasisFunction);
			
        //double *currentSolutionArray;
        //int ierr = VecGetArray(currentSolution, &currentSolutionArray);
       
        ParallelMonodomainPde<SPACE_DIM> *pPde= (ParallelMonodomainPde<SPACE_DIM> *) pAbstractPde;
        
        if (!pPde->IsOdeSolvedAtAnyNode())
        {
        	pPde->ComputeAllNonlinearSourceTerms(currentSolution);
			/*int rank;MPI_Comm_rank(PETSC_COMM_WORLD,&rank);
            for (int i=0;i<11;i++){
            		std::cout<<rank<<" "<<i <<" "<<pPde->solutionCache[ i ]<<" second\n";
            }*/
        }

        const MatrixDouble *inverseJacobian = rElement.GetInverseJacobian();
        double jacobian_determinant = rElement.GetJacobianDeterminant();
        
        // Initialise element contributions to zero
        const int num_nodes = rElement.GetNumNodes();
                
        for(int quad_index=0; quad_index < quad_rule.GetNumQuadPoints(); quad_index++)
        {
            Point<ELEMENT_DIM> quad_point = quad_rule.GetQuadPoint(quad_index);

            std::vector<double>       phi     = rBasisFunction.ComputeBasisFunctions(quad_point);
            std::vector<VectorDouble> gradPhi = rBasisFunction.ComputeTransformedBasisFunctionDerivatives
                                                (quad_point, *inverseJacobian);
            Point<SPACE_DIM> x(0,0,0);
            double u=0;
            double sourceTerm = 0;
            for(int i=0; i<num_nodes; i++)
            {
                for(int j=0; j<SPACE_DIM; j++)
                {
                    x.SetCoordinate(j, x[j] + phi[i]*rElement.GetNodeLocation(i,j));
                }
                
                u  += phi[i]*pPde->solutionCache[ rElement.GetNodeGlobalIndex(i) ];
                sourceTerm += phi[i]*pPde->ComputeNonlinearSourceTermAtNode( *(rElement.GetNode(i)), pPde->solutionCache[rElement.GetNodeGlobalIndex(i)] );
                
                //std::cout << rank<<" Node "<< rElement.GetNodeGlobalIndex(i)<<" "<<pPde->ComputeNonlinearSourceTermAtNode( *(rElement.GetNode(i)), currentSolutionArray[rElement.GetNodeGlobalIndex(i)] ) << "\n";
            }

            //std::cout << "\n\n" << "source = " << sourceTerm << std::flush;

            double wJ = jacobian_determinant * quad_rule.GetWeight(quad_index);  
            for (int row=0; row < num_nodes; row++)
            {
                for (int col=0; col < num_nodes; col++)
                {
                    double integrand_val1 = (1.0/SimpleDg0ParabolicAssembler<ELEMENT_DIM, SPACE_DIM>::mDt) * pPde->ComputeDuDtCoefficientFunction(x) * phi[row] * phi[col];
                    rAElem(row,col) += integrand_val1 * wJ; 

                    double integrand_val2 = gradPhi[row].dot(pPde->ComputeDiffusionTerm(x) * gradPhi[col]);                             
                    rAElem(row,col) += integrand_val2 * wJ;
                    
                    //int rank; MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
                    //std::cout<<rank<<" "<<row<<" "<<col<<" "<<rAElem(row,col)<<"\n";
                }

                // RHS
//				double vec_integrand_val1 // = (pPde->ComputeLinearSourceTerm(x) + 
//					= pPde->ComputeNonlinearSourceTermAtNode( *(rElement.GetNode(quad_index)), currentSolutionArray[rElement.GetNodeGlobalIndex(quad_index)] ) * phi[row];
//				vec_integrand_val1 = pPde->ComputeNonlinearSourceTermAtNode( *( rElement.GetNode(quad_index) ), u)) * phi[row];
                
                
                double vec_integrand_val1 = sourceTerm * phi[row];
                rBElem(row) += vec_integrand_val1 * wJ;
				
                double vec_integrand_val2 = (1.0/SimpleDg0ParabolicAssembler<ELEMENT_DIM, SPACE_DIM>::mDt) * pPde->ComputeDuDtCoefficientFunction(x) * u * phi[row];
                rBElem(row) += vec_integrand_val2 * wJ;             
			}
        }
        
    }       



public:
	/**
	 * Constructors just call the base class versions.
	 */
	ParallelMonodomainDg0Assembler(int numPoints = 2) :
		SimpleDg0ParabolicAssembler<ELEMENT_DIM,SPACE_DIM>(numPoints)
	{
	}
	ParallelMonodomainDg0Assembler(AbstractBasisFunction<ELEMENT_DIM> *pBasisFunction,
							AbstractBasisFunction<ELEMENT_DIM-1> *pSurfaceBasisFunction,
							int numPoints = 2) :
		SimpleDg0ParabolicAssembler<ELEMENT_DIM,SPACE_DIM>(pBasisFunction, pSurfaceBasisFunction, numPoints)
	{
	}
};

#endif //_PARALLELMONODOMAINDG0ASSEMBLER_HPP_
