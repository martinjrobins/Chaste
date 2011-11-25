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

#ifndef _TESTSIMPLELINEARPARABOLICSOLVERLONG_HPP_
#define _TESTSIMPLELINEARPARABOLICSOLVERLONG_HPP_

/*
 * TestSimpleLinearParabolicSolver.hpp
 *
 * Test suite for the SimpleLinearParabolicSolver class.
 *
 * Tests the class for the solution of parabolic pdes in 1D, 2D and 3D with and
 * without source terms with neumann and dirichlet booundary conditions.
 */

#include <cxxtest/TestSuite.h>
#include "TetrahedralMesh.hpp"
#include <petsc.h>
#include <vector>
#include <cmath>

#include <sys/stat.h> // for mkdir

#include "BoundaryConditionsContainer.hpp"
#include "ConstBoundaryCondition.hpp"
#include "SimpleLinearParabolicSolver.hpp"
#include "ParallelColumnDataWriter.hpp"
#include "TrianglesMeshReader.hpp"
#include "FemlabMeshReader.hpp"
#include "HeatEquation.hpp"
#include "HeatEquationWithSourceTerm.hpp"
#include "PetscTools.hpp"
#include "PetscSetupAndFinalize.hpp"

class TestSimpleLinearParabolicSolverLong : public CxxTest::TestSuite
{
public:
    // test 2D problem - takes a long time to run.
    // solution is incorrect to specified tolerance.
    void xTestSimpleLinearParabolicSolver2DNeumannWithSmallTimeStepAndFineMesh()
    {
        // Create mesh from mesh reader
        FemlabMeshReader<2,2> mesh_reader("mesh/test/data/",
                                          "femlab_fine_square_nodes.dat",
                                          "femlab_fine_square_elements.dat",
                                          "femlab_fine_square_edges.dat");

        TetrahedralMesh<2,2> mesh;
        mesh.ConstructFromMeshReader(mesh_reader);

        // Instantiate PDE object
        HeatEquation<2> pde;

        // Boundary conditions - zero dirichlet on boundary;
        BoundaryConditionsContainer<2,2,1> bcc;
        TetrahedralMesh<2,2>::BoundaryNodeIterator iter = mesh.GetBoundaryNodeIteratorBegin();

        while (iter != mesh.GetBoundaryNodeIteratorEnd())
        {
            double x = (*iter)->GetPoint()[0];
            double y = (*iter)->GetPoint()[1];

            ConstBoundaryCondition<2>* p_dirichlet_boundary_condition =
                new ConstBoundaryCondition<2>(x);

            if (fabs(y) < 0.01)
            {
                bcc.AddDirichletBoundaryCondition(*iter, p_dirichlet_boundary_condition);
            }

            if (fabs(y - 1.0) < 0.01)
            {
                bcc.AddDirichletBoundaryCondition(*iter, p_dirichlet_boundary_condition);
            }

            if (fabs(x) < 0.01)
            {
                bcc.AddDirichletBoundaryCondition(*iter, p_dirichlet_boundary_condition);
            }

            iter++;
        }

        TetrahedralMesh<2,2>::BoundaryElementIterator surf_iter = mesh.GetBoundaryElementIteratorBegin();
        ConstBoundaryCondition<2>* p_neumann_boundary_condition =
            new ConstBoundaryCondition<2>(1.0);

        while (surf_iter != mesh.GetBoundaryElementIteratorEnd())
        {
            int node = (*surf_iter)->GetNodeGlobalIndex(0);
            double x = mesh.GetNode(node)->GetPoint()[0];

            if (fabs(x - 1.0) < 0.01)
            {
                bcc.AddNeumannBoundaryCondition(*surf_iter, p_neumann_boundary_condition);
            }

            surf_iter++;
        }

        // Solver
        SimpleLinearParabolicSolver<2,2> solver(&mesh,&pde,&bcc);

        // Initial condition u(0,x,y) = sin(0.5*M_PI*x)*sin(M_PI*y)+x
        std::vector<double> init_cond(mesh.GetNumNodes());
        for (unsigned i=0; i<mesh.GetNumNodes(); i++)
        {
            double x = mesh.GetNode(i)->GetPoint()[0];
            double y = mesh.GetNode(i)->GetPoint()[1];
            init_cond[i] = sin(0.5*M_PI*x)*sin(M_PI*y)+x;
        }
        Vec initial_condition = PetscTools::CreateVec(init_cond);

        double t_end = 0.1;
        solver.SetTimes(0, t_end);
        solver.SetTimeStep(0.001);

        solver.SetInitialCondition(initial_condition);

        Vec result = solver.Solve();
        ReplicatableVector result_repl(result);

        // Check solution is u = e^{-5/4*M_PI*M_PI*t} sin(0.5*M_PI*x)*sin(M_PI*y)+x, t=0.1
        for (unsigned i=0; i<result_repl.GetSize(); i++)
        {
            double x = mesh.GetNode(i)->GetPoint()[0];
            double y = mesh.GetNode(i)->GetPoint()[1];
            double u = exp((-5/4)*M_PI*M_PI*t_end) * sin(0.5*M_PI*x) * sin(M_PI*y) + x;
            TS_ASSERT_DELTA(result_repl[i], u, 0.001);
        }

        VecDestroy(result);
        VecDestroy(initial_condition);
    }

