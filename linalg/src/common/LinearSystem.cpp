/** Linear System implementation.
*
*
*/
#include "LinearSystem.hpp"
#include "PetscException.hpp"
#include <iostream>
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"
#include <cassert>
#include "EventHandler.hpp"



LinearSystem::LinearSystem(PetscInt lhsVectorSize)
:mMatNullSpace(NULL),
mDestroyPetscObjects(true),
mKspIsSetup(false),
mMatrixIsConstant(false),
mRelativeTolerance(1e-6)
{
    VecCreate(PETSC_COMM_WORLD, &mRhsVector);
    VecSetSizes(mRhsVector, PETSC_DECIDE, lhsVectorSize);
    VecSetFromOptions(mRhsVector);
    
    PetscTools::SetupMat(mLhsMatrix, lhsVectorSize, lhsVectorSize);
        
    mSize = lhsVectorSize;
    
    VecGetOwnershipRange(mRhsVector, &mOwnershipRangeLo, &mOwnershipRangeHi);
}

/**
 * Create a linear system, where the size is based on the size of a given
 * PETSc vec.
 * The LHS & RHS vectors will be created by duplicating this vector's
 * settings.  This should avoid problems with using VecScatter on
 * bidomain simulation results.
 */
LinearSystem::LinearSystem(Vec templateVector)
:mMatNullSpace(NULL),
mDestroyPetscObjects(true),
mKspIsSetup(false),
mMatrixIsConstant(false),
mRelativeTolerance(1e-6)
{
    VecDuplicate(templateVector, &mRhsVector);
    VecGetSize(mRhsVector, &mSize);
    VecGetOwnershipRange(mRhsVector, &mOwnershipRangeLo, &mOwnershipRangeHi);
    PetscInt local_size = mOwnershipRangeHi - mOwnershipRangeLo;

    PetscTools::SetupMat(mLhsMatrix, mSize, mSize, MATMPIAIJ, local_size, local_size);

}

/**
 * Create a linear system which wraps the provided PETSc objects so we can
 * access them using our API.  Either of the objects may be NULL, but at
 * least one of them must not be.
 * 
 * Useful for storing residuals and jacobians when solving nonlinear PDEs.
 */
LinearSystem::LinearSystem(Vec residualVector, Mat jacobianMatrix)
:mMatNullSpace(NULL),
mDestroyPetscObjects(false),
mKspIsSetup(false),
mMatrixIsConstant(false),
mRelativeTolerance(1e-6)
{
    assert(residualVector || jacobianMatrix);
    mRhsVector = residualVector;
    mLhsMatrix = jacobianMatrix;
    
    PetscInt mat_size=0, vec_size=0;
    if (mRhsVector)
    {
        VecGetSize(mRhsVector, &vec_size);
        mSize = (unsigned)vec_size;
        VecGetOwnershipRange(mRhsVector, &mOwnershipRangeLo, &mOwnershipRangeHi);
    }
    if (mLhsMatrix)
    {
        PetscInt mat_cols;
        MatGetSize(mLhsMatrix, &mat_size, &mat_cols);
        assert(mat_size == mat_cols);
        mSize = (unsigned)mat_size; 
        MatGetOwnershipRange(mLhsMatrix, &mOwnershipRangeLo, &mOwnershipRangeHi);
    }
    assert(!mRhsVector || !mLhsMatrix || vec_size == mat_size);   
}

LinearSystem::~LinearSystem()
{
    if (mDestroyPetscObjects)
    {
        VecDestroy(mRhsVector);
        MatDestroy(mLhsMatrix);
    }
    if (mMatNullSpace)
    {
        MatNullSpaceDestroy(mMatNullSpace);
    }
    if (mKspIsSetup)
    {
        KSPDestroy(mKspSolver);
    }
}

void LinearSystem::SetMatrixElement(PetscInt row, PetscInt col, double value)
{
    if (row >= mOwnershipRangeLo && row < mOwnershipRangeHi)
    {
        MatSetValue(mLhsMatrix, row, col, value, INSERT_VALUES);
    }
}

void LinearSystem::AddToMatrixElement(PetscInt row, PetscInt col, double value)
{
    if (row >= mOwnershipRangeLo && row < mOwnershipRangeHi)
    {
        MatSetValue(mLhsMatrix, row, col, value, ADD_VALUES);
    }
}


void LinearSystem::AssembleFinalLinearSystem()
{
    AssembleFinalLhsMatrix();
    AssembleRhsVector();
}


void LinearSystem::AssembleIntermediateLinearSystem()
{
    AssembleIntermediateLhsMatrix();
    AssembleRhsVector();
}

