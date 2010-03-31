/*

Copyright (C) University of Oxford, 2005-2010

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

#ifndef TESTCARDIACSIMULATIONARCHIVER_HPP_
#define TESTCARDIACSIMULATIONARCHIVER_HPP_

#include <sys/stat.h> // For chmod()

#include <cxxtest/TestSuite.h>

#include "CardiacSimulationArchiver.hpp" // Needs to be before other Chaste code

#include "Exception.hpp"
#include "DistributedVector.hpp"
#include "DistributedVectorFactory.hpp"
#include "ArchiveOpener.hpp"

#include "AbstractCardiacCell.hpp"
#include "PlaneStimulusCellFactory.hpp"
#include "ZeroStimulusCellFactory.hpp"
#include "LuoRudyIModel1991OdeSystem.hpp"
#include "BackwardEulerFoxModel2002Modified.hpp"
#include "FaberRudy2000Version3.hpp"

#include "BidomainProblem.hpp"
#include "MonodomainProblem.hpp"
#include "CompareHdf5ResultsFiles.hpp"
#include "PetscSetupAndFinalize.hpp"

#include "Electrodes.hpp"
#include "SimpleBathProblemSetup.hpp"

#include "NumericFileComparison.hpp"

/// For checkpoint migration tests
#define ABS_TOL 1e-6

class TestCardiacSimulationArchiver : public CxxTest::TestSuite
{
private:

    std::vector<double> mSolutionReplicated1d2ms;///<Used to test differences between tests

public:

    /*
     *  Simple bidomain simulation to test against in TestArchivingWithHelperClass below
     */
    void TestSimpleBidomain1D() throw(Exception)
    {
        HeartConfig::Instance()->SetIntracellularConductivities(Create_c_vector(0.0005));
        HeartConfig::Instance()->SetExtracellularConductivities(Create_c_vector(0.0005));
        HeartConfig::Instance()->SetSurfaceAreaToVolumeRatio(1.0);
        HeartConfig::Instance()->SetCapacitance(1.0);

        HeartConfig::Instance()->SetSimulationDuration(2.0); //ms
        HeartConfig::Instance()->SetMeshFileName("mesh/test/data/1D_0_to_1mm_10_elements");
        HeartConfig::Instance()->SetOutputDirectory("BidomainSimple1d");
        HeartConfig::Instance()->SetOutputFilenamePrefix("BidomainLR91_1d");

        PlaneStimulusCellFactory<LuoRudyIModel1991OdeSystem, 1> cell_factory;
        BidomainProblem<1> bidomain_problem( &cell_factory );

        bidomain_problem.Initialise();
        bidomain_problem.Solve();

        // check some voltages
        ReplicatableVector solution_replicated(bidomain_problem.GetSolution());

        double atol=5e-3;

        TS_ASSERT_DELTA(solution_replicated[1], -16.4861, atol);
        TS_ASSERT_DELTA(solution_replicated[2], 22.8117, atol);
        TS_ASSERT_DELTA(solution_replicated[3], -16.4893, atol);
        TS_ASSERT_DELTA(solution_replicated[5], -16.5617, atol);
        TS_ASSERT_DELTA(solution_replicated[7], -16.6761, atol);
        TS_ASSERT_DELTA(solution_replicated[9], -16.8344, atol);
        TS_ASSERT_DELTA(solution_replicated[10], 25.3148, atol);

        for (unsigned index=0; index<solution_replicated.GetSize(); index++)
        {
            mSolutionReplicated1d2ms.push_back(solution_replicated[index]);
        }
    }

    void TestArchivingWithHelperClass()
    {
        // Save
        {
            HeartConfig::Instance()->SetIntracellularConductivities(Create_c_vector(0.0005));
            HeartConfig::Instance()->SetExtracellularConductivities(Create_c_vector(0.0005));
            //HeartConfig::Instance()->SetMeshFileName("mesh/test/data/1D_0_to_1mm_10_elements");
            HeartConfig::Instance()->SetOutputDirectory("BiProblemArchiveHelper");
            HeartConfig::Instance()->SetOutputFilenamePrefix("BidomainLR91_1d");
            HeartConfig::Instance()->SetSurfaceAreaToVolumeRatio(1.0);
            HeartConfig::Instance()->SetCapacitance(1.0);

            PlaneStimulusCellFactory<LuoRudyIModel1991OdeSystem, 1> cell_factory;
            BidomainProblem<1> bidomain_problem( &cell_factory );

            /// \todo: Make this test pass if the mesh is set via HeartConfig
            TrianglesMeshReader<1,1> mesh_reader("mesh/test/data/1D_0_to_1mm_10_elements");
            DistributedTetrahedralMesh<1,1> mesh;
            mesh.ConstructFromMeshReader(mesh_reader);
            bidomain_problem.SetMesh(&mesh);

            bidomain_problem.Initialise();
            HeartConfig::Instance()->SetSimulationDuration(1.0); //ms
            bidomain_problem.Solve();

            CardiacSimulationArchiver<BidomainProblem<1> >::Save(bidomain_problem, "bidomain_problem_archive_helper", false);
        }

        // Load
        {
            BidomainProblem<1> *p_bidomain_problem;
            p_bidomain_problem = CardiacSimulationArchiver<BidomainProblem<1> >::Load("bidomain_problem_archive_helper");

            HeartConfig::Instance()->SetSimulationDuration(2.0); //ms
            p_bidomain_problem->Solve();

            // check some voltages
            ReplicatableVector solution_replicated(p_bidomain_problem->GetSolution());
            double atol=5e-3;
            TS_ASSERT_DELTA(solution_replicated[1], -16.4861, atol);
            TS_ASSERT_DELTA(solution_replicated[2], 22.8117, atol);
            TS_ASSERT_DELTA(solution_replicated[3], -16.4893, atol);
            TS_ASSERT_DELTA(solution_replicated[5], -16.5617, atol);
            TS_ASSERT_DELTA(solution_replicated[7], -16.6761, atol);
            TS_ASSERT_DELTA(solution_replicated[9], -16.8344, atol);
            TS_ASSERT_DELTA(solution_replicated[10], 25.3148, atol);

            for (unsigned index=0; index<solution_replicated.GetSize(); index++)
            {
                //Shouldn't differ from the original run at all
                TS_ASSERT_DELTA(solution_replicated[index], mSolutionReplicated1d2ms[index],  5e-11);
            }
            // check output file contains results for the whole simulation
            TS_ASSERT(CompareFilesViaHdf5DataReader("BiProblemArchiveHelper", "BidomainLR91_1d", true,
                                                    "BidomainSimple1d", "BidomainLR91_1d", true));

            // Free memory
            delete p_bidomain_problem;
        }

        {
            // Coverage of "couldn't find file" exception
            BidomainProblem<1> *p_bidomain_problem;
            TS_ASSERT_THROWS_CONTAINS(p_bidomain_problem = CardiacSimulationArchiver<BidomainProblem<1> >::Load("missing_directory"),
                                      "Unable to open archive information file:");
        }
    }

    /**
     *  Test used to generate data for the acceptance test resume_bidomain. We run the same simulation as in save_bidomain
     *  and archive it. resume_bidomain will load it and resume the simulation.
     *
     *  If the archiving format changes, both acceptance tests (save_bidomain and resume_bidomain) and the second part of this
     *  test will fail. Do the following to fix them.
     *
     *  Updated save_bidomain/ChasteResults_10ms_arch_0.chaste
     *    This can be easily done with texttest GUI. Run save_bidomain test to find that ChasteResults_10ms_arch_0.chaste
     *    contains differences (but NO other file). Highlight it and use the Save button on the window top left part.
     *
scons build=GccOpt_hostconfig,boost=1-33-1  test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp -r /tmp/$USER/testoutput/save_bidomain/ ~/eclipse/workspace/Chaste/apps/texttest/chaste/resume_bidomain/
cp -r /tmp/$USER/testoutput/SaveBidomain/ ~/eclipse/workspace/Chaste/apps/texttest/chaste/resume_bidomain/
     */
    void TestGenerateResultsForResumeBidomain()
    {
        HeartConfig::Instance()->SetUseFixedSchemaLocation(true);
        HeartConfig::Instance()->SetParametersFile("apps/texttest/chaste/save_bidomain/ChasteParameters.xml");
        // We reset the mesh filename to include the relative path
        HeartConfig::Instance()->SetMeshFileName("mesh/test/data/cube_1626_elements");

        TS_ASSERT_EQUALS(HeartConfig::Instance()->GetSimulationDuration(), 10.0);
        TS_ASSERT_EQUALS(HeartConfig::Instance()->GetMeshName(),
                         "mesh/test/data/cube_1626_elements");
        TS_ASSERT(HeartConfig::Instance()->GetDefaultIonicModel().Hardcoded().present());
        TS_ASSERT_EQUALS(HeartConfig::Instance()->GetDefaultIonicModel().Hardcoded().get(),
                         cp::ionic_models_available_type::Fox2002BackwardEuler);

        HeartConfig::Instance()->SetOutputDirectory("SaveBidomain");
        HeartConfig::Instance()->SetOutputFilenamePrefix("BidomainLR91_3d"); ///\todo it's not LR91

        // This cell factory should apply the same stimulus described in the xml config file.
        PlaneStimulusCellFactory<BackwardEulerFoxModel2002Modified, 3> cell_factory(-80000.0, 1.0);
        BidomainProblem<3> bidomain_problem( &cell_factory );

        bidomain_problem.Initialise();
        bidomain_problem.Solve();

        CardiacSimulationArchiver<BidomainProblem<3> >::Save(bidomain_problem, "save_bidomain", false);

        if (PetscTools::IsSequential())
        {
            // We can't use CardiacSimulationArchiver::Load for this, as it can't read files in the repository
            ArchiveOpener<boost::archive::text_iarchive, std::ifstream> arch_opener(
                "apps/texttest/chaste/resume_bidomain/save_bidomain/",
                "archive.arch",
                false);
            boost::archive::text_iarchive* p_arch = arch_opener.GetCommonArchive();

            BidomainProblem<3> *p_bidomain_problem;
            TS_ASSERT_THROWS_NOTHING((*p_arch) >> p_bidomain_problem);
            //Double check that the acceptance test archive really is valid
            TS_ASSERT_EQUALS(p_bidomain_problem->mMeshFilename, "");
            TS_ASSERT_EQUALS(p_bidomain_problem->mPrintOutput, true);
            TS_ASSERT_EQUALS(p_bidomain_problem->mNodesToOutput.size(), 0u);
            TS_ASSERT_EQUALS(p_bidomain_problem->mCurrentTime, 10.0);
            delete p_bidomain_problem;
        }
    }


    /**
     *  Test used to generate data for the acceptance test resume_monodomain. We run the same simulation as in save_monodomain
     *  and archive it. resume_monodomain will load it and resume the simulation.
     *
     *  If the archiving format changes, both acceptance tests (save_monodomain and resume_monodomain) and the second part of this
     *  test will fail. Do the following to fix them.
     *
     *  Updated save_monodomain/ChasteResults_10ms_arch_0.chaste
     *    This can be easily done with texttest GUI. Run save_monodomain test to find that ChasteResults_10ms_arch_0.chaste
     *    contains differences (but NO other file). Highlight it and use the Save button on the window top left part.
     *
scons build=GccOpt_hostconfig,boost=1-33-1  test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp -r /tmp/$USER/testoutput/save_monodomain/ ~/eclipse/workspace/Chaste/apps/texttest/chaste/resume_monodomain/
cp -r /tmp/$USER/testoutput/SaveMonodomain/ ~/eclipse/workspace/Chaste/apps/texttest/chaste/resume_monodomain/
 */
    void TestGenerateResultsForResumeMonodomain()
    {
        HeartConfig::Instance()->SetUseFixedSchemaLocation(true);
        HeartConfig::Instance()->SetParametersFile("apps/texttest/chaste/save_monodomain/ChasteParameters.xml");

        TS_ASSERT_EQUALS(HeartConfig::Instance()->GetSimulationDuration(), 10.0);
        TS_ASSERT(HeartConfig::Instance()->GetDefaultIonicModel().Hardcoded().present());
        TS_ASSERT_EQUALS(HeartConfig::Instance()->GetDefaultIonicModel().Hardcoded().get(),
                         cp::ionic_models_available_type::Fox2002BackwardEuler);

        HeartConfig::Instance()->SetOutputDirectory("SaveMonodomain");
        HeartConfig::Instance()->SetOutputFilenamePrefix("MonodomainLR91_2d"); ///\todo it's not LR91

        // This cell factory should apply the same stimulus described in the xml config file.
        PlaneStimulusCellFactory<BackwardEulerFoxModel2002Modified, 2> cell_factory(-600000.0, 1.0);
        MonodomainProblem<2> monodomain_problem( &cell_factory );

        monodomain_problem.Initialise();
        monodomain_problem.Solve();

        CardiacSimulationArchiver<MonodomainProblem<2> >::Save(monodomain_problem, "save_monodomain", false);

        if (PetscTools::IsSequential())
        {
            // We can't use CardiacSimulationArchiver::Load for this, as it can't read files in the repository
            ArchiveOpener<boost::archive::text_iarchive, std::ifstream> arch_opener(
                "apps/texttest/chaste/resume_monodomain/save_monodomain/",
                "archive.arch",
                false);
            boost::archive::text_iarchive* p_arch = arch_opener.GetCommonArchive();

            MonodomainProblem<2> *p_monodomain_problem;
            TS_ASSERT_THROWS_NOTHING((*p_arch) >> p_monodomain_problem);
            //Double check that the acceptance test archive really is valid
            TS_ASSERT_EQUALS(p_monodomain_problem->mMeshFilename, "");
            TS_ASSERT_EQUALS(p_monodomain_problem->mPrintOutput, true);
            TS_ASSERT_EQUALS(p_monodomain_problem->mNodesToOutput.size(), 0u);
            TS_ASSERT_EQUALS(p_monodomain_problem->mCurrentTime, 10.0);
            delete p_monodomain_problem;
        }
    }

    /***********************************************************************
     *                                                                     *
     *         Below this point are the checkpoint migration tests         *
     *                                                                     *
     * Note that archives are created for these using a variety of numbers *
     * of processes: 1, 2, and 3, so we get a good range of migration test *
     * cases.  The continuous build runs this testsuite with 1,2 processes *
     * and the nightly Parallel_Continuous tests use 3.                    *
     ***********************************************************************/