    /**
     * Simple Parabolic PDE u' = del squared u
     *
     * With u = 0 on the boundaries of the unit cube. Subject to the initial
     * condition u(0,x,y,z)=sin( PI x)sin( PI y)sin( PI z).
     */
    void TestSimpleLinearParabolicSolver3DZeroDirich()
    {
        // read mesh on [0,1]x[0,1]x[0,1]
        TrianglesMeshReader<3,3> mesh_reader("mesh/test/data/cube_136_elements");
        TetrahedralMesh<3,3> mesh;
        mesh.ConstructFromMeshReader(mesh_reader);

        // Instantiate PDE object
        HeatEquation<3> pde;

        // Boundary conditions - zero dirichlet everywhere on boundary
        BoundaryConditionsContainer<3,3,1> bcc;
        bcc.DefineZeroDirichletOnMeshBoundary(&mesh);

        // Solver
        SimpleLinearParabolicSolver<3,3> solver(&mesh,&pde,&bcc);

        /*
         * Choose initial condition sin(x*pi)*sin(y*pi)*sin(z*pi) as
         * this is an eigenfunction of the heat equation.
         */
        std::vector<double> init_cond(mesh.GetNumNodes());
        for (unsigned i=0; i<mesh.GetNumNodes(); i++)
        {
            double x = mesh.GetNode(i)->GetPoint()[0];
            double y = mesh.GetNode(i)->GetPoint()[1];
            double z = mesh.GetNode(i)->GetPoint()[2];
            init_cond[i] = sin(x*M_PI)*sin(y*M_PI)*sin(z*M_PI);
        }
        Vec initial_condition = PetscTools::CreateVec(init_cond);

        double t_end = 0.1;
        solver.SetTimes(0, t_end);
        solver.SetTimeStep(0.001);

        solver.SetInitialCondition(initial_condition);

        Vec result = solver.Solve();
        ReplicatableVector result_repl(result);

        // Check solution is u = e^{-3*t*pi*pi} sin(x*pi)*sin(y*pi)*sin(z*pi), t=0.1
        for (unsigned i=0; i<result_repl.GetSize(); i++)
        {
            double x = mesh.GetNode(i)->GetPoint()[0];
            double y = mesh.GetNode(i)->GetPoint()[1];
            double z = mesh.GetNode(i)->GetPoint()[2];
            double u = exp(-3*t_end*M_PI*M_PI)*sin(x*M_PI)*sin(y*M_PI)*sin(z*M_PI);
            TS_ASSERT_DELTA(result_repl[i], u, 0.1);
        }

        VecDestroy(initial_condition);
        VecDestroy(result);
    }

