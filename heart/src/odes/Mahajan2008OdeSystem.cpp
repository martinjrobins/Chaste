// Model: mahajan_shiferaw_2008_version01
// Processed by pycml - CellML Tools in Python
//     (translate: 5150, pycml: 5150)
// on Tue Mar 10 16:09:35 2009

#include <cmath>
#include <cassert>
#include "AbstractCardiacCell.hpp"
#include "Exception.hpp"
#include "OdeSystemInformation.hpp"
#include "AbstractStimulusFunction.hpp"
#include "Mahajan2008OdeSystem.hpp"

Mahajan2008OdeSystem::Mahajan2008OdeSystem(AbstractIvpOdeSolver *pSolver, 
                                        AbstractStimulusFunction* pIntracellularStimulus)
        : AbstractCardiacCell(pSolver, 26, 0, pIntracellularStimulus)
    {
        // Time units: ms
        // 
        mpSystemInfo = OdeSystemInformation<Mahajan2008OdeSystem>::Instance();
        Init();
    }
    
    Mahajan2008OdeSystem::~Mahajan2008OdeSystem(void)
    {
    }
    
 //   void Mahajan2008OdeSystem::VerifyGatingVariables()
 //   {}

    double Mahajan2008OdeSystem::GetIIonic()
    {
        std::vector<double>& rY = rGetStateVariables();
        double var_cell__V = rY[0];
        // Units: mV; Initial value: -87.169816169406
        double var_INa__xm = rY[1];
        // Units: dimensionless; Initial value: 0.001075453357
        double var_INa__xh = rY[2];
        // Units: dimensionless; Initial value: 0.990691306716
        double var_INa__xj = rY[3];
        // Units: dimensionless; Initial value: 0.993888937283
        double var_ICaL__c1 = rY[4];
        // Units: dimensionless; Initial value: 0.000018211252
        double var_ICaL__c2 = rY[5];
        // Units: dimensionless; Initial value: 0.979322592773
        double var_ICaL__xi1ca = rY[6];
        // Units: dimensionless; Initial value: 0.001208153482
        double var_ICaL__xi1ba = rY[7];
        // Units: dimensionless; Initial value: 0.000033616596
        double var_ICaL__xi2ca = rY[8];
        // Units: dimensionless; Initial value: 0.004173008466
        double var_ICaL__xi2ba = rY[9];
        // Units: dimensionless; Initial value: 0.015242594688
        double var_IKr__xr = rY[10];
        // Units: dimensionless; Initial value: 0.007074239331
        double var_IKs__xs1 = rY[11];
        // Units: dimensionless; Initial value: 0.048267587131
        double var_IKs__xs2 = rY[12];
        // Units: dimensionless; Initial value: 0.105468807033
        double var_Ito__xtos = rY[13];
        // Units: dimensionless; Initial value: 0.00364776906
        double var_Ito__ytos = rY[14];
        // Units: dimensionless; Initial value: 0.174403618112
        double var_Ito__xtof = rY[15];
        // Units: dimensionless; Initial value: 0.003643592594
        double var_Ito__ytof = rY[16];
        // Units: dimensionless; Initial value: 0.993331326442
        double var_Na__Na_i = rY[19];
        // Units: mM; Initial value: 11.441712311614
        double var_Ca__Ca_submem = rY[21];
        // Units: uM; Initial value: 0.226941113355
        double var_Ca__Ca_i = rY[22];
        // Units: uM; Initial value: 0.256752008084
        
        const double var_Environment__R = 8.314472;
        const double var_Environment__T = 308.0;
        const double var_Environment__F = 96.4853415;
        const double var_Environment__K_o = 5.4;
        const double var_Environment__Ca_o = 1.8;
        const double var_Environment__Na_o = 136.0;
        double var_Environment__FonRT = var_Environment__F / (var_Environment__R * var_Environment__T);
        const double var_reversal_potentials__K_i = 140.0;
        double var_reversal_potentials__FonRT = var_Environment__FonRT;
        double var_reversal_potentials__K_o = var_Environment__K_o;
        double var_reversal_potentials__ek = (1.0 / var_reversal_potentials__FonRT) * log(var_reversal_potentials__K_o / var_reversal_potentials__K_i);
        double var_IK1__ek = var_reversal_potentials__ek;
        double var_IK1__V = var_cell__V;
        double var_IK1__aki = 1.02 / (1.0 + exp(0.2385 * ((var_IK1__V - var_IK1__ek) - 59.215)));
        double var_IK1__bki = ((0.49124 * exp(0.08032 * ((var_IK1__V - var_IK1__ek) + 5.476))) + (1.0 * exp(0.06175 * ((var_IK1__V - var_IK1__ek) - 594.31)))) / (1.0 + exp((-0.5143) * ((var_IK1__V - var_IK1__ek) + 4.753)));
        double var_IK1__xkin = var_IK1__aki / (var_IK1__aki + var_IK1__bki);
        double var_IK1__K_o = var_Environment__K_o;
        const double var_IK1__gkix = 0.3;
        double var_IK1__xik1 = var_IK1__gkix * sqrt(var_IK1__K_o / 5.4) * var_IK1__xkin * (var_IK1__V - var_IK1__ek);
        double var_cell__xik1 = var_IK1__xik1;
        double var_Ito__ek = var_reversal_potentials__ek;
        const double var_Ito__gtos = 0.04;
        double var_Ito__V = var_cell__V;
        double var_Ito__rt2 = (var_Ito__V + 33.5) / 10.0;
        double var_Ito__rs_inf = 1.0 / (1.0 + exp(var_Ito__rt2));
        double var_Ito__xitos = var_Ito__gtos * var_Ito__xtos * (var_Ito__ytos + (0.5 * var_Ito__rs_inf)) * (var_Ito__V - var_Ito__ek);
        const double var_Ito__gtof = 0.11;
        double var_Ito__xitof = var_Ito__gtof * var_Ito__xtof * var_Ito__ytof * (var_Ito__V - var_Ito__ek);
        double var_Ito__xito = var_Ito__xitos + var_Ito__xitof;
        double var_cell__xito = var_Ito__xito;
        const double var_INaK__xkmko = 1.5;
        double var_INaK__V = var_cell__V;
        double var_INaK__FonRT = var_Environment__FonRT;
        double var_INaK__Na_o = var_Environment__Na_o;
        double var_INaK__sigma = (exp(var_INaK__Na_o / 67.3) - 1.0) / 7.0;
        double var_INaK__fNaK = 1.0 / (1.0 + (0.1245 * exp((-0.1) * var_INaK__V * var_INaK__FonRT)) + (0.0365 * var_INaK__sigma * exp((-var_INaK__V) * var_INaK__FonRT)));
        double var_INaK__Na_i = var_Na__Na_i;
        double var_INaK__K_o = var_Environment__K_o;
        const double var_INaK__xkmnai = 12.0;
        const double var_INaK__gNaK = 1.5;
        double var_INaK__xiNaK = (((var_INaK__gNaK * var_INaK__fNaK * var_INaK__Na_i) / (var_INaK__Na_i + var_INaK__xkmnai)) * var_INaK__K_o) / (var_INaK__K_o + var_INaK__xkmko);
        double var_cell__xiNaK = var_INaK__xiNaK;
        const double var_cell__wca = 8.0;
        const double var_INaCa__gNaCa = 0.84;
        double var_INaCa__V = var_cell__V;
        double var_INaCa__FonRT = var_Environment__FonRT;
        double var_INaCa__zw4 = 1.0 + (0.2 * exp(var_INaCa__V * (0.35 - 1.0) * var_INaCa__FonRT));
        const double var_INaCa__xkdna = 0.3;
        double var_INaCa__Ca_submem = var_Ca__Ca_submem;
        double var_INaCa__aloss = 1.0 / (1.0 + pow(var_INaCa__xkdna / var_INaCa__Ca_submem, 3.0));
        double var_INaCa__Na_o = var_Environment__Na_o;
        double var_INaCa__Na_i = var_Na__Na_i;
        double var_INaCa__Ca_o = var_Environment__Ca_o;
        double var_Ca__csm = var_Ca__Ca_submem / 1000.0;
        double var_INaCa__csm = var_Ca__csm;
        double var_INaCa__yz4 = (pow(var_INaCa__Na_i, 3.0) * var_INaCa__Ca_o) + (pow(var_INaCa__Na_o, 3.0) * var_INaCa__csm);
        const double var_INaCa__xmcao = 1.3;
        const double var_INaCa__xmnao = 87.5;
        double var_INaCa__yz1 = (var_INaCa__xmcao * pow(var_INaCa__Na_i, 3.0)) + (pow(var_INaCa__xmnao, 3.0) * var_INaCa__csm);
        const double var_INaCa__xmcai = 0.0036;
        const double var_INaCa__xmnai = 12.3;
        double var_INaCa__yz3 = var_INaCa__xmcai * pow(var_INaCa__Na_o, 3.0) * (1.0 + pow(var_INaCa__Na_i / var_INaCa__xmnai, 3.0));
        double var_INaCa__yz2 = pow(var_INaCa__xmnai, 3.0) * var_INaCa__Ca_o * (1.0 + (var_INaCa__csm / var_INaCa__xmcai));
        double var_INaCa__zw8 = var_INaCa__yz1 + var_INaCa__yz2 + var_INaCa__yz3 + var_INaCa__yz4;
        double var_INaCa__zw3 = (pow(var_INaCa__Na_i, 3.0) * var_INaCa__Ca_o * exp(var_INaCa__V * 0.35 * var_INaCa__FonRT)) - (pow(var_INaCa__Na_o, 3.0) * var_INaCa__csm * exp(var_INaCa__V * (0.35 - 1.0) * var_INaCa__FonRT));
        double var_INaCa__jNaCa = (var_INaCa__gNaCa * var_INaCa__aloss * var_INaCa__zw3) / (var_INaCa__zw4 * var_INaCa__zw8);
        double var_INaCa__wca = var_cell__wca;
        double var_INaCa__xiNaCa = var_INaCa__wca * var_INaCa__jNaCa;
        double var_cell__xiNaCa = var_INaCa__xiNaCa;
        double var_ICaL__Ca_o = var_Environment__Ca_o;
        double var_ICaL__V = var_cell__V;
        const double var_ICaL__pca = 0.00054;
        double var_ICaL__FonRT = var_Environment__FonRT;
        double var_ICaL__F = var_Environment__F;
        double var_ICaL__za = var_ICaL__V * 2.0 * var_ICaL__FonRT;
        double var_ICaL__csm = var_Ca__csm;
        double var_ICaL__rxa = (fabs(var_ICaL__za) < 0.001) ? ((4.0 * var_ICaL__pca * var_ICaL__F * var_ICaL__FonRT * ((var_ICaL__csm * exp(var_ICaL__za)) - (0.341 * var_ICaL__Ca_o))) / (2.0 * var_ICaL__FonRT)) : ((4.0 * var_ICaL__pca * var_ICaL__V * var_ICaL__F * var_ICaL__FonRT * ((var_ICaL__csm * exp(var_ICaL__za)) - (0.341 * var_ICaL__Ca_o))) / (exp(var_ICaL__za) - 1.0));
        const double var_ICaL__gca = 182.0;
        double var_ICaL__po = (((((1.0 - var_ICaL__xi1ca) - var_ICaL__xi2ca) - var_ICaL__xi1ba) - var_ICaL__xi2ba) - var_ICaL__c1) - var_ICaL__c2;
        double var_ICaL__jca = var_ICaL__gca * var_ICaL__po * var_ICaL__rxa;
        double var_ICaL__wca = var_cell__wca;
        double var_ICaL__xica = 2.0 * var_ICaL__wca * var_ICaL__jca;
        double var_cell__xica = var_ICaL__xica;
        const double var_IKr__gkr = 0.0125;
        double var_IKr__K_o = var_Environment__K_o;
        double var_IKr__V = var_cell__V;
        double var_IKr__rg = 1.0 / (1.0 + exp((var_IKr__V + 33.0) / 22.4));
        double var_IKr__ek = var_reversal_potentials__ek;
        double var_IKr__xikr = var_IKr__gkr * sqrt(var_IKr__K_o / 5.4) * var_IKr__xr * var_IKr__rg * (var_IKr__V - var_IKr__ek);
        double var_cell__xikr = var_IKr__xikr;
        double var_IKs__Ca_i = var_Ca__Ca_i;
        double var_IKs__gksx = 1.0 + (0.8 / (1.0 + pow(0.5 / var_IKs__Ca_i, 3.0)));
        const double var_IKs__gks = 0.1386;
        double var_IKs__V = var_cell__V;
        const double var_reversal_potentials__prNaK = 0.01833;
        double var_reversal_potentials__Na_o = var_Environment__Na_o;
        double var_reversal_potentials__Na_i = var_Na__Na_i;
        double var_reversal_potentials__eks = (1.0 / var_reversal_potentials__FonRT) * log((var_reversal_potentials__K_o + (var_reversal_potentials__prNaK * var_reversal_potentials__Na_o)) / (var_reversal_potentials__K_i + (var_reversal_potentials__prNaK * var_reversal_potentials__Na_i)));
        double var_IKs__eks = var_reversal_potentials__eks;
        double var_IKs__xiks = var_IKs__gks * var_IKs__gksx * var_IKs__xs1 * var_IKs__xs2 * (var_IKs__V - var_IKs__eks);
        double var_cell__xiks = var_IKs__xiks;
        double var_INa__V = var_cell__V;
        const double var_INa__gna = 12.0;
        double var_reversal_potentials__ena = (1.0 / var_reversal_potentials__FonRT) * log(var_reversal_potentials__Na_o / var_reversal_potentials__Na_i);
        double var_INa__ena = var_reversal_potentials__ena;
        double var_INa__xina = var_INa__gna * var_INa__xh * var_INa__xj * var_INa__xm * var_INa__xm * var_INa__xm * (var_INa__V - var_INa__ena);
        double var_cell__xina = var_INa__xina;
        
        return var_cell__xik1+var_cell__xito+var_cell__xiNaK+var_cell__xiNaCa+var_cell__xica+var_cell__xina+var_cell__xikr+var_cell__xiks;
    }
    
    void Mahajan2008OdeSystem::EvaluateYDerivatives(
            double var_Environment__time,
            const std::vector<double> &rY,
            std::vector<double> &rDY)
    {
        // Inputs:
        // Time units: ms
        var_Environment__time *= 1.0;
        double var_cell__V = rY[0];
        // Units: mV; Initial value: -87.169816169406
        double var_INa__xm = rY[1];
        // Units: dimensionless; Initial value: 0.001075453357
        double var_INa__xh = rY[2];
        // Units: dimensionless; Initial value: 0.990691306716
        double var_INa__xj = rY[3];
        // Units: dimensionless; Initial value: 0.993888937283
        double var_ICaL__c1 = rY[4];
        // Units: dimensionless; Initial value: 0.000018211252
        double var_ICaL__c2 = rY[5];
        // Units: dimensionless; Initial value: 0.979322592773
        double var_ICaL__xi1ca = rY[6];
        // Units: dimensionless; Initial value: 0.001208153482
        double var_ICaL__xi1ba = rY[7];
        // Units: dimensionless; Initial value: 0.000033616596
        double var_ICaL__xi2ca = rY[8];
        // Units: dimensionless; Initial value: 0.004173008466
        double var_ICaL__xi2ba = rY[9];
        // Units: dimensionless; Initial value: 0.015242594688
        double var_IKr__xr = rY[10];
        // Units: dimensionless; Initial value: 0.007074239331
        double var_IKs__xs1 = rY[11];
        // Units: dimensionless; Initial value: 0.048267587131
        double var_IKs__xs2 = rY[12];
        // Units: dimensionless; Initial value: 0.105468807033
        double var_Ito__xtos = rY[13];
        // Units: dimensionless; Initial value: 0.00364776906
        double var_Ito__ytos = rY[14];
        // Units: dimensionless; Initial value: 0.174403618112
        double var_Ito__xtof = rY[15];
        // Units: dimensionless; Initial value: 0.003643592594
        double var_Ito__ytof = rY[16];
        // Units: dimensionless; Initial value: 0.993331326442
        double var_Irel__Ca_JSR = rY[17];
        // Units: uM; Initial value: 97.505463697266
        double var_Irel__xir = rY[18];
        // Units: uM_per_ms; Initial value: 0.006679257264
        double var_Na__Na_i = rY[19];
        // Units: mM; Initial value: 11.441712311614
        double var_Ca__Ca_dyad = rY[20];
        // Units: uM; Initial value: 1.716573130685
        double var_Ca__Ca_submem = rY[21];
        // Units: uM; Initial value: 0.226941113355
        double var_Ca__Ca_i = rY[22];
        // Units: uM; Initial value: 0.256752008084
        double var_Ca__Ca_NSR = rY[23];
        // Units: uM; Initial value: 104.450004990523
        double var_Ca__tropi = rY[24];
        // Units: uM; Initial value: 22.171689894953
        double var_Ca__trops = rY[25];
        // Units: uM; Initial value: 19.864701949854
        
        
        // Mathematics
        const double var_Environment__R = 8.314472;
        const double var_Environment__T = 308.0;
        const double var_Environment__F = 96.4853415;
        const double var_Environment__K_o = 5.4;
        const double var_Environment__Ca_o = 1.8;
        const double var_Environment__Na_o = 136.0;
        double var_Environment__FonRT = var_Environment__F / (var_Environment__R * var_Environment__T);
        const double var_reversal_potentials__K_i = 140.0;
        double var_reversal_potentials__FonRT = var_Environment__FonRT;
        double var_reversal_potentials__K_o = var_Environment__K_o;
        double var_reversal_potentials__ek = (1.0 / var_reversal_potentials__FonRT) * log(var_reversal_potentials__K_o / var_reversal_potentials__K_i);
        double var_IK1__ek = var_reversal_potentials__ek;
        double var_IK1__V = var_cell__V;
        double var_IK1__aki = 1.02 / (1.0 + exp(0.2385 * ((var_IK1__V - var_IK1__ek) - 59.215)));
        double var_IK1__bki = ((0.49124 * exp(0.08032 * ((var_IK1__V - var_IK1__ek) + 5.476))) + (1.0 * exp(0.06175 * ((var_IK1__V - var_IK1__ek) - 594.31)))) / (1.0 + exp((-0.5143) * ((var_IK1__V - var_IK1__ek) + 4.753)));
        double var_IK1__xkin = var_IK1__aki / (var_IK1__aki + var_IK1__bki);
        double var_IK1__K_o = var_Environment__K_o;
        const double var_IK1__gkix = 0.3;
        double var_IK1__xik1 = var_IK1__gkix * sqrt(var_IK1__K_o / 5.4) * var_IK1__xkin * (var_IK1__V - var_IK1__ek);
        double var_cell__xik1 = var_IK1__xik1;
        double var_Ito__ek = var_reversal_potentials__ek;
        const double var_Ito__gtos = 0.04;
        double var_Ito__V = var_cell__V;
        double var_Ito__rt2 = (var_Ito__V + 33.5) / 10.0;
        double var_Ito__rs_inf = 1.0 / (1.0 + exp(var_Ito__rt2));
        double var_Ito__xitos = var_Ito__gtos * var_Ito__xtos * (var_Ito__ytos + (0.5 * var_Ito__rs_inf)) * (var_Ito__V - var_Ito__ek);
        const double var_Ito__gtof = 0.11;
        double var_Ito__xitof = var_Ito__gtof * var_Ito__xtof * var_Ito__ytof * (var_Ito__V - var_Ito__ek);
        double var_Ito__xito = var_Ito__xitos + var_Ito__xitof;
        double var_cell__xito = var_Ito__xito;
        const double var_INaK__xkmko = 1.5;
        double var_INaK__V = var_cell__V;
        double var_INaK__FonRT = var_Environment__FonRT;
        double var_INaK__Na_o = var_Environment__Na_o;
        double var_INaK__sigma = (exp(var_INaK__Na_o / 67.3) - 1.0) / 7.0;
        double var_INaK__fNaK = 1.0 / (1.0 + (0.1245 * exp((-0.1) * var_INaK__V * var_INaK__FonRT)) + (0.0365 * var_INaK__sigma * exp((-var_INaK__V) * var_INaK__FonRT)));
        double var_INaK__Na_i = var_Na__Na_i;
        double var_INaK__K_o = var_Environment__K_o;
        const double var_INaK__xkmnai = 12.0;
        const double var_INaK__gNaK = 1.5;
        double var_INaK__xiNaK = (((var_INaK__gNaK * var_INaK__fNaK * var_INaK__Na_i) / (var_INaK__Na_i + var_INaK__xkmnai)) * var_INaK__K_o) / (var_INaK__K_o + var_INaK__xkmko);
        double var_cell__xiNaK = var_INaK__xiNaK;
        const double var_cell__wca = 8.0;
        const double var_INaCa__gNaCa = 0.84;
        double var_INaCa__V = var_cell__V;
        double var_INaCa__FonRT = var_Environment__FonRT;
        double var_INaCa__zw4 = 1.0 + (0.2 * exp(var_INaCa__V * (0.35 - 1.0) * var_INaCa__FonRT));
        const double var_INaCa__xkdna = 0.3;
        double var_INaCa__Ca_submem = var_Ca__Ca_submem;
        double var_INaCa__aloss = 1.0 / (1.0 + pow(var_INaCa__xkdna / var_INaCa__Ca_submem, 3.0));
        double var_INaCa__Na_o = var_Environment__Na_o;
        double var_INaCa__Na_i = var_Na__Na_i;
        double var_INaCa__Ca_o = var_Environment__Ca_o;
        double var_Ca__csm = var_Ca__Ca_submem / 1000.0;
        double var_INaCa__csm = var_Ca__csm;
        double var_INaCa__yz4 = (pow(var_INaCa__Na_i, 3.0) * var_INaCa__Ca_o) + (pow(var_INaCa__Na_o, 3.0) * var_INaCa__csm);
        const double var_INaCa__xmcao = 1.3;
        const double var_INaCa__xmnao = 87.5;
        double var_INaCa__yz1 = (var_INaCa__xmcao * pow(var_INaCa__Na_i, 3.0)) + (pow(var_INaCa__xmnao, 3.0) * var_INaCa__csm);
        const double var_INaCa__xmcai = 0.0036;
        const double var_INaCa__xmnai = 12.3;
        double var_INaCa__yz3 = var_INaCa__xmcai * pow(var_INaCa__Na_o, 3.0) * (1.0 + pow(var_INaCa__Na_i / var_INaCa__xmnai, 3.0));
        double var_INaCa__yz2 = pow(var_INaCa__xmnai, 3.0) * var_INaCa__Ca_o * (1.0 + (var_INaCa__csm / var_INaCa__xmcai));
        double var_INaCa__zw8 = var_INaCa__yz1 + var_INaCa__yz2 + var_INaCa__yz3 + var_INaCa__yz4;
        double var_INaCa__zw3 = (pow(var_INaCa__Na_i, 3.0) * var_INaCa__Ca_o * exp(var_INaCa__V * 0.35 * var_INaCa__FonRT)) - (pow(var_INaCa__Na_o, 3.0) * var_INaCa__csm * exp(var_INaCa__V * (0.35 - 1.0) * var_INaCa__FonRT));
        double var_INaCa__jNaCa = (var_INaCa__gNaCa * var_INaCa__aloss * var_INaCa__zw3) / (var_INaCa__zw4 * var_INaCa__zw8);
        double var_INaCa__wca = var_cell__wca;
        double var_INaCa__xiNaCa = var_INaCa__wca * var_INaCa__jNaCa;
        double var_cell__xiNaCa = var_INaCa__xiNaCa;
        double var_ICaL__Ca_o = var_Environment__Ca_o;
        double var_ICaL__V = var_cell__V;
        const double var_ICaL__pca = 0.00054;
        double var_ICaL__FonRT = var_Environment__FonRT;
        double var_ICaL__F = var_Environment__F;
        double var_ICaL__za = var_ICaL__V * 2.0 * var_ICaL__FonRT;
        double var_ICaL__csm = var_Ca__csm;
        double var_ICaL__rxa = (fabs(var_ICaL__za) < 0.001) ? ((4.0 * var_ICaL__pca * var_ICaL__F * var_ICaL__FonRT * ((var_ICaL__csm * exp(var_ICaL__za)) - (0.341 * var_ICaL__Ca_o))) / (2.0 * var_ICaL__FonRT)) : ((4.0 * var_ICaL__pca * var_ICaL__V * var_ICaL__F * var_ICaL__FonRT * ((var_ICaL__csm * exp(var_ICaL__za)) - (0.341 * var_ICaL__Ca_o))) / (exp(var_ICaL__za) - 1.0));
        const double var_ICaL__gca = 182.0;
        double var_ICaL__po = (((((1.0 - var_ICaL__xi1ca) - var_ICaL__xi2ca) - var_ICaL__xi1ba) - var_ICaL__xi2ba) - var_ICaL__c1) - var_ICaL__c2;
        double var_ICaL__jca = var_ICaL__gca * var_ICaL__po * var_ICaL__rxa;
        double var_ICaL__wca = var_cell__wca;
        double var_ICaL__xica = 2.0 * var_ICaL__wca * var_ICaL__jca;
        double var_cell__xica = var_ICaL__xica;
        const double var_IKr__gkr = 0.0125;
        double var_IKr__K_o = var_Environment__K_o;
        double var_IKr__V = var_cell__V;
        double var_IKr__rg = 1.0 / (1.0 + exp((var_IKr__V + 33.0) / 22.4));
        double var_IKr__ek = var_reversal_potentials__ek;
        double var_IKr__xikr = var_IKr__gkr * sqrt(var_IKr__K_o / 5.4) * var_IKr__xr * var_IKr__rg * (var_IKr__V - var_IKr__ek);
        double var_cell__xikr = var_IKr__xikr;
        double var_IKs__Ca_i = var_Ca__Ca_i;
        double var_IKs__gksx = 1.0 + (0.8 / (1.0 + pow(0.5 / var_IKs__Ca_i, 3.0)));
        const double var_IKs__gks = 0.1386;
        double var_IKs__V = var_cell__V;
        const double var_reversal_potentials__prNaK = 0.01833;
        double var_reversal_potentials__Na_o = var_Environment__Na_o;
        double var_reversal_potentials__Na_i = var_Na__Na_i;
        double var_reversal_potentials__eks = (1.0 / var_reversal_potentials__FonRT) * log((var_reversal_potentials__K_o + (var_reversal_potentials__prNaK * var_reversal_potentials__Na_o)) / (var_reversal_potentials__K_i + (var_reversal_potentials__prNaK * var_reversal_potentials__Na_i)));
        double var_IKs__eks = var_reversal_potentials__eks;
        double var_IKs__xiks = var_IKs__gks * var_IKs__gksx * var_IKs__xs1 * var_IKs__xs2 * (var_IKs__V - var_IKs__eks);
        double var_cell__xiks = var_IKs__xiks;
        double var_cell__i_Stim = GetStimulus((1.0/1)*var_Environment__time);
        double var_INa__V = var_cell__V;
        const double var_INa__gna = 12.0;
        double var_reversal_potentials__ena = (1.0 / var_reversal_potentials__FonRT) * log(var_reversal_potentials__Na_o / var_reversal_potentials__Na_i);
        double var_INa__ena = var_reversal_potentials__ena;
        double var_INa__xina = var_INa__gna * var_INa__xh * var_INa__xj * var_INa__xm * var_INa__xm * var_INa__xm * (var_INa__V - var_INa__ena);
        double var_cell__xina = var_INa__xina;
        double var_cell__Itotal = -(var_cell__xina + var_cell__xik1 + var_cell__xikr + var_cell__xiks + var_cell__xito + var_cell__xiNaCa + var_cell__xica + var_cell__xiNaK + var_cell__i_Stim);
        double var_INa__am = (fabs(var_INa__V + 47.13) > 0.001) ? ((0.32 * 1.0 * (var_INa__V + 47.13)) / (1.0 - exp((-0.1) * (var_INa__V + 47.13)))) : 3.2;
        double var_INa__bm = 0.08 * exp((-var_INa__V) / 11.0);
        double var_INa__ah = (var_INa__V < (-40.0)) ? (0.135 * exp((80.0 + var_INa__V) / (-6.8))) : 0.0;
        double var_INa__bh = (var_INa__V < (-40.0)) ? ((3.56 * exp(0.079 * var_INa__V)) + (310000.0 * exp(0.35 * var_INa__V))) : (1.0 / (0.13 * (1.0 + exp((var_INa__V + 10.66) / (-11.1)))));
        double var_INa__aj = (var_INa__V < (-40.0)) ? (((((-127140.0) * exp(0.2444 * var_INa__V)) - (3.474e-05 * exp((-0.04391) * var_INa__V))) * 1.0 * (var_INa__V + 37.78)) / (1.0 + exp(0.311 * (var_INa__V + 79.23)))) : 0.0;
        double var_INa__bj = (var_INa__V < (-40.0)) ? ((0.1212 * exp((-0.01052) * var_INa__V)) / (1.0 + exp((-0.1378) * (var_INa__V + 40.14)))) : ((0.3 * exp((-2.535e-07) * var_INa__V)) / (1.0 + exp((-0.1) * (var_INa__V + 32.0))));
        double var_ICaL__Ca_dyad = var_Ca__Ca_dyad;
        const double var_ICaL__vth = 0.0;
        const double var_ICaL__s6 = 8.0;
        double var_ICaL__poinf = 1.0 / (1.0 + exp((-(var_ICaL__V - var_ICaL__vth)) / var_ICaL__s6));
        const double var_ICaL__cat = 3.0;
        double var_ICaL__fca = 1.0 / (1.0 + pow(var_ICaL__cat / var_ICaL__Ca_dyad, 3.0));
        const double var_ICaL__vx =  -40.0;
        const double var_ICaL__sx = 3.0;
        const double var_ICaL__vy =  -40.0;
        const double var_ICaL__sy = 4.0;
        const double var_ICaL__vyr =  -40.0;
        const double var_ICaL__syr = 11.32;
        const double var_ICaL__cpt = 6.09365;
        const double var_ICaL__taupo = 1.0;
        double var_ICaL__alpha = var_ICaL__poinf / var_ICaL__taupo;
        double var_ICaL__beta = (1.0 - var_ICaL__poinf) / var_ICaL__taupo;
        double var_ICaL__k1 = 0.024168 * var_ICaL__fca;
        const double var_ICaL__k2 = 0.000103615;
        const double var_ICaL__k1t = 0.00413;
        const double var_ICaL__k2t = 0.00224;
        double var_ICaL__poi = 1.0 / (1.0 + exp((-(var_ICaL__V - var_ICaL__vx)) / var_ICaL__sx));
        const double var_ICaL__tau3 = 3.0;
        double var_ICaL__k3 = (1.0 - var_ICaL__poi) / var_ICaL__tau3;
        double var_ICaL__k3t = var_ICaL__k3;
        double var_ICaL__Ps = 1.0 / (1.0 + exp((-(var_ICaL__V - var_ICaL__vyr)) / var_ICaL__syr));
        const double var_ICaL__tca = 78.0329;
        double var_ICaL__tau_ca = (var_ICaL__tca / (1.0 + pow(var_ICaL__Ca_dyad / var_ICaL__cpt, 4.0))) + 0.1;
        double var_ICaL__recov = 10.0 + (4954.0 * exp(var_ICaL__V / 15.6));
        double var_ICaL__Pr = 1.0 - (1.0 / (1.0 + exp((-(var_ICaL__V - var_ICaL__vy)) / var_ICaL__sy)));
        double var_ICaL__tauca = ((var_ICaL__recov - var_ICaL__tau_ca) * var_ICaL__Pr) + var_ICaL__tau_ca;
        double var_ICaL__k6 = (var_ICaL__fca * var_ICaL__Ps) / var_ICaL__tauca;
        double var_ICaL__k5 = (1.0 - var_ICaL__Ps) / var_ICaL__tauca;
        double var_ICaL__tauba = ((var_ICaL__recov - 450.0) * var_ICaL__Pr) + 450.0;
        double var_ICaL__k6t = var_ICaL__Ps / var_ICaL__tauba;
        double var_ICaL__k5t = (1.0 - var_ICaL__Ps) / var_ICaL__tauba;
        double var_ICaL__k4 = (((((var_ICaL__k3 * var_ICaL__alpha) / var_ICaL__beta) * var_ICaL__k1) / var_ICaL__k2) * var_ICaL__k5) / var_ICaL__k6;
        double var_ICaL__k4t = (((((var_ICaL__k3t * var_ICaL__alpha) / var_ICaL__beta) * var_ICaL__k1t) / var_ICaL__k2t) * var_ICaL__k5t) / var_ICaL__k6t;
        const double var_ICaL__r1 = 0.3;
        const double var_ICaL__r2 = 3.0;
        double var_ICaL__s1 = 0.0182688 * var_ICaL__fca;
        const double var_ICaL__s1t = 0.00195;
        double var_ICaL__s2 = (((var_ICaL__s1 * var_ICaL__r1) / var_ICaL__r2) * var_ICaL__k2) / var_ICaL__k1;
        double var_ICaL__s2t = (((var_ICaL__s1t * var_ICaL__r1) / var_ICaL__r2) * var_ICaL__k2t) / var_ICaL__k1t;
        double var_IKr__xkrv1 = (fabs(var_IKr__V + 7.0) > 0.001) ? ((0.00138 * 1.0 * (var_IKr__V + 7.0)) / (1.0 - exp((-0.123) * (var_IKr__V + 7.0)))) : (0.00138 / 0.123);
        double var_IKr__xkrv2 = (fabs(var_IKr__V + 10.0) > 0.001) ? ((0.00061 * 1.0 * (var_IKr__V + 10.0)) / (exp(0.145 * (var_IKr__V + 10.0)) - 1.0)) : (0.00061 / 0.145);
        double var_IKr__taukr = 1.0 / (var_IKr__xkrv1 + var_IKr__xkrv2);
        double var_IKr__xkrinf = 1.0 / (1.0 + exp((-(var_IKr__V + 50.0)) / 7.5));
        double var_IKs__xs1ss = 1.0 / (1.0 + exp((-(var_IKs__V - 1.5)) / 16.7));
        double var_IKs__xs2ss = var_IKs__xs1ss;
        double var_IKs__tauxs1 = (fabs(var_IKs__V + 30.0) < (0.001 / 0.0687)) ? (1.0 / ((7.19e-05 / 0.148) + (0.000131 / 0.0687))) : (1.0 / (((7.19e-05 * (var_IKs__V + 30.0)) / (1.0 - exp((-0.148) * (var_IKs__V + 30.0)))) + ((0.000131 * (var_IKs__V + 30.0)) / (exp(0.0687 * (var_IKs__V + 30.0)) - 1.0))));
        double var_IKs__tauxs2 = 4.0 * var_IKs__tauxs1;
        double var_Ito__rt1 = (-(var_Ito__V + 3.0)) / 15.0;
        double var_Ito__rt3 = (var_Ito__V + 60.0) / 10.0;
        double var_Ito__rt4 = (((-var_Ito__V) / 30.0) * var_Ito__V) / 30.0;
        double var_Ito__rt5 = (var_Ito__V + 33.5) / 10.0;
        double var_Ito__xtos_inf = 1.0 / (1.0 + exp(var_Ito__rt1));
        double var_Ito__ytos_inf = 1.0 / (1.0 + exp(var_Ito__rt2));
        double var_Ito__xtof_inf = var_Ito__xtos_inf;
        double var_Ito__ytof_inf = var_Ito__ytos_inf;
        double var_Ito__txs = (9.0 / (1.0 + exp(-var_Ito__rt1))) + 0.5;
        double var_Ito__tys = (3000.0 / (1.0 + exp(var_Ito__rt3))) + 30.0;
        double var_Ito__txf = (3.5 * exp(var_Ito__rt4)) + 1.5;
        double var_Ito__tyf = (20.0 / (1.0 + exp(var_Ito__rt5))) + 20.0;
        double var_Irel__V = var_cell__V;
        double var_Irel__Ca_NSR = var_Ca__Ca_NSR;
        double var_Ileak_Iup_Ixfer__Ca_NSR = var_Ca__Ca_NSR;
        const double var_Ileak_Iup_Ixfer__gleak = 2.069e-05;
        const double var_Ileak_Iup_Ixfer__kj = 50.0;
        double var_Ileak_Iup_Ixfer__Ca_i = var_Ca__Ca_i;
        double var_Ileak_Iup_Ixfer__jleak = ((var_Ileak_Iup_Ixfer__gleak * var_Ileak_Iup_Ixfer__Ca_NSR * var_Ileak_Iup_Ixfer__Ca_NSR) / ((var_Ileak_Iup_Ixfer__Ca_NSR * var_Ileak_Iup_Ixfer__Ca_NSR) + (var_Ileak_Iup_Ixfer__kj * var_Ileak_Iup_Ixfer__kj))) * ((var_Ileak_Iup_Ixfer__Ca_NSR * 16.667) - var_Ileak_Iup_Ixfer__Ca_i);
        double var_Ca__jleak = var_Ileak_Iup_Ixfer__jleak;
        const double var_Ileak_Iup_Ixfer__cup = 0.5;
        const double var_Ileak_Iup_Ixfer__vup = 0.4;
        double var_Ileak_Iup_Ixfer__jup = (var_Ileak_Iup_Ixfer__vup * var_Ileak_Iup_Ixfer__Ca_i * var_Ileak_Iup_Ixfer__Ca_i) / ((var_Ileak_Iup_Ixfer__Ca_i * var_Ileak_Iup_Ixfer__Ca_i) + (var_Ileak_Iup_Ixfer__cup * var_Ileak_Iup_Ixfer__cup));
        double var_Ca__jup = var_Ileak_Iup_Ixfer__jup;
        double var_Ca__xir = var_Irel__xir;
        double var_Ca__dCa_JSR = ((-var_Ca__xir) + var_Ca__jup) - var_Ca__jleak;
        double var_Irel__dCa_JSR = var_Ca__dCa_JSR;
        double var_Irel__po = var_ICaL__po;
        double var_Irel__rxa = var_ICaL__rxa;
        const double var_Irel__cstar = 90.0;
        const double var_Irel__gryr = 2.58079;
        const double var_Irel__gbarsr = 26841.8;
        const double var_Irel__gdyad = 9000.0;
        const double var_Irel__ax = 0.3576;
        const double var_Irel__ay = 0.05;
        const double var_Irel__av = 11.3;
        double var_Irel__bv = ((1.0 - var_Irel__av) * var_Irel__cstar) - 50.0;
        double var_Irel__Qr0 = ((var_Irel__Ca_JSR > 50.0) && (var_Irel__Ca_JSR < var_Irel__cstar)) ? ((var_Irel__Ca_JSR - 50.0) / 1.0) : (var_Irel__Ca_JSR >= var_Irel__cstar) ? ((var_Irel__av * var_Irel__Ca_JSR) + var_Irel__bv) : 0.0;
        double var_Irel__Qr = (var_Irel__Ca_NSR * var_Irel__Qr0) / var_Irel__cstar;
        double var_Irel__sparkV = exp((-var_Irel__ay) * (var_Irel__V + 30.0)) / (1.0 + exp((-var_Irel__ay) * (var_Irel__V + 30.0)));
        double var_Irel__spark_rate = (var_Irel__gryr / 1.0) * var_Irel__po * fabs(var_Irel__rxa) * var_Irel__sparkV;
        const double var_Irel__taua = 100.0;
        const double var_Irel__taur = 30.0;
        double var_Irel__xirp = (((var_Irel__po * var_Irel__Qr * fabs(var_Irel__rxa) * var_Irel__gbarsr) / 1.0) * exp((-var_Irel__ax) * (var_Irel__V + 30.0))) / (1.0 + exp((-var_Irel__ax) * (var_Irel__V + 30.0)));
        double var_Irel__xicap = var_Irel__po * var_Irel__gdyad * fabs(var_Irel__rxa);
        double var_Irel__xiryr = var_Irel__xirp + var_Irel__xicap;
        double var_Na__wca = var_cell__wca;
        double var_Na__xina = var_INa__xina;
        double var_Na__xiNaK = var_INaK__xiNaK;
        double var_Na__xiNaCa = var_INaCa__xiNaCa;
        double var_Ca__xiryr = var_Irel__xiryr;
        const double var_Ca__bcal = 24.0;
        const double var_Ca__xkcal = 7.0;
        const double var_Ca__srmax = 47.0;
        const double var_Ca__srkd = 0.6;
        const double var_Ca__bmem = 15.0;
        const double var_Ca__kmem = 0.3;
        const double var_Ca__bsar = 42.0;
        const double var_Ca__ksar = 13.0;
        double var_Ca__bpxs = (var_Ca__bcal * var_Ca__xkcal) / ((var_Ca__xkcal + var_Ca__Ca_submem) * (var_Ca__xkcal + var_Ca__Ca_submem));
        double var_Ca__spxs = (var_Ca__srmax * var_Ca__srkd) / ((var_Ca__srkd + var_Ca__Ca_submem) * (var_Ca__srkd + var_Ca__Ca_submem));
        double var_Ca__mempxs = (var_Ca__bmem * var_Ca__kmem) / ((var_Ca__kmem + var_Ca__Ca_submem) * (var_Ca__kmem + var_Ca__Ca_submem));
        double var_Ca__sarpxs = (var_Ca__bsar * var_Ca__ksar) / ((var_Ca__ksar + var_Ca__Ca_submem) * (var_Ca__ksar + var_Ca__Ca_submem));
        double var_Ca__dcsib = 1.0 / (1.0 + var_Ca__bpxs + var_Ca__spxs + var_Ca__mempxs + var_Ca__sarpxs);
        double var_Ca__bpxi = (var_Ca__bcal * var_Ca__xkcal) / ((var_Ca__xkcal + var_Ca__Ca_i) * (var_Ca__xkcal + var_Ca__Ca_i));
        double var_Ca__spxi = (var_Ca__srmax * var_Ca__srkd) / ((var_Ca__srkd + var_Ca__Ca_i) * (var_Ca__srkd + var_Ca__Ca_i));
        double var_Ca__mempxi = (var_Ca__bmem * var_Ca__kmem) / ((var_Ca__kmem + var_Ca__Ca_i) * (var_Ca__kmem + var_Ca__Ca_i));
        double var_Ca__sarpxi = (var_Ca__bsar * var_Ca__ksar) / ((var_Ca__ksar + var_Ca__Ca_i) * (var_Ca__ksar + var_Ca__Ca_i));
        double var_Ca__dciib = 1.0 / (1.0 + var_Ca__bpxi + var_Ca__spxi + var_Ca__mempxi + var_Ca__sarpxi);
        const double var_Ca__xkon = 0.0327;
        const double var_Ca__xkoff = 0.0196;
        const double var_Ca__btrop = 70.0;
        double var_Ca__xbi = (var_Ca__xkon * var_Ca__Ca_i * (var_Ca__btrop - var_Ca__tropi)) - (var_Ca__xkoff * var_Ca__tropi);
        double var_Ca__xbs = (var_Ca__xkon * var_Ca__Ca_submem * (var_Ca__btrop - var_Ca__trops)) - (var_Ca__xkoff * var_Ca__trops);
        const double var_Ca__taud = 4.0;
        const double var_Ca__taups = 0.5;
        double var_Ca__jd = (var_Ca__Ca_submem - var_Ca__Ca_i) / var_Ca__taud;
        double var_Ca__jNaCa = var_INaCa__jNaCa;
        double var_Ca__jca = var_ICaL__jca;
        
        double d_dt_cell__V;
        if (mSetVoltageDerivativeToZero)
        {
            d_dt_cell__V = 0.0;
        }
        else
        {
            d_dt_cell__V = var_cell__Itotal;
        }
        
        double d_dt_INa__xh = (var_INa__ah * (1.0 - var_INa__xh)) - (var_INa__bh * var_INa__xh);
        double d_dt_INa__xj = (var_INa__aj * (1.0 - var_INa__xj)) - (var_INa__bj * var_INa__xj);
        double d_dt_INa__xm = (var_INa__am * (1.0 - var_INa__xm)) - (var_INa__bm * var_INa__xm);
        double d_dt_ICaL__c1 = ((var_ICaL__alpha * var_ICaL__c2) + (var_ICaL__k2 * var_ICaL__xi1ca) + (var_ICaL__k2t * var_ICaL__xi1ba) + (var_ICaL__r2 * var_ICaL__po)) - ((var_ICaL__beta + var_ICaL__r1 + var_ICaL__k1t + var_ICaL__k1) * var_ICaL__c1);
        double d_dt_ICaL__c2 = ((var_ICaL__beta * var_ICaL__c1) + (var_ICaL__k5 * var_ICaL__xi2ca) + (var_ICaL__k5t * var_ICaL__xi2ba)) - ((var_ICaL__k6 + var_ICaL__k6t + var_ICaL__alpha) * var_ICaL__c2);
        double d_dt_ICaL__xi1ca = ((var_ICaL__k1 * var_ICaL__c1) + (var_ICaL__k4 * var_ICaL__xi2ca) + (var_ICaL__s1 * var_ICaL__po)) - ((var_ICaL__k3 + var_ICaL__k2 + var_ICaL__s2) * var_ICaL__xi1ca);
        double d_dt_ICaL__xi1ba = ((var_ICaL__k1t * var_ICaL__c1) + (var_ICaL__k4t * var_ICaL__xi2ba) + (var_ICaL__s1t * var_ICaL__po)) - ((var_ICaL__k3t + var_ICaL__k2t + var_ICaL__s2t) * var_ICaL__xi1ba);
        double d_dt_ICaL__xi2ca = ((var_ICaL__k3 * var_ICaL__xi1ca) + (var_ICaL__k6 * var_ICaL__c2)) - ((var_ICaL__k5 + var_ICaL__k4) * var_ICaL__xi2ca);
        double d_dt_ICaL__xi2ba = ((var_ICaL__k3t * var_ICaL__xi1ba) + (var_ICaL__k6t * var_ICaL__c2)) - ((var_ICaL__k5t + var_ICaL__k4t) * var_ICaL__xi2ba);
        double d_dt_IKr__xr = (var_IKr__xkrinf - var_IKr__xr) / var_IKr__taukr;
        double d_dt_IKs__xs1 = (var_IKs__xs1ss - var_IKs__xs1) / var_IKs__tauxs1;
        double d_dt_IKs__xs2 = (var_IKs__xs2ss - var_IKs__xs2) / var_IKs__tauxs2;
        double d_dt_Ito__xtos = (var_Ito__xtos_inf - var_Ito__xtos) / var_Ito__txs;
        double d_dt_Ito__ytos = (var_Ito__ytos_inf - var_Ito__ytos) / var_Ito__tys;
        double d_dt_Ito__xtof = (var_Ito__xtof_inf - var_Ito__xtof) / var_Ito__txf;
        double d_dt_Ito__ytof = (var_Ito__ytof_inf - var_Ito__ytof) / var_Ito__tyf;
        double d_dt_Irel__Ca_JSR = (var_Irel__Ca_NSR - var_Irel__Ca_JSR) / var_Irel__taua;
        double d_dt_Irel__xir = (var_Irel__spark_rate * var_Irel__Qr) - ((var_Irel__xir * (1.0 - ((var_Irel__taur * var_Irel__dCa_JSR) / var_Irel__Ca_NSR))) / var_Irel__taur);
        double d_dt_Na__Na_i = (-(var_Na__xina + (3.0 * var_Na__xiNaK) + (3.0 * var_Na__xiNaCa))) / (var_Na__wca * 1000.0);
        double d_dt_Ca__Ca_dyad = var_Ca__xiryr - ((var_Ca__Ca_dyad - var_Ca__Ca_submem) / var_Ca__taups);
        double d_dt_Ca__Ca_submem = var_Ca__dcsib * ((50.0 * (((var_Ca__xir - var_Ca__jd) - var_Ca__jca) + var_Ca__jNaCa)) - var_Ca__xbs);
        double d_dt_Ca__Ca_i = var_Ca__dciib * (((var_Ca__jd - var_Ca__jup) + var_Ca__jleak) - var_Ca__xbi);
        double d_dt_Ca__Ca_NSR = var_Ca__dCa_JSR;
        double d_dt_Ca__tropi = var_Ca__xbi;
        double d_dt_Ca__trops = var_Ca__xbs;
        
        rDY[0] = 1.0*d_dt_cell__V;
        rDY[1] = 1.0*d_dt_INa__xm;
        rDY[2] = 1.0*d_dt_INa__xh;
        rDY[3] = 1.0*d_dt_INa__xj;
        rDY[4] = 1.0*d_dt_ICaL__c1;
        rDY[5] = 1.0*d_dt_ICaL__c2;
        rDY[6] = 1.0*d_dt_ICaL__xi1ca;
        rDY[7] = 1.0*d_dt_ICaL__xi1ba;
        rDY[8] = 1.0*d_dt_ICaL__xi2ca;
        rDY[9] = 1.0*d_dt_ICaL__xi2ba;
        rDY[10] = 1.0*d_dt_IKr__xr;
        rDY[11] = 1.0*d_dt_IKs__xs1;
        rDY[12] = 1.0*d_dt_IKs__xs2;
        rDY[13] = 1.0*d_dt_Ito__xtos;
        rDY[14] = 1.0*d_dt_Ito__ytos;
        rDY[15] = 1.0*d_dt_Ito__xtof;
        rDY[16] = 1.0*d_dt_Ito__ytof;
        rDY[17] = 1.0*d_dt_Irel__Ca_JSR;
        rDY[18] = 1.0*d_dt_Irel__xir;
        rDY[19] = 1.0*d_dt_Na__Na_i;
        rDY[20] = 1.0*d_dt_Ca__Ca_dyad;
        rDY[21] = 1.0*d_dt_Ca__Ca_submem;
        rDY[22] = 1.0*d_dt_Ca__Ca_i;
        rDY[23] = 1.0*d_dt_Ca__Ca_NSR;
        rDY[24] = 1.0*d_dt_Ca__tropi;
        rDY[25] = 1.0*d_dt_Ca__trops;
    }
    


