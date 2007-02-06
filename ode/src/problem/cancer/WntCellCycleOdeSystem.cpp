#include "WntCellCycleOdeSystem.hpp"
#include <cmath>
#include <cassert>
#include <vector>
#include "ReplicatableVector.hpp"

/**
 * Constructor
 * 
 * @param WntLevel is a non-dimensional Wnt value between 0 and 1. This sets up the Wnt pathway in its steady state.
 * 
 */
WntCellCycleOdeSystem::WntCellCycleOdeSystem(double WntLevel) 
  : AbstractOdeSystem(8)
{
    /*
     * State variables
	% 
	% 0. r = pRb
	% 1. e = E2F1 (This is the S-phase indicator)
	% 2. i = CycD (inactive)
	% 3. j = CycD (active)
	% 4. p = pRb-p
	% 5. c = APC (Active)
	% 6. b = Beta-Catenin
	% 7. WntLevel 
	*/
	Init();
	
	// These three lines set up a wnt signalling pathway in a steady state
	double destruction_level = ma5d/(ma4d*WntLevel+ma5d);
    double beta_cat_level = ma2d/(ma2d+ma3d*destruction_level);
    	
	mVariableNames.push_back("pRb");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(7.357000000000000e-01);
    
    mVariableNames.push_back("E2F1");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(1.713000000000000e-01);
    
    mVariableNames.push_back("CycD_i");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(6.900000000000001e-02);
    
    mVariableNames.push_back("CycD_a");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(3.333333333333334e-03);
    
    mVariableNames.push_back("pRb_p");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(1.000000000000000e-04);
    
    mVariableNames.push_back("APC");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(destruction_level);

    mVariableNames.push_back("Beta_Cat");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(beta_cat_level);
    
    mVariableNames.push_back("Wnt");
    mVariableUnits.push_back("non_dim");
    mInitialConditions.push_back(WntLevel);
    
    mNumberOfStateVariables=8;
}


/**
 * Destructor
 */
WntCellCycleOdeSystem::~WntCellCycleOdeSystem(void)
{
    // Do nothing
}


void WntCellCycleOdeSystem::Init()
{
    // Initialize model parameters
    // Swat (2004) Parameters
	double k1 = 1.0;
	double k2 = 1.6;
	double k3 = 0.05;
	double k16 = 0.4;
	double k34 = 0.04;
	double k43 = 0.01;
	double k61 = 0.3;
	double k23 = 0.3;
	double a = 0.04;
	double J11 = 0.5;
	double J12 = 5.0;
	double J61 = 5.0;
	double J62 = 8.0;
	double J13 = 0.002;
	double J63 = 2.0;
	double Km1 = 0.5;
	double Km2 = 4.0;
	double Km4 = 0.3;
	double kp = 0.05;
	double phi_pRb = 0.005;
	double phi_E2F1 = 0.1;
	double phi_CycDi = 0.023;
	double phi_CycDa = 0.03;
	double phi_pRbp = 0.06;
	// Gary Parameters
	double a1 = 0.423;
	double a2 = 2.57e-4;
	double a3 = 1.72;
	double a4 = 10.0;
	double a5 = 0.5;
	double WntMax = 10.0;
	
    //Gary's parameter to break the build... double mitogenic_factorF = 5.0e-4;
    double mitogenic_factorF = 6.0e-4;
    //\todo change this without breaking the build
    double APC_Total = 0.02;

//  Non-dimensionalise...
	mk2d = k2/(Km2*phi_E2F1);
	mk3d = k3*a1*mitogenic_factorF/(Km4*phi_E2F1*a2);
	mk34d = k34/phi_E2F1;
	mk43d = k43/phi_E2F1;
	mk23d = k23*Km2/(Km4*phi_E2F1);
	mad = a/Km2;
	mJ11d = J11*phi_E2F1/k1;
	mJ12d = J12*phi_E2F1/k1;
	mJ13d = J13*phi_E2F1/k1;
	mJ61d = J61*phi_E2F1/k1;
	mJ62d = J62*phi_E2F1/k1;
	mJ63d = J63*phi_E2F1/k1;
	mKm1d = Km1/Km2;
	mkpd = kp/(Km2*phi_E2F1);
	mphi_r = phi_pRb/phi_E2F1;
	mphi_i = phi_CycDi/phi_E2F1;
	mphi_j = phi_CycDa/phi_E2F1;
	mphi_p = phi_pRbp/phi_E2F1;
	ma2d = a2/phi_E2F1;
	ma3d = a3*APC_Total/phi_E2F1;
	ma4d = a4*WntMax/phi_E2F1;
	ma5d = a5/phi_E2F1;
	mk16d = k16*Km4/phi_E2F1;
	mk61d = k61/phi_E2F1;
	mPhiE2F1 = phi_E2F1;
}

/**
 * Returns a vector representing the RHS of the odes at each time step, y' = [y1' ... yn'].
 * Some ODE solver will call this function repeatedly to solve for y = [y1 ... yn].
 *
 * @param rDY filled in with the resulting derivatives (using Mirams et al. (2007?) system of equations)
 */
void WntCellCycleOdeSystem::EvaluateYDerivatives(double time, const std::vector<double> &rY, std::vector<double> &rDY)
{
    double r = rY[0];
    double e = rY[1];
    double i = rY[2];
    double j = rY[3];
    double p = rY[4];
    double c = rY[5];
    double b = rY[6];
    double WntLevel = rY[7];
    
    double dx1 = 0.0;
    double dx2 = 0.0;
    double dx3 = 0.0;
    double dx4 = 0.0;
    double dx5 = 0.0;
    double dx6 = 0.0;
    double dx7 = 0.0;
    
    /*
	% The variables are
	% 1. r = pRb
	% 2. e = E2F1
	% 3. i = CycD (inactive)
	% 4. j = CycD (active)
	% 5. p = pRb-p
	% 6. c = APC (Active)
	% 7. b = Beta-Catenin
	*/

	// dr
	dx1 = e/(mKm1d+e)*mJ11d/(mJ11d+r)*mJ61d/(mJ61d+p) - mk16d*r*j+mk61d*p-mphi_r*r;
	// de
	dx2 = mkpd+mk2d*(mad*mad+e*e)/(1+e*e)*mJ12d/(mJ12d+r)*mJ62d/(mJ62d+p) - e;
	// di
	dx3 = mk3d*b + mk23d*e*mJ13d/(mJ13d+r)*mJ63d/(mJ63d+p) + mk43d*j - mk34d*i*j/(1+j) - mphi_i*i;
	// dj
	dx4 = mk34d*i*j/(1+j) - (mk43d+mphi_j)*j;
	// dp
	dx5 = mk16d*r*j - mk61d*p - mphi_p*p;
	// da
	dx6 = ma5d*(1.0-c) - ma4d*WntLevel*c;
	// db
	dx7 = ma2d*(1.0-b) - ma3d*b*c;
	
	
	double factor = mPhiE2F1*60.0;  // Convert non-dimensional d/dt s to d/dt in hours.
    
    rDY[0] = dx1*factor;
    rDY[1] = dx2*factor;
//    std::cout << "d/dt E2F1 = " << dx2*factor << "\n";
    rDY[2] = dx3*factor;
    rDY[3] = dx4*factor;
    rDY[4] = dx5*factor;
    rDY[5] = dx6*factor;
    rDY[6] = dx7*factor;
    //std::cout << "d/dt beta = " << dx7*factor << "\n";
    rDY[7] = 0.0; // Do not change the Wnt level.
}

