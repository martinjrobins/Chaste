#ifndef _TESTMONODOMAINPDE_HPP_
#define _TESTMONODOMAINPDE_HPP_

#include <cxxtest/TestSuite.h>
#include <cmath>

// Declare a subfunction to compute the ionic current
double GetIIonic(	double membrane_V,
					double fast_sodium_current_m_gate_m,
					double fast_sodium_current_h_gate_h,
					double fast_sodium_current_j_gate_j,
					double slow_inward_current_d_gate_d,
					double slow_inward_current_f_gate_f,
					double time_dependent_potassium_current_X_gate_X,
					double intracellular_calcium_concentration_Cai);

class TestMonodomainPde : public CxxTest::TestSuite
{
    public:
    
    void testMonodomainPdeConstructor( void )
    {
        int num_nodes=2;
        double big_time_step=1.0;
        //MonodomainPde monodomain_pde(num_nodes, big_time_step);
        
        double v = -9999; // This voltage will be ignored
        double m = 0.0017;
        double h = 0.9833;
        double j = 0.9895;
        double d = 0.003;
        double f = 1;
        double x = 0.0056;
        double caI = 0.0002;
        double magnitudeOfStimulus = -80.0;  
        double durationOfStimulus  = 0.5 ;
        
        double i_stim = 0.0; // Set i_stim to zero by default
        
        double i_ionic;
        double i_total;
        
                  
        std::vector<double> initialConditions;
        initialConditions.push_back(h);
        initialConditions.push_back(j);
        initialConditions.push_back(m);
        initialConditions.push_back(caI);
        initialConditions.push_back(v);
        initialConditions.push_back(d);
        initialConditions.push_back(f);
        initialConditions.push_back(x);
        
        //MonodomainPde.SetUniversalInitialConditions(initialConditions);
        
        AbstractStimulusFunction *pStimulus = new InitialStimulus(magnitudeOfStimulus, durationOfStimulus);
        int node_num = 0;     
        //MonodomainPde.SetStimulusFunctionAtNode(node_num, pStimulus);
                       
         // __________________________________________________________________
        /*
         * If want values for t=1 ms with stimulus, use data below
         */
        v = 4.2094e+01;
        m = 9.9982e-01;
        h = 9.4396e-02; 
        j = 8.7622e-01;
        d = 2.1953e-02;
        f = 9.9889e-01;
        x = 6.8886e-03;
        caI = 1.9979e-04;
        
        magnitudeOfStimulus = -80.0;  
        i_stim = magnitudeOfStimulus;
        
        i_ionic = GetIIonic(v,m,h,j,d,f,x,caI);
        i_total = i_stim + i_ionic;
        
       // TS_ASSERT_DELTA(MonodomainPde.ComputeLinearSourceTermAtNode(node_num, v),i_total,0.000001);
        
         /*
         * If want values for t=1 ms with NO stimulus, use data below
         */          
        v = -8.4512e+01;
        m =  1.6761e-03;
        h = 9.8327e-01; 
        j =  9.8950e-01;
        d = 2.9986e-03;
        f = 1.0000e-00;
        x = 5.6003e-03;
        caI = 1.9854e-04;
                              
        i_ionic = GetIIonic(v,m,h,j,d,f,x,caI);
        i_total = i_stim + i_ionic;
        
        //TS_ASSERT_DELTA(MonodomainPde.ComputeLinearSourceTermAtNode(node_num, v),i_total,0.000001);
    }
};


