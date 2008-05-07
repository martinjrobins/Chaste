/*

Copyright (C) University of Oxford, 2008

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
#ifndef TESTPARALLELWRITERPERFORMANCE_HPP_
#define TESTPARALLELWRITERPERFORMANCE_HPP_

#include <cxxtest/TestSuite.h>
#include "ParallelColumnDataWriter.hpp"
#include "DistributedVector.hpp"
#include <petsc.h>
#include <petscvec.h>
#include "PetscSetupAndFinalize.hpp"

class TestParallelWriterPerformance : public CxxTest::TestSuite
{
public:
    void Test1()
    {
        const unsigned SIZE=1000;
        const unsigned REPETITIONS=10;
        // create a distibuted vector
        DistributedVector::SetProblemSize(SIZE);
        Vec petsc_vec=DistributedVector::CreateVec();
        DistributedVector distributed_vector(petsc_vec);
        for (DistributedVector::Iterator index = DistributedVector::Begin();
             index!= DistributedVector::End();
             ++index)
        {
            distributed_vector[index] =  -(double)(index.Local*index.Global);
        }
        distributed_vector.Restore();
        
        // set up a parallel writer
        ParallelColumnDataWriter parallel_writer("TestParallelWriterPerformance","ParallelColumnWriter");
        unsigned time_var_id = parallel_writer.DefineUnlimitedDimension("Time","msecs");
        parallel_writer.DefineFixedDimension("Node","dimensionless", SIZE);
        unsigned var1_id = parallel_writer.DefineVariable("Var1","LightYears");
        parallel_writer.EndDefineMode();
        
        // write multiple times
        for (unsigned i=0; i<REPETITIONS; i++)
        {
            double time=(double)i;
            parallel_writer.PutVariable(time_var_id, time);
            parallel_writer.PutVector(var1_id, petsc_vec);
            parallel_writer.AdvanceAlongUnlimitedDimension();
        }
        
        VecDestroy(petsc_vec);
    }
};
#endif /*TESTPARALLELWRITERPERFORMANCE_HPP_*/
