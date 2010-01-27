#ifndef CELLLUO_RUDY_1991FROMCELLML_HPP_
#define CELLLUO_RUDY_1991FROMCELLML_HPP_

//! @file
//! 
//! This source file was generated from CellML.
//! 
//! Model: luo_rudy_1991
//! 
//! Processed by pycml - CellML Tools in Python
//!     (translate: 7844, pycml: 7844)
//! on Wed Jan 27 16:38:45 2010
//! 
//! <autogenerated>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <cmath>
#include <cassert>
#include "AbstractCardiacCell.hpp"
#include "Exception.hpp"
#include "OdeSystemInformation.hpp"
#include "AbstractStimulusFunction.hpp"

class Cellluo_rudy_1991FromCellML : public AbstractCardiacCell
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & boost::serialization::base_object<AbstractCardiacCell >(*this);
    }
    
private:
public:
    Cellluo_rudy_1991FromCellML(
            boost::shared_ptr<AbstractIvpOdeSolver> pSolver,
            boost::shared_ptr<AbstractStimulusFunction> pIntracellularStimulus)
        : AbstractCardiacCell(
                pSolver,
                8,
                0,
                pIntracellularStimulus)
    {
        // Time units: millisecond
        // 
        mpSystemInfo = OdeSystemInformation<Cellluo_rudy_1991FromCellML>::Instance();
        Init();

    }
    
    ~Cellluo_rudy_1991FromCellML(void)
    {
    }
    
    void VerifyStateVariables()
    {}

    double GetIIonic()
    {
        std::vector<double>& rY = rGetStateVariables();
        double var_membrane__V = rY[0];
        // Units: millivolt; Initial value: -83.853
        double var_fast_sodium_current_m_gate__m = rY[1];
        // Units: dimensionless; Initial value: 0.00187018
        double var_fast_sodium_current_h_gate__h = rY[2];
        // Units: dimensionless; Initial value: 0.9804713
        double var_fast_sodium_current_j_gate__j = rY[3];
        // Units: dimensionless; Initial value: 0.98767124
        double var_slow_inward_current_d_gate__d = rY[4];
        // Units: dimensionless; Initial value: 0.00316354
        double var_slow_inward_current_f_gate__f = rY[5];
        // Units: dimensionless; Initial value: 0.99427859
        double var_time_dependent_potassium_current_X_gate__X = rY[6];
        // Units: dimensionless; Initial value: 0.16647703
        double var_intracellular_calcium_concentration__Cai = rY[7];
        // Units: millimolar; Initial value: 0.0002
        
        const double var_membrane__R = 8314.0;
        const double var_membrane__T = 310.0;
        const double var_membrane__F = 96484.6;
        const double var_fast_sodium_current__j = var_fast_sodium_current_j_gate__j;
        const double var_fast_sodium_current__h = var_fast_sodium_current_h_gate__h;
        const double var_fast_sodium_current__g_Na = 23.0;
        const double var_fast_sodium_current__m = var_fast_sodium_current_m_gate__m;
        const double var_fast_sodium_current__V = var_membrane__V;
        const double var_fast_sodium_current__R = var_membrane__R;
        const double var_fast_sodium_current__F = var_membrane__F;
        const double var_ionic_concentrations__Nao = 140.0;
        const double var_fast_sodium_current__Nao = var_ionic_concentrations__Nao;
        const double var_ionic_concentrations__Nai = 18.0;
        const double var_fast_sodium_current__Nai = var_ionic_concentrations__Nai;
        const double var_fast_sodium_current__T = var_membrane__T;
        const double var_fast_sodium_current__E_Na = ((var_fast_sodium_current__R * var_fast_sodium_current__T) / var_fast_sodium_current__F) * log(var_fast_sodium_current__Nao / var_fast_sodium_current__Nai);
        const double var_fast_sodium_current__i_Na = var_fast_sodium_current__g_Na * pow(var_fast_sodium_current__m, 3.0) * var_fast_sodium_current__h * var_fast_sodium_current__j * (var_fast_sodium_current__V - var_fast_sodium_current__E_Na);
        const double var_membrane__i_Na = var_fast_sodium_current__i_Na;
        const double var_slow_inward_current__d = var_slow_inward_current_d_gate__d;
        const double var_slow_inward_current__f = var_slow_inward_current_f_gate__f;
        const double var_slow_inward_current__V = var_membrane__V;
        const double var_slow_inward_current__Cai = var_intracellular_calcium_concentration__Cai;
        const double var_slow_inward_current__E_si = 7.7 - (13.0287 * log(var_slow_inward_current__Cai / 1.0));
        const double var_slow_inward_current__i_si = 0.09 * var_slow_inward_current__d * var_slow_inward_current__f * (var_slow_inward_current__V - var_slow_inward_current__E_si);
        const double var_membrane__i_si = var_slow_inward_current__i_si;
        const double var_time_dependent_potassium_current__V = var_membrane__V;
        const double var_time_dependent_potassium_current_Xi_gate__V = var_time_dependent_potassium_current__V;
        const double var_time_dependent_potassium_current_Xi_gate__Xi = (var_time_dependent_potassium_current_Xi_gate__V > (-100.0)) ? ((2.837 * (exp(0.04 * (var_time_dependent_potassium_current_Xi_gate__V + 77.0)) - 1.0)) / ((var_time_dependent_potassium_current_Xi_gate__V + 77.0) * exp(0.04 * (var_time_dependent_potassium_current_Xi_gate__V + 35.0)))) : 1.0;
        const double var_time_dependent_potassium_current__Xi = var_time_dependent_potassium_current_Xi_gate__Xi;
        const double var_ionic_concentrations__Ko = 5.4;
        const double var_time_dependent_potassium_current__Ko = var_ionic_concentrations__Ko;
        const double var_time_dependent_potassium_current__g_K = 0.282 * sqrt(var_time_dependent_potassium_current__Ko / 5.4);
        const double var_time_dependent_potassium_current__X = var_time_dependent_potassium_current_X_gate__X;
        const double var_time_dependent_potassium_current__PR_NaK = 0.01833;
        const double var_time_dependent_potassium_current__F = var_membrane__F;
        const double var_time_dependent_potassium_current__Nao = var_ionic_concentrations__Nao;
        const double var_ionic_concentrations__Ki = 145.0;
        const double var_time_dependent_potassium_current__Ki = var_ionic_concentrations__Ki;
        const double var_time_dependent_potassium_current__Nai = var_ionic_concentrations__Nai;
        const double var_time_dependent_potassium_current__T = var_membrane__T;
        const double var_time_dependent_potassium_current__R = var_membrane__R;
        const double var_time_dependent_potassium_current__E_K = ((var_time_dependent_potassium_current__R * var_time_dependent_potassium_current__T) / var_time_dependent_potassium_current__F) * log((var_time_dependent_potassium_current__Ko + (var_time_dependent_potassium_current__PR_NaK * var_time_dependent_potassium_current__Nao)) / (var_time_dependent_potassium_current__Ki + (var_time_dependent_potassium_current__PR_NaK * var_time_dependent_potassium_current__Nai)));
        const double var_time_dependent_potassium_current__i_K = var_time_dependent_potassium_current__g_K * var_time_dependent_potassium_current__X * var_time_dependent_potassium_current__Xi * (var_time_dependent_potassium_current__V - var_time_dependent_potassium_current__E_K);
        const double var_membrane__i_K = var_time_dependent_potassium_current__i_K;
        const double var_time_independent_potassium_current__V = var_membrane__V;
        const double var_time_independent_potassium_current_K1_gate__V = var_time_independent_potassium_current__V;
        const double var_time_independent_potassium_current__Ki = var_ionic_concentrations__Ki;
        const double var_time_independent_potassium_current__R = var_membrane__R;
        const double var_time_independent_potassium_current__F = var_membrane__F;
        const double var_time_independent_potassium_current__Ko = var_ionic_concentrations__Ko;
        const double var_time_independent_potassium_current__T = var_membrane__T;
        const double var_time_independent_potassium_current__E_K1 = ((var_time_independent_potassium_current__R * var_time_independent_potassium_current__T) / var_time_independent_potassium_current__F) * log(var_time_independent_potassium_current__Ko / var_time_independent_potassium_current__Ki);
        const double var_time_independent_potassium_current_K1_gate__E_K1 = var_time_independent_potassium_current__E_K1;
        const double var_time_independent_potassium_current_K1_gate__beta_K1 = ((0.49124 * exp(0.08032 * ((var_time_independent_potassium_current_K1_gate__V + 5.476) - var_time_independent_potassium_current_K1_gate__E_K1))) + (1.0 * exp(0.06175 * (var_time_independent_potassium_current_K1_gate__V - (var_time_independent_potassium_current_K1_gate__E_K1 + 594.31))))) / (1.0 + exp((-0.5143) * ((var_time_independent_potassium_current_K1_gate__V - var_time_independent_potassium_current_K1_gate__E_K1) + 4.753)));
        const double var_time_independent_potassium_current_K1_gate__alpha_K1 = 1.02 / (1.0 + exp(0.2385 * ((var_time_independent_potassium_current_K1_gate__V - var_time_independent_potassium_current_K1_gate__E_K1) - 59.215)));
        const double var_time_independent_potassium_current_K1_gate__K1_infinity = var_time_independent_potassium_current_K1_gate__alpha_K1 / (var_time_independent_potassium_current_K1_gate__alpha_K1 + var_time_independent_potassium_current_K1_gate__beta_K1);
        const double var_time_independent_potassium_current__K1_infinity = var_time_independent_potassium_current_K1_gate__K1_infinity;
        const double var_time_independent_potassium_current__g_K1 = 0.6047 * sqrt(var_time_independent_potassium_current__Ko / 5.4);
        const double var_time_independent_potassium_current__i_K1 = var_time_independent_potassium_current__g_K1 * var_time_independent_potassium_current__K1_infinity * (var_time_independent_potassium_current__V - var_time_independent_potassium_current__E_K1);
        const double var_membrane__i_K1 = var_time_independent_potassium_current__i_K1;
        const double var_plateau_potassium_current__g_Kp = 0.0183;
        const double var_plateau_potassium_current__V = var_membrane__V;
        const double var_plateau_potassium_current__Kp = 1.0 / (1.0 + exp((7.488 - var_plateau_potassium_current__V) / 5.98));
        const double var_plateau_potassium_current__E_K1 = var_time_independent_potassium_current__E_K1;
        const double var_plateau_potassium_current__E_Kp = var_plateau_potassium_current__E_K1;
        const double var_plateau_potassium_current__i_Kp = var_plateau_potassium_current__g_Kp * var_plateau_potassium_current__Kp * (var_plateau_potassium_current__V - var_plateau_potassium_current__E_Kp);
        const double var_membrane__i_Kp = var_plateau_potassium_current__i_Kp;
        const double var_background_current__E_b =  -59.87;
        const double var_background_current__g_b = 0.03921;
        const double var_background_current__V = var_membrane__V;
        const double var_background_current__i_b = var_background_current__g_b * (var_background_current__V - var_background_current__E_b);
        const double var_membrane__i_b = var_background_current__i_b;
        
        return (var_membrane__i_Na+var_membrane__i_si+var_membrane__i_K+var_membrane__i_K1+var_membrane__i_Kp+var_membrane__i_b);
    }
    
    void EvaluateYDerivatives(
            double var_environment__time,
            const std::vector<double>& rY,
            std::vector<double>& rDY)
    {
        // Inputs:
        // Time units: millisecond
        var_environment__time *= 1.0;
        double var_membrane__V = rY[0];
        // Units: millivolt; Initial value: -83.853
        double var_fast_sodium_current_m_gate__m = rY[1];
        // Units: dimensionless; Initial value: 0.00187018
        double var_fast_sodium_current_h_gate__h = rY[2];
        // Units: dimensionless; Initial value: 0.9804713
        double var_fast_sodium_current_j_gate__j = rY[3];
        // Units: dimensionless; Initial value: 0.98767124
        double var_slow_inward_current_d_gate__d = rY[4];
        // Units: dimensionless; Initial value: 0.00316354
        double var_slow_inward_current_f_gate__f = rY[5];
        // Units: dimensionless; Initial value: 0.99427859
        double var_time_dependent_potassium_current_X_gate__X = rY[6];
        // Units: dimensionless; Initial value: 0.16647703
        double var_intracellular_calcium_concentration__Cai = rY[7];
        // Units: millimolar; Initial value: 0.0002
        
        
        // Mathematics
        const double var_membrane__R = 8314.0;
        const double var_membrane__T = 310.0;
        const double var_membrane__F = 96484.6;
        const double var_membrane__C = 1.0;
        double var_membrane__I_stim = GetStimulus((1.0/1)*var_environment__time);
        const double var_fast_sodium_current__j = var_fast_sodium_current_j_gate__j;
        const double var_fast_sodium_current__h = var_fast_sodium_current_h_gate__h;
        const double var_fast_sodium_current__g_Na = 23.0;
        const double var_fast_sodium_current__m = var_fast_sodium_current_m_gate__m;
        const double var_fast_sodium_current__V = var_membrane__V;
        const double var_fast_sodium_current__R = var_membrane__R;
        const double var_fast_sodium_current__F = var_membrane__F;
        const double var_ionic_concentrations__Nao = 140.0;
        const double var_fast_sodium_current__Nao = var_ionic_concentrations__Nao;
        const double var_ionic_concentrations__Nai = 18.0;
        const double var_fast_sodium_current__Nai = var_ionic_concentrations__Nai;
        const double var_fast_sodium_current__T = var_membrane__T;
        const double var_fast_sodium_current__E_Na = ((var_fast_sodium_current__R * var_fast_sodium_current__T) / var_fast_sodium_current__F) * log(var_fast_sodium_current__Nao / var_fast_sodium_current__Nai);
        const double var_fast_sodium_current__i_Na = var_fast_sodium_current__g_Na * pow(var_fast_sodium_current__m, 3.0) * var_fast_sodium_current__h * var_fast_sodium_current__j * (var_fast_sodium_current__V - var_fast_sodium_current__E_Na);
        const double var_membrane__i_Na = var_fast_sodium_current__i_Na;
        const double var_slow_inward_current__d = var_slow_inward_current_d_gate__d;
        const double var_slow_inward_current__f = var_slow_inward_current_f_gate__f;
        const double var_slow_inward_current__V = var_membrane__V;
        const double var_slow_inward_current__Cai = var_intracellular_calcium_concentration__Cai;
        const double var_slow_inward_current__E_si = 7.7 - (13.0287 * log(var_slow_inward_current__Cai / 1.0));
        const double var_slow_inward_current__i_si = 0.09 * var_slow_inward_current__d * var_slow_inward_current__f * (var_slow_inward_current__V - var_slow_inward_current__E_si);
        const double var_membrane__i_si = var_slow_inward_current__i_si;
        const double var_time_dependent_potassium_current__V = var_membrane__V;
        const double var_time_dependent_potassium_current_Xi_gate__V = var_time_dependent_potassium_current__V;
        const double var_time_dependent_potassium_current_Xi_gate__Xi = (var_time_dependent_potassium_current_Xi_gate__V > (-100.0)) ? ((2.837 * (exp(0.04 * (var_time_dependent_potassium_current_Xi_gate__V + 77.0)) - 1.0)) / ((var_time_dependent_potassium_current_Xi_gate__V + 77.0) * exp(0.04 * (var_time_dependent_potassium_current_Xi_gate__V + 35.0)))) : 1.0;
        const double var_time_dependent_potassium_current__Xi = var_time_dependent_potassium_current_Xi_gate__Xi;
        const double var_ionic_concentrations__Ko = 5.4;
        const double var_time_dependent_potassium_current__Ko = var_ionic_concentrations__Ko;
        const double var_time_dependent_potassium_current__g_K = 0.282 * sqrt(var_time_dependent_potassium_current__Ko / 5.4);
        const double var_time_dependent_potassium_current__X = var_time_dependent_potassium_current_X_gate__X;
        const double var_time_dependent_potassium_current__PR_NaK = 0.01833;
        const double var_time_dependent_potassium_current__F = var_membrane__F;
        const double var_time_dependent_potassium_current__Nao = var_ionic_concentrations__Nao;
        const double var_ionic_concentrations__Ki = 145.0;
        const double var_time_dependent_potassium_current__Ki = var_ionic_concentrations__Ki;
        const double var_time_dependent_potassium_current__Nai = var_ionic_concentrations__Nai;
        const double var_time_dependent_potassium_current__T = var_membrane__T;
        const double var_time_dependent_potassium_current__R = var_membrane__R;
        const double var_time_dependent_potassium_current__E_K = ((var_time_dependent_potassium_current__R * var_time_dependent_potassium_current__T) / var_time_dependent_potassium_current__F) * log((var_time_dependent_potassium_current__Ko + (var_time_dependent_potassium_current__PR_NaK * var_time_dependent_potassium_current__Nao)) / (var_time_dependent_potassium_current__Ki + (var_time_dependent_potassium_current__PR_NaK * var_time_dependent_potassium_current__Nai)));
        const double var_time_dependent_potassium_current__i_K = var_time_dependent_potassium_current__g_K * var_time_dependent_potassium_current__X * var_time_dependent_potassium_current__Xi * (var_time_dependent_potassium_current__V - var_time_dependent_potassium_current__E_K);
        const double var_membrane__i_K = var_time_dependent_potassium_current__i_K;
        const double var_time_independent_potassium_current__V = var_membrane__V;
        const double var_time_independent_potassium_current_K1_gate__V = var_time_independent_potassium_current__V;
        const double var_time_independent_potassium_current__Ki = var_ionic_concentrations__Ki;
        const double var_time_independent_potassium_current__R = var_membrane__R;
        const double var_time_independent_potassium_current__F = var_membrane__F;
        const double var_time_independent_potassium_current__Ko = var_ionic_concentrations__Ko;
        const double var_time_independent_potassium_current__T = var_membrane__T;
        const double var_time_independent_potassium_current__E_K1 = ((var_time_independent_potassium_current__R * var_time_independent_potassium_current__T) / var_time_independent_potassium_current__F) * log(var_time_independent_potassium_current__Ko / var_time_independent_potassium_current__Ki);
        const double var_time_independent_potassium_current_K1_gate__E_K1 = var_time_independent_potassium_current__E_K1;
        const double var_time_independent_potassium_current_K1_gate__beta_K1 = ((0.49124 * exp(0.08032 * ((var_time_independent_potassium_current_K1_gate__V + 5.476) - var_time_independent_potassium_current_K1_gate__E_K1))) + (1.0 * exp(0.06175 * (var_time_independent_potassium_current_K1_gate__V - (var_time_independent_potassium_current_K1_gate__E_K1 + 594.31))))) / (1.0 + exp((-0.5143) * ((var_time_independent_potassium_current_K1_gate__V - var_time_independent_potassium_current_K1_gate__E_K1) + 4.753)));
        const double var_time_independent_potassium_current_K1_gate__alpha_K1 = 1.02 / (1.0 + exp(0.2385 * ((var_time_independent_potassium_current_K1_gate__V - var_time_independent_potassium_current_K1_gate__E_K1) - 59.215)));
        const double var_time_independent_potassium_current_K1_gate__K1_infinity = var_time_independent_potassium_current_K1_gate__alpha_K1 / (var_time_independent_potassium_current_K1_gate__alpha_K1 + var_time_independent_potassium_current_K1_gate__beta_K1);
        const double var_time_independent_potassium_current__K1_infinity = var_time_independent_potassium_current_K1_gate__K1_infinity;
        const double var_time_independent_potassium_current__g_K1 = 0.6047 * sqrt(var_time_independent_potassium_current__Ko / 5.4);
        const double var_time_independent_potassium_current__i_K1 = var_time_independent_potassium_current__g_K1 * var_time_independent_potassium_current__K1_infinity * (var_time_independent_potassium_current__V - var_time_independent_potassium_current__E_K1);
        const double var_membrane__i_K1 = var_time_independent_potassium_current__i_K1;
        const double var_plateau_potassium_current__g_Kp = 0.0183;
        const double var_plateau_potassium_current__V = var_membrane__V;
        const double var_plateau_potassium_current__Kp = 1.0 / (1.0 + exp((7.488 - var_plateau_potassium_current__V) / 5.98));
        const double var_plateau_potassium_current__E_K1 = var_time_independent_potassium_current__E_K1;
        const double var_plateau_potassium_current__E_Kp = var_plateau_potassium_current__E_K1;
        const double var_plateau_potassium_current__i_Kp = var_plateau_potassium_current__g_Kp * var_plateau_potassium_current__Kp * (var_plateau_potassium_current__V - var_plateau_potassium_current__E_Kp);
        const double var_membrane__i_Kp = var_plateau_potassium_current__i_Kp;
        const double var_background_current__E_b =  -59.87;
        const double var_background_current__g_b = 0.03921;
        const double var_background_current__V = var_membrane__V;
        const double var_background_current__i_b = var_background_current__g_b * (var_background_current__V - var_background_current__E_b);
        const double var_membrane__i_b = var_background_current__i_b;
        const double var_fast_sodium_current_m_gate__V = var_fast_sodium_current__V;
        const double var_fast_sodium_current_m_gate__alpha_m = (0.32 * (var_fast_sodium_current_m_gate__V + 47.13)) / (1.0 - exp((-0.1) * (var_fast_sodium_current_m_gate__V + 47.13)));
        const double var_fast_sodium_current_m_gate__beta_m = 0.08 * exp((-var_fast_sodium_current_m_gate__V) / 11.0);
        const double var_fast_sodium_current_h_gate__V = var_fast_sodium_current__V;
        const double var_fast_sodium_current_h_gate__alpha_h = (var_fast_sodium_current_h_gate__V < (-40.0)) ? (0.135 * exp((80.0 + var_fast_sodium_current_h_gate__V) / (-6.8))) : 0.0;
        const double var_fast_sodium_current_h_gate__beta_h = (var_fast_sodium_current_h_gate__V < (-40.0)) ? ((3.56 * exp(0.079 * var_fast_sodium_current_h_gate__V)) + (310000.0 * exp(0.35 * var_fast_sodium_current_h_gate__V))) : (1.0 / (0.13 * (1.0 + exp((var_fast_sodium_current_h_gate__V + 10.66) / (-11.1)))));
        const double var_fast_sodium_current_j_gate__V = var_fast_sodium_current__V;
        const double var_fast_sodium_current_j_gate__alpha_j = (var_fast_sodium_current_j_gate__V < (-40.0)) ? (((((-127140.0) * exp(0.2444 * var_fast_sodium_current_j_gate__V)) - (3.474e-05 * exp((-0.04391) * var_fast_sodium_current_j_gate__V))) * (var_fast_sodium_current_j_gate__V + 37.78)) / (1.0 + exp(0.311 * (var_fast_sodium_current_j_gate__V + 79.23)))) : 0.0;
        const double var_fast_sodium_current_j_gate__beta_j = (var_fast_sodium_current_j_gate__V < (-40.0)) ? ((0.1212 * exp((-0.01052) * var_fast_sodium_current_j_gate__V)) / (1.0 + exp((-0.1378) * (var_fast_sodium_current_j_gate__V + 40.14)))) : ((0.3 * exp((-2.535e-07) * var_fast_sodium_current_j_gate__V)) / (1.0 + exp((-0.1) * (var_fast_sodium_current_j_gate__V + 32.0))));
        const double var_slow_inward_current_d_gate__V = var_slow_inward_current__V;
        const double var_slow_inward_current_d_gate__alpha_d = (0.095 * exp((-0.01) * (var_slow_inward_current_d_gate__V - 5.0))) / (1.0 + exp((-0.072) * (var_slow_inward_current_d_gate__V - 5.0)));
        const double var_slow_inward_current_d_gate__beta_d = (0.07 * exp((-0.017) * (var_slow_inward_current_d_gate__V + 44.0))) / (1.0 + exp(0.05 * (var_slow_inward_current_d_gate__V + 44.0)));
        const double var_slow_inward_current_f_gate__V = var_slow_inward_current__V;
        const double var_slow_inward_current_f_gate__alpha_f = (0.012 * exp((-0.008) * (var_slow_inward_current_f_gate__V + 28.0))) / (1.0 + exp(0.15 * (var_slow_inward_current_f_gate__V + 28.0)));
        const double var_slow_inward_current_f_gate__beta_f = (0.0065 * exp((-0.02) * (var_slow_inward_current_f_gate__V + 30.0))) / (1.0 + exp((-0.2) * (var_slow_inward_current_f_gate__V + 30.0)));
        const double var_time_dependent_potassium_current_X_gate__V = var_time_dependent_potassium_current__V;
        const double var_time_dependent_potassium_current_X_gate__alpha_X = (0.0005 * exp(0.083 * (var_time_dependent_potassium_current_X_gate__V + 50.0))) / (1.0 + exp(0.057 * (var_time_dependent_potassium_current_X_gate__V + 50.0)));
        const double var_time_dependent_potassium_current_X_gate__beta_X = (0.0013 * exp((-0.06) * (var_time_dependent_potassium_current_X_gate__V + 20.0))) / (1.0 + exp((-0.04) * (var_time_dependent_potassium_current_X_gate__V + 20.0)));
        const double var_intracellular_calcium_concentration__i_si = var_slow_inward_current__i_si;
        
        double d_dt_membrane__V;
        if (mSetVoltageDerivativeToZero)
        {
            d_dt_membrane__V = 0.0;
        }
        else
        {
            d_dt_membrane__V = ((-1.0) / var_membrane__C) * (var_membrane__I_stim + var_membrane__i_Na + var_membrane__i_si + var_membrane__i_K + var_membrane__i_K1 + var_membrane__i_Kp + var_membrane__i_b);
        }
        
        const double d_dt_fast_sodium_current_m_gate__m = (var_fast_sodium_current_m_gate__alpha_m * (1.0 - var_fast_sodium_current_m_gate__m)) - (var_fast_sodium_current_m_gate__beta_m * var_fast_sodium_current_m_gate__m);
        const double d_dt_fast_sodium_current_h_gate__h = (var_fast_sodium_current_h_gate__alpha_h * (1.0 - var_fast_sodium_current_h_gate__h)) - (var_fast_sodium_current_h_gate__beta_h * var_fast_sodium_current_h_gate__h);
        const double d_dt_fast_sodium_current_j_gate__j = (var_fast_sodium_current_j_gate__alpha_j * (1.0 - var_fast_sodium_current_j_gate__j)) - (var_fast_sodium_current_j_gate__beta_j * var_fast_sodium_current_j_gate__j);
        const double d_dt_slow_inward_current_d_gate__d = (var_slow_inward_current_d_gate__alpha_d * (1.0 - var_slow_inward_current_d_gate__d)) - (var_slow_inward_current_d_gate__beta_d * var_slow_inward_current_d_gate__d);
        const double d_dt_slow_inward_current_f_gate__f = (var_slow_inward_current_f_gate__alpha_f * (1.0 - var_slow_inward_current_f_gate__f)) - (var_slow_inward_current_f_gate__beta_f * var_slow_inward_current_f_gate__f);
        const double d_dt_time_dependent_potassium_current_X_gate__X = (var_time_dependent_potassium_current_X_gate__alpha_X * (1.0 - var_time_dependent_potassium_current_X_gate__X)) - (var_time_dependent_potassium_current_X_gate__beta_X * var_time_dependent_potassium_current_X_gate__X);
        const double d_dt_intracellular_calcium_concentration__Cai = (((-0.0001) / 1.0) * var_intracellular_calcium_concentration__i_si) + (0.07 * (0.0001 - var_intracellular_calcium_concentration__Cai));
        
        rDY[0] = 1.0*d_dt_membrane__V;
        rDY[1] = 1.0*d_dt_fast_sodium_current_m_gate__m;
        rDY[2] = 1.0*d_dt_fast_sodium_current_h_gate__h;
        rDY[3] = 1.0*d_dt_fast_sodium_current_j_gate__j;
        rDY[4] = 1.0*d_dt_slow_inward_current_d_gate__d;
        rDY[5] = 1.0*d_dt_slow_inward_current_f_gate__f;
        rDY[6] = 1.0*d_dt_time_dependent_potassium_current_X_gate__X;
        rDY[7] = 1.0*d_dt_intracellular_calcium_concentration__Cai;
    }
    
};