double GetIIonic(	double membrane_V,
					double fast_sodium_current_m_gate_m,
					double fast_sodium_current_h_gate_h,
					double fast_sodium_current_j_gate_j,
					double slow_inward_current_d_gate_d,
					double slow_inward_current_f_gate_f,
					double time_dependent_potassium_current_X_gate_X,
					double intracellular_calcium_concentration_Cai)
{	
	// Define some constants
			        
   double membrane_F = 96485.0;
   double membrane_R = 8314;
   double membrane_T = 310.0;
   
   double background_current_E_b = -59.87;
   double background_current_g_b = 0.03921;
   
   double fast_sodium_current_g_Na = 23.0;
   double ionic_concentrations_Ki = 145.0;
   double ionic_concentrations_Ko = 5.4;
   double ionic_concentrations_Nai = 18.0;
   double ionic_concentrations_Nao = 140.0;
   
   double fast_sodium_current_E_Na = ((membrane_R * membrane_T) / membrane_F) * 
                              log(ionic_concentrations_Nao / ionic_concentrations_Nai);
   
   double plateau_potassium_current_g_Kp = 0.0183;
   double time_dependent_potassium_current_PR_NaK = 0.01833;
   
   
  double background_current_i_b = background_current_g_b*(membrane_V-background_current_E_b);

   double fast_sodium_current_h_gate_alpha_h;

   if (membrane_V < -40.0)
   {
      fast_sodium_current_h_gate_alpha_h = 0.135*exp((80.0+membrane_V)/-6.8);
   }
   else
   {
      fast_sodium_current_h_gate_alpha_h = 0.0;
   }

   double fast_sodium_current_h_gate_beta_h;

   if (membrane_V < -40.0)
   {
      fast_sodium_current_h_gate_beta_h = 3.56*exp(0.079*membrane_V)+3.1e5*exp(0.35*membrane_V);
   }
   else
   {
      fast_sodium_current_h_gate_beta_h = 1.0/(0.13*(1.0+exp((membrane_V+10.66)/-11.1)));
   }

   double fast_sodium_current_h_gate_h_prime = fast_sodium_current_h_gate_alpha_h*(1.0-fast_sodium_current_h_gate_h)-fast_sodium_current_h_gate_beta_h*fast_sodium_current_h_gate_h;

   double fast_sodium_current_j_gate_alpha_j;

   if (membrane_V < -40.0)
   {
      fast_sodium_current_j_gate_alpha_j = (-1.2714e5*exp(0.2444*membrane_V)-3.474e-5*exp(-0.04391*membrane_V))*(membrane_V+37.78)/(1.0+exp(0.311*(membrane_V+79.23)));
   }
   else
   {
      fast_sodium_current_j_gate_alpha_j = 0.0;
   }

   double fast_sodium_current_j_gate_beta_j;

   if (membrane_V < -40.0)
   {
      fast_sodium_current_j_gate_beta_j = 0.1212*exp(-0.01052*membrane_V)/(1.0+exp(-0.1378*(membrane_V+40.14)));
   }
   else
   {
      fast_sodium_current_j_gate_beta_j = 0.3*exp(-2.535e-7*membrane_V)/(1.0+exp(-0.1*(membrane_V+32.0)));
   }

   double fast_sodium_current_j_gate_j_prime = fast_sodium_current_j_gate_alpha_j*(1.0-fast_sodium_current_j_gate_j)-fast_sodium_current_j_gate_beta_j*fast_sodium_current_j_gate_j;
   double fast_sodium_current_m_gate_alpha_m = 0.32*(membrane_V+47.13)/(1.0-exp(-0.1*(membrane_V+47.13)));
   double fast_sodium_current_m_gate_beta_m = 0.08*exp(-membrane_V/11.0);
   double fast_sodium_current_m_gate_m_prime = fast_sodium_current_m_gate_alpha_m*(1.0-fast_sodium_current_m_gate_m)-fast_sodium_current_m_gate_beta_m*fast_sodium_current_m_gate_m;
   double fast_sodium_current_i_Na = fast_sodium_current_g_Na*pow(fast_sodium_current_m_gate_m, 3.0)*fast_sodium_current_h_gate_h*fast_sodium_current_j_gate_j*(membrane_V-fast_sodium_current_E_Na);
 
   double slow_inward_current_d_gate_alpha_d = 0.095*exp(-0.01*(membrane_V-5.0))/(1.0+exp(-0.072*(membrane_V-5.0)));
   double slow_inward_current_d_gate_beta_d = 0.07*exp(-0.017*(membrane_V+44.0))/(1.0+exp(0.05*(membrane_V+44.0)));
   double slow_inward_current_d_gate_d_prime = slow_inward_current_d_gate_alpha_d*(1.0-slow_inward_current_d_gate_d)-slow_inward_current_d_gate_beta_d*slow_inward_current_d_gate_d;
   
   double slow_inward_current_f_gate_alpha_f = 0.012*exp(-0.008*(membrane_V+28.0))/(1.0+exp(0.15*(membrane_V+28.0)));
   double slow_inward_current_f_gate_beta_f = 0.0065*exp(-0.02*(membrane_V+30.0))/(1.0+exp(-0.2*(membrane_V+30.0)));
   double slow_inward_current_f_gate_f_prime = slow_inward_current_f_gate_alpha_f*(1.0-slow_inward_current_f_gate_f)-slow_inward_current_f_gate_beta_f*slow_inward_current_f_gate_f;
   
   double slow_inward_current_E_si = 7.7-13.0287*log(intracellular_calcium_concentration_Cai);
   double slow_inward_current_i_si = 0.09*slow_inward_current_d_gate_d*slow_inward_current_f_gate_f*(membrane_V-slow_inward_current_E_si);
   double intracellular_calcium_concentration_Cai_prime = -1e-4*slow_inward_current_i_si+0.07*(1e-4-intracellular_calcium_concentration_Cai);
   double time_dependent_potassium_current_g_K = 0.282*sqrt(ionic_concentrations_Ko/5.4);

   double time_dependent_potassium_current_Xi_gate_Xi;

   if (membrane_V > -100.0)
   {
      time_dependent_potassium_current_Xi_gate_Xi = 2.837*(exp(0.04*(membrane_V+77.0))-1.0)/((membrane_V+77.0)*exp(0.04*(membrane_V+35.0)));
   }
   else
   {
      time_dependent_potassium_current_Xi_gate_Xi = 1.0;
   }
   
   double time_dependent_potassium_current_X_gate_alpha_X = 0.0005*exp(0.083*(membrane_V+50.0))/(1.0+exp(0.057*(membrane_V+50.0)));
   double time_dependent_potassium_current_X_gate_beta_X = 0.0013*exp(-0.06*(membrane_V+20.0))/(1.0+exp(-0.04*(membrane_V+20.0)));
   double time_dependent_potassium_current_X_gate_X_prime = time_dependent_potassium_current_X_gate_alpha_X*(1.0-time_dependent_potassium_current_X_gate_X)-time_dependent_potassium_current_X_gate_beta_X*time_dependent_potassium_current_X_gate_X;
 
   double time_dependent_potassium_current_E_K = ((membrane_R*membrane_T)/membrane_F)*log((ionic_concentrations_Ko+time_dependent_potassium_current_PR_NaK*ionic_concentrations_Nao)/(ionic_concentrations_Ki+time_dependent_potassium_current_PR_NaK*ionic_concentrations_Nai));
   double time_dependent_potassium_current_i_K = time_dependent_potassium_current_g_K*time_dependent_potassium_current_X_gate_X*time_dependent_potassium_current_Xi_gate_Xi*(membrane_V-time_dependent_potassium_current_E_K);
   double time_independent_potassium_current_g_K1 = 0.6047*sqrt(ionic_concentrations_Ko/5.4);
   double time_independent_potassium_current_E_K1 =((membrane_R*membrane_T)/membrane_F)*log(ionic_concentrations_Ko/ionic_concentrations_Ki);
   double time_independent_potassium_current_K1_gate_alpha_K1 = 1.02/(1.0+exp(0.2385*(membrane_V-time_independent_potassium_current_E_K1-59.215)));
   double time_independent_potassium_current_K1_gate_beta_K1 = (0.49124*exp(0.08032*(membrane_V+5.476-time_independent_potassium_current_E_K1))+exp(0.06175*(membrane_V-(time_independent_potassium_current_E_K1+594.31))))/(1.0+exp(-0.5143*(membrane_V-time_independent_potassium_current_E_K1+4.753)));
   double time_independent_potassium_current_K1_gate_K1_infinity = time_independent_potassium_current_K1_gate_alpha_K1/(time_independent_potassium_current_K1_gate_alpha_K1+time_independent_potassium_current_K1_gate_beta_K1);
   double time_independent_potassium_current_i_K1 = time_independent_potassium_current_g_K1*time_independent_potassium_current_K1_gate_K1_infinity*(membrane_V-time_independent_potassium_current_E_K1);
   double plateau_potassium_current_Kp = 1.0/(1.0+exp((7.488-membrane_V)/5.98));
   double plateau_potassium_current_E_Kp = time_independent_potassium_current_E_K1;
   double plateau_potassium_current_i_Kp = plateau_potassium_current_g_Kp*plateau_potassium_current_Kp*(membrane_V-plateau_potassium_current_E_Kp);
   double i_ionic = fast_sodium_current_i_Na+slow_inward_current_i_si+time_dependent_potassium_current_i_K+time_independent_potassium_current_i_K1+plateau_potassium_current_i_Kp+background_current_i_b;
   return i_ionic;
}

#endif //_TESTMONODOMAINPDE_HPP_
