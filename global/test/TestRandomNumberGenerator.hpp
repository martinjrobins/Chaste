/*

Copyright (c) 2005-2014, University of Oxford.
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

#ifndef TESTRANDOMNUMBERGENERATOR_HPP_
#define TESTRANDOMNUMBERGENERATOR_HPP_
#include <cxxtest/TestSuite.h>

#include "CheckpointArchiveTypes.hpp"

#include "OutputFileHandler.hpp"
#include "RandomNumberGenerator.hpp"

//This test is always run sequentially (never in parallel)
#include "FakePetscSetup.hpp"

#ifdef _MSC_VER
#define srandom srand
#define random rand
#endif

class TestRandomNumberGenerator : public CxxTest::TestSuite
{
public:

    double ran1;

    void LongerTestToFindPeriod()
    {
        srand(0);
        int first = rand();
        unsigned period_srand;
        for (period_srand=0; ; period_srand++)
        {
            if (rand() == first)
            {
                break;
            }
        }

        // Period of rand and random will be about 2^31-1 == INT_MAX ~= UINT_MAX/2
        TS_ASSERT_LESS_THAN_EQUALS(2147483647U /*2^31-1*/, period_srand);
        TS_ASSERT_LESS_THAN_EQUALS((unsigned)INT_MAX, period_srand);
        TS_ASSERT_LESS_THAN_EQUALS(period_srand, UINT_MAX);

        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        double double_first = p_gen->ranf();

        unsigned period_class;
        for (period_class=0; ; period_class++)
        {
            if (p_gen->ranf() == double_first)
            {
                break;
            }
        }
        RandomNumberGenerator::Destroy();

        // The main point of this class is to see whether the underlying generator has exactly the period of srand
        TS_ASSERT_DIFFERS(period_class, period_srand);
    }

    void TestRandomNumbersCreation()
    {
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        ran1 = p_gen->ranf();
        RandomNumberGenerator::Destroy();
    }

    void TestNewMethodSeed()
    {
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        double ran2 = p_gen->ranf();
        TS_ASSERT_DELTA(ran1, ran2, 1e-7);

        RandomNumberGenerator::Destroy();
    }

    void TestOtherRandomStuffDoesNotDestroyRandomSequence()
    {
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();

        // First reseed and get the first random number (as above)
        p_gen->Reseed(0);
        double ran2 = p_gen->ranf();
        TS_ASSERT_DELTA(ran1, ran2, 1e-7);

        // Now reseed, do something else random and then get "the first" random number
        p_gen->Reseed(0);
        std::vector<unsigned> some_vector(10);
        std::random_shuffle(some_vector.begin(), some_vector.end());
        double ran3 = p_gen->ranf();
        TS_ASSERT_DELTA(ran1, ran3, 1e-7);

        // Again - with rand()
        p_gen->Reseed(0);
        rand();
        double ran4 = p_gen->ranf();
        TS_ASSERT_DELTA(ran1, ran4, 1e-7);

        // Again - with random()
        p_gen->Reseed(0);
        // random(); // removed as this is not present on Windows.
        double ran5 = p_gen->ranf();
        TS_ASSERT_DELTA(ran1, ran5, 1e-7);

        // Again - with nothing
        p_gen->Reseed(0);
        double ran6 = p_gen->ranf();
        TS_ASSERT_DELTA(ran1, ran6, 1e-7);

        RandomNumberGenerator::Destroy();
    }

    void TestDifferentRandomSeed()
    {
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        p_gen->Reseed(0);
        double ran2 = p_gen->ranf();

        p_gen->Reseed(36);
        double ran3 = p_gen->ranf();
        TS_ASSERT_DIFFERS(ran2, ran3);

        p_gen->Reseed(36);
        double ran4 = p_gen->ranf();
        TS_ASSERT_DELTA(ran4, ran3, 1e-7);

        RandomNumberGenerator::Destroy();
    }

    void TestArchiveRandomNumberGenerator()
    {
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetOutputDirectoryFullPath() + "random_number.arch";

        std::vector<double> generated_numbers;

        // Create and archive random number generator
        {
            // Save random number generator
            RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
            p_gen->Reseed(7);

            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            // Generate some random numbers before archiving
            for (unsigned i=0; i<11; i++)
            {
                p_gen->ranf();
                p_gen->randMod(1 + 3*i);
                p_gen->StandardNormalRandomDeviate();
                p_gen->NormalRandomDeviate(0.5, 0.1);
            }

            SerializableSingleton<RandomNumberGenerator>* const p_wrapper = p_gen->GetSerializationWrapper();
            output_arch << p_wrapper;

            // Make sure saving it twice gets the same instance
            {
                SerializableSingleton<RandomNumberGenerator>* const p_wrapper_copy = p_gen->GetSerializationWrapper();
                output_arch << p_wrapper_copy;
            }

            // Generator saved here - record the next 11 uniform numbers
            for (unsigned i=0; i<11; i++)
            {
                double random = p_gen->ranf();
                generated_numbers.push_back(random);
            }

            // Record some numbers from the normal distribution too.
            for (unsigned i=0; i<11; i++)
            {
                double random = p_gen->NormalRandomDeviate(0.5, 0.1);
                generated_numbers.push_back(random);
            }

            RandomNumberGenerator::Destroy();
        }

        // Restore
        {
            RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
            p_gen->Reseed(25); // any old seed, different from that above
            for (unsigned i=0; i<7; i++) // generate some numbers
            {
                p_gen->ranf();
                p_gen->NormalRandomDeviate(0.5, 0.1);
            }

            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);

            SerializableSingleton<RandomNumberGenerator>* p_orig_wrapper = p_gen->GetSerializationWrapper();
            {
                SerializableSingleton<RandomNumberGenerator>* p_wrapper;
                input_arch >> p_wrapper;
                TS_ASSERT_DIFFERS(p_wrapper, p_gen->GetSerializationWrapper());
                TS_ASSERT_EQUALS(p_orig_wrapper, p_gen->GetSerializationWrapper());
                TS_ASSERT_EQUALS(p_gen, RandomNumberGenerator::Instance());
            }

            {
                SerializableSingleton<RandomNumberGenerator>* p_wrapper;
                input_arch >> p_wrapper;
                TS_ASSERT_DIFFERS(p_wrapper, p_gen->GetSerializationWrapper());
                TS_ASSERT_EQUALS(p_orig_wrapper, p_gen->GetSerializationWrapper());
                TS_ASSERT_EQUALS(p_gen, RandomNumberGenerator::Instance());
            }

            // Random Number generator restored; now check it generates the same numbers as the one we saved
            for (unsigned i=0; i<generated_numbers.size(); i++)
            {
                double random;
                if (i<11)
                {
                    random = p_gen->ranf();
                }
                else
                {
                    random = p_gen->NormalRandomDeviate(0.5, 0.1);
                }
                TS_ASSERT_DELTA(random, generated_numbers[i], 1e-12);
            }

            RandomNumberGenerator::Destroy();
        }
    }

    void TestPermutationShuffle() throw(Exception)
    {
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        p_gen->Reseed(0);

        std::vector<unsigned> shuffled_results;
        p_gen->Shuffle(5, shuffled_results);

        for (unsigned i=0; i<5; i++)
        {
            bool found = false;
            for (unsigned j=0; j<shuffled_results.size(); j++)
            {
                if (shuffled_results[j] == i)
                {
                    found = true;
                    break;
                }
            }
            TS_ASSERT_EQUALS(found, true);
        }

        unsigned num_trials = 1000000;
        unsigned results[5][5];
        for (unsigned i=0; i<5; i++)
        {
            for (unsigned j=0; j<5; j++)
            {
                results[i][j] = 0u;
            }
        }

        for (unsigned trial=0; trial<num_trials; trial++)
        {
            p_gen->Shuffle(5, shuffled_results);
            for (unsigned i=0; i<5; i++)
            {
                for (unsigned j=0; j<5; j++)
                {
                    if (shuffled_results[j] == i)
                    {
                        results[i][j]++;
                    }
                }
            }
        }

        for (unsigned i=0; i<5; i++)
        {
            for (unsigned j=0; j<5; j++)
            {
                // Probability of i going to position j
                double prob = (double)results[i][j]/num_trials;

                /*
                 * This test could fail with very low probability (just rerun).
                 *
                 * We accept 0.199 to 0.201 with a million trials.
                 */
                TS_ASSERT_DELTA(prob, 0.2, 1e-3);
            }
        }
    }

    void TestGenericShuffle()
    {
        const unsigned test_size = 10;
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();

        p_gen->Reseed(0);

        // Make an empty vector and put it through the unsigned version of shuffle
        std::vector<unsigned> empty_perm_for_shuffle;
        p_gen->Shuffle(test_size, empty_perm_for_shuffle);

        p_gen->Reseed(0);

        // Make an identity permutation vector and put it through the generic version of shuffle
        std::vector<boost::shared_ptr<unsigned> > identity_perm_for_shuffle(test_size);
        for (unsigned i=0; i<test_size; i++)
        {
            boost::shared_ptr<unsigned> p_i(new unsigned(i));
            identity_perm_for_shuffle[i] = p_i;
        }
        p_gen->Shuffle(identity_perm_for_shuffle);

        for (unsigned i=0; i<test_size; i++)
        {
            TS_ASSERT_EQUALS(empty_perm_for_shuffle[i], *identity_perm_for_shuffle[i]);
        }

        // Cover null case
        std::vector<boost::shared_ptr<unsigned> > just_empty;
        p_gen->Shuffle(just_empty);
    }

    void TestReproducibilityAcrossPlatforms()
    {
        // This test checks that the underlying RNG is the doing the same thing on your operating system
        RandomNumberGenerator::Destroy();
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        p_gen->Reseed(42);

        TS_ASSERT_DELTA(p_gen->ranf(), 0.3745, 1e-4);

        std::vector<unsigned> empty_perm_for_shuffle;
        p_gen->Shuffle(10, empty_perm_for_shuffle);

        TS_ASSERT_EQUALS(empty_perm_for_shuffle[0], 3u);
        TS_ASSERT_EQUALS(empty_perm_for_shuffle[5], 5u);

        TS_ASSERT_EQUALS(p_gen->randMod(999), 40u);
        TS_ASSERT_EQUALS(p_gen->randMod(999), 832u);
        TS_ASSERT_DELTA(p_gen->ranf(), 0.0580, 1e-4);

        TS_ASSERT_EQUALS(p_gen->randMod(6), 0u);
        TS_ASSERT_EQUALS(p_gen->randMod(6), 3u);
        TS_ASSERT_DELTA(p_gen->ranf(), 0.3337, 1e-4);

        for (unsigned i=0; i<1000; i++)
        {
            p_gen->StandardNormalRandomDeviate();
        }
        TS_ASSERT_DELTA(p_gen->StandardNormalRandomDeviate(), 0.9870, 1e-4);
        TS_ASSERT_DELTA(p_gen->NormalRandomDeviate(256.0, 0.5), 255.8389, 1e-4);
    }

    void TestGammaRandomDeviate()
    {
        RandomNumberGenerator::Destroy();
        RandomNumberGenerator* p_gen = RandomNumberGenerator::Instance();
        p_gen->Reseed(5);

        TS_ASSERT_DELTA(p_gen->GammaRandomDeviate(1.0, 1.0), 0.2510, 1e-4);
        TS_ASSERT_DELTA(p_gen->GammaRandomDeviate(2.0, 1.0), 1.3033, 1e-4);
        TS_ASSERT_DELTA(p_gen->GammaRandomDeviate(1.0, 2.0), 3.5595, 1e-4);
        TS_ASSERT_DELTA(p_gen->GammaRandomDeviate(3.5, 2.9), 12.6437, 1e-4);
    }
};

#endif /*TESTRANDOMNUMBERGENERATOR_HPP_*/
