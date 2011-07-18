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
#ifndef LINEARPARABOLICPDESYSTEMWITHCOUPLEDODESYSTEMSOLVER_HPP_
#define LINEARPARABOLICPDESYSTEMWITHCOUPLEDODESYSTEMSOLVER_HPP_

#include "AbstractAssemblerSolverHybrid.hpp"
#include "AbstractDynamicLinearPdeSolver.hpp"
#include "AbstractLinearParabolicPdeSystemForCoupledOdeSystem.hpp"
#include "TetrahedralMesh.hpp"
#include "BoundaryConditionsContainer.hpp"
#include "AbstractOdeSystemForCoupledPdeSystem.hpp"
#include "CvodeAdaptor.hpp"
#include "BackwardEulerIvpOdeSolver.hpp"
#include "VtkMeshWriter.hpp"

#include <boost/mpl/if.hpp>
#include <boost/mpl/void.hpp>

/**
 * A class for solving systems of parabolic PDEs and ODEs, which may be coupled
 * via their source terms:
 *
 * d/dt (u_i) = div (D(x) grad (u_i)) + f_i (x, u_1, ..., u_p, v_1, ..., v_q),  i=1,...,p,
 * d/dt (v_j) = g_j(x, u_1, ..., u_p, v_1, ..., v_q),  j=1,...,q.
 *
 * The solver class is templated over spatial dimension and PDE problem dimension (p).
 */