private:
    // Helper functions for the migration tests defined below.
    template<class Problem, unsigned DIM>
    Problem* DoMigrateAndBasicTests(const std::string& rArchiveDirectory,
                                    const std::string& rRefArchiveDir,
                                    const std::string& rSourceDir,
                                    const unsigned totalNumCells,
                                    bool isDistributedMesh=false,
                                    double currentTime=0.0)
    {
        // Do the migration to sequential
        Problem* p_problem = CardiacSimulationArchiver<Problem>::Migrate(rArchiveDirectory);

        // Some basic tests that we have the right data
        TS_ASSERT_EQUALS(p_problem->mMeshFilename, "");
        TS_ASSERT_EQUALS(p_problem->mPrintOutput, true);
        TS_ASSERT_EQUALS(p_problem->mNodesToOutput.size(), 0u);
        TS_ASSERT_EQUALS(p_problem->mCurrentTime, currentTime);
        DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
        TS_ASSERT(p_factory->GetOriginalFactory());
        TS_ASSERT_EQUALS(p_factory->GetOriginalFactory()->GetProblemSize(), totalNumCells);
        unsigned local_num_cells = p_factory->GetLocalOwnership();
        TS_ASSERT_EQUALS(p_problem->GetPde()->rGetCellsDistributed().size(), local_num_cells);
        TS_ASSERT_EQUALS(p_problem->rGetMesh().GetNumAllNodes(), totalNumCells);
        TS_ASSERT_EQUALS(p_problem->rGetMesh().GetNumNodes(), totalNumCells);
        TS_ASSERT_EQUALS(&(p_problem->rGetMesh()), p_problem->GetPde()->pGetMesh());
        // Check the mesh is/isn't the parallel variety
        const DistributedTetrahedralMesh<DIM,DIM>* p_dist_mesh = dynamic_cast<const DistributedTetrahedralMesh<DIM,DIM>*>(p_problem->GetPde()->pGetMesh());
        if (isDistributedMesh)
        {
            TS_ASSERT(p_dist_mesh != NULL);
        }
        else
        {
            TS_ASSERT(p_dist_mesh == NULL);
        }

        // All real cells should be at initial conditions if we're starting at t=0.
        if (currentTime == 0.0)
        {
            std::vector<double> inits;
            for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
            {
                AbstractCardiacCell* p_cell = p_problem->GetPde()->GetCardiacCell(i);
                FakeBathCell* p_fake_cell = dynamic_cast<FakeBathCell*>(p_cell);
                if (p_fake_cell == NULL)
                {
                    if (inits.empty())
                    {
                        inits = p_cell->GetInitialConditions();
                    }
                    std::vector<double>& r_state = p_cell->rGetStateVariables();
                    TS_ASSERT_EQUALS(r_state.size(), inits.size());
                    for (unsigned j=0; j<r_state.size(); j++)
                    {
                        TS_ASSERT_DELTA(r_state[j], inits[j], 1e-10);
                    }
                }
            }
        }

        // Save it to an archive for the current number of processes
        CardiacSimulationArchiver<Problem>::Save(*p_problem, rArchiveDirectory + "/new_archive");

        // Compare with the global archive from the previous test
        OutputFileHandler handler(rArchiveDirectory, false);
        std::string ref_archive = handler.GetChasteTestOutputDirectory() + rRefArchiveDir + "/archive.arch";
        std::string my_archive = handler.GetOutputDirectoryFullPath() + "new_archive/archive.arch";
        EXPECT0(system, "cmp " + ref_archive + " " + my_archive);
        //THIS WON'T WORK WITH DIFFERENT VERSIONS OF BOOST:
#ifndef BOOST_VERSION
        TS_FAIL("This test needs to know the version of Boost with which it was compiled.");
#endif
#if BOOST_VERSION >= 103400 && BOOST_VERSION < 103500
        if (PetscTools::IsSequential())
        {
            /*
             * This would differ because we get extra copies of the ODE solver and intracellular stimulus objects
             * (one per original process which had them).
             * If this fails you probably just need to copy a new reference_0_archive file from "my_archive.0",
             * (running with Boost 1.34 - this code isn't executed otherwise!)
             * but do check that's the case!
             *
             * (Assuming 1.34)
scons test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp /tmp/$USER/testoutput/TestLoadAsSequential/new_archive/archive.arch.0 ./heart/test/data/checkpoint_migration/reference_0_archive
cp /tmp/$USER/testoutput/TestLoadAsSequentialWithBath/new_archive/archive.arch.0 ./heart/test/data/checkpoint_migration_with_bath/reference_0_archive
cp /tmp/chaste/testoutput/TestLoadAsSequentialWithBathAndDistributedMesh/new_archive/archive.arch.0 ./heart/test/data/checkpoint_migration_with_bath_and_distributed_mesh/reference_0_archive
cp /tmp/$USER/testoutput/TestBcsOnNonMasterOnly/new_archive/archive.arch.0 ./heart/test/data/checkpoint_migration_bcs_on_non_master_only/reference_0_archive
cp /tmp/$USER/testoutput/TestMigrateAfterSolve/new_archive/archive.arch.0 ./heart/test/data/checkpoint_migration_after_solve/reference_0_archive
             */
            EXPECT0(system, "cmp " + rSourceDir + "reference_0_archive " + my_archive + ".0");
        }
#endif

        // Return the problem for further tests
        return p_problem;
    }

    /**
     * Write results to a subfolder of the original output dir, to avoid over-writing
     * original results & archives.
     *
     * Also change the end time if we're resuming a simulation, and copy the previous
     * results h5 file into the new subfolder in that case, so that it can be extended.
     */
    void SetOutputDirAndEndTime(const std::string& rDirToCopyFrom,
                                const std::string& rRefArchiveDir,
                                const std::string& rSubDir,
                                double endTime)
    {
        // Change output directory to avoid over-writing original results & archives
        std::string output_dir = rRefArchiveDir + "/" + rSubDir;
        HeartConfig::Instance()->SetOutputDirectory(output_dir);
        if (endTime > 0.0)
        {
            HeartConfig::Instance()->SetSimulationDuration(endTime);
            // Also need to copy the old results into the new output folder
            OutputFileHandler handler1(rDirToCopyFrom, false); // must be collective
            OutputFileHandler handler2(output_dir, false); // must be collective
            if (PetscTools::AmMaster())
            {
                std::string prev_results_path = handler1.GetOutputDirectoryFullPath() + "simulation.h5";
                std::string new_path = handler2.GetOutputDirectoryFullPath() + "simulation.h5";
                EXPECT0(system, "cp " + prev_results_path + " " + new_path);
            }
        }
    }

    template<class Problem>
    void DoSimulationsAfterMigrationAndCompareResults(Problem* pProblem,
                                                      const std::string& rArchiveDirectory,
                                                      const std::string& rRefArchiveDir,
                                                      unsigned numVars,
                                                      double endTime=0.0)
    {
        // Simulate this problem
        SetOutputDirAndEndTime(rArchiveDirectory, rRefArchiveDir, "mig1", endTime);
        pProblem->Solve();
        // Replicate the results vector
        ReplicatableVector migrated_soln_1(pProblem->GetSolution());
        // and destroy the problem, so we don't get confusion from 2 problems at the same time
        delete pProblem;

        // Compare the results with simulating the archive from the previous test
        Problem* p_orig_problem = CardiacSimulationArchiver<Problem>::Load(rRefArchiveDir);
        SetOutputDirAndEndTime(rRefArchiveDir, rRefArchiveDir, "orig", endTime);
        p_orig_problem->Solve();
        ReplicatableVector orig_soln(p_orig_problem->GetSolution());
        TS_ASSERT_EQUALS(migrated_soln_1.GetSize(), orig_soln.GetSize());
        for (unsigned i=0; i<migrated_soln_1.GetSize(); i++)
        {
            double tol;
            if (PetscTools::GetNumProcs() < 4)
            {
                tol = ABS_TOL;
            }
            else
            {
                // This is horrible, but it seems that the change in partitioning when using a constructed
                // mesh leads to slight differences in PETSc's linear solve (we think)...
                tol = 300*ABS_TOL;
            }
            TS_ASSERT_DELTA(orig_soln[i], migrated_soln_1[i], tol);
        }
        delete p_orig_problem;

        // Now try loading the migrated simulation that we saved above
        Problem* p_problem = CardiacSimulationArchiver<Problem>::Load(rArchiveDirectory + "/new_archive");
        SetOutputDirAndEndTime(rArchiveDirectory, rRefArchiveDir, "mig2", endTime);
        // Change end time?
        if (endTime > 0.0)
        {
            HeartConfig::Instance()->SetSimulationDuration(endTime);
        }
        p_problem->Solve();
        // and again compare the results
        ReplicatableVector migrated_soln_2(p_problem->GetSolution());
        TS_ASSERT_EQUALS(migrated_soln_1.GetSize(), migrated_soln_2.GetSize());
        for (unsigned i=0; i<migrated_soln_1.GetSize(); i++)
        {
            TS_ASSERT_DELTA(migrated_soln_2[i], migrated_soln_1[i], ABS_TOL);
        }
        delete p_problem;
    }