    /**
     * Simple Parabolic PDE u' = del squared u + 1
     *
     * With u = -(1/6)(x^2+y^2+z^2) on the boundaries of the unit cube.
     *
     * Subject to the initial condition
     * u(0,x,y,z)=sin( PI x)sin( PI y)sin( PI z) - (1/6)(x^2+y^2+z^2)
     *
     */
    void TestSimpleLinearParabolicSolver3DZeroDirichWithSourceTerm()
    {
        // Create mesh from mesh reader
        TrianglesMeshReader<3,3> mesh_reader("mesh/test/data/cube_136_elements");
        TetrahedralMesh<3,3> mesh;
        mesh.ConstructFromMeshReader(mesh_reader);

        // Instantiate PDE object
        HeatEquationWithSourceTerm<3> pde;

        // Boundary conditions
        BoundaryConditionsContainer<3,3,1> bcc;
        TetrahedralMesh<3,3>::BoundaryNodeIterator iter = mesh.GetBoundaryNodeIteratorBegin();

        while (iter < mesh.GetBoundaryNodeIteratorEnd())
        {
            double x = (*iter)->GetPoint()[0];
            double y = (*iter)->GetPoint()[1];
            double z = (*iter)->GetPoint()[2];
            ConstBoundaryCondition<3>* p_dirichlet_boundary_condition =
                new ConstBoundaryCondition<3>(-1.0/6*(x*x+y*y+z*z));
            bcc.AddDirichletBoundaryCondition(*iter, p_dirichlet_boundary_condition);
            iter++;
        }

        // Solver
        SimpleLinearParabolicSolver<3,3> solver(&mesh,&pde,&bcc);

        // Initial condition u(0,x) = sin(x*pi)*sin(y*pi)*sin(z*pi)-1/6*(x^2+y^2+z^2)
        Vec initial_condition = PetscTools::CreateVec(mesh.GetNumNodes());

        double* p_initial_condition;
        VecGetArray(initial_condition, &p_initial_condition);

        int lo, hi;
        VecGetOwnershipRange(initial_condition, &lo, &hi);
        for (int global_index = lo; global_index < hi; global_index++)
        {
            int local_index = global_index - lo;
            double x = mesh.GetNode(global_index)->GetPoint()[0];
            double y = mesh.GetNode(global_index)->GetPoint()[1];
            double z = mesh.GetNode(global_index)->GetPoint()[2];
            p_initial_condition[local_index] = sin(x*M_PI)*sin(y*M_PI)*sin(z*M_PI)-1.0/6*(x*x+y*y+z*z);
        }
        VecRestoreArray(initial_condition, &p_initial_condition);

        double t_end = 0.1;
        solver.SetTimes(0, t_end);
        solver.SetTimeStep(0.01);

        solver.SetInitialCondition(initial_condition);
        Vec result = solver.Solve();

        // Check result
        double* p_result;
        VecGetArray(result, &p_result);

        // Solution should be u = e^{-t*2*pi*pi} sin(x*pi) sin(y*pi) sin(z*pi) - 1/6(x^2+y^2+z^2), t=0.1
        for (int global_index = lo; global_index < hi; global_index++)
        {
            int local_index = global_index - lo;
            double x = mesh.GetNode(global_index)->GetPoint()[0];
            double y = mesh.GetNode(global_index)->GetPoint()[1];
            double z = mesh.GetNode(global_index)->GetPoint()[2];
            double u = exp(-t_end*3*M_PI*M_PI)*sin(x*M_PI)*sin(y*M_PI)*sin(z*M_PI)-1.0/6*(x*x+y*y+z*z);
            TS_ASSERT_DELTA(p_result[local_index], u, 0.1);
        }
        VecRestoreArray(result, &p_result);
        VecDestroy(initial_condition);
        VecDestroy(result);
    }

