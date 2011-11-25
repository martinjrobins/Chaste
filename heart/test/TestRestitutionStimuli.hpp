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

#ifndef _TESTRESTITUTIONSTIMULI_HPP_
#define _TESTRESTITUTIONSTIMULI_HPP_

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <cxxtest/TestSuite.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <boost/shared_ptr.hpp>

#include "Exception.hpp"
#include "S1S2Stimulus.hpp"
#include "DynamicRestitutionStimulus.hpp"
#include "OutputFileHandler.hpp"

class TestRestitutionStimuli : public CxxTest::TestSuite
{
private:
    void DoesDynamicStimulusPerformCorrectly(DynamicRestitutionStimulus* pStimulus, double magnitude, double duration_of_stimulus,
                                                double startTime, std::vector<double> pacing_cycle_lengths, unsigned number_of_pulses)
    {
        double time = startTime;
        for (unsigned pacing_length_index = 0; pacing_length_index<pacing_cycle_lengths.size(); pacing_length_index++)
        {
            for (unsigned pulse_index=0; pulse_index<number_of_pulses; pulse_index++)
            {
                double pulse_time = time;
                TS_ASSERT_DELTA(pStimulus->GetStimulus(pulse_time-0.01),0.0, 1e-9);
                TS_ASSERT_DELTA(pStimulus->GetStimulus(pulse_time+0.01),magnitude, 1e-9);
                TS_ASSERT_DELTA(pStimulus->GetStimulus(pulse_time+duration_of_stimulus-0.01),magnitude, 1e-9);
                TS_ASSERT_DELTA(pStimulus->GetStimulus(pulse_time+duration_of_stimulus+0.01),0.0, 1e-9);

                time = time + pacing_cycle_lengths[pacing_length_index];
            }
        }
    }

    void DoesStimulusPerformCorrectly(S1S2Stimulus* pStimulus, double magnitude, double duration_of_stimulus,
                                       double duration_of_s1, double s1_period, double startTime, std::vector<double> s2_periods)
    {
        for (unsigned experiment_index=0 ; experiment_index<s2_periods.size() ; experiment_index++)
        {
            std::cout << "Experiment = " << experiment_index << "\n" << std::flush;
            pStimulus->SetS2ExperimentPeriodIndex(experiment_index);

            double time = 0.0;
            // First S1 Pulse
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),0.0, 1e-9);
            time = startTime + 0.1;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),magnitude, 1e-9);
            time = startTime + duration_of_stimulus + 0.1;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),0.0, 1e-9);

            // First S2 Pulse (or final S1 pulse for physiologists)
            time = duration_of_s1 + startTime + 0.1;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),magnitude, 1e-9);
            time = duration_of_s1 + startTime + duration_of_stimulus + 0.1;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),0.0, 1e-9);

            // Second S2 Pulse (or S2 pulse for physiologists!)
            time = duration_of_s1 + s2_periods[experiment_index] + startTime - 0.1;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),0.0, 1e-9);
            time = duration_of_s1 + s2_periods[experiment_index] + startTime + 0.1;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),magnitude, 1e-9);
            time = duration_of_s1 + s2_periods[experiment_index] + duration_of_stimulus + startTime - 0.1 ;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),magnitude, 1e-9);
            time = duration_of_s1 + 2*s2_periods[experiment_index]+ duration_of_stimulus + startTime + 0.1 ;
            TS_ASSERT_DELTA(pStimulus->GetStimulus(time),0.0, 1e-9);
        }
    }