public:
    void TestMigrationExceptions() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration_exception/";
        std::string target_directory = "TestMigrationExceptions";
        OutputFileHandler handler(target_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestMigrationExceptions");
        BidomainProblem<3>* p_problem;
        // Cover exceptions
        TS_ASSERT_THROWS_CONTAINS(p_problem = CardiacSimulationArchiver<BidomainProblem<3> >::Migrate(target_directory),
                                  "Cannot load main archive file: ");
        TS_ASSERT_THROWS_CONTAINS(p_problem = CardiacSimulationArchiver<BidomainProblem<3> >::Migrate("non_existent_dir"),
                                  "Unable to open archive information file: ");
    }

    /**
     * Run this in parallel (build=_3) to create the archive for TestLoadAsSequential.
     *
scons build=GccOpt_hostconfig,boost=1-33-1_3 test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp /tmp/$USER/testoutput/TestCreateArchiveForLoadAsSequential/?* ./heart/test/data/checkpoint_migration/
     *
     * Sets up a simulation and archives it without solving at all.
     *
     * When running sequentially, this creates an archive we can compare with
     * that produced by the next test.
     *
     * Generates a 3d cube mesh with 125 nodes, corners at (0,0,0) and (1,1,1)
     * with nodal spacing of 0.25cm.
     */
    void TestCreateArchiveForLoadAsSequential() throw (Exception)
    {
        std::string directory = "TestCreateArchiveForLoadAsSequential";
        HeartConfig::Instance()->Reset();
        HeartConfig::Instance()->SetSlabDimensions(1, 1, 1, 0.2);
        HeartConfig::Instance()->SetSimulationDuration(0.2);
        HeartConfig::Instance()->SetOutputDirectory(directory);
        HeartConfig::Instance()->SetOutputFilenamePrefix("simulation");
        HeartConfig::Instance()->SetUseAbsoluteTolerance(ABS_TOL);

        PlaneStimulusCellFactory<BackwardEulerFoxModel2002Modified, 3> cell_factory(-80000.0, 1.0);
        BidomainProblem<3> bidomain_problem( &cell_factory );

        bidomain_problem.Initialise();

        CardiacSimulationArchiver<BidomainProblem<3> >::Save(bidomain_problem, directory);

        // This is quite a nasty exception to cover, as you need to be able to write to all files
        // in the directory exception archive.info.  It also has to be done in parallel too, to
        // cover the exception replication.
        OutputFileHandler handler(directory, false);
        std::string info_path = handler.GetOutputDirectoryFullPath() + "archive.info";
        if (PetscTools::AmMaster())
        {
            chmod(info_path.c_str(), 0444);
        }
        TS_ASSERT_THROWS_CONTAINS(CardiacSimulationArchiver<BidomainProblem<3> >::Save(bidomain_problem, directory, false),
                                  "Unable to open archive information file");
        if (PetscTools::AmMaster())
        {
            chmod(info_path.c_str(), 0666);
        }
    }

    /**
     * #1159 - the first part of migrating a checkpoint to a different number of processes.
     *
     * The original LoadAsSequential and LoadFromSequential methods are now deprecated, since
     * we can do everything with a single Migrate method.  But the tests are still named after
     * the original methods.
     */
    void TestLoadAsSequential() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration/";
        std::string archive_directory = "TestLoadAsSequential";
        std::string ref_archive_dir = "TestCreateArchiveForLoadAsSequential";
        OutputFileHandler handler(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestLoadAsSequential");

        // Do the migration
        const unsigned num_cells = 216u;
        BidomainProblem<3>* p_problem = DoMigrateAndBasicTests<BidomainProblem<3>,3>(archive_directory, ref_archive_dir, source_directory, num_cells, true);

        // All cells at x=0 should have a SimpleStimulus(-80000, 1).
        DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
        for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
        {
            AbstractCardiacCell* p_cell = p_problem->GetPde()->GetCardiacCell(i);
            double x = p_problem->rGetMesh().GetNode(i)->GetPoint()[0];

            if (x*x < 1e-10)
            {
                // Stim exists
                TS_ASSERT_DELTA(p_cell->GetStimulus(0.0), -80000.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(1.0), -80000.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(1.001), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(-1e-10), 0.0, 1e-10);
            }
            else
            {
                // No stim
                TS_ASSERT_DELTA(p_cell->GetStimulus(0.0), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(1.0), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(1.001), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(-1e-10), 0.0, 1e-10);
            }
        }

        // Test bccs - none defined in this problem
        TS_ASSERT(! p_problem->mpDefaultBoundaryConditionsContainer);
        TS_ASSERT(! p_problem->mpBoundaryConditionsContainer);

        DoSimulationsAfterMigrationAndCompareResults(p_problem, archive_directory, ref_archive_dir, 2);
    }


    /**
     * Run this in parallel (build=_3) to create the archive for TestLoadAsSequentialWithBath.
     *
scons build=GccOpt_hostconfig,boost=1-33-1_3 test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp  /tmp/$USER/testoutput/TestCreateArchiveForLoadAsSequentialWithBath/?* ./heart/test/data/checkpoint_migration_with_bath/
cp  /tmp/$USER/testoutput/TestCreateArchiveForLoadAsSequentialWithBathAndDistributedMesh/?* ./heart/test/data/checkpoint_migration_with_bath_and_distributed_mesh/
     *
     * Sets up a simulation and archives it without solving at all.
     *
     * When running sequentially, this creates an archive we can compare with
     * that produced by the next test.
     */
    void TestCreateArchiveForLoadAsSequentialWithBath() throw (Exception)
    {
        HeartConfig::Instance()->Reset();
        HeartConfig::Instance()->SetSimulationDuration(0.2);
        HeartConfig::Instance()->SetOutputFilenamePrefix("simulation");
        HeartConfig::Instance()->SetUseAbsoluteTolerance(ABS_TOL);
        
        // boundary flux for Phi_e. -10e3 is under threshold, -14e3 crashes the cell model
        HeartConfig::Instance()->SetOdeTimeStep(0.001);  // ms
        double boundary_flux = -11.0e3;
        double duration = 1.9; // of the stimulus, in ms
	        
        {
		    std::string directory = "TestCreateArchiveForLoadAsSequentialWithBath";
		    HeartConfig::Instance()->SetOutputDirectory(directory);
        
	        TetrahedralMesh<2,2>* p_mesh = Load2dMeshAndSetCircularTissue<TetrahedralMesh<2,2> >(
	            "mesh/test/data/2D_0_to_1mm_400_elements", 0.05, 0.05, 0.02);
	        ZeroStimulusCellFactory<LuoRudyIModel1991OdeSystem, 2> cell_factory;
	        
	        boost::shared_ptr<Electrodes<2> > p_electrodes(
	            new Electrodes<2>(*p_mesh, false/*don't ground*/, 0/*x*/, 0.0/*x=0*/, 0.1/*x=1*/,
	                              boundary_flux, 0.0, duration));
	
	        BidomainProblem<2> bidomain_problem( &cell_factory, true );
	        bidomain_problem.SetElectrodes(p_electrodes);
	        bidomain_problem.SetMesh(p_mesh);
	        bidomain_problem.Initialise();
	
	        CardiacSimulationArchiver<BidomainProblem<2> >::Save(bidomain_problem, directory, false);
	
	        delete p_mesh;
        }
        
        // And now with a distributed mesh, for coverage
        {
		    std::string directory = "TestCreateArchiveForLoadAsSequentialWithBathAndDistributedMesh";
		    HeartConfig::Instance()->SetOutputDirectory(directory);
		    
	        DistributedTetrahedralMesh<2,2>* p_mesh = Load2dMeshAndSetCircularTissue<DistributedTetrahedralMesh<2,2> >(
	            "mesh/test/data/2D_0_to_1mm_400_elements", 0.05, 0.05, 0.02);
	        ZeroStimulusCellFactory<LuoRudyIModel1991OdeSystem, 2> cell_factory;
	
	        boost::shared_ptr<Electrodes<2> > p_electrodes(
	            new Electrodes<2>(*p_mesh, false/*don't ground*/, 0/*x*/, 0.0/*x=0*/, 0.1/*x=1*/,
	                              boundary_flux, 0.0, duration));
	
	        BidomainProblem<2> bidomain_problem( &cell_factory, true );
	        bidomain_problem.SetElectrodes(p_electrodes);
	        bidomain_problem.SetMesh(p_mesh);
	        bidomain_problem.Initialise();
	
	        CardiacSimulationArchiver<BidomainProblem<2> >::Save(bidomain_problem, directory, false);
	
	        delete p_mesh;
        }
    }

    void TestLoadAsSequentialWithBath() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration_with_bath/";
        std::string archive_directory = "TestLoadAsSequentialWithBath";
        std::string ref_archive_dir = "TestCreateArchiveForLoadAsSequentialWithBath";
        OutputFileHandler handler(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestLoadAsSequentialWithBath");

        BidomainProblem<2>* p_problem;
        // Do the migration to sequential
        const unsigned num_cells = 221u;
        p_problem = DoMigrateAndBasicTests<BidomainProblem<2>,2>(archive_directory, ref_archive_dir, source_directory, num_cells, false);

        // All cells should have no stimulus.
        DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
        for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
        {
            AbstractCardiacCell* p_cell = p_problem->GetPde()->GetCardiacCell(i);
            AbstractStimulusFunction* p_stim = p_cell->GetStimulusFunction().get();
            ZeroStimulus* p_zero_stim = dynamic_cast<ZeroStimulus*>(p_stim);
            TS_ASSERT(p_zero_stim != NULL);
            TS_ASSERT_DELTA(p_cell->GetStimulus(0.0), 0.0, 1e-10);
            TS_ASSERT_DELTA(p_cell->GetStimulus(1.0), 0.0, 1e-10);
        }

        // Test bccs
        TS_ASSERT( ! p_problem->mpDefaultBoundaryConditionsContainer);
        TS_ASSERT( ! p_problem->mpBoundaryConditionsContainer);
        // Problem's BCC will be created first time BidomainProblem::Solve() is called,
        // so check the Electrodes' BCC directly
        boost::shared_ptr<BoundaryConditionsContainer<2,2,2> > p_bcc = p_problem->mpElectrodes->GetBoundaryConditionsContainer();
        // We have neumann boundary conditions from the electrodes
        TS_ASSERT(p_bcc->AnyNonZeroNeumannConditions());
        for (BoundaryConditionsContainer<2,2,2>::NeumannMapIterator it = p_bcc->BeginNeumann();
             it != p_bcc->EndNeumann();
             ++it)
        {
            ChastePoint<2> centroid(it->first->CalculateCentroid());
            // Negative flux at x=0
            if (fabs(centroid[0] - 0.0) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(it->first, centroid, 1), -11e3, 1e-10);
            }
            // Positive flux at x=0.1
            else if (fabs(centroid[0] - 0.1) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(it->first, centroid, 1), +11e3, 1e-10);
            }
            else
            {
                TS_FAIL("Unexpected Neumann BC found.");
            }
        }
        // There are no dirichlet BCs.
        TS_ASSERT( ! p_bcc->HasDirichletBoundaryConditions());
        // Now check that all relevant boundary elements have neumann conditions
        for (AbstractTetrahedralMesh<2,2>::BoundaryElementIterator iter = p_problem->rGetMesh().GetBoundaryElementIteratorBegin();
             iter != p_problem->rGetMesh().GetBoundaryElementIteratorEnd();
             iter++)
        {
            ChastePoint<2> centroid((*iter)->CalculateCentroid());
            if (fabs(centroid[0] - 0.0) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,1), -11e3, 1e-10);
            }
            else if (fabs(centroid[0] - 0.1) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,1), +11e3, 1e-10);
            }
            else
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,1), 0, 1e-10);
            }
            // No neumann stimulus applied to V
            TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,0), 0, 1e-10);
        }

        DoSimulationsAfterMigrationAndCompareResults(p_problem, archive_directory, ref_archive_dir, 2);
        
        // And now a shorter test with a distributed mesh, for coverage
        source_directory = "heart/test/data/checkpoint_migration_with_bath_and_distributed_mesh/";
        archive_directory = "TestLoadAsSequentialWithBathAndDistributedMesh";
        ref_archive_dir = "TestCreateArchiveForLoadAsSequentialWithBathAndDistributedMesh";
        OutputFileHandler handler2(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler2.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestLoadAsSequentialWithBathAndDistributedMesh");
        
        BidomainProblem<2>* p_problem2;
        // Do the migration to sequential
        p_problem2 = DoMigrateAndBasicTests<BidomainProblem<2>,2>(archive_directory, ref_archive_dir, source_directory, num_cells, true);
        delete p_problem2;
    }