template<unsigned ELEMENT_DIM, unsigned SPACE_DIM=ELEMENT_DIM, unsigned PROBLEM_DIM=1>
class LinearParabolicPdeSystemWithCoupledOdeSystemSolver
    : public AbstractAssemblerSolverHybrid<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM, NORMAL>,
      public AbstractDynamicLinearPdeSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>
{
private:

    /** Pointer to the mesh. */
    AbstractTetrahedralMesh<ELEMENT_DIM, SPACE_DIM>* mpMesh;

    /** The PDE system to be solved. */
    AbstractLinearParabolicPdeSystemForCoupledOdeSystem<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>* mpPdeSystem;

    /** Vector of pointers to ODE systems, defined at nodes. */
    std::vector<AbstractOdeSystemForCoupledPdeSystem*> mOdeSystemsAtNodes;

    /** The values of the ODE system state variables, interpolated at a quadrature point. */
    std::vector<double> mInterpolatedOdeStateVariables;

    /** The ODE solver. */
    AbstractIvpOdeSolver* mpOdeSolver;

    /**
     * A sampling timestep for writing results to file. Set to
     * PdeSimulationTime::GetPdeTimeStep() in the constructor;
     * may be overwritten using the SetSamplingTimeStep() method.
     */
    double mSamplingTimeStep;

    /** Whether ODE systems are present (if not, then the system comprises coupled PDEs only). */
    bool mOdeSystemsPresent;

    /** Output directory (a subfolder of tmp/[USERNAME]/testoutput). */
    std::string mOutputDirectory;;

    /** Meta results file for VTK. */
    out_stream mpVtkMetaFile;

    /**
     * Write the current results to mpVtkMetaFile.
     */
    void WriteVtkResultsToFile();

    /**
     * The term to be added to the element stiffness matrix.
     *
     * @param rPhi The basis functions, rPhi(i) = phi_i, i=1..numBases
     * @param rGradPhi Basis gradients, rGradPhi(i,j) = d(phi_j)/d(X_i)
     * @param rX The point in space
     * @param rU The unknown as a vector, u(i) = u_i
     * @param rGradU The gradient of the unknown as a matrix, rGradU(i,j) = d(u_i)/d(X_j)
     * @param pElement Pointer to the element
     */
    c_matrix<double, PROBLEM_DIM*(ELEMENT_DIM+1), PROBLEM_DIM*(ELEMENT_DIM+1)> ComputeMatrixTerm(
        c_vector<double, ELEMENT_DIM+1>& rPhi,
        c_matrix<double, SPACE_DIM, ELEMENT_DIM+1>& rGradPhi,
        ChastePoint<SPACE_DIM>& rX,
        c_vector<double,PROBLEM_DIM>& rU,
        c_matrix<double, PROBLEM_DIM, SPACE_DIM>& rGradU,
        Element<ELEMENT_DIM, SPACE_DIM>* pElement);

    /**
     * The term to be added to the element stiffness vector.
     *
     * @param rPhi The basis functions, rPhi(i) = phi_i, i=1..numBases
     * @param rGradPhi Basis gradients, rGradPhi(i,j) = d(phi_j)/d(X_i)
     * @param rX The point in space
     * @param rU The unknown as a vector, u(i) = u_i
     * @param rGradU The gradient of the unknown as a matrix, rGradU(i,j) = d(u_i)/d(X_j)
     * @param pElement Pointer to the element
     */
    c_vector<double, PROBLEM_DIM*(ELEMENT_DIM+1)> ComputeVectorTerm(
        c_vector<double, ELEMENT_DIM+1>& rPhi,
        c_matrix<double, SPACE_DIM, ELEMENT_DIM+1>& rGradPhi,
        ChastePoint<SPACE_DIM>& rX,
        c_vector<double,PROBLEM_DIM>& rU,
        c_matrix<double,PROBLEM_DIM,SPACE_DIM>& rGradU,
        Element<ELEMENT_DIM, SPACE_DIM>* pElement);

    /**
     * The term arising from boundary conditions to be added to the element
     * stiffness vector.
     *
     * @param rSurfaceElement the element which is being considered.
     * @param rPhi The basis functions, rPhi(i) = phi_i, i=1..numBases
     * @param rX The point in space
     */
    c_vector<double, PROBLEM_DIM*ELEMENT_DIM> ComputeVectorSurfaceTerm(
        const BoundaryElement<ELEMENT_DIM-1,SPACE_DIM>& rSurfaceElement,
        c_vector<double, ELEMENT_DIM>& rPhi,
        ChastePoint<SPACE_DIM>& rX);

    /**
     * Reset the member variable mInterpolatedOdeStateVariables.
     */
    void ResetInterpolatedQuantities();

    /**
     * Update the member variable mInterpolatedOdeStateVariables by computing the
     * interpolated value of each ODE state variable at each Gauss point
     *
     * @param phiI
     * @param pNode pointer to a Node
     */
    void IncrementInterpolatedQuantities(double phiI, const Node<SPACE_DIM>* pNode);

    /**
     * Initialise method: sets up the linear system (using the mesh to
     * determine the number of unknowns per row to preallocate) if it is not
     * already set up. Can use an initial solution as PETSc template,
     * or base it on the mesh size.
     *
     * @param initialSolution Initial solution (defaults to NULL) for Petsc to use as a template.
     */
    void InitialiseForSolve(Vec initialSolution=NULL);

    /**
     * Completely set up the linear system that has to be solved each timestep.
     *
     * @param currentSolution The current solution which can be used in setting up
     *  the linear system if needed (NULL if there isn't a current solution)
     * @param computeMatrix Whether to compute the LHS matrix of the linear system
     *   (mainly for dynamic solves).
     */
    void SetupLinearSystem(Vec currentSolution, bool computeMatrix);

public:

    /**
     * Constructor.
     *
     * @param pMesh pointer to the mesh
     * @param pPdeSystem pointer to the PDE system
     * @param pBoundaryConditions pointer to the boundary conditions. Can be NULL, to allow concrete assembler-solver
     *        to, say, create standard boundary conditions its constructor, and then set it. If so, the concrete solver
     *        must make sure it calls this->SetApplyNeummanBoundaryConditionsToVector(p_bcc)
     * @param odeSystemsAtNodes optional vector of pointers to ODE systems, defined at nodes
     * @param pOdeSolver optional pointer to an ODE solver (defaults to NULL)
     */
    LinearParabolicPdeSystemWithCoupledOdeSystemSolver(TetrahedralMesh<ELEMENT_DIM, SPACE_DIM>* pMesh,
                                                       AbstractLinearParabolicPdeSystemForCoupledOdeSystem<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>* pPdeSystem,
                                                       BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>* pBoundaryConditions,
                                                       std::vector<AbstractOdeSystemForCoupledPdeSystem*> odeSystemsAtNodes=std::vector<AbstractOdeSystemForCoupledPdeSystem*>(),
                                                       AbstractIvpOdeSolver* pOdeSolver=NULL);

    /**
     * Overridden PrepareForSetupLinearSystem() method.
     * Pass the current solution to the PDE system to the ODE system and solve it over the next timestep.
     *
     * @param currentPdeSolution the solution to the PDE system at the current time
     */
    void PrepareForSetupLinearSystem(Vec currentPdeSolution);

    /**
     * Set mOutputDirectory.
     *
     * @param outputDirectory the output directory to use
     */
    void SetOutputDirectory(std::string outputDirectory);

    /**
     * Set mSamplingTimeStep.
     *
     * @param samplingTimeStep the sampling timestep to use
     */
    void SetSamplingTimeStep(double samplingTimeStep);

    /**
     * Solve the coupled PDE/ODE system over the pre-specified time interval,
     * and record results using mSamplingTimeStep.
     *
     * \todo (#1777) here we are assuming:
     *    - SPACE_DIM > 1
     *    - VTK is installed
     *    - SetOutputDirectory() has been called on the solver object
     *    - SetTimes() has been called on the solver object
     *    - SetTimeStep() has been called on the solver object
     *    - SetSamplingTimeStep() has been called on the solver object
     */
    void SolveAndWriteResultsToFile();

    /**
     * Solve the coupled PDE/ODE system over a specified time interval,
     * and record results using mSamplingTimeStep. Called by SolveAndWriteResultsToFile().
     *
     * @param startTime the start time
     * @param endTime the end time
     * @param numTimeStepsElapsed the number of timesteps that have elapsed
     */
    void SolveAndWriteResultsToFileForTimes(double startTime, double endTime, unsigned numTimeStepsElapsed);
};