public:

    void TestS1S2StimulusSetup(void) throw (Exception)
    {
        double magnitude = 50;
        double duration_of_stimulus = 5;
        double duration_of_s1 = 10000;
        double s1_period = 1000;
        double startTime = 50;
        std::vector<double> s2_periods;
        s2_periods.push_back(1000);
        s2_periods.push_back(900);
        s2_periods.push_back(800);
        s2_periods.push_back(700);

        S1S2Stimulus stimulus(magnitude, duration_of_stimulus, duration_of_s1, s1_period, startTime, s2_periods);

        DoesStimulusPerformCorrectly(&stimulus, magnitude, duration_of_stimulus,
                                      duration_of_s1, s1_period, startTime, s2_periods);

        TS_ASSERT_THROWS_THIS(stimulus.SetS2ExperimentPeriodIndex(s2_periods.size()),
                              "There are fewer S2 frequency values than the one you have requested.");

        TS_ASSERT_EQUALS(stimulus.GetNumS2FrequencyValues(),s2_periods.size());
    }

    void TestDynamicRestitutionCellStimulusSetup(void) throw (Exception)
    {
        double magnitude = 50;
        double duration_of_stimulus = 5;
        double startTime = 50;

        std::vector<double> pacing_cycle_lengths;
        pacing_cycle_lengths.push_back(1000);
        pacing_cycle_lengths.push_back(900);
        pacing_cycle_lengths.push_back(800);
        pacing_cycle_lengths.push_back(700);

        unsigned number_of_pulses = 10;

        DynamicRestitutionStimulus stimulus(magnitude, duration_of_stimulus, startTime, pacing_cycle_lengths, number_of_pulses);

        DoesDynamicStimulusPerformCorrectly(&stimulus, magnitude, duration_of_stimulus,
                                            startTime, pacing_cycle_lengths, number_of_pulses);

    }


    void TestArchivingS1S2Stimulus(void) throw(Exception)
    {
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetOutputDirectoryFullPath() + "s1s2_stimulus.arch";

        double magnitude = 50;
        double duration_of_stimulus = 5;
        double duration_of_s1 = 10000;
        double s1_period = 1000;
        double startTime = 50;
        std::vector<double> s2_periods;
        s2_periods.push_back(1000);
        s2_periods.push_back(900);
        s2_periods.push_back(800);
        s2_periods.push_back(700);

        // Create and archive stimulus
        {
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            AbstractStimulusFunction* const p_stimulus = new S1S2Stimulus(magnitude, duration_of_stimulus, duration_of_s1, s1_period, startTime, s2_periods);

            // Should always archive a pointer
            output_arch << p_stimulus;

            delete p_stimulus;
        }

        // Restore
        {
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);

            // Create a pointer
            AbstractStimulusFunction* p_loaded_stim;
            input_arch >> p_loaded_stim;

            DoesStimulusPerformCorrectly(static_cast<S1S2Stimulus*>(p_loaded_stim), magnitude, duration_of_stimulus,
                                                  duration_of_s1, s1_period, startTime, s2_periods);

            delete p_loaded_stim;
        }
    }

    void TestArchivingDynamicStimulus(void) throw(Exception)
    {
        OutputFileHandler handler("archive",false);
        std::string archive_filename;
        archive_filename = handler.GetOutputDirectoryFullPath() + "dynamic_stimulus.arch";

        double magnitude = 50;
        double duration_of_stimulus = 5;
        double startTime = 50;

        std::vector<double> pacing_cycle_lengths;
        pacing_cycle_lengths.push_back(1000);
        pacing_cycle_lengths.push_back(900);
        pacing_cycle_lengths.push_back(800);
        pacing_cycle_lengths.push_back(700);

        unsigned number_of_pulses = 10;


        // Create and archive stimulus
        {
            std::ofstream ofs(archive_filename.c_str());
            boost::archive::text_oarchive output_arch(ofs);

            AbstractStimulusFunction* const p_stimulus = new DynamicRestitutionStimulus(magnitude, duration_of_stimulus, startTime, pacing_cycle_lengths, number_of_pulses);

            // Should always archive a pointer
            output_arch << p_stimulus;

            delete p_stimulus;
        }

        // Restore
        {
            std::ifstream ifs(archive_filename.c_str(), std::ios::binary);
            boost::archive::text_iarchive input_arch(ifs);

            // Create a pointer
            AbstractStimulusFunction* p_loaded_stim;
            input_arch >> p_loaded_stim;

            DoesDynamicStimulusPerformCorrectly(static_cast<DynamicRestitutionStimulus*>(p_loaded_stim),
                                                magnitude, duration_of_stimulus, startTime,
                                                pacing_cycle_lengths, number_of_pulses);

            delete p_loaded_stim;
        }
    }


};


#endif //_TESTRESTITUTIONCELLSTIMULI_HPP_