private:
    template<class Problem, unsigned DIM>
    Problem* DoMigrateFromSequentialAndBasicTests(
            const std::string& rArchiveDirectory,
            const std::string& rRefArchiveDir,
            const unsigned totalNumCells,
            bool isDistributedMesh,
            bool isConstructedMesh,
            double currentTime=0.0)
    {
        // Do the migration
        Problem* p_problem = CardiacSimulationArchiver<Problem>::Migrate(rArchiveDirectory);

        // Some basic tests that we have the right data
        TS_ASSERT_EQUALS(p_problem->mMeshFilename, "");
        TS_ASSERT_EQUALS(p_problem->mPrintOutput, true);
        TS_ASSERT_EQUALS(p_problem->mNodesToOutput.size(), 0u);
        TS_ASSERT_EQUALS(p_problem->mCurrentTime, currentTime);
        DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
        TS_ASSERT(p_factory->GetOriginalFactory());
        TS_ASSERT_EQUALS(p_factory->GetOriginalFactory()->GetProblemSize(), totalNumCells);
        TS_ASSERT_EQUALS(p_factory->GetOriginalFactory()->GetNumProcs(), 1u);
        TS_ASSERT_EQUALS(p_problem->GetPde()->rGetCellsDistributed().size(), p_factory->GetLocalOwnership());
        TS_ASSERT_EQUALS(p_problem->rGetMesh().GetNumAllNodes(), totalNumCells);
        TS_ASSERT_EQUALS(p_problem->rGetMesh().GetNumNodes(), totalNumCells);
        TS_ASSERT_EQUALS(&(p_problem->rGetMesh()), p_problem->GetPde()->pGetMesh());
        // Check the mesh is/isn't the parallel variety
        const DistributedTetrahedralMesh<DIM,DIM>* p_dist_mesh = dynamic_cast<const DistributedTetrahedralMesh<DIM,DIM>*>(p_problem->GetPde()->pGetMesh());
        if (isDistributedMesh)
        {
            TS_ASSERT(p_dist_mesh != NULL);
        }
        else
        {
            TS_ASSERT(p_dist_mesh == NULL);
        }

        if (currentTime == 0.0)
        {
            // All cells should be at initial conditions.
            if (p_factory->GetHigh() > p_factory->GetLow())
            {
                std::vector<double> inits;
                for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
                {
                    AbstractCardiacCell* p_cell = p_problem->GetPde()->GetCardiacCell(i);
                    FakeBathCell* p_fake_cell = dynamic_cast<FakeBathCell*>(p_cell);
                    if (p_fake_cell == NULL)
                    {
                        if (inits.empty())
                        {
                            inits = p_cell->GetInitialConditions();
                        }
                        std::vector<double>& r_state = p_cell->rGetStateVariables();
                        TS_ASSERT_EQUALS(r_state.size(), inits.size());
                        for (unsigned j=0; j<r_state.size(); j++)
                        {
                            TS_ASSERT_DELTA(r_state[j], inits[j], 1e-10);
                        }
                    }
                }
            }
        }

        // Save it to a normal archive
        CardiacSimulationArchiver<Problem>::Save(*p_problem, rArchiveDirectory + "/new_archive");

        // Compare with the archive from the previous test
        OutputFileHandler handler(rArchiveDirectory, false);
        std::string ref_archive = handler.GetChasteTestOutputDirectory() + rRefArchiveDir + "/archive.arch";
        std::string my_archive = handler.GetOutputDirectoryFullPath() + "new_archive/archive.arch";
        ///\todo When #1199 is done, re-instate this if
//        if (PetscTools::GetNumProcs() > 1)
//        {
            // If we're loading on 1 process, then the archives will differ in one digit:
            // DistributedTetrahedralMesh::mMetisPartitioning !
            EXPECT0(system, "diff " + ref_archive + " " + my_archive);
//        }
        if (!isConstructedMesh || PetscTools::GetNumProcs() < 4)
        {
            // This works because the original archive was created by a single process,
            // but only if the mesh was read from disk: a constructed mesh doesn't use
            // a DUMB partition, so the archives differ if the number of processes doesn't
            // divide the number of rows in the mesh.
            std::stringstream proc_id;
            proc_id << PetscTools::GetMyRank();
            std::string suffix = "." + proc_id.str();
            // We can't do a straight diff:
            //EXPECT0(system, "diff -I 'serialization::archive' " + ref_archive + suffix + " " + my_archive + suffix);
            // because we may have done a little simulation already, so there may be differences
            // between serial and parallel results, below the numerical solver tolerance.

            std::string file1 = ref_archive + suffix;
            std::string file2 = my_archive + suffix;
            NumericFileComparison same_data(file1, file2);
            TS_ASSERT(same_data.CompareFiles(ABS_TOL, 1)); //Tolerance of comparison.  Ignore first line (first 1 lines).
        }

        return p_problem;
    }

