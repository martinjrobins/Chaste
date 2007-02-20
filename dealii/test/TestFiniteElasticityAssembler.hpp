#ifndef TESTFINITEELASTICITYASSEMBLER_HPP_
#define TESTFINITEELASTICITYASSEMBLER_HPP_

#include <cxxtest/TestSuite.h>
#include "FiniteElasticityAssembler.cpp"

#include "TriangulationVertexIterator.hpp"
#include "DofVertexIterator.hpp"

#include "MooneyRivlinMaterialLaw.hpp"
#include "PolynomialMaterialLaw3d.hpp"
#include "ExponentialMaterialLaw.hpp"

#define DIMENSION 2

//template<int DIM>
//class ConstantVectorFunction : public Function<DIM>
//{
//private :
//    unsigned mNumComponents;
//    std::vector<double> mValues;
//    
//public:
//    ConstantVectorFunction(unsigned numComponents)
//        : Function<DIM>()
//    {
//        mNumComponents = numComponents;
//        mValues.resize(numComponents);
//        for(unsigned i=0; i<mNumComponents; i++)
//        {
//            mValues[i] = 10.0;
//        }
//    };
//      
//    double value(const Point<DIM>   &p,
//                 const unsigned int  component) 
//    {
//        assert(0);
//        assert(component < mNumComponents);
//        return mValues[component];
//    }
//    
//    void SetValue(unsigned component, double value)
//    {
//        assert(component < mNumComponents);
//        mValues[component] = value;
//    }
//};



class TestFiniteElasticityAssembler : public CxxTest::TestSuite
{
public :
    void test2dProblemOnSquare() throw(Exception)
    {
        Vector<double> body_force(2);
        body_force(0) = 6.0;
    
        MooneyRivlinMaterialLaw<2> mooney_rivlin_law(2.0);


        Triangulation<2> mesh;
        GridGenerator::hyper_cube(mesh, 0.0, 1.0); 
        mesh.refine_global(3);


        Triangulation<2>::cell_iterator element_iter = mesh.begin();
        while(element_iter!=mesh.end())
        {
            for(unsigned face_index=0; face_index<GeometryInfo<3>::faces_per_cell; face_index++)
            {
                if(element_iter->face(face_index)->at_boundary()) 
                {
                    double x = element_iter->face(face_index)->center()(0);
                    
                    if(fabs(x)<1e-4)
                    {
                        // z=0, label as fixed boundary
                        element_iter->face(face_index)->set_boundary_indicator(FIXED_BOUNDARY);
                    }
                    else 
                    {
                        // z!=0 or 1, label as neumann boundary
                        element_iter->face(face_index)->set_boundary_indicator(NEUMANN_BOUNDARY);
                    }
                }
            }
            element_iter++;
        }


        FiniteElasticityAssembler<2> finite_elasticity(&mesh,
                                                       &mooney_rivlin_law,
                                                       body_force,
                                                       1.0,
                                                       "finite_elas/simple2d");
                                                         
        finite_elasticity.Solve();

        Vector<double>& solution = finite_elasticity.GetSolutionVector();
        DoFHandler<2>& dof_handler = finite_elasticity.GetDofHandler();

        DofVertexIterator<2> vertex_iter(&mesh, &dof_handler);
        
        while(!vertex_iter.ReachedEnd())
        {
            unsigned vertex_index = vertex_iter.GetVertexGlobalIndex();
            Point<2> old_posn = vertex_iter.GetVertex();
            
            Point<2> new_posn;
            new_posn(0) = old_posn(0)+solution(vertex_iter.GetDof(0));
            new_posn(1) = old_posn(1)+solution(vertex_iter.GetDof(1));
            
            // todo: TEST THESE!!

            std::cout << vertex_index << " " << old_posn(0) << " " << old_posn(1)
                                      << " " << new_posn(0) << " " << new_posn(1) << "\n";
                                      

            //// UPDATE THE NODE POSITIONS
            // GetVertex returns a reference to a Point<DIM>, so this changes the mesh
            // directly. Do this so the new volume can be computed
            vertex_iter.GetVertex()[0] = new_posn(0);         
            vertex_iter.GetVertex()[1] = new_posn(1);         
                                      
            vertex_iter.Next();
        }

        // compute the deformed volume
        double deformed_volume = 0.0;
        //Triangulation<2>::active_cell_iterator  
        element_iter = mesh.begin_active();
        while(element_iter!=mesh.end())
        {
            double element_volume = element_iter->measure();
            TS_ASSERT_DELTA(element_volume, 1.0/mesh.n_active_cells(), 1e-1); 
            
            deformed_volume += element_volume;
            element_iter++;
        }
        
        TS_ASSERT_DELTA(deformed_volume, 1.0, 1e-2);
    }
    


