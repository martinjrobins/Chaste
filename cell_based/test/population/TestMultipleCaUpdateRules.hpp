/*

Copyright (c) 2005-2013, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifndef TESTMULTIPLLECAUPDATERULES_HPP_
#define TESTMULTIPLLECAUPDATERULES_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "AbstractMultipleCaUpdateRule.hpp"
#include "DiffusionMultipleCaUpdateRule.hpp"
#include "CellsGenerator.hpp"
#include "FixedDurationGenerationBasedCellCycleModel.hpp"
#include "MultipleCaBasedCellPopulation.hpp"
#include "AbstractCellBasedTestSuite.hpp"
#include "WildTypeCellMutationState.hpp"
#include "CellLabel.hpp"
#include "PottsMeshGenerator.hpp"
#include "NodesOnlyMesh.hpp"
#include "NodeBasedCellPopulation.hpp"
#include "SmartPointers.hpp"
#include "FileComparison.hpp"

#include "PetscSetupAndFinalize.hpp"

class TestMulitpleCaUpdateRules : public AbstractCellBasedTestSuite
{
public:

    void TestDiffusionMultipleCaUpdateRuleIn2d() throw (Exception)
    {
        // timestep and size of domain to let us calculate the probabilities of movement.
        double delta_t = 1;
        double delta_x = 1;
        double diffusion_parameter = 0.1;

        // Create an update law system
        DiffusionMultipleCaUpdateRule<2> diffusion_update_rule;

        // Test get/set methods
        TS_ASSERT_DELTA(diffusion_update_rule.GetDiffusionParameter(), 0.5, 1e-12);

        diffusion_update_rule.SetDiffusionParameter(1.0);

        TS_ASSERT_DELTA(diffusion_update_rule.GetDiffusionParameter(), 1.0, 1e-12);

        diffusion_update_rule.SetDiffusionParameter(diffusion_parameter);

        // Test EvaluateProbability()

        // Create a simple 2D PottsMesh
        PottsMeshGenerator<2> generator(5, 0, 0, 5, 0, 0);
        PottsMesh<2>* p_mesh = generator.GetMesh();

        // Create cells
        std::vector<CellPtr> cells;
        MAKE_PTR(DifferentiatedCellProliferativeType, p_diff_type);
        CellsGenerator<FixedDurationGenerationBasedCellCycleModel, 2> cells_generator;
        cells_generator.GenerateBasicRandom(cells, 1u, p_diff_type);

        // Specify where cells lie here we have one cell on the bottom left site
        std::vector<unsigned> location_indices;
        location_indices.push_back(0u);

        // Create cell population
        MultipleCaBasedCellPopulation<2u> cell_population(*p_mesh, cells, location_indices);

        // Note we just pass a pointer to the only cell as DiffusionUpdateRule is independent of cell type.
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(0,1,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(0,6,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(0,5,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);

        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,1,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,6,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,10,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);

        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(6,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(7,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(10,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(12,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(15,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(16,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(17,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);

        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(24,19,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(24,18,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(24,23,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
    }

    void TestDiffusionMultipleCaUpdateRuleIn2dWithMultipleCells() throw (Exception)
    {
        // timestep and size of domain to let us calculate the probabilities of movement.
        double delta_t = 1;
        double delta_x = 1;
        double diffusion_parameter = 0.1;

        // Create an update law system
        DiffusionMultipleCaUpdateRule<2> diffusion_update_rule;

        // Test get/set methods
        TS_ASSERT_DELTA(diffusion_update_rule.GetDiffusionParameter(), 0.5, 1e-12);

        diffusion_update_rule.SetDiffusionParameter(1.0);

        TS_ASSERT_DELTA(diffusion_update_rule.GetDiffusionParameter(), 1.0, 1e-12);

        diffusion_update_rule.SetDiffusionParameter(diffusion_parameter);

        // Test EvaluateProbability()

        // Create a simple 2D PottsMesh
        PottsMeshGenerator<2> generator(5, 0, 0, 5, 0, 0);
        PottsMesh<2>* p_mesh = generator.GetMesh();

        // Create cells
        std::vector<CellPtr> cells;
        MAKE_PTR(DifferentiatedCellProliferativeType, p_diff_type);
        CellsGenerator<FixedDurationGenerationBasedCellCycleModel, 2> cells_generator;
        cells_generator.GenerateBasicRandom(cells, 10u, p_diff_type);

        // Specify where cells lie here we have one cell on the bottom left site
        std::vector<unsigned> location_indices;

        // Two cells will be added in locations 0 and 1
        location_indices.push_back(0u);
        location_indices.push_back(0u);
        location_indices.push_back(1u);
        location_indices.push_back(1u);

        // Then other cells are added to the lattice to check if the probabilities are still the same
        for (unsigned i=4; i<10; i++)
        {
            location_indices.push_back(i);
        }

        // Create cell population
        MultipleCaBasedCellPopulation<2u> cell_population(*p_mesh, cells, location_indices, 2);

        // Note we just pass a pointer to the first cell as DiffusionUpdateRule is independent of cell type.
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(0,1,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(0,6,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(0,5,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);

        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,1,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,6,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,10,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(5,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);

        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(6,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(7,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(10,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(12,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(15,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(16,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(17,11,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);

        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(24,19,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(24,18,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/4.0,1e-6);
        TS_ASSERT_DELTA(diffusion_update_rule.EvaluateProbability(24,23,cell_population, delta_t, delta_x, cells[0]),diffusion_parameter*delta_t/delta_x/delta_x/2.0,1e-6);
    }

    void TestArchiveDiffusionMultipleCaUpdateRule() throw(Exception)
    {
        OutputFileHandler handler("archive", false);
        std::string archive_filename = handler.GetOutputDirectoryFullPath() + "DiffusionMultipleCaUpdateRule.arch";

        {
            DiffusionMultipleCaUpdateRule<2> update_rule;

            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            // Set member variables
            update_rule.SetDiffusionParameter(1.0);

            // Serialize via pointer to most abstract class possible
            AbstractMultipleCaUpdateRule<2>* const p_update_rule = &update_rule;
            output_arch << p_update_rule;
        }

        {
            AbstractMultipleCaUpdateRule<2>* p_update_rule;

            // Create an input archive
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);

            // Restore from the archive
            input_arch >> p_update_rule;

            // Test the member data
            TS_ASSERT_DELTA((static_cast<DiffusionMultipleCaUpdateRule<2>*>(p_update_rule))->GetDiffusionParameter(), 1.0, 1e-6);

            // Tidy up
            delete p_update_rule;
        }
    }

    void TestUpdateRuleOutputUpdateRuleInfo()
    {
        std::string output_directory = "TestMultipleCaUpdateRulesOutputParameters";
        OutputFileHandler output_file_handler(output_directory, false);

        // Test with VolumeConstraintPottsUpdateRule
        DiffusionMultipleCaUpdateRule<2> diffusion_update_rule;
        diffusion_update_rule.SetDiffusionParameter(1.0);

        TS_ASSERT_EQUALS(diffusion_update_rule.GetIdentifier(), "DiffusionMultipleCaUpdateRule-2");

        out_stream diffusion_update_rule_parameter_file = output_file_handler.OpenOutputFile("diffusion_update_rule_results.parameters");
        diffusion_update_rule.OutputUpdateRuleInfo(diffusion_update_rule_parameter_file);
        diffusion_update_rule_parameter_file->close();

        // Compare the generated file in test output with a reference copy in the source code.
        FileFinder generated = output_file_handler.FindFile("diffusion_update_rule_results.parameters");
        FileFinder reference("cell_based/test/data/TestMultipleCaUpdateRules/diffusion_update_rule_results.parameters",
                RelativeTo::ChasteSourceRoot);
        FileComparison comparer(generated, reference);
        TS_ASSERT(comparer.CompareFiles());
    }
};

#endif /*TESTPOTTSUPDATERULES_HPP_*/