///////////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////////

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
c_matrix<double, PROBLEM_DIM*(ELEMENT_DIM+1), PROBLEM_DIM*(ELEMENT_DIM+1)> LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::ComputeMatrixTerm(
    c_vector<double, ELEMENT_DIM+1>& rPhi,
    c_matrix<double, SPACE_DIM, ELEMENT_DIM+1>& rGradPhi,
    ChastePoint<SPACE_DIM>& rX,
    c_vector<double,PROBLEM_DIM>& rU,
    c_matrix<double, PROBLEM_DIM, SPACE_DIM>& rGradU,
    Element<ELEMENT_DIM, SPACE_DIM>* pElement)
{
    double timestep_inverse = PdeSimulationTime::GetPdeTimeStepInverse();
    c_matrix<double, PROBLEM_DIM*(ELEMENT_DIM+1), PROBLEM_DIM*(ELEMENT_DIM+1)> matrix_term = zero_matrix<double>(PROBLEM_DIM*(ELEMENT_DIM+1), PROBLEM_DIM*(ELEMENT_DIM+1));

    // Loop over PDEs and populate matrix_term
    for (unsigned pde_index=0; pde_index<PROBLEM_DIM; pde_index++)
    {
        double this_dudt_coefficient = mpPdeSystem->ComputeDuDtCoefficientFunction(rX, pde_index);
        c_matrix<double, SPACE_DIM, SPACE_DIM> this_pde_diffusion_term = mpPdeSystem->ComputeDiffusionTerm(rX, pde_index, pElement);
        c_matrix<double, 1*(ELEMENT_DIM+1), 1*(ELEMENT_DIM+1)> this_stiffness_matrix =
            prod(trans(rGradPhi), c_matrix<double, SPACE_DIM, ELEMENT_DIM+1>(prod(this_pde_diffusion_term, rGradPhi)) )
                + timestep_inverse * this_dudt_coefficient * outer_prod(rPhi, rPhi);

        ///\todo (#1777) This is probably a horribly inefficient way of building the matrix!
        for (unsigned i=0; i<ELEMENT_DIM+1; i++)
        {
            for (unsigned j=0; j<ELEMENT_DIM+1; j++)
            {
                matrix_term(i*PROBLEM_DIM + pde_index, j*PROBLEM_DIM + pde_index) = this_stiffness_matrix(i,j);
            }
        }
    }
    return matrix_term;
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
c_vector<double, PROBLEM_DIM*(ELEMENT_DIM+1)> LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::ComputeVectorTerm(
    c_vector<double, ELEMENT_DIM+1>& rPhi,
    c_matrix<double, SPACE_DIM, ELEMENT_DIM+1>& rGradPhi,
    ChastePoint<SPACE_DIM>& rX,
    c_vector<double,PROBLEM_DIM>& rU,
    c_matrix<double,PROBLEM_DIM,SPACE_DIM>& rGradU,
    Element<ELEMENT_DIM, SPACE_DIM>* pElement)
{
    double timestep_inverse = PdeSimulationTime::GetPdeTimeStepInverse();
    c_vector<double, PROBLEM_DIM*(ELEMENT_DIM+1)> vector_term = zero_vector<double>(PROBLEM_DIM*(ELEMENT_DIM+1));

    // Loop over PDEs and populate vector_term
    for (unsigned pde_index=0; pde_index<PROBLEM_DIM; pde_index++)
    {
        double this_dudt_coefficient = mpPdeSystem->ComputeDuDtCoefficientFunction(rX, pde_index);
        double this_source_term = mpPdeSystem->ComputeSourceTerm(rX, rU, mInterpolatedOdeStateVariables, pde_index);
        c_vector<double, ELEMENT_DIM+1> this_vector_term = (this_source_term + timestep_inverse*this_dudt_coefficient*rU(pde_index))* rPhi;

        ///\todo (#1777) This is probably a horribly inefficient way of building the vector!
        for (unsigned i=0; i<ELEMENT_DIM+1; i++)
        {
            vector_term(i*PROBLEM_DIM + pde_index) = this_vector_term(i);
        }
    }

    return vector_term;
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
c_vector<double, PROBLEM_DIM*ELEMENT_DIM> LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::ComputeVectorSurfaceTerm(
    const BoundaryElement<ELEMENT_DIM-1,SPACE_DIM>& rSurfaceElement,
    c_vector<double, ELEMENT_DIM>& rPhi,
    ChastePoint<SPACE_DIM>& rX)
{
    c_vector<double, PROBLEM_DIM*ELEMENT_DIM> vector_surface_term = zero_vector<double>(PROBLEM_DIM*ELEMENT_DIM);

    // Loop over PDEs and populate vector_surface_term
    for (unsigned pde_index=0; pde_index<PROBLEM_DIM; pde_index++)
    {
        // D_times_gradu_dot_n = [D grad(u)].n, D=diffusion matrix
        double this_D_times_gradu_dot_n = this->mpBoundaryConditions->GetNeumannBCValue(&rSurfaceElement, rX, pde_index);
        c_vector<double, ELEMENT_DIM> this_vector_surface_term = rPhi * this_D_times_gradu_dot_n;

        ///\todo (#1777) This is probably a horribly inefficient way of building the vector!
        for (unsigned i=0; i<ELEMENT_DIM; i++)
        {
            vector_surface_term(i*PROBLEM_DIM + pde_index) = this_vector_surface_term(i);
        }
    }

    return vector_surface_term;
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::ResetInterpolatedQuantities()
{
    mInterpolatedOdeStateVariables.clear();

    if (mOdeSystemsPresent)
    {
        unsigned num_state_variables = mOdeSystemsAtNodes[0]->GetNumberOfStateVariables();
        mInterpolatedOdeStateVariables.resize(num_state_variables, 0.0);
    }
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::IncrementInterpolatedQuantities(double phiI, const Node<SPACE_DIM>* pNode)
{
    if (mOdeSystemsPresent)
    {
        unsigned num_state_variables = mOdeSystemsAtNodes[0]->GetNumberOfStateVariables();

        for (unsigned i=0; i<num_state_variables ; i++)
        {
            mInterpolatedOdeStateVariables[i] += phiI * mOdeSystemsAtNodes[pNode->GetIndex()]->rGetStateVariables()[i];
        }
    }
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::InitialiseForSolve(Vec initialSolution)
{
    if (this->mpLinearSystem == NULL)
    {
        unsigned preallocation = mpMesh->CalculateMaximumContainingElementsPerProcess() + ELEMENT_DIM;
        if (ELEMENT_DIM > 1)
        {
            // Highest connectivity is closed
            preallocation--;
        }
        preallocation *= PROBLEM_DIM;

        /*
         * Use the currrent solution (ie the initial solution) as the
         * template in the alternative constructor of LinearSystem.
         * This is to avoid problems with VecScatter.
         */
        this->mpLinearSystem = new LinearSystem(initialSolution, preallocation);
    }

    assert(this->mpLinearSystem);
    this->mpLinearSystem->SetMatrixIsSymmetric(true);
    this->mpLinearSystem->SetKspType("cg");
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::SetupLinearSystem(Vec currentSolution, bool computeMatrix)
{
    SetupGivenLinearSystem(currentSolution, computeMatrix, this->mpLinearSystem);
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::LinearParabolicPdeSystemWithCoupledOdeSystemSolver(
        TetrahedralMesh<ELEMENT_DIM, SPACE_DIM>* pMesh,
        AbstractLinearParabolicPdeSystemForCoupledOdeSystem<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>* pPdeSystem,
        BoundaryConditionsContainer<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>* pBoundaryConditions,
        std::vector<AbstractOdeSystemForCoupledPdeSystem*> odeSystemsAtNodes,
        AbstractIvpOdeSolver* pOdeSolver)
    : AbstractAssemblerSolverHybrid<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM, NORMAL>(pMesh, pBoundaryConditions),
      AbstractDynamicLinearPdeSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>(pMesh),
      mpMesh(pMesh),
      mpPdeSystem(pPdeSystem),
      mOdeSystemsAtNodes(odeSystemsAtNodes),
      mpOdeSolver(pOdeSolver),
      mSamplingTimeStep(DOUBLE_UNSET),
      mOdeSystemsPresent(false)
{
    this->mpBoundaryConditions = pBoundaryConditions;

    /*
     * If any ODE systems are passed in to the constructor, then we aren't just
     * solving a coupled PDE system, in which case the number of ODE system objects
     * must match the number of nodes in the finite element mesh.
     */
    if (!mOdeSystemsAtNodes.empty())
    {
        mOdeSystemsPresent = true;
        assert(mOdeSystemsAtNodes.size() == mpMesh->GetNumNodes());

        /*
         * In this case, if an ODE solver is not explicitly passed into the
         * constructor, then we create a default solver.
         */
        if (!mpOdeSolver)
        {
#ifdef CHASTE_CVODE
            mpOdeSolver = new CvodeAdaptor;
#else
            mpOdeSolver = new BackwardEulerIvpOdeSolver(mOdeSystemsAtNodes[0]->GetNumberOfStateVariables());
#endif //CHASTE_CVODE
        }
    }
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::PrepareForSetupLinearSystem(Vec currentPdeSolution)
{
    double time = PdeSimulationTime::GetTime();
    double dt = PdeSimulationTime::GetPdeTimeStep();

    ReplicatableVector soln_repl(currentPdeSolution);
    std::vector<double> current_soln_this_node;

    // Loop over nodes
    for (unsigned node_index=0; node_index<mpMesh->GetNumNodes(); node_index++)
    {
        // Reset the vector current_soln_this_node
        current_soln_this_node.clear();
        current_soln_this_node.resize(PROBLEM_DIM, 0.0);

        // Store the current solution to the PDE system at this node
        for (unsigned pde_index=0; pde_index<PROBLEM_DIM; pde_index++)
        {
            double current_soln_this_pde_this_node = soln_repl[PROBLEM_DIM*node_index + pde_index];
            current_soln_this_node[pde_index] = current_soln_this_pde_this_node;
        }

        if (mOdeSystemsPresent)
        {
            // Pass it into the ODE system at this node
            mOdeSystemsAtNodes[node_index]->SetPdeSolution(current_soln_this_node);

            // Solve ODE system at this node
            mpOdeSolver->SolveAndUpdateStateVariable(mOdeSystemsAtNodes[node_index], time, time+dt, dt);
        }
    }
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::SetOutputDirectory(std::string outputDirectory)
{
    mOutputDirectory = outputDirectory;
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::SetSamplingTimeStep(double samplingTimeStep)
{
    assert(samplingTimeStep >= this->mIdealTimeStep);
    mSamplingTimeStep = samplingTimeStep;
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::SolveAndWriteResultsToFile()
{
	if (mSamplingTimeStep == DOUBLE_UNSET)
	{
		EXCEPTION("SetSamplingTimeStep() must be called prior to SolveAndWriteResultsToFile()");
	}

#ifdef CHASTE_VTK
    // Create a .pvd output file
    OutputFileHandler output_file_handler(mOutputDirectory, false);
    mpVtkMetaFile = output_file_handler.OpenOutputFile("results.pvd");
    *mpVtkMetaFile << "<?xml version=\"1.0\"?>\n";
    *mpVtkMetaFile << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">\n";
    *mpVtkMetaFile << "    <Collection>\n";

    // Store the 'true' start and end times and the number of nodes in the mesh
    double true_t_start = this->mTstart;
    double true_t_end = this->mTend;

    ///\todo (#1777) this is a fudge!
    unsigned num_sampling_timesteps = (unsigned)((true_t_end + 1e-6 - true_t_start)/mSamplingTimeStep);

    ///\todo (#1777) output VTK for initial conditions

    // Main time loop
    double this_t_end = 0.0;
    unsigned num_sampling_timesteps_elapsed = 0;
    while (num_sampling_timesteps_elapsed < num_sampling_timesteps)
    {
        double this_t_start = true_t_start + num_sampling_timesteps_elapsed*mSamplingTimeStep;
        this_t_end = true_t_start + (num_sampling_timesteps_elapsed+1)*mSamplingTimeStep;

        SolveAndWriteResultsToFileForTimes(this_t_start, this_t_end, num_sampling_timesteps_elapsed);

        num_sampling_timesteps_elapsed++;
    }

    // Deal with last sampling timestep if necessary
    ///\todo (#1777) sort out aforementioned fudge!
    if (this_t_end < true_t_end)
    {
        SolveAndWriteResultsToFileForTimes(this_t_end, true_t_end, num_sampling_timesteps_elapsed);
    }

    // Close .pvd output file
    *mpVtkMetaFile << "    </Collection>\n";
    *mpVtkMetaFile << "</VTKFile>\n";
    mpVtkMetaFile->close();
#endif //CHASTE_VTK
}

template<unsigned ELEMENT_DIM, unsigned SPACE_DIM, unsigned PROBLEM_DIM>
void LinearParabolicPdeSystemWithCoupledOdeSystemSolver<ELEMENT_DIM, SPACE_DIM, PROBLEM_DIM>::SolveAndWriteResultsToFileForTimes(double startTime, double endTime, unsigned numTimeStepsElapsed)
{
#ifdef CHASTE_VTK
    // Store the number of nodes in the mesh
    unsigned num_nodes = mpMesh->GetNumNodes();

    // Reset start and end times
    this->SetTimes(startTime, endTime);

    // Create a new VTK file for this time step
    std::stringstream time;
    time << numTimeStepsElapsed;
    VtkMeshWriter<ELEMENT_DIM, SPACE_DIM> mesh_writer(mOutputDirectory, "results_"+time.str(), false);

    // Solve the system up to the new end time
    Vec soln = this->Solve();
    ReplicatableVector soln_repl(soln);

    /*
     * We first loop over PDEs. For each PDE we store the solution
     * at each node in a vector, then pass this vector to the mesh
     * writer.
     */
    for (unsigned pde_index=0; pde_index<PROBLEM_DIM; pde_index++)
    {
        // Store the solution of this PDE at each node
        std::vector<double> pde_index_data;
        pde_index_data.resize(num_nodes, 0.0);
        for (unsigned node_index=0; node_index<num_nodes; node_index++)
        {
            pde_index_data[node_index] = soln_repl[PROBLEM_DIM*node_index + pde_index];
        }

        // Add this data to the mesh writer
        std::stringstream data_name;
        data_name << "PDE variable " << pde_index;
        mesh_writer.AddPointData(data_name.str(), pde_index_data);
    }

    if (mOdeSystemsPresent)
    {
        /*
         * We cannot loop over ODEs like PDEs, since the solutions are not
         * stored in one place. Therefore we build up a large 'vector of
         * vectors', then pass each component of this vector to the mesh
         * writer.
         */
        std::vector<std::vector<double> > ode_data;
        unsigned num_odes = mOdeSystemsAtNodes[0]->rGetStateVariables().size();
        for (unsigned ode_index=0; ode_index<num_odes; ode_index++)
        {
            std::vector<double> ode_index_data;
            ode_index_data.resize(num_nodes, 0.0);
            ode_data.push_back(ode_index_data);
        }

        for (unsigned node_index=0; node_index<num_nodes; node_index++)
        {
            std::vector<double> all_odes_this_node = mOdeSystemsAtNodes[node_index]->rGetStateVariables();
            for (unsigned i=0; i<num_odes; i++)
            {
                ode_data[i][node_index] = all_odes_this_node[i];
            }
        }

        for (unsigned ode_index=0; ode_index<num_odes; ode_index++)
        {
            std::vector<double> ode_index_data = ode_data[ode_index];

            // Add this data to the mesh writer
            std::stringstream data_name;
            data_name << "ODE variable " << ode_index;
            mesh_writer.AddPointData(data_name.str(), ode_index_data);
        }
    }

    mesh_writer.WriteFilesUsingMesh(*mpMesh);
    *mpVtkMetaFile << "        <DataSet timestep=\"";
    *mpVtkMetaFile << numTimeStepsElapsed;
    *mpVtkMetaFile << "\" group=\"\" part=\"0\" file=\"results_";
    *mpVtkMetaFile << numTimeStepsElapsed;
    *mpVtkMetaFile << ".vtu\"/>\n";
#endif // CHASTE_VTK
}

#endif /*LINEARPARABOLICPDESYSTEMWITHCOUPLEDODESYSTEMSOLVER_HPP_*/