    // Run same simulation on two meshes (one more refined than the other)
    // and test they agree on shared gridpoints
    void no___test2dProblemOnSquareForConvergence() throw(Exception)
    {
        ////////////////////////////////////////////////
        // run 1: on a mesh which is refined 3 times..
        ////////////////////////////////////////////////
        Vector<double> body_force(2);
        body_force(0) = 6.0;
    
        MooneyRivlinMaterialLaw<2> mooney_rivlin_law(2.0);

        Triangulation<2> mesh;
        GridGenerator::hyper_cube(mesh, 0.0, 1.0); 
        mesh.refine_global(3);

        FiniteElasticityAssembler<2> finite_elasticity(&mesh,
                                                       &mooney_rivlin_law,
                                                       body_force,
                                                       1.0,
                                                       "finite_elas/simple2d");
        finite_elasticity.Solve();

        Vector<double>& solution = finite_elasticity.GetSolutionVector();
        DoFHandler<2>& dof_handler = finite_elasticity.GetDofHandler();
        DofVertexIterator<2> vertex_iter(&mesh, &dof_handler);
        
        unsigned num_vertices = mesh.n_vertices(); 
        std::vector<double> new_x(num_vertices);
        std::vector<double> new_y(num_vertices);

        for(unsigned i=0; i<num_vertices; i++)
        {
            new_x[i] = 0.0;
            new_y[i] = 0.0;
        }
        
        // store the new position
        while(!vertex_iter.ReachedEnd())
        {
            unsigned vertex_index = vertex_iter.GetVertexGlobalIndex();
            Point<2> old_posn = vertex_iter.GetVertex();
            
            new_x[vertex_index] = old_posn(0)+solution(vertex_iter.GetDof(0));                          
            new_y[vertex_index] = old_posn(1)+solution(vertex_iter.GetDof(1));

            vertex_iter.Next();
        }


        //////////////////////////////////////////////////////////////
        // run 2: same problem, on a mesh which is refined 4 times..
        //////////////////////////////////////////////////////////////
        Triangulation<2> mesh_refined;
        GridGenerator::hyper_cube(mesh_refined, 0.0, 1.0); 

        mesh_refined.refine_global(4);

        FiniteElasticityAssembler<2> finite_elasticity_ref(&mesh_refined,
                                                           &mooney_rivlin_law,
                                                           body_force,
                                                           1.0,
                                                           "finite_elas/simple2d");
        finite_elasticity_ref.Solve();



        //////////////////////////////////////////////////////////////
        // compare the solution with that on the previous mesh
        //////////////////////////////////////////////////////////////
        Vector<double>& solution_ref = finite_elasticity_ref.GetSolutionVector();
        DoFHandler<2>& dof_handler_ref = finite_elasticity_ref.GetDofHandler();

        DofVertexIterator<2> vertex_iter_ref(&mesh_refined, &dof_handler_ref);
        while(!vertex_iter_ref.ReachedEnd())
        {
            unsigned vertex_index = vertex_iter_ref.GetVertexGlobalIndex();
            
            // if vertex_index < num_vertices (in the first mesh), this
            // node is in both meshes, so compare the results
            if(vertex_index < num_vertices)
            {
                Point<2> old_posn = vertex_iter_ref.GetVertex();
            
                Point<2> new_posn;
                new_posn(0) = old_posn(0)+solution_ref(vertex_iter_ref.GetDof(0));
                new_posn(1) = old_posn(1)+solution_ref(vertex_iter_ref.GetDof(1));
                
                TS_ASSERT_DELTA( new_posn(0), new_x[vertex_index], 1e-2 );
                TS_ASSERT_DELTA( new_posn(1), new_y[vertex_index], 1e-2 );
            }
            
            vertex_iter_ref.Next();
        }
        
        // check nothing has changed
        TS_ASSERT_DELTA( new_x[6], 1.2158, 1e-3); 
        TS_ASSERT_DELTA( new_y[6], 0.5, 1e-3); 
    }



