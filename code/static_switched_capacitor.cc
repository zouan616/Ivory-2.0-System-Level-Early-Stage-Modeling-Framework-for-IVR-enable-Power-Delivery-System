#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <dlfcn.h> /* to load libraries*/
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "static_buck.h"

using namespace std;




void Static_Switched_Capacitor(double Vin, double Vout, double P, double Area_buget, double ripple, double Efficiency_buget, int Optimization_Target)
{
//user input
double V_in = Vin;
double V_out = Vout;
double P_load = P;
double Area = Area_buget*0.000001;
double Delta_v_range = ripple;         //Percentage
double Acceptable_Efficiency = Efficiency_buget;



//Optimization Target
//0 for maximize efficiency; 1 for smallest area
int Optimization = Optimization_Target;




//technology information
double C_bridge;
C_bridge = Read_in_Technology(1);
cout << "C_bridge: " << C_bridge << endl;
double C_driver = 0.000000002;
C_driver = Read_in_Technology(2);
cout << "C_driver: " << C_driver << endl;
double I_control = 0.002;
I_control = Read_in_Technology(3);
cout << "I_control: " << I_control << endl;
double R_ind = 50000000;
R_ind = Read_in_Technology(4);
cout << "R_ind: " << R_ind << endl;
double R_sw = 0.001;
R_sw = Read_in_Technology(5);
cout << "R_sw: " << R_sw << endl;
double Area_driver = 1*0.000001;
Area_driver = Read_in_Technology(6);
cout << "Area_driver: " << Area_driver << endl;
double Leak_sw = 0.5;
Leak_sw = Read_in_Technology(7);
cout << "Leak_sw: " << Leak_sw << endl;
double Switching_loss = 0.001;
Switching_loss = Read_in_Technology(8);
cout << "Switching_loss: " << Switching_loss << endl;
double Area_driver_controller = 0.05*0.000001; //0.05 mm2
Area_driver_controller = Read_in_Technology(9);
cout << "Area_driver_controller: " << Area_driver_controller << endl;
double L_density = 0.002;
L_density = Read_in_Technology(10);
cout << "L_density: " << L_density << endl;
double C_density = 0.05;
C_density = Read_in_Technology(11);
cout << "C_density: " << C_density << endl;


//boundary setting
double F_max = 200*1000000;
double F_min = 50*1000;
double L_sw_min = 120*0.000000001;
double W_sw_hi_min = 120*0.000000001;
double W_sw_hi_max = 1*0.01;
double W_sw_low_min = 120*0.000000001;
double W_sw_low_max = 1*0.01;
double L_max = Area*L_density;
double C_max = Area*C_density;
double N_max = 16;

//pre calculation
double V_driver;
double D;
double I_load;
V_driver = V_in;
D = V_out/V_in;
I_load = P_load/V_out;

double M_sw;
double M_cap;
double F_sw, F_sw_tep;
double W_sw_hi, W_sw_hi_tep;
double W_sw_low, W_sw_low_tep;
double L, L_tep, L_total, L_min;
double C, C_tep, C_total, C_min;
double N, N_tep;

double Efficiency = 0;
double Efficiency_tep;
double Actual_area;
double Final_area;
double Converter_power;
double Area_tep;
double P_driver;
double P_control;
double P_sw;
double P_c;
double I_c;

double Delta_i;
double Delta_v;






double Ratio[6][6]; //input output ratio good or not. 1 is worst and 0 is best
double Ratio_tep;
double Input_ratio;
double Output_ratio;
for(int i = 1; i < 6; i++)
{
  for(int j = 1; j < 6; j++)
  {
    Ratio[i][j] = 1;
  }
}
Ratio_tep = 1;
//exam feasible input output ratio
for(int i = 1; i < 6; i++)
{
  for(int j = 1; j <= i; j++)
  {
  if((V_out <= (V_in*(double)(j)/(double)(i))))
    {
      Ratio[i][j] = (double)((V_in*(double)(j)/(double)(i))-V_out)/(double)V_out;
      cout << "Feasible Input and Output Ratio: " << i << " : " << j << "----" << "Actual Output: " << V_in*(double)(j)/(double)(i) << endl;
    }
  }
}
//Find the best configuration: D = 50%. Referred to "Design Technology for Fully Integrated Static Switched Capacitor DC-DC Converter" by Phuc
//Smaller ratio, better configuration
M_sw = 2;
M_cap = 2;


for(int i = 1; i < 6; i++)
{
  for(int j = 1; j <= i; j++)
  {
  if( Ratio[i][j] < Ratio_tep)
    {
      Ratio_tep = Ratio[i][j];
      Input_ratio = i;
      Output_ratio = j;
    }
  }
}





if(Optimization == 0)
{
//start Optimization
double F_sw_step = 100000;
double W_sw_step = 0.0001;
double C_step = (Area/100)*C_density;
double N_step = 2;

//First Level Optimization
for(F_sw = F_min; F_sw <= F_max; F_sw = F_sw + F_sw_step)
{
		cout << "Level 1 Optimization: %" << 100*(F_sw-F_min)/(F_max-F_min) << endl;
    for(N = 1; N <= N_max; N = N + N_step)
    {
			if(N == 3)
			{
				N = N - 1;
			}
    I_c = I_load/N;
		for(W_sw_low = W_sw_low_min; W_sw_low <= W_sw_low_max; W_sw_low = W_sw_low + W_sw_step)
        {

			for(W_sw_hi = W_sw_hi_min; W_sw_hi <= W_sw_hi_max; W_sw_hi = W_sw_hi + W_sw_step)
        	{

					for(C = 0; C <= C_max/N; C = C + C_step/N)
					{


							Actual_area = N * (Area_driver_controller + (W_sw_hi + W_sw_low)*Area_driver + C/C_density);
							if(Actual_area <= Area)
								{

								Delta_v = I_load/(2*C*N*F_sw);

									if(Delta_v <= V_out * Delta_v_range)
									{

										P_driver = ((C_bridge+C_driver)*(W_sw_hi+W_sw_low))*V_driver*F_sw;
										P_control = I_control*V_driver;
										P_sw = I_c*I_c*R_sw*M_sw/(W_sw_hi + W_sw_low);
										P_c = I_c * Delta_v/2;
										Converter_power = (P_driver + P_control + P_sw + P_c)*N;
										Efficiency_tep = P_load/(P_load + Converter_power);

										if(Efficiency_tep > Efficiency)
										{
											Efficiency = Efficiency_tep;
                      Final_area = Actual_area;
											F_sw_tep = F_sw;
											W_sw_hi_tep = W_sw_hi;
											W_sw_low_tep = W_sw_low;
											C_tep = C;
											C_total = C*N;
											N_tep = N;

											cout << "" << endl;
											cout << "Efficiency: "<< Efficiency << endl;
											cout << "F_sw: "<< F_sw_tep << endl;
											cout << "W_sw_hi: "<< W_sw_hi_tep << endl;
											cout << "W_sw_low: "<< W_sw_low_tep << endl;
											cout << "C_total: "<< C_total << endl;
											cout << "N: "<< N_tep << endl;
											cout << "Area: " << Final_area*1000000 << "mm2" << endl;

											cout << "P_driver: " << P_driver*N << endl;
											cout << "P_control: " << P_control*N << endl;
											cout << "P_sw: " << P_sw*N << endl;
                      cout << "P_c: " << P_c << endl;
										}
									}

								}



						}
        	}

        }
    }
}


//--------------------Level 2 Optimization-------------------------------------
//Second Level Optimization
F_min = F_sw_tep - 5*F_sw_step;
if(F_min < 0)
{
	F_min = 0;
}
F_max = F_sw_tep + 5*F_sw_step;
F_sw_step = F_sw_step/10;
W_sw_low_min = W_sw_low_tep - 5*W_sw_step;
if(W_sw_low_min < 0)
{
	W_sw_low_min = 0;
}
W_sw_low_max = W_sw_low_tep + 5*W_sw_step;
W_sw_hi_min = W_sw_hi_tep - 5*W_sw_step;
if(W_sw_hi_min < 0)
{
	W_sw_hi_min = 0;
}
W_sw_hi_max = W_sw_hi_tep + 5*W_sw_step;
W_sw_step = W_sw_step/10;
C_step = C_step/10;
N_step = 2;


for(F_sw = F_min; F_sw <= F_max; F_sw = F_sw + F_sw_step)
{
		cout << "Level 2 Optimization: %" << 100*(F_sw-F_min)/(F_max-F_min) << endl;
    for(N = 1; N <= N_max; N = N + N_step)
    {
			if(N == 3)
			{
				N = N - 1;
			}
    I_c = I_load/N;
		for(W_sw_low = W_sw_low_min; W_sw_low <= W_sw_low_max; W_sw_low = W_sw_low + W_sw_step)
        {

			for(W_sw_hi = W_sw_hi_min; W_sw_hi <= W_sw_hi_max; W_sw_hi = W_sw_hi + W_sw_step)
        	{

					for(C = 0; C <= C_max/N; C = C + C_step/N)
					{


							Actual_area = N * (Area_driver_controller + (W_sw_hi + W_sw_low)*Area_driver + C/C_density);
							if(Actual_area <= Area)
								{

								Delta_v = I_load/(2*C*N*F_sw);

									if(Delta_v <= V_out * Delta_v_range)
									{

										P_driver = ((C_bridge+C_driver)*(W_sw_hi+W_sw_low))*V_driver*F_sw;
										P_control = I_control*V_driver;
										P_sw = I_c*I_c*R_sw*M_sw/(W_sw_hi + W_sw_low);
										P_c = I_c * Delta_v/2;
										Converter_power = (P_driver + P_control + P_sw + P_c)*N;
										Efficiency_tep = P_load/(P_load + Converter_power);

										if(Efficiency_tep > Efficiency)
										{
											Efficiency = Efficiency_tep;
											F_sw_tep = F_sw;
											W_sw_hi_tep = W_sw_hi;
											W_sw_low_tep = W_sw_low;
											C_tep = C;
											C_total = C*N;
											N_tep = N;

											cout << "" << endl;
											cout << "Efficiency: "<< Efficiency << endl;
											cout << "F_sw: "<< F_sw_tep << endl;
											cout << "W_sw_hi: "<< W_sw_hi_tep << endl;
											cout << "W_sw_low: "<< W_sw_low_tep << endl;
											cout << "C_total: "<< C_total << endl;
											cout << "N: "<< N_tep << endl;
											cout << "Area: " << Actual_area*1000000 << endl;

											cout << "P_driver: " << P_driver*N << endl;
											cout << "P_control: " << P_control*N << endl;
											cout << "P_sw: " << P_sw*N << endl;
                      cout << "P_c: " << P_c << endl;
										}
									}

								}



						}
        	}

        }
    }
}
}

if(Optimization == 1)
{
  //start Optimization

  double F_sw_step = 100000;
  double W_sw_step = 0.0001;
  double N_step = 4;
  for(double Min_Area = 1*0.000001; Min_Area < 500*0.000001; Min_Area = Min_Area + 1*0.000001)
  {
    double C_step = (Min_Area/10)*C_density;
    double C_max = Min_Area*C_density;
    cout << "Smallest Optimization: %" << 100*Min_Area/(500*0.000001) << endl;
    for(F_sw = F_min; F_sw <= F_max; F_sw = F_sw + F_sw_step)
    {
        cout << "For Area: " << Min_Area  <<" Optimization: %" << 100*(F_sw-F_min)/(F_max-F_min) << endl;
        for(N = 1; N <= N_max; N = N + N_step)
        {
    			if(N == 5)
    			{
    				N = N - 1;
    			}
        I_c = I_load/N;
    		for(W_sw_low = W_sw_low_min; W_sw_low <= W_sw_low_max; W_sw_low = W_sw_low + W_sw_step)
            {

    			for(W_sw_hi = W_sw_hi_min; W_sw_hi <= W_sw_hi_max; W_sw_hi = W_sw_hi + W_sw_step)
            	{

    					for(C = 0; C <= C_max/N; C = C + C_step/N)
    					{
    							Actual_area = N * (Area_driver_controller + (W_sw_hi + W_sw_low)*Area_driver + C/C_density);
    							if(Actual_area <= Min_Area)
    								{

    								Delta_v = I_load/(2*C*N*F_sw);

    									if(Delta_v <= V_out * Delta_v_range)
    									{

    										P_driver = ((C_bridge+C_driver)*(W_sw_hi+W_sw_low))*V_driver*F_sw;
    										P_control = I_control*V_driver;
    										P_sw = I_c*I_c*R_sw*M_sw/(W_sw_hi + W_sw_low);
    										P_c = I_c * Delta_v/2;
    										Converter_power = (P_driver + P_control + P_sw + P_c)*N;
    										Efficiency_tep = P_load/(P_load + Converter_power);

    										if(Efficiency_tep > Acceptable_Efficiency)
    										{
    											Efficiency = Efficiency_tep;
                          Final_area = Actual_area;
    											F_sw_tep = F_sw;
    											W_sw_hi_tep = W_sw_hi;
    											W_sw_low_tep = W_sw_low;
    											C_tep = C;
    											C_total = C*N;
    											N_tep = N;

    											cout << "" << endl;
    											cout << "Efficiency: "<< Efficiency << endl;
    											cout << "F_sw: "<< F_sw_tep << endl;
    											cout << "W_sw_hi: "<< W_sw_hi_tep << endl;
    											cout << "W_sw_low: "<< W_sw_low_tep << endl;
    											cout << "C_total: "<< C_total << endl;
    											cout << "N: "<< N_tep << endl;
    											cout << "Area: " << Final_area*1000000 << "mm2" << endl;

    											cout << "P_driver: " << P_driver*N << endl;
    											cout << "P_control: " << P_control*N << endl;
    											cout << "P_sw: " << P_sw*N << endl;
                          cout << "P_c: " << P_c << endl;
                          exit(0);
    										}
    									}

    								}



    						}
            	}

            }
        }
    }
  }

cout << "Sorry! We cannot find the min area you want. Try to change area boundary." << endl;

}






cout << "" << endl;
cout << "-------------------------------------------------------" << endl;
cout << "Optimized Input and Output Ratio: " << Input_ratio << " : " << Output_ratio << "----" << "Actual Output Without Feedback Control: " << V_in*Output_ratio/Input_ratio << endl;
cout << "-------------------------------------------------------" << endl;








}