template<>
void OdeSystemInformation<Mahajan2008OdeSystem>::Initialise(void)
{
    // Time units: ms
    // 
    this->mVariableNames.push_back("V");
    this->mVariableUnits.push_back("mV");
    this->mInitialConditions.push_back(-87.169816169406);

    this->mVariableNames.push_back("xm");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.001075453357);

    this->mVariableNames.push_back("xh");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.990691306716);

    this->mVariableNames.push_back("xj");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.993888937283);

    this->mVariableNames.push_back("c1");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.000018211252);

    this->mVariableNames.push_back("c2");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.979322592773);

    this->mVariableNames.push_back("xi1ca");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.001208153482);

    this->mVariableNames.push_back("xi1ba");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.000033616596);

    this->mVariableNames.push_back("xi2ca");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.004173008466);

    this->mVariableNames.push_back("xi2ba");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.015242594688);

    this->mVariableNames.push_back("xr");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.007074239331);

    this->mVariableNames.push_back("xs1");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.048267587131);

    this->mVariableNames.push_back("xs2");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.105468807033);

    this->mVariableNames.push_back("xtos");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.00364776906);

    this->mVariableNames.push_back("ytos");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.174403618112);

    this->mVariableNames.push_back("xtof");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.003643592594);

    this->mVariableNames.push_back("ytof");
    this->mVariableUnits.push_back("dimensionless");
    this->mInitialConditions.push_back(0.993331326442);

    this->mVariableNames.push_back("Ca_JSR");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(97.505463697266);

    this->mVariableNames.push_back("xir");
    this->mVariableUnits.push_back("uM_per_ms");
    this->mInitialConditions.push_back(0.006679257264);

    this->mVariableNames.push_back("Na_i");
    this->mVariableUnits.push_back("mM");
    this->mInitialConditions.push_back(11.441712311614);

    this->mVariableNames.push_back("Ca_dyad");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(1.716573130685);

    this->mVariableNames.push_back("Ca_submem");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(0.226941113355);

    this->mVariableNames.push_back("Ca_i");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(0.256752008084);

    this->mVariableNames.push_back("Ca_NSR");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(104.450004990523);

    this->mVariableNames.push_back("tropi");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(22.171689894953);

    this->mVariableNames.push_back("trops");
    this->mVariableUnits.push_back("uM");
    this->mInitialConditions.push_back(19.864701949854);

    this->mInitialised = true;
}