    void test3dProblemOnCubeAxialStretchingFixedDisplacement() throw(Exception)
    {
        Vector<double> body_force(3); // zero vector
        body_force(2)=5;
    
        MooneyRivlinMaterialLaw<3> mooney_rivlin_law(2.0,2.0);

        Triangulation<3> mesh;
        GridGenerator::hyper_cube(mesh, 0.0, 1.0); 
        mesh.refine_global(1);


        Triangulation<3>::cell_iterator element_iter = mesh.begin();
        while(element_iter!=mesh.end())
        {
            for(unsigned face_index=0; face_index<GeometryInfo<3>::faces_per_cell; face_index++)
            {
                if(element_iter->face(face_index)->at_boundary()) 
                {
                    double z = element_iter->face(face_index)->center()(2);
                    
                    if(fabs(z)<1e-4)
                    {
                        // z=0, label as fixed boundary
                        element_iter->face(face_index)->set_boundary_indicator(FIXED_BOUNDARY);
                    }
                    else if(fabs(z-1)<1e-4)
                    {
                        // z=1, label as dirichlet boundary
                        element_iter->face(face_index)->set_boundary_indicator(DIRICHLET_BOUNDARY);
                    }
                    else 
                    {
                        // z!=0 or 1, label as neumann boundary
                        element_iter->face(face_index)->set_boundary_indicator(NEUMANN_BOUNDARY);
                    }
                }
            }
            element_iter++;
        }

        FiniteElasticityAssembler<3> finite_elasticity(&mesh,
                                                       &mooney_rivlin_law,
                                                       body_force,
                                                       1.0,
                                                       "finite_elas/axial3d");

        DoFHandler<3>& dof_handler = finite_elasticity.GetDofHandler();


        std::map<unsigned,double> boundary_values;
        
        std::vector<bool> component_mask(3+1); // dim+1
        component_mask[0] = true;
        component_mask[1] = true;
        component_mask[2] = true;
        component_mask[3] = false;

        VectorTools::interpolate_boundary_values(dof_handler,
                                                 FIXED_BOUNDARY,
                                                 ZeroFunction<3>(3+1),  // note the "+1" here! - number of components
                                                 boundary_values,
                                                 component_mask);

        assert(!boundary_values.empty());


        VectorTools::interpolate_boundary_values(dof_handler,
                                                 DIRICHLET_BOUNDARY,
//                                                 ZeroFunction<3>(4),
                                                 ComponentSelectFunction<3>(2,-0.1,4),
//                                                 ConstantFunction<3>(0.05,3+1),
                                                 boundary_values,
                                                 component_mask);

        assert(!boundary_values.empty());


        finite_elasticity.SetBoundaryValues(boundary_values);

        
        finite_elasticity.Solve();


        Vector<double>& solution = finite_elasticity.GetSolutionVector();


        DofVertexIterator<3> dof_vertex_iter(&mesh, &dof_handler);

        
        while(!dof_vertex_iter.ReachedEnd())
        {
            unsigned vertex_index = dof_vertex_iter.GetVertexGlobalIndex();
            Point<3> old_posn = dof_vertex_iter.GetVertex();
            
            Point<3> new_posn;
            new_posn(0) = old_posn(0)+solution(dof_vertex_iter.GetDof(0));
            new_posn(1) = old_posn(1)+solution(dof_vertex_iter.GetDof(1));
            new_posn(2) = old_posn(2)+solution(dof_vertex_iter.GetDof(2));
            
            // todo: TEST THESE!!

            std::cout << vertex_index << " " << old_posn(0) << " " << old_posn(1) << " " << old_posn(2) 
                                      << " " << new_posn(0) << " " << new_posn(1) << " " << new_posn(2)
                                      << "\n";

            //// UPDATE THE NODE POSITIONS
            // GetVertex returns a reference to a Point<DIM>, so this changes the mesh
            // directly. Do this so the new volume can be computed
            dof_vertex_iter.GetVertex()[0] = new_posn(0);         
            dof_vertex_iter.GetVertex()[1] = new_posn(1);         
            dof_vertex_iter.GetVertex()[2] = new_posn(2);         
                                      
            dof_vertex_iter.Next();
        }

        // compute the deformed volume
        double deformed_volume = 0.0;
        //Triangulation<3>::active_cell_iterator  
        element_iter = mesh.begin_active();
        while(element_iter!=mesh.end())
        {
            double element_volume = element_iter->measure();
            TS_ASSERT_DELTA(element_volume, 1.0/mesh.n_active_cells(), 1e-6); 
            
            deformed_volume += element_volume;
            element_iter++;
        }
        
        TS_ASSERT_DELTA(deformed_volume, 1.0, 1e-2);


    }

    
    void no______________test3dProblemOnCube() throw(Exception)
    {
        Vector<double> body_force(3);
        body_force(1) = 10.0;
    
        MooneyRivlinMaterialLaw<3> mooney_rivlin_law(2.0,2.0);

        Triangulation<3> mesh;
        GridGenerator::hyper_cube(mesh, 0.0, 0.1); 
        mesh.refine_global(2);

        FiniteElasticityAssembler<3> finite_elasticity(&mesh,
                                                       &mooney_rivlin_law,
                                                       body_force,
                                                       1.0,
                                                       "finite_elas/simple3d");
        finite_elasticity.Solve();

        Vector<double>& solution = finite_elasticity.GetSolutionVector();
        DoFHandler<3>& dof_handler = finite_elasticity.GetDofHandler();


        DofVertexIterator<3> vertex_iter(&mesh, &dof_handler);
        
        while(!vertex_iter.ReachedEnd())
        {
            unsigned vertex_index = vertex_iter.GetVertexGlobalIndex();
            Point<3> old_posn = vertex_iter.GetVertex();
            
            Point<3> new_posn;
            new_posn(0) = old_posn(0)+solution(vertex_iter.GetDof(0));
            new_posn(1) = old_posn(1)+solution(vertex_iter.GetDof(1));
            new_posn(2) = old_posn(2)+solution(vertex_iter.GetDof(2));
            
            // todo: TEST THESE!!

            std::cout << vertex_index << " " << old_posn(0) << " " << old_posn(1) << " " << old_posn(2) 
                                      << " " << new_posn(0) << " " << new_posn(1) << " " << new_posn(2)
                                      << "\n";
            vertex_iter.Next();
        }
    }



//    void test3dProblemOnCube2() throw(Exception)
//    {
//        Vector<double> body_force(3);
//        body_force(0) = 20.0;
//    
//        std::vector<std::vector<double> > alpha = PolynomialMaterialLaw3d::GetZeroedAlpha(2);
///*        
//        c10 = 310
//        c01 = 300
//        c20 = 2250
//        c02 = 3800
//        c11 = 4720
//        density rho = 940
//*/
//        alpha[1][0] = 0.31;
//        alpha[0][1] = 0.3;
//        alpha[2][0] = 2.25;
//        alpha[0][2] = 3.8;
//        alpha[1][1] = 4.72;
//
//       
//        double density = 0.94;       
//       
//        PolynomialMaterialLaw3d poly_law(2,alpha); 
//
//
//        Triangulation<3> mesh;
//        GridGenerator::hyper_cube(mesh, 0.0, 0.1);  // <-- 0.1m!! 
//        mesh.refine_global(2);
//
//        FiniteElasticityAssembler<3> finite_elasticity(&mesh,
//                                                       &poly_law,
//                                                       body_force,
//                                                       density,
//                                                       "finite_elas/simple3d");
//        finite_elasticity.Solve();
//
//
//        Vector<double>& solution = finite_elasticity.GetSolutionVector();
//        DoFHandler<3>& dof_handler = finite_elasticity.GetDofHandler();
//
//
//        DofVertexIterator<3> vertex_iter(&mesh, &dof_handler);
//        
//        while(!vertex_iter.ReachedEnd())
//        {
//            unsigned vertex_index = vertex_iter.GetVertexGlobalIndex();
//            Point<3> old_posn = vertex_iter.GetVertex();
//            
//            Point<3> new_posn;
//            new_posn(0) = old_posn(0)+solution(vertex_iter.GetDof(0));
//            new_posn(1) = old_posn(1)+solution(vertex_iter.GetDof(1));
//            new_posn(2) = old_posn(2)+solution(vertex_iter.GetDof(2));
//            
//            // todo: TEST THESE!!
//
//            std::cout << vertex_index << " " << old_posn(0) << " " << old_posn(1) << " " << old_posn(2) 
//                                      << " " << new_posn(0) << " " << new_posn(1) << " " << new_posn(2)
//                                      << "\n";
//            vertex_iter.Next();
//        }
//    }

};
#endif /*TESTFINITEELASTICITYASSEMBLER_HPP_*/
