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

#include "OffLatticeSimulationWithPdes.hpp"
#include "MeshBasedCellPopulationWithGhostNodes.hpp"
#include "NodeBasedCellPopulation.hpp"
#include "BoundaryConditionsContainer.hpp"
#include "ConstBoundaryCondition.hpp"
#include "SimpleLinearEllipticSolver.hpp"
#include "OffLatticeSimulationWithPdesAssembler.hpp"
#include "CellwiseData.hpp"
#include "AbstractTwoBodyInteractionForce.hpp"
#include "TrianglesMeshReader.hpp"
#include "TrianglesMeshWriter.hpp"
#include "OutputFileHandler.hpp"
#include "Exception.hpp"
#include "PetscTools.hpp"
#include "CellBasedEventHandler.hpp"
#include "Debug.hpp"

template<unsigned DIM>
OffLatticeSimulationWithPdes<DIM>::OffLatticeSimulationWithPdes(AbstractCellPopulation<DIM>& rCellPopulation,
                                                        std::vector<PdeAndBoundaryConditions<DIM>*> pdeAndBcCollection,
                                                        bool deleteCellPopulationInDestructor,
                                                        bool initialiseCells)
    : OffLatticeSimulation<DIM>(rCellPopulation,
                               deleteCellPopulationInDestructor,
                               initialiseCells),
      mPdeAndBcCollection(pdeAndBcCollection),
      mWriteAverageRadialPdeSolution(false),
      mWriteDailyAverageRadialPdeSolution(false),
      mSetBcsOnCoarseBoundary(true),
      mNumRadialIntervals(0), // 'unset' value
      mpCoarsePdeMesh(NULL)
{
    // We must be using a mesh-based or node based cell population
    // assert(dynamic_cast<MeshBasedCellPopulation<DIM>*>(&(this->mrCellPopulation)) != NULL);
	assert((dynamic_cast<NodeBasedCellPopulation<DIM>*>(&(this->mrCellPopulation)) != NULL) || (dynamic_cast<MeshBasedCellPopulation<DIM>*>(&(this->mrCellPopulation)) != NULL));

    // We must not have any ghost nodes
    assert(dynamic_cast<MeshBasedCellPopulationWithGhostNodes<DIM>*>(&(this->mrCellPopulation)) == NULL);
}