public:
    /**
     * Run this in sequential to create the archive for TestLoadFromSequential.
scons build=GccOpt_hostconfig,boost=1-33-1 test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp /tmp/$USER/testoutput/TestCreateArchiveForLoadFromSequential/?* ./heart/test/data/checkpoint_migration_from_seq/
     *
     * Sets up a simulation and archives it without solving at all.
     *
     * When running in parallel, this creates an archive we can compare with
     * that produced by the next test.
     *
     * Generates a 3d cube mesh with 125 nodes, corners at (0,0,0) and (1,1,1)
     * with nodal spacing of 0.25cm.
     */
    void TestCreateArchiveForLoadFromSequential() throw (Exception)
    {
        std::string directory = "TestCreateArchiveForLoadFromSequential";
        HeartConfig::Instance()->Reset();
        HeartConfig::Instance()->SetSlabDimensions(1, 1, 1, 0.2);
        HeartConfig::Instance()->SetSimulationDuration(0.2);
        HeartConfig::Instance()->SetOutputDirectory(directory);
        HeartConfig::Instance()->SetOutputFilenamePrefix("simulation");
        HeartConfig::Instance()->SetUseAbsoluteTolerance(ABS_TOL);

        PlaneStimulusCellFactory<FaberRudy2000Version3, 3> cell_factory(-25500.0, 2.0);
        MonodomainProblem<3> monodomain_problem( &cell_factory );

        monodomain_problem.Initialise();

        CardiacSimulationArchiver<MonodomainProblem<3> >::Save(monodomain_problem, directory);
    }

    /**
     * #1159 - the second part of migrating a checkpoint to a different number of processes.
     */
    void TestLoadFromSequential() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration_from_seq/";
        std::string archive_directory = "TestLoadFromSequential";
        std::string ref_archive_dir = "TestCreateArchiveForLoadFromSequential";
        OutputFileHandler handler(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestLoadFromSequential");

        // Loading from a sequential archive should work just as well running sequentially as in parallel -
        // if running sequentially it's essentially just the same as a normal load.
        const unsigned num_cells = 216u;
        MonodomainProblem<3>* p_problem = DoMigrateFromSequentialAndBasicTests<MonodomainProblem<3>,3>(archive_directory, ref_archive_dir, num_cells, true, true);

        // All cells at x=0 should have a SimpleStimulus(-25500, 2).
        DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
        for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
        {
            AbstractCardiacCell* p_cell = p_problem->GetPde()->GetCardiacCell(i);
            double x = p_problem->rGetMesh().GetNode(i)->GetPoint()[0];

            if (x*x < 1e-10)
            {
                // Stim exists
                TS_ASSERT_DELTA(p_cell->GetStimulus(0.0), -25500.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(2.0), -25500.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(2.001), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(-1e-10), 0.0, 1e-10);
            }
            else
            {
                // No stim
                TS_ASSERT_DELTA(p_cell->GetStimulus(0.0), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(2.0), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(2.001), 0.0, 1e-10);
                TS_ASSERT_DELTA(p_cell->GetStimulus(-1e-10), 0.0, 1e-10);
            }
        }

        // Test bccs - none defined in this problem
        TS_ASSERT(! p_problem->mpDefaultBoundaryConditionsContainer);
        TS_ASSERT(! p_problem->mpBoundaryConditionsContainer);

        DoSimulationsAfterMigrationAndCompareResults(p_problem, archive_directory, ref_archive_dir, 1);
    }

    /**
     * Run this in sequential to create the archive for TestLoadFromSequentialWithBath.
scons build=GccOpt_hostconfig,boost=1-33-1  test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp /tmp/$USER/testoutput/TestCreateArchiveForLoadFromSequentialWithBath/?* ./heart/test/data/checkpoint_migration_from_seq_with_bath/
     *
     * Sets up a simulation and archives it, solving for one PDE step first to set up default BCs.
     *
     * When running in parallel, this creates an archive we can compare directly with
     * that produced by the next test.
     */
    void TestCreateArchiveForLoadFromSequentialWithBath() throw (Exception)
    {
        std::string directory = "TestCreateArchiveForLoadFromSequentialWithBath";
        HeartConfig::Instance()->Reset();
        HeartConfig::Instance()->SetSimulationDuration(0.2);
        HeartConfig::Instance()->SetOutputDirectory(directory);
        HeartConfig::Instance()->SetOutputFilenamePrefix("simulation");
        HeartConfig::Instance()->SetUseAbsoluteTolerance(ABS_TOL);

        DistributedTetrahedralMesh<2,2>* p_mesh = Load2dMeshAndSetCircularTissue<DistributedTetrahedralMesh<2,2> >(
            "mesh/test/data/2D_0_to_1mm_400_elements", 0.05, 0.05, 0.02);
        ZeroStimulusCellFactory<LuoRudyIModel1991OdeSystem, 2> cell_factory;

        // boundary flux for Phi_e. -10e3 is under threshold, -14e3 crashes the cell model
        HeartConfig::Instance()->SetOdeTimeStep(0.001);  // ms
        double boundary_flux = -11.0e3;
        double duration = 1.9; // of the stimulus, in ms
        boost::shared_ptr<Electrodes<2> > p_electrodes(
            new Electrodes<2>(*p_mesh, true/*do ground*/, 0/*x*/, 0.0/*x=0*/, 0.1/*x=1*/,
                              boundary_flux, 0.0, duration));

        BidomainProblem<2> bidomain_problem( &cell_factory, true );
        bidomain_problem.SetElectrodes(p_electrodes);
        bidomain_problem.SetMesh(p_mesh);

        // We solve for a small period of time so BCCs are created (next test wants to check it)
        HeartConfig::Instance()->SetSimulationDuration(0.1);
        bidomain_problem.Initialise();
        bidomain_problem.Solve();

        // We increase the simulation time so next test finds something left to simulate in the archive
        HeartConfig::Instance()->SetSimulationDuration(0.2);

        CardiacSimulationArchiver<BidomainProblem<2> >::Save(bidomain_problem, directory, false);
        
        delete p_mesh;
    }

    void TestLoadFromSequentialWithBath() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration_from_seq_with_bath/";
        std::string archive_directory = "TestLoadFromSequentialWithBath";
        std::string ref_archive_dir = "TestCreateArchiveForLoadFromSequentialWithBath";
        OutputFileHandler handler(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestLoadFromSequentialWithBath");

        // Loading from a sequential archive should work just as well running sequentially as in parallel -
        // if running sequentially it's essentially just the same as a normal load.
        const unsigned num_cells = 221u;
        BidomainProblem<2>* p_problem = DoMigrateFromSequentialAndBasicTests<BidomainProblem<2>,2>(archive_directory, ref_archive_dir, num_cells, true, false, 0.1);

        // All cells should have a ZeroStimulus.
        DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
        for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
        {
            AbstractCardiacCell* p_cell = p_problem->GetPde()->GetCardiacCell(i);
            AbstractStimulusFunction* p_stim = p_cell->GetStimulusFunction().get();
            ZeroStimulus* p_zero_stim = dynamic_cast<ZeroStimulus*>(p_stim);
            TS_ASSERT(p_zero_stim != NULL);
            TS_ASSERT_DELTA(p_cell->GetStimulus(0.0), 0.0, 1e-10);
            TS_ASSERT_DELTA(p_cell->GetStimulus(1.0), 0.0, 1e-10);
        }

        // Test bccs
        TS_ASSERT(p_problem->mpDefaultBoundaryConditionsContainer); /// \todo: see todo in BidomainProblem.cpp:344
        TS_ASSERT(p_problem->mpBoundaryConditionsContainer);
        boost::shared_ptr<BoundaryConditionsContainer<2,2,2> > p_bcc = p_problem->mpBoundaryConditionsContainer;
        // We have neumann and dirichlet boundary conditions from the electrodes (at least on some processes)
        if (PetscTools::IsSequential())
        {
            TS_ASSERT(p_bcc->AnyNonZeroNeumannConditions());
        }
        TS_ASSERT(p_bcc->HasDirichletBoundaryConditions());
        for (BoundaryConditionsContainer<2,2,2>::NeumannMapIterator it = p_bcc->BeginNeumann();
             it != p_bcc->EndNeumann();
             ++it)
        {
            ChastePoint<2> centroid(it->first->CalculateCentroid());
            // Negative flux at x=0
            if (fabs(centroid[0] - 0.0) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(it->first, centroid, 1), -11e3, 1e-10);
            }
            else
            {
                TS_FAIL("Unexpected Neumann BC found.");
            }
        }
        // Now check that all relevant boundary elements have neumann conditions
        for (DistributedTetrahedralMesh<2,2>::BoundaryElementIterator iter = p_problem->rGetMesh().GetBoundaryElementIteratorBegin();
             iter != p_problem->rGetMesh().GetBoundaryElementIteratorEnd();
             iter++)
        {
            ChastePoint<2> centroid((*iter)->CalculateCentroid());
            if (fabs(centroid[0] - 0.0) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,1), -11e3, 1e-10);
            }
            else
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,1), 0, 1e-10);
            }
            // No neumann stimulus applied to V
            TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*iter,centroid,0), 0, 1e-10);
        }
        // Check dirichlet conditions exist only at x=0.1
        for (AbstractTetrahedralMesh<2,2>::NodeIterator iter=p_problem->rGetMesh().GetNodeIteratorBegin();
             iter != p_problem->rGetMesh().GetNodeIteratorEnd();
             ++iter)
        {
            Node<2>* p_node = &(*iter);
            double x = p_node->rGetLocation()[0];
            if (fabs(x - 0.1) < 1e-6)
            {
                TS_ASSERT(p_bcc->HasDirichletBoundaryCondition(p_node, 1));
                TS_ASSERT_DELTA(p_bcc->GetDirichletBCValue(p_node, 1), 0, 1e-10); // grounded
            }
            else
            {
                TS_ASSERT(!p_bcc->HasDirichletBoundaryCondition(p_node, 1));
            }
            // No dirichlet condition on V
            TS_ASSERT(!p_bcc->HasDirichletBoundaryCondition(p_node, 0));
        }

        DoSimulationsAfterMigrationAndCompareResults(p_problem, archive_directory, ref_archive_dir, 2, 0.2);
    }

    /**
     * The next pair of tests cover the case where the master process (that creates the archive)
     * has no boundary conditions, but at least one other process does.
     * We set a zero dirichlet boundary condition on the right end of a parallel 1d mesh.
     *
scons build=GccOpt_hostconfig,boost=1-33-1_2  test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp /tmp/$USER/testoutput/TestCreateArchiveForBcsOnNonMasterOnly/?* ./heart/test/data/checkpoint_migration_bcs_on_non_master_only/
     *
     * Sets up a simulation and archives it without solving at all.
     *
     * When running sequentially, this creates an archive we can compare with
     * that produced by the next test.
     */
    void TestCreateArchiveForBcsOnNonMasterOnly() throw (Exception)
    {
        std::string directory = "TestCreateArchiveForBcsOnNonMasterOnly";
        HeartConfig::Instance()->Reset();
        HeartConfig::Instance()->SetSimulationDuration(0.1); //ms
        HeartConfig::Instance()->SetOutputDirectory(directory);
        HeartConfig::Instance()->SetOutputFilenamePrefix("simulation");
        HeartConfig::Instance()->SetUseAbsoluteTolerance(ABS_TOL);

        TrianglesMeshReader<1,1> reader("mesh/test/data/1D_0_to_1_10_elements");
        DistributedTetrahedralMesh<1,1> mesh;
        mesh.ConstructFromMeshReader(reader);
        // Set the whole mesh to be bath
        DistributedVectorFactory* p_factory = mesh.GetDistributedVectorFactory();
        for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
        {
            mesh.GetNode(i)->SetRegion(HeartRegionCode::BATH);
        }
        for (DistributedTetrahedralMesh<1,1>::ElementIterator it = mesh.GetElementIteratorBegin();
             it != mesh.GetElementIteratorEnd();
             ++it)
        {
            it->SetRegion(HeartRegionCode::BATH);
        }
        mesh.SetMeshHasChangedSinceLoading();

        ZeroStimulusCellFactory<LuoRudyIModel1991OdeSystem, 1> cell_factory;

        BidomainProblem<1> bidomain_problem( &cell_factory, true );
        bidomain_problem.SetMesh(&mesh);

        // Boundary condition - fix phi=0 on RHS
        unsigned rhs_index = 10u;
        if (p_factory->IsGlobalIndexLocal(rhs_index))
        {
            boost::shared_ptr<BoundaryConditionsContainer<1,1,2> > p_bcc(new BoundaryConditionsContainer<1,1,2>);
            ConstBoundaryCondition<1>* p_zero_stim = new ConstBoundaryCondition<1>(0.0);
            Node<1>* p_node = mesh.GetNode(rhs_index);
            p_bcc->AddDirichletBoundaryCondition(p_node, p_zero_stim, 1u);
            bidomain_problem.SetBoundaryConditionsContainer(p_bcc);
        }

        bidomain_problem.Initialise();

        // And Save
        CardiacSimulationArchiver<BidomainProblem<1> >::Save(bidomain_problem, directory);
    }

    void TestBcsOnNonMasterOnly() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration_bcs_on_non_master_only/";
        std::string archive_directory = "TestBcsOnNonMasterOnly";
        std::string ref_archive_dir = "TestCreateArchiveForBcsOnNonMasterOnly";
        OutputFileHandler handler(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestBcsOnNonMasterOnly");

        const unsigned num_nodes = 11u;
        BidomainProblem<1>* p_problem = DoMigrateAndBasicTests<BidomainProblem<1>,1>(archive_directory, ref_archive_dir, source_directory, num_nodes, true);

        // Test the bccs - zero dirichlet on RHS only
        TS_ASSERT(! p_problem->mpDefaultBoundaryConditionsContainer);
        boost::shared_ptr<BoundaryConditionsContainer<1,1,2> > p_bcc = p_problem->mpBoundaryConditionsContainer;
        // If running on 2 procs we now don't migrate, so process 0 doesn't have a BCC.
        if (PetscTools::GetNumProcs() != 2)
        {
            TS_ASSERT(p_bcc);
            TS_ASSERT(! p_bcc->AnyNonZeroNeumannConditions());
            TS_ASSERT( p_bcc->HasDirichletBoundaryConditions());
            DistributedVectorFactory* p_factory = p_problem->rGetMesh().GetDistributedVectorFactory();
            for (unsigned i=p_factory->GetLow(); i<p_factory->GetHigh(); i++)
            {
                Node<1>* p_node = p_problem->rGetMesh().GetNode(i);
                TS_ASSERT(! p_bcc->HasDirichletBoundaryCondition(p_node, 0u));
                if (i == num_nodes-1)
                {
                    TS_ASSERT(p_bcc->HasDirichletBoundaryCondition(p_node, 1u));
                    TS_ASSERT_EQUALS(p_bcc->GetDirichletBCValue(p_node, 1u), 0u);
                }
                else
                {
                    TS_ASSERT(! p_bcc->HasDirichletBoundaryCondition(p_node, 1u));
                }
            }
        }

        DoSimulationsAfterMigrationAndCompareResults(p_problem, archive_directory, ref_archive_dir, 2);
    }

    /**
     * The next pair of tests covers the case where we migrate after a solve.
     * This means we will have default boundary conditions included in the archive.
     * Saving the simulation in parallel, and using a parallel mesh, means we also cover the
     * direct call of mpBoundaryConditionsContainer->MergeFromArchive in LoadExtraArchive.
     *
     * Run this in parallel (build=_2) to create the archive for TestMigrateAfterSolve.
scons build=GccOpt_hostconfig,boost=1-33-1_2 test_suite=heart/test/TestCardiacSimulationArchiver.hpp
cp /tmp/$USER/testoutput/TestCreateArchiveForMigrateAfterSolve/archive/?* ./heart/test/data/checkpoint_migration_after_solve/
     *
     * Sets up a simulation and archives it without solving at all.
     *
     * When running sequentially, this creates an archive we can compare with
     * that produced by the next test.
     */
    void TestCreateArchiveForMigrateAfterSolve() throw (Exception)
    {
        std::string directory = "TestCreateArchiveForMigrateAfterSolve";
        HeartConfig::Instance()->Reset();
        HeartConfig::Instance()->SetSimulationDuration(0.1); //ms
        HeartConfig::Instance()->SetOutputDirectory(directory);
        HeartConfig::Instance()->SetOutputFilenamePrefix("simulation");
        HeartConfig::Instance()->SetUseAbsoluteTolerance(ABS_TOL);

        TrianglesMeshReader<1,1> reader("mesh/test/data/1D_0_to_1_100_elements");
        DistributedTetrahedralMesh<1,1> mesh;
        mesh.ConstructFromMeshReader(reader);

        // Stimulate at LHS
        PlaneStimulusCellFactory<LuoRudyIModel1991OdeSystem, 1> cell_factory;

        MonodomainProblem<1> monodomain_problem( &cell_factory );
        monodomain_problem.SetMesh(&mesh);
        monodomain_problem.Initialise();

        // Solve...
        monodomain_problem.Solve();

        // ...and Save
        CardiacSimulationArchiver<MonodomainProblem<1> >::Save(monodomain_problem, directory + "/archive");

        // Copy the simulation.h5 file into the archive folder, for convenience
        OutputFileHandler handler(directory, false);
        if (PetscTools::AmMaster())
        {
            std::string path = handler.GetOutputDirectoryFullPath();
            EXPECT0(system, "cp " + path + "simulation.h5 " + path + "archive/");
        }
    }

    void TestMigrateAfterSolve() throw (Exception)
    {
        // We can only load simulations from CHASTE_TEST_OUTPUT, so copy the archives there
        std::string source_directory = "heart/test/data/checkpoint_migration_after_solve/";
        std::string archive_directory = "TestMigrateAfterSolve";
        std::string ref_archive_dir = "TestCreateArchiveForMigrateAfterSolve/archive";
        OutputFileHandler handler(archive_directory); // Clear the target directory
        if (PetscTools::AmMaster())
        {
            EXPECT0(system, "cp " + source_directory + "* " + handler.GetOutputDirectoryFullPath());
        }
        PetscTools::Barrier("TestMigrateAfterSolve");

        const unsigned num_nodes = 101u;
        MonodomainProblem<1>* p_problem = DoMigrateAndBasicTests<MonodomainProblem<1>,1>(archive_directory, ref_archive_dir, source_directory, num_nodes, true, 0.1);

        // Test the bccs - zero neumann on the boundary
        TS_ASSERT( p_problem->mpDefaultBoundaryConditionsContainer);
        boost::shared_ptr<BoundaryConditionsContainer<1,1,1> > p_bcc = p_problem->mpBoundaryConditionsContainer;
        TS_ASSERT(p_bcc);
        TS_ASSERT_EQUALS(p_bcc, p_problem->mpDefaultBoundaryConditionsContainer);
        TS_ASSERT(! p_bcc->AnyNonZeroNeumannConditions());
        TS_ASSERT(! p_bcc->HasDirichletBoundaryConditions());
        // Check all conditions are on the boundary
        for (BoundaryConditionsContainer<1,1,1>::NeumannMapIterator it = p_bcc->BeginNeumann();
             it != p_bcc->EndNeumann();
             ++it)
        {
            ChastePoint<1> centroid(it->first->CalculateCentroid());
            if (fabs(centroid[0] - 0.0) < 1e-6 || fabs(centroid[0] - 1.0) < 1e-6)
            {
                TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(it->first, centroid, 0), 0.0, 1e-10);
            }
            else
            {
                TS_FAIL("Unexpected Neumann BC found.");
            }
        }
        // Now check that all relevant boundary elements have neumann conditions
        for (DistributedTetrahedralMesh<1,1>::BoundaryElementIterator it = p_problem->rGetMesh().GetBoundaryElementIteratorBegin();
             it != p_problem->rGetMesh().GetBoundaryElementIteratorEnd();
             it++)
        {
            ChastePoint<1> centroid((*it)->CalculateCentroid());
            TS_ASSERT(p_bcc->HasNeumannBoundaryCondition(*it, 0));
            TS_ASSERT_DELTA(p_bcc->GetNeumannBCValue(*it, centroid, 0), 0, 1e-10);
        }

        DoSimulationsAfterMigrationAndCompareResults(p_problem, archive_directory, ref_archive_dir, 1, 0.2);
    }
};

#endif /*TESTCARDIACSIMULATIONARCHIVER_HPP_*/