void LinearSystem::AssembleFinalLhsMatrix()
{
    MatAssemblyBegin(mLhsMatrix, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(mLhsMatrix, MAT_FINAL_ASSEMBLY);
}


void LinearSystem::AssembleIntermediateLhsMatrix()
{
    MatAssemblyBegin(mLhsMatrix, MAT_FLUSH_ASSEMBLY);
    MatAssemblyEnd(mLhsMatrix, MAT_FLUSH_ASSEMBLY);
}

void LinearSystem::AssembleRhsVector()
{
    VecAssemblyBegin(mRhsVector);
    VecAssemblyEnd(mRhsVector);
}



void LinearSystem::SetRhsVectorElement(PetscInt row, double value)
{
    if (row >= mOwnershipRangeLo && row < mOwnershipRangeHi)
    {
        VecSetValues(mRhsVector, 1, &row, &value, INSERT_VALUES);
    }
}

void LinearSystem::AddToRhsVectorElement(PetscInt row, double value)
{
    if (row >= mOwnershipRangeLo && row < mOwnershipRangeHi)
    {
        VecSetValues(mRhsVector, 1, &row, &value, ADD_VALUES);
    }
}

void LinearSystem::DisplayMatrix()
{
    MatView(mLhsMatrix,PETSC_VIEWER_STDOUT_WORLD);
}

void LinearSystem::DisplayRhs()
{
    VecView(mRhsVector,PETSC_VIEWER_STDOUT_WORLD);
}

void LinearSystem::SetMatrixRow(PetscInt row, double value)
{
    if (row >= mOwnershipRangeLo && row < mOwnershipRangeHi)
    {
        PetscInt rows, cols;
        MatGetSize(mLhsMatrix, &rows, &cols);
        for (PetscInt i=0; i<cols; i++)
        {
            this->SetMatrixElement(row, i, value);
        }
    }
}

void LinearSystem::ZeroMatrixRow(PetscInt row)
{
    MatAssemblyBegin(mLhsMatrix, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(mLhsMatrix, MAT_FINAL_ASSEMBLY);
    double diag_zero=0.0;
    // MatZeroRows allows a non-zero value to be placed on the diagonal
    // diag_zero is the value to put in the diagonal
    
#if (PETSC_VERSION_MINOR == 2) //Old API
    IS is;
    ISCreateGeneral(PETSC_COMM_WORLD,1,&row,&is);
    MatZeroRows(mLhsMatrix, is, &diag_zero);
    ISDestroy(is);
#else
    
    MatZeroRows(mLhsMatrix, 1, &row, diag_zero);
#endif
    
}

void LinearSystem::ZeroRhsVector()
{
    double *p_rhs_vector_array;
    VecGetArray(mRhsVector, &p_rhs_vector_array);
    for (PetscInt local_index=0; local_index<mOwnershipRangeHi - mOwnershipRangeLo; local_index++)
    {
        p_rhs_vector_array[local_index]=0.0;
    }
    VecRestoreArray(mRhsVector, &p_rhs_vector_array);
}


void LinearSystem::ZeroLhsMatrix()
{
    MatZeroEntries(mLhsMatrix);
}

void LinearSystem::ZeroLinearSystem()
{
    ZeroRhsVector();
    ZeroLhsMatrix();
}



unsigned LinearSystem::GetSize()
{
    return (unsigned) mSize;
}

void LinearSystem::SetNullBasis(Vec nullBasis[], unsigned numberOfBases)
{
    PETSCEXCEPT( MatNullSpaceCreate(PETSC_COMM_WORLD, PETSC_FALSE, numberOfBases, nullBasis, &mMatNullSpace) );
}

/**
 * Get this process' ownership range of the contents of the system
 */
void LinearSystem::GetOwnershipRange(PetscInt &lo, PetscInt &hi)
{
    lo = mOwnershipRangeLo;
    hi = mOwnershipRangeHi;
}

/**
 * Return an element of the matrix.
 * May only be called for elements you own.
 */
double LinearSystem::GetMatrixElement(PetscInt row, PetscInt col)
{
    assert(mOwnershipRangeLo <= row && row < mOwnershipRangeHi);
    PetscInt row_as_array[1];
    row_as_array[0] = row;
    PetscInt col_as_array[1];
    col_as_array[0] = col;
    
    double ret_array[1];
    
    MatGetValues(mLhsMatrix, 1, row_as_array, 1, col_as_array, ret_array);
    
    return ret_array[0];
}

/**
 * Return an element of the RHS vector.
 * May only be called for elements you own.
 */
double LinearSystem::GetRhsVectorElement(PetscInt row)
{
    assert(mOwnershipRangeLo <= row && row < mOwnershipRangeHi);
    
    double *p_rhs_vector;
    PetscInt local_index=row-mOwnershipRangeLo;
    VecGetArray(mRhsVector, &p_rhs_vector);
    double answer=p_rhs_vector[local_index];
    VecRestoreArray(mRhsVector, &p_rhs_vector);
    
    return answer;
}

/**
 * Get access to the rhs vector directly. Shouldn't generally need to be called.
 */
Vec& LinearSystem::rGetRhsVector()
{
    return mRhsVector;
}

/**
 * Get access to the lhs matrix directly. Shouldn't generally need to be called.
 */
Mat& LinearSystem::rGetLhsMatrix()
{
    return mLhsMatrix;
}

/** Force PETSc to treat the matrix in this linear system as symmetric from now on
 */
void LinearSystem::SetMatrixIsSymmetric()
{
    MatSetOption(mLhsMatrix, MAT_SYMMETRIC);
    MatSetOption(mLhsMatrix, MAT_SYMMETRY_ETERNAL);
}
void LinearSystem::SetMatrixIsConstant(bool matrixIsConstant)
{
    mMatrixIsConstant=matrixIsConstant;
}
    
void LinearSystem::SetRelativeTolerance(double relativeTolerance)
{
    mRelativeTolerance=relativeTolerance;
}
//Vec LinearSystem::Solve(AbstractLinearSolver *solver, Vec lhsGuess)
//{
//    return solver->Solve(mLhsMatrix, mRhsVector, mSize, mMatNullSpace, lhsGuess);
//}    
//    
Vec LinearSystem::Solve(Vec lhsGuess)
{
    /* The following lines are very useful for debugging
     *    MatView(mLhsMatrix,    PETSC_VIEWER_STDOUT_WORLD);
     *    VecView(mRhsVector,    PETSC_VIEWER_STDOUT_WORLD);
     */
    //Double check that the non-zero pattern hasn't changed
    MatInfo mat_info;
    MatGetInfo(mLhsMatrix, MAT_GLOBAL_SUM, &mat_info);
    
    if (!mKspIsSetup)
    {
        mPointerToMatrix=mLhsMatrix;
        mNonZerosUsed=mat_info.nz_used;
        //MatNorm(lhsMatrix, NORM_FROBENIUS, &mMatrixNorm);
        PC prec; //Type of pre-conditioner
        
        KSPCreate(PETSC_COMM_WORLD, &mKspSolver);
        //See
        //http://www-unix.mcs.anl.gov/petsc/petsc-2/snapshots/petsc-current/docs/manualpages/KSP/KSPSetOperators.html
        //The preconditioner flag (last argument) in the following calls says
        //how to reuse the preconditioner on subsequent iterations
        if (mMatrixIsConstant)
        {
            KSPSetOperators(mKspSolver, mLhsMatrix, mLhsMatrix, SAME_PRECONDITIONER);
        }
        else
        {
            KSPSetOperators(mKspSolver, mLhsMatrix, mLhsMatrix, SAME_NONZERO_PATTERN);
        }
        
        // Default relative tolerance appears to be 1e-5.  This ain't so great.
        KSPSetTolerances(mKspSolver, mRelativeTolerance, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT);
        
        // Turn off pre-conditioning if the system size is very small
        KSPGetPC(mKspSolver, &prec);
        if (mSize <= 4)
        {
            PCSetType(prec, PCNONE);
        }
        else
        {
            PCSetType(prec, PCJACOBI);
        }
        
        if (mMatNullSpace)
        {
            PETSCEXCEPT( KSPSetNullSpace(mKspSolver, mMatNullSpace) );
        }
        
        if (lhsGuess)
        {
            //Assume that the user of this method will always be kind enough 
            //to give us a reasonable guess.
            KSPSetInitialGuessNonzero(mKspSolver,PETSC_TRUE);
        }
         
        KSPSetFromOptions(mKspSolver) ;
        KSPSetUp(mKspSolver);
        
        mKspIsSetup = true;
    }
    else
    {
        #define COVERAGE_IGNORE
        if (mNonZerosUsed!=mat_info.nz_used)
        {
            EXCEPTION("SimpleLinearSolver doesn't allow the non-zero pattern of a matrix to change. (I think you changed it).");
        }
        
        #undef COVERAGE_IGNORE
    }
    
    // Create solution vector
    //\todo Should it be compulsory for the caller to supply this and manage the memory?
    Vec lhs_vector;
    VecDuplicate(mRhsVector, &lhs_vector);//Sets the same size (doesn't copy)
    if (lhsGuess)
    {           
        VecCopy(lhsGuess, lhs_vector);  
    }
    
    try {
        if (mPointerToMatrix!=mLhsMatrix)//Check that the matrix isn't wandering all over the place
        {
            EXCEPTION("Matrix location has changed");
        }
        EventHandler::BeginEvent(SOLVE_LINEAR_SYSTEM);
        PETSCEXCEPT(KSPSolve(mKspSolver, mRhsVector, lhs_vector));
        EventHandler::EndEvent(SOLVE_LINEAR_SYSTEM);
    
        // Check that solver converged and throw if not
        KSPConvergedReason reason;
        KSPGetConvergedReason(mKspSolver, &reason);
        KSPEXCEPT(reason);
    }
    catch (const Exception& e)
    {
        // Destroy solution vector on error to avoid memory leaks
        VecDestroy(lhs_vector);
        throw e;
    }
    
    return lhs_vector;
}