template<unsigned DIM>
OffLatticeSimulationWithPdes<DIM>::~OffLatticeSimulationWithPdes()
{
    if (mpCoarsePdeMesh)
    {
        delete mpCoarsePdeMesh;
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SetPdeAndBcCollection(std::vector<PdeAndBoundaryConditions<DIM>*> pdeAndBcCollection)
{
    mPdeAndBcCollection = pdeAndBcCollection;
}

template<unsigned DIM>
Vec OffLatticeSimulationWithPdes<DIM>::GetCurrentPdeSolution(unsigned pdeIndex)
{
    assert(pdeIndex<mPdeAndBcCollection.size());
    return mPdeAndBcCollection[pdeIndex]->GetSolution();
}

//////////////////////////////////////////////////////////////////////////////
//                          Setup/AfterSolve methods                        //
//////////////////////////////////////////////////////////////////////////////

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::WriteVisualizerSetupFile()
{
    for (unsigned i=0; i<this->mForceCollection.size(); i++)
    {
        boost::shared_ptr<AbstractForce<DIM> > p_force = this->mForceCollection[i];
        if (boost::dynamic_pointer_cast<AbstractTwoBodyInteractionForce<DIM> >(p_force))
        {
            double cutoff = (boost::static_pointer_cast<AbstractTwoBodyInteractionForce<DIM> >(p_force))->GetCutOffLength();
            *(this->mpVizSetupFile) << "Cutoff\t" << cutoff << "\n";
        }
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SetupSolve()
{
	// If we're using a NodeBasedCellPopulation - assert that we have a CoarsePdeMesh to solve it on
	if ((dynamic_cast<NodeBasedCellPopulation<DIM>*>(&(this->mrCellPopulation)) != NULL) && mpCoarsePdeMesh==NULL)
	{
		EXCEPTION("Trying to solve a PDE on a NodeBasedCellPopulation without setting up a coarse mesh. Try calling UseCoarseMesh()");
	}
    if (mpCoarsePdeMesh != NULL)
    {
        InitialiseCoarsePdeMesh();
        SetupWriteCoarsePdeSolution();
    }

    // We must initially have at least one cell in the cell-based simulation
    assert(this->mrCellPopulation.GetNumRealCells() != 0);

    SetupWritePdeSolution();
    double current_time = SimulationTime::Instance()->GetTime();
    WritePdeSolution(current_time);
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SetupWritePdeSolution()
{
    OutputFileHandler output_file_handler(this->mSimulationOutputDirectory+"/", false);
    if (PetscTools::AmMaster())
    {
        mpVizPdeSolutionResultsFile = output_file_handler.OpenOutputFile("results.vizpdesolution");
        *this->mpVizSetupFile << "PDE \n";
        if (mWriteAverageRadialPdeSolution)
        {
            mpAverageRadialPdeSolutionResultsFile = output_file_handler.OpenOutputFile("radial_dist.dat");
        }
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SetupWriteCoarsePdeSolution()
{
    OutputFileHandler output_file_handler(this->mSimulationOutputDirectory+"/", false);
    if (PetscTools::AmMaster())
    {
        mpVizCoarsePdeSolutionResultsFile = output_file_handler.OpenOutputFile("results.vizcoarsepdesolution");
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::UseCoarsePdeMesh(double stepSize, double meshWidth)
{
    assert(!mPdeAndBcCollection.empty());
    for (unsigned pde_index=0; pde_index<mPdeAndBcCollection.size(); pde_index++)
    {
        assert(mPdeAndBcCollection[pde_index]->HasAveragedSourcePde());
    }

    CreateCoarsePdeMesh(stepSize, meshWidth);
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::CreateCoarsePdeMesh(double stepSize, double meshWidth)
{
	// Create coarse mesh
	mpCoarsePdeMesh = new TetrahedralMesh<DIM,DIM>;
	switch(DIM)
	{
		case 1:
			mpCoarsePdeMesh->ConstructRegularSlabMesh(stepSize, meshWidth);
			break;
		case 2:
			mpCoarsePdeMesh->ConstructRegularSlabMesh(stepSize, meshWidth, meshWidth);
			break;
		case 3:
			mpCoarsePdeMesh->ConstructRegularSlabMesh(stepSize, meshWidth, meshWidth, meshWidth);
			break;
		default:
			NEVER_REACHED;
	}


	// Find centre of coarse PDE mesh
	c_vector<double,DIM> centre_of_coarse_mesh = zero_vector<double>(DIM);
	c_vector<double,DIM> centre_of_cell_population=this->GetCellPopulationLocation();
	for (unsigned i=0; i<mpCoarsePdeMesh->GetNumNodes(); i++)
	{
		centre_of_coarse_mesh += mpCoarsePdeMesh->GetNode(i)->rGetLocation();
	}
	centre_of_coarse_mesh /= mpCoarsePdeMesh->GetNumNodes();

	// Translate centre of coarse PDE mesh to the origin
	mpCoarsePdeMesh->Translate(centre_of_cell_population-centre_of_coarse_mesh);

	// Write mesh to file
	WriteCoarseMeshToFile();
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::InitialiseCoarsePdeMesh()
{
    mCellPdeElementMap.clear();

    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
        cell_iter != this->mrCellPopulation.End();
        ++cell_iter)
    {
        // Find the element of mpCoarsePdeMesh that contains this cell
        const ChastePoint<DIM>& r_position_of_cell = this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter);
        unsigned elem_index = mpCoarsePdeMesh->GetContainingElementIndex(r_position_of_cell);
        mCellPdeElementMap[*cell_iter] = elem_index;
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::AfterSolve()
{
    if (this->mrCellPopulation.GetNumRealCells() != 0 && PetscTools::AmMaster())
    {
        mpVizPdeSolutionResultsFile->close();

        if (mWriteAverageRadialPdeSolution)
        {
            WriteAverageRadialPdeSolution(SimulationTime::Instance()->GetTime(), mNumRadialIntervals);
            mpAverageRadialPdeSolutionResultsFile->close();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//                             PostSolve methods                            //
//////////////////////////////////////////////////////////////////////////////

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SolvePde()
{
    if (mpCoarsePdeMesh != NULL)
    {
        SolvePdeUsingCoarseMesh();
        return;
    }

    assert(!mPdeAndBcCollection.empty());

    for (unsigned pde_index=0; pde_index<mPdeAndBcCollection.size(); pde_index++)
    {
        assert(mPdeAndBcCollection[pde_index]);
        assert(mPdeAndBcCollection[pde_index]->HasAveragedSourcePde() == false);
    }

    // Note: If not using a coarse PDE mesh, we MUST be using a MeshBasedCellPopulation
    // Make sure the mesh is in a nice state
    this->mrCellPopulation.Update();

    TetrahedralMesh<DIM,DIM>& r_mesh = static_cast<MeshBasedCellPopulation<DIM>*>(&(this->mrCellPopulation))->rGetMesh();
    CellwiseData<DIM>::Instance()->ReallocateMemory();

    // Loop over elements of mPdeAndBcCollection
    for (unsigned pde_index=0; pde_index<mPdeAndBcCollection.size(); pde_index++)
    {
        // Get pointer to this PdeAndBoundaryConditions object
        PdeAndBoundaryConditions<DIM>* p_pde_and_bc = mPdeAndBcCollection[pde_index];

        // Set up boundary conditions
        AbstractBoundaryCondition<DIM>* p_bc = p_pde_and_bc->GetBoundaryCondition();

        BoundaryConditionsContainer<DIM,DIM,1> bcc(false);
        if (p_pde_and_bc->IsNeumannBoundaryCondition())
        {
            for (typename TetrahedralMesh<DIM,DIM>::BoundaryElementIterator elem_iter = r_mesh.GetBoundaryElementIteratorBegin();
                 elem_iter != r_mesh.GetBoundaryElementIteratorEnd();
                 ++elem_iter)
            {
                bcc.AddNeumannBoundaryCondition(*elem_iter, p_bc);
            }
        }
        else // assuming that if it's not Neumann, then it's Dirichlet
        {
            for (typename TetrahedralMesh<DIM,DIM>::BoundaryNodeIterator node_iter = r_mesh.GetBoundaryNodeIteratorBegin();
                 node_iter != r_mesh.GetBoundaryNodeIteratorEnd();
                 ++node_iter)
            {
                bcc.AddDirichletBoundaryCondition(*node_iter, p_bc);
            }
        }

        /*
         * Set up solver. This is a purpose-made elliptic solver which must
         * interpolate contributions to source terms from nodes onto Gauss points,
         * because the PDE solution is only stored at the cells (nodes).
         */
        OffLatticeSimulationWithPdesAssembler<DIM> solver(&r_mesh, p_pde_and_bc->GetPde(), &bcc);

        PetscInt size_of_soln_previous_step = 0;

        if (p_pde_and_bc->GetSolution())
        {
            VecGetSize(p_pde_and_bc->GetSolution(), &size_of_soln_previous_step);
        }
        if (size_of_soln_previous_step == (int)r_mesh.GetNumNodes())
        {
            // We make an initial guess which gets copied by the Solve method of
            // SimpleLinearSolver, so we need to delete it too.
            Vec initial_guess;
            VecDuplicate(p_pde_and_bc->GetSolution(), &initial_guess);
            VecCopy(p_pde_and_bc->GetSolution(), initial_guess);

            // Use current solution as the initial guess
            p_pde_and_bc->DestroySolution(); // Solve method makes its own mCurrentPdeSolution
            p_pde_and_bc->SetSolution(solver.Solve(initial_guess));
            VecDestroy(initial_guess);
        }
        else
        {
            if (p_pde_and_bc->GetSolution())
            {
                assert(size_of_soln_previous_step != 0);
                p_pde_and_bc->DestroySolution();
            }
            p_pde_and_bc->SetSolution(solver.Solve());
        }

        ReplicatableVector solution_repl(p_pde_and_bc->GetSolution());

        // Update cellwise data
        for (unsigned i=0; i<r_mesh.GetNumNodes(); i++)
        {
            double solution = solution_repl[i];
            unsigned index = r_mesh.GetNode(i)->GetIndex();
            CellwiseData<DIM>::Instance()->SetValue(solution, index, pde_index);
        }
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SolvePdeUsingCoarseMesh()
{
    assert(!mPdeAndBcCollection.empty());

    for (unsigned pde_index=0; pde_index<mPdeAndBcCollection.size(); pde_index++)
    {
        assert(mPdeAndBcCollection[pde_index]);
        assert(mPdeAndBcCollection[pde_index]->HasAveragedSourcePde() == true);
    }

    TetrahedralMesh<DIM,DIM>& r_mesh = *mpCoarsePdeMesh;
    CellwiseData<DIM>::Instance()->ReallocateMemory();

    // Loop over cells and calculate centre of distribution
    c_vector<double, DIM> centre = zero_vector<double>(DIM);
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
        cell_iter != this->mrCellPopulation.End();
        ++cell_iter)
    {
        centre += this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter);
    }
    centre /= this->mrCellPopulation.GetNumRealCells();

    // Find max radius
    double max_radius = 0.0;
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
        cell_iter != this->mrCellPopulation.End();
        ++cell_iter)
    {
        double radius = norm_2(centre - this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter));
        if (radius > max_radius)
        {
            max_radius = radius;
        }
    }

    // Loop over elements of mPdeAndBcCollection
    for (unsigned pde_index=0; pde_index<mPdeAndBcCollection.size(); pde_index++)
    {
        // Get pointer to this PdeAndBoundaryConditions object
        PdeAndBoundaryConditions<DIM>* p_pde_and_bc = mPdeAndBcCollection[pde_index];

        // Set up boundary conditions
        AbstractBoundaryCondition<DIM>* p_bc = p_pde_and_bc->GetBoundaryCondition();

        BoundaryConditionsContainer<DIM,DIM,1> bcc(false);
        if (p_pde_and_bc->IsNeumannBoundaryCondition())
        {
            EXCEPTION("Neumann BCs not yet implemented when using a coarse PDE mesh");
        }
        else if (!mSetBcsOnCoarseBoundary)
        {
            // Get the set of coarse element indices that contain cells
            std::set<unsigned> coarse_element_indices_in_map;
            for (typename AbstractCentreBasedCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
                cell_iter != this->mrCellPopulation.End();
                ++cell_iter)
            {
                coarse_element_indices_in_map.insert(mCellPdeElementMap[*cell_iter]);
            }

            // Find the node indices that associated with elements whose
            // indices are NOT in the set coarse_element_indices_in_map
            std::set<unsigned> coarse_mesh_boundary_node_indices;

            for (unsigned i=0; i<r_mesh.GetNumElements(); i++)
            {
                // If the element index is NOT in the set...
                if (coarse_element_indices_in_map.find(i) == coarse_element_indices_in_map.end())
                {
                    // ... then get the element...
                    Element<DIM,DIM>* p_element = r_mesh.GetElement(i);

                    // ... and add its associated nodes to coarse_mesh_boundary_node_indices
                    for (unsigned local_index=0; local_index<DIM+1; local_index++)
                    {
                        unsigned node_index = p_element->GetNodeGlobalIndex(local_index);
                        coarse_mesh_boundary_node_indices.insert(node_index);
                    }
                }
            }

            // Apply boundary condition to the nodes in the set coarse_mesh_boundary_node_indices
            for (std::set<unsigned>::iterator iter = coarse_mesh_boundary_node_indices.begin();
                 iter != coarse_mesh_boundary_node_indices.end();
                 ++iter)
            {
            	bcc.AddDirichletBoundaryCondition(r_mesh.GetNode(*iter), p_bc, 0, false);
            }
        }
        else if(mSetBcsOnCoarseBoundary)
        {
        	// Find the node indices that associated with elements whose
            // indices are NOT in the set coarse_element_indices_in_map
            std::set<unsigned> coarse_mesh_boundary_node_indices;

            for (unsigned i=0;i<mpCoarsePdeMesh->GetNumNodes();i++)
            {
            	if(mpCoarsePdeMesh->GetNode(i)->IsBoundaryNode())
            	{
            		coarse_mesh_boundary_node_indices.insert(i);
            	}
            }

            // Apply boundary condition to the nodes in the set coarse_mesh_boundary_node_indices
            for (std::set<unsigned>::iterator iter = coarse_mesh_boundary_node_indices.begin();
                 iter != coarse_mesh_boundary_node_indices.end();
                 ++iter)
            {
            	bcc.AddDirichletBoundaryCondition(r_mesh.GetNode(*iter), p_bc, 0, true);
            }
        }

        PetscInt size_of_soln_previous_step = 0;

        if (p_pde_and_bc->GetSolution())
        {
            VecGetSize(p_pde_and_bc->GetSolution(), &size_of_soln_previous_step);
        }
double begin=MPI_Wtime();
        p_pde_and_bc->SetUpSourceTermsForAveragedSourcePde(mpCoarsePdeMesh);
double end=MPI_Wtime();
PRINT_VARIABLE(end-begin);
        SimpleLinearEllipticSolver<DIM,DIM> solver(mpCoarsePdeMesh, p_pde_and_bc->GetPde(), &bcc);

        if (size_of_soln_previous_step == (int)r_mesh.GetNumNodes())
        {
            // We make an initial guess which gets copied by the Solve method of
            // SimpleLinearSolver, so we need to delete it too.
            Vec initial_guess;
            VecDuplicate(p_pde_and_bc->GetSolution(), &initial_guess);
            VecCopy(p_pde_and_bc->GetSolution(), initial_guess);

            // Use current solution as the initial guess
            p_pde_and_bc->DestroySolution(); // Solve method makes its own mCurrentPdeSolution
            p_pde_and_bc->SetSolution(solver.Solve(initial_guess));
            VecDestroy(initial_guess);
        }
        else
        {
            /*
             * Eventually we will enable the coarse PDE mesh to change size, for example
             * in the case of a spheroid that grows a lot (see #630). In this case we should
             * uncomment the following code.
             *
            if (mCurrentPdeSolution)
            {
                assert(0);
                VecDestroy(mCurrentPdeSolution);
            }
            *
            */
            p_pde_and_bc->SetSolution(solver.Solve());
        }

        /*
         * Write solution to file if required.
         */
         
        ReplicatableVector solution_repl(p_pde_and_bc->GetSolution());

        if (SimulationTime::Instance()->GetTimeStepsElapsed() % this->mSamplingTimestepMultiple == 0)
        {
			double current_time = SimulationTime::Instance()->GetTime();
			(*mpVizCoarsePdeSolutionResultsFile) << current_time << "\t";

			for (unsigned i=0; i<solution_repl.GetSize();i++)
			{
				c_vector<double,DIM> location = mpCoarsePdeMesh->GetNode(i)->rGetLocation();
				double solution = solution_repl[i];
				(*mpVizCoarsePdeSolutionResultsFile) << i << " ";
				for (unsigned k=0; k<DIM; k++)
				{
					(*mpVizCoarsePdeSolutionResultsFile) << location[k] << " ";
				}
				(*mpVizCoarsePdeSolutionResultsFile) << solution << " ";
			}
			(*mpVizCoarsePdeSolutionResultsFile) << "\n";
        }

        /*
         * Update cellwise data - since the cells are not nodes on the coarse
         * mesh, we have to interpolate from the nodes of the coarse mesh onto
         * the cell locations.
         */

        for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
             cell_iter != this->mrCellPopulation.End();
             ++cell_iter)
        {
            // Find coarse mesh element containing cell
            unsigned elem_index = FindCoarseElementContainingCell(*cell_iter);

            Element<DIM,DIM>* p_element = mpCoarsePdeMesh->GetElement(elem_index);

            const ChastePoint<DIM>& r_position_of_cell = this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter);

            c_vector<double,DIM+1> weights = p_element->CalculateInterpolationWeights(r_position_of_cell);

            double interpolated_solution = 0.0;

            for (unsigned i=0; i<DIM+1/*num_nodes*/; i++)
            {
                double nodal_value = solution_repl[ p_element->GetNodeGlobalIndex(i) ];
                interpolated_solution += nodal_value*weights(i);
            }

            unsigned index = this->mrCellPopulation.GetLocationIndexUsingCell(*cell_iter);
            CellwiseData<DIM>::Instance()->SetValue(interpolated_solution, index,pde_index);
        }
    }
}

template<unsigned DIM>
unsigned OffLatticeSimulationWithPdes<DIM>::FindCoarseElementContainingCell(CellPtr pCell)
{
    // Get containing element at last timestep from mCellPdeElementMap
    unsigned old_element_index = mCellPdeElementMap[pCell];

    // Create a std::set of guesses for the current containing element
    std::set<unsigned> test_elements;
    test_elements.insert(old_element_index);

    Element<DIM,DIM>* p_element = mpCoarsePdeMesh->GetElement(old_element_index);

    for (unsigned local_index=0; local_index<DIM+1; local_index++)
    {
        std::set<unsigned> element_indices = p_element->GetNode(local_index)->rGetContainingElementIndices();

        for (std::set<unsigned>::iterator iter = element_indices.begin();
             iter != element_indices.end();
             ++iter)
        {
            test_elements.insert(*iter);
        }
    }

    // Find new element, using the previous one as a guess
    const ChastePoint<DIM>& r_cell_position = this->mrCellPopulation.GetLocationOfCellCentre(pCell);
    unsigned new_element_index = mpCoarsePdeMesh->GetContainingElementIndex(r_cell_position, false, test_elements);

    // Update mCellPdeElementMap
    mCellPdeElementMap[pCell] = new_element_index;

    return new_element_index;
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::PostSolve()
{
	CellBasedEventHandler::BeginEvent(CellBasedEventHandler::PDE);
    SolvePde();
    CellBasedEventHandler::EndEvent(CellBasedEventHandler::PDE);

    // Save results to file
    SimulationTime* p_time = SimulationTime::Instance();

    double time_next_step = p_time->GetTime() + p_time->GetTimeStep();

    if ((p_time->GetTimeStepsElapsed()+1)%this->mSamplingTimestepMultiple == 0)
    {
        WritePdeSolution(time_next_step);
    }

#define COVERAGE_IGNORE
    if (mWriteDailyAverageRadialPdeSolution)
    {
        ///\todo Worry about round-off errors
        unsigned num_timesteps_per_day = (unsigned) (DBL_EPSILON + 24/SimulationTime::Instance()->GetTimeStep());

        if ((p_time->GetTimeStepsElapsed()+1) % num_timesteps_per_day == 0)
        {
            WriteAverageRadialPdeSolution(time_next_step, mNumRadialIntervals);
        }
    }
#undef COVERAGE_IGNORE

}

template<unsigned DIM>
c_vector<double,DIM> OffLatticeSimulationWithPdes<DIM>::GetCellPopulationLocation()
{
	// Loop over cells and calculate centre of mass
	c_vector<double,DIM> cell_population_centre = zero_vector<double>(DIM);
	for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
		cell_iter != this->mrCellPopulation.End();
		++cell_iter)
	{
		cell_population_centre += this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter);
	}
	cell_population_centre /= this->mrCellPopulation.GetNumRealCells();

	return cell_population_centre;
}

template<unsigned DIM>
c_vector<double,DIM> OffLatticeSimulationWithPdes<DIM>::GetCellPopulationSize()
{
	// Find cell population size
	c_vector<double,DIM> population_centre=this->GetCellPopulationLocation();
	c_vector<double,DIM> cell_population_max_size;
	for (unsigned i=0; i<DIM; i++)
	{
		cell_population_max_size[i] = 0.0;
	}
	for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
		cell_iter != this->mrCellPopulation.End();
		++cell_iter)
	{

		for (unsigned i=0; i<DIM; i++)
		{
			double displacement = abs(population_centre[i] - (this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter))[i]);

			if (displacement > cell_population_max_size[i])
			{
				cell_population_max_size[i] = displacement;
			}
		}
	}

	return cell_population_max_size;

}

//////////////////////////////////////////////////////////////////////////////
//                             Output methods                               //
//////////////////////////////////////////////////////////////////////////////

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::WritePdeSolution(double time)
{
    if (PetscTools::AmMaster())
    {
        // Since there are no ghost nodes, the number of nodes must equal the number of real cells
        assert(this->mrCellPopulation.GetNumNodes() == this->mrCellPopulation.GetNumRealCells());

        (*mpVizPdeSolutionResultsFile) << time << "\t";

        for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
             cell_iter != this->mrCellPopulation.End();
             ++cell_iter)
        {
            unsigned global_index = this->mrCellPopulation.GetLocationIndexUsingCell(*cell_iter);
            (*mpVizPdeSolutionResultsFile) << global_index << " ";

            const c_vector<double,DIM>& position = this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter);
            for (unsigned i=0; i<DIM; i++)
            {
                (*mpVizPdeSolutionResultsFile) << position[i] << " ";
            }

            double solution = CellwiseData<DIM>::Instance()->GetValue(*cell_iter);
            (*mpVizPdeSolutionResultsFile) << solution << " ";
        }
        (*mpVizPdeSolutionResultsFile) << "\n";
    }
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SetWriteAverageRadialPdeSolution(unsigned numRadialIntervals, bool writeDailyResults)
{
    mWriteAverageRadialPdeSolution = true;
    mNumRadialIntervals = numRadialIntervals;
    mWriteDailyAverageRadialPdeSolution = writeDailyResults;
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::SetImposeBcsOnPerimeterOfPopulation()
{
	// Perhaps should throw an exception
	// EXCEPTION("This method is not fully working and tested yet.");
	mSetBcsOnCoarseBoundary = false;
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::WriteAverageRadialPdeSolution(double time, unsigned numRadialIntervals)
{
    (*mpAverageRadialPdeSolutionResultsFile) << time << " ";

    // Calculate the centre of the cell population
    c_vector<double,DIM> centre = zero_vector<double>(DIM);
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
         cell_iter != this->mrCellPopulation.End();
         ++cell_iter)
    {
       centre += (this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter));
    }
    centre /= ((double) this->mrCellPopulation.GetNumNodes());

    // Calculate the distance between each node and the centre of the cell population, as well as the maximum of these
    std::map<double, CellPtr> distance_cell_map;
    double max_distance_from_centre = 0.0;
    for (typename AbstractCellPopulation<DIM>::Iterator cell_iter = this->mrCellPopulation.Begin();
         cell_iter != this->mrCellPopulation.End();
         ++cell_iter)
    {
        double distance = norm_2(this->mrCellPopulation.GetLocationOfCellCentre(*cell_iter) - centre);
        distance_cell_map[distance] = *cell_iter;

        if (distance > max_distance_from_centre)
        {
            max_distance_from_centre = distance;
        }
    }

    // Create vector of radius intervals
    std::vector<double> radius_intervals;
    for (unsigned i=0; i<numRadialIntervals; i++)
    {
        double upper_radius = max_distance_from_centre*((double) i+1)/((double) numRadialIntervals);
        radius_intervals.push_back(upper_radius);
    }

    // Calculate PDE solution in each radial interval
    double lower_radius = 0.0;
    for (unsigned i=0; i<numRadialIntervals; i++)
    {
        unsigned counter = 0;
        double average_solution = 0.0;

        for (std::map<double, CellPtr>::iterator iter = distance_cell_map.begin();
             iter != distance_cell_map.end();
             ++iter)
        {
            if (iter->first > lower_radius && iter->first <= radius_intervals[i])
            {
                average_solution += CellwiseData<DIM>::Instance()->GetValue(iter->second);
                counter++;
            }
        }
        if (counter > 0)
        {
            average_solution /= (double) counter;
        }

        // Write results to file
        (*mpAverageRadialPdeSolutionResultsFile) << radius_intervals[i] << " " << average_solution << " ";
        lower_radius = radius_intervals[i];
    }
    (*mpAverageRadialPdeSolutionResultsFile) << "\n";
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::WriteCoarseMeshToFile()
{
    TrianglesMeshWriter<DIM,DIM> mesh_writer(this->mSimulationOutputDirectory+"/coarse_mesh_output", "coarse_mesh",false);
	mesh_writer.WriteFilesUsingMesh(*mpCoarsePdeMesh);
}

template<unsigned DIM>
void OffLatticeSimulationWithPdes<DIM>::OutputSimulationParameters(out_stream& rParamsFile)
{
    *rParamsFile << "\t\t<WriteAverageRadialPdeSolution>" << mWriteAverageRadialPdeSolution << "</WriteAverageRadialPdeSolution>\n";
    *rParamsFile << "\t\t<WriteDailyAverageRadialPdeSolution>" << mWriteDailyAverageRadialPdeSolution << "</WriteDailyAverageRadialPdeSolution>\n";

    // Call method on direct parent class
    OffLatticeSimulation<DIM>::OutputSimulationParameters(rParamsFile);
}

/////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
/////////////////////////////////////////////////////////////////////////////

template class OffLatticeSimulationWithPdes<1>;
template class OffLatticeSimulationWithPdes<2>;
template class OffLatticeSimulationWithPdes<3>;

// Serialization for Boost >= 1.36
#include "SerializationExportWrapperForCpp.hpp"
EXPORT_TEMPLATE_CLASS_SAME_DIMS(OffLatticeSimulationWithPdes)