    /**
     * Simple Parabolic PDE u' = del squared u
     *
     * With u = x on 5 boundaries of the unit cube, and
     * u_n = 1 on the x face of the cube.
     *
     * Subject to the initial condition
     * u(0,x,y,z)=sin( PI x)sin( PI y)sin( PI z) + x
     */
    void TestSimpleLinearParabolicSolver3DNeumannOnCoarseMesh()
    {
        // Create mesh from mesh reader
        TrianglesMeshReader<3,3> mesh_reader("mesh/test/data/cube_136_elements");

        TetrahedralMesh<3,3> mesh;
        mesh.ConstructFromMeshReader(mesh_reader);

        // Instantiate PDE object
        HeatEquation<3> pde;

        // Boundary conditions
        BoundaryConditionsContainer<3,3,1> bcc;
        TetrahedralMesh<3,3>::BoundaryNodeIterator iter = mesh.GetBoundaryNodeIteratorBegin();

        while (iter != mesh.GetBoundaryNodeIteratorEnd())
        {
            double x = (*iter)->GetPoint()[0];
            double y = (*iter)->GetPoint()[1];
            double z = (*iter)->GetPoint()[2];


            if ((fabs(y) < 0.01) || (fabs(y - 1.0) < 0.01) ||
                (fabs(x) < 0.01) ||
                (fabs(z) < 0.01) || (fabs(z - 1.0) < 0.01) )
            {
                ConstBoundaryCondition<3>* p_dirichlet_boundary_condition =
                    new ConstBoundaryCondition<3>(x);
                bcc.AddDirichletBoundaryCondition(*iter, p_dirichlet_boundary_condition);
            }

            iter++;
        }

        TetrahedralMesh<3,3>::BoundaryElementIterator surf_iter = mesh.GetBoundaryElementIteratorBegin();
        ConstBoundaryCondition<3>* p_neumann_boundary_condition =
            new ConstBoundaryCondition<3>(1.0);

        while (surf_iter != mesh.GetBoundaryElementIteratorEnd())
        {
            int node = (*surf_iter)->GetNodeGlobalIndex(0);
            double x = mesh.GetNode(node)->GetPoint()[0];

            if (fabs(x - 1.0) < 0.01)
            {
                bcc.AddNeumannBoundaryCondition(*surf_iter, p_neumann_boundary_condition);
            }

            surf_iter++;
        }

        // Solver
        SimpleLinearParabolicSolver<3,3> solver(&mesh,&pde,&bcc);

        // Initial condition u(0,x,y) = sin(0.5*PI*x)*sin(PI*y)+x
        Vec initial_condition = PetscTools::CreateVec(mesh.GetNumNodes());

        double* p_initial_condition;
        VecGetArray(initial_condition, &p_initial_condition);

        int lo, hi;
        VecGetOwnershipRange(initial_condition, &lo, &hi);
        for (int global_index = lo; global_index < hi; global_index++)
        {
            int local_index = global_index - lo;
            double x = mesh.GetNode(global_index)->GetPoint()[0];
            double y = mesh.GetNode(global_index)->GetPoint()[1];
            double z = mesh.GetNode(global_index)->GetPoint()[2];

            p_initial_condition[local_index] = sin(0.5*M_PI*x)*sin(M_PI*y)*sin(M_PI*z)+x;
        }
        VecRestoreArray(initial_condition, &p_initial_condition);

        solver.SetTimes(0, 0.1);
        solver.SetTimeStep(0.01);

        solver.SetInitialCondition(initial_condition);
        Vec result = solver.Solve();

        // Check result
        double* p_result;
        VecGetArray(result, &p_result);

        // Solution should be u = e^{-5/2*PI*PI*t} sin(0.5*PI*x)*sin(PI*y)*sin(PI*z)+x, t=0.1
        for (int global_index = lo; global_index < hi; global_index++)
        {
            int local_index = global_index - lo;
            double x = mesh.GetNode(global_index)->GetPoint()[0];
            double y = mesh.GetNode(global_index)->GetPoint()[1];
            double z = mesh.GetNode(global_index)->GetPoint()[2];

            double u = exp((-5/2)*M_PI*M_PI*0.1) * sin(0.5*M_PI*x) * sin(M_PI*y)* sin(M_PI*z) + x;
            TS_ASSERT_DELTA(p_result[local_index], u, u*0.15);
        }
        VecRestoreArray(result, &p_result);
        VecDestroy(initial_condition);
        VecDestroy(result);
    }
};

#endif //_TESTSIMPLELINEARPARABOLICSOLVERLONG_HPP_