template<>
void OdeSystemInformation<Cellluo_rudy_1991FromCellML>::Initialise(void)
{
    // Time units: millisecond
    // 
    this->mVariableNames.push_back("V");
    this->mVariableUnits.push_back("millivolt");
    this->mInitialConditions.push_back(-83.853);

    this->mVariableNames.push_back("m");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.00187018);

    this->mVariableNames.push_back("h");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.9804713);

    this->mVariableNames.push_back("j");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.98767124);

    this->mVariableNames.push_back("d");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.00316354);

    this->mVariableNames.push_back("f");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.99427859);

    this->mVariableNames.push_back("X");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.16647703);

    this->mVariableNames.push_back("Cai");
    this->mVariableUnits.push_back("millimolar");
    this->mInitialConditions.push_back(0.0002);

    this->mInitialised = true;
}


// Needs to be included last
#include "TemplatedExport.hpp"
CHASTE_CLASS_EXPORT(Cellluo_rudy_1991FromCellML)

namespace boost
{
    namespace serialization
    {
        template<class Archive>
        inline void save_construct_data(
            Archive & ar, const Cellluo_rudy_1991FromCellML * t, const unsigned int fileVersion)
        {
            const boost::shared_ptr<AbstractIvpOdeSolver> p_solver = t->GetSolver();
            const boost::shared_ptr<AbstractStimulusFunction> p_stimulus = t->GetStimulusFunction();
            ar << p_solver;
            ar << p_stimulus;
        }
        
        template<class Archive>
        inline void load_construct_data(
            Archive & ar, Cellluo_rudy_1991FromCellML * t, const unsigned int fileVersion)
        {
            boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
            boost::shared_ptr<AbstractStimulusFunction> p_stimulus;
            ar >> p_solver;
            ar >> p_stimulus;
            ::new(t)Cellluo_rudy_1991FromCellML(p_solver, p_stimulus);
        }
        
    }
    
}

#endif // CELLLUO_RUDY_1991FROMCELLML_HPP_
