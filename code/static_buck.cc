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
using namespace std;


double Read_in_Technology(int n)
{
	ifstream infile("Technology.csv");
	const char* data;
	string temp[100];
	for(int i = 0; i < n; i++)
	{
	getline(infile,temp[i]);
  }

	char s[1024];
	int m;
	for( m=0;m<temp[n-1].length();m++)
	    {
	    s[m] = temp[n-1][m];
	    }
	    const char *d = " ,*";
	    char *p;
	    p = strtok(s,d);


	    int outputloadnum = 0;

	    while(outputloadnum < 2)
	    {
	    data = p;
	    outputloadnum = outputloadnum + 1;
	    p=strtok(NULL,d);
	    }

			return atof(data);
}



void Static_Buck(double Vin, double Vout, double P, double Area_buget, double ripple, double Efficiency_buget, int Optimization_Target)
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

//start Optimization
double F_sw, F_sw_tep;
double W_sw_hi, W_sw_hi_tep;
double W_sw_low, W_sw_low_tep;
double L, L_tep, L_total, L_min;
double C, C_tep, C_total, C_min;
double N, N_tep;


double Efficiency = 0;
double Efficiency_tep;
double Converter_power;
double Actual_area;
double P_driver;
double P_control;
double P_sw;
double P_ind;
double I_ind;

double Delta_i;
double Delta_v;



if(Optimization == 0)
{
//Optimization Step------After 2 levels Optimization, the step will become 0.01*step
double F_sw_step = 1000000;
double W_sw_step = 0.0001;
double L_step = (Area/100)*L_density; // The efficiency is very sensitive to L, so the L_step should be smaller
double C_step = (Area/10)*C_density;
double N_step = 15;

//First Level Optimization
for(F_sw = F_min; F_sw <= F_max; F_sw = F_sw + F_sw_step)
{
		cout << "Level 1 Optimization: %" << 100*(F_sw-F_min)/(F_max-F_min) << endl;
    for(N = 1; N <= N_max; N = N + N_step)
    {
			if(N == 5)
			{
				N = N - 1;
			}
		for(W_sw_low = W_sw_low_min; W_sw_low <= W_sw_low_max; W_sw_low = W_sw_low + W_sw_step)
        {

			for(W_sw_hi = W_sw_hi_min; W_sw_hi <= W_sw_hi_max; W_sw_hi = W_sw_hi + W_sw_step)
        	{

					for(C = 0; C <= C_max/N; C = C + C_step/N)
					{

						for(L = 0; L <= L_max/N; L = L + L_step/N)
						{
              //dynamic model constraints
						//if(log10(L) <= (-0.35*log10(C)-12.0)) // 0.5us DVFS
						//if(log10(L) <= (-0.35*log10(C)-12.0)) // 0.5us DVFS
						//if(log10(L) <= (-0.3*log10(C)-11.1)) // 2us DVFS
						//if(log10(L) <= (-0.33*log10(C)-11)) // 2us DVFS
						{
							Actual_area = N * (Area_driver_controller + (W_sw_hi + W_sw_low)*Area_driver + L/L_density + C/C_density);
							if(Actual_area <= Area)
								{
								I_ind = I_load/N;
								Delta_i = (V_in-V_out)*D/(F_sw*L);
								Delta_i = Delta_i * N * (D-(floor(D*N)/N)) * ((1+floor(D*N))/N - D) / ((1-D)*D);
									//Delta_v = (Delta_i/(8*F_sw*C))*(0.25*(1/(D*(1-D))))*(1/(N*N));
									//Delta_v = (Delta_i/(8*F_sw*C))*(0.25*(1/(D*(1-D))))*(1/(N));
									//Delta_v = (Delta_i/(F_sw*C*N));
									Delta_v = (Delta_i/(F_sw*C));
									if(Delta_v <= Delta_v_range)
									{
										P_driver = ((C_bridge+C_driver)*(W_sw_hi+W_sw_low))*V_driver*F_sw;
										P_control = I_control*V_driver;
										P_sw = (D*R_sw/W_sw_hi + (1-D)*R_sw/W_sw_hi)*(I_ind*I_ind + (1/12)*Delta_i*Delta_i) + Leak_sw*(W_sw_hi+W_sw_low) + Switching_loss*(W_sw_hi+W_sw_low)*I_ind*F_sw;
										P_ind = R_ind * L * (I_ind*I_ind + (1/12)*Delta_i*Delta_i);
										Converter_power = (P_driver + P_control + P_sw + P_ind)*N;
										Efficiency_tep = P_load/(P_load + Converter_power);
										if(Efficiency_tep > Efficiency)
										{
											Efficiency = Efficiency_tep;
											F_sw_tep = F_sw;
											W_sw_hi_tep = W_sw_hi;
											W_sw_low_tep = W_sw_low;
											L_tep = L;
											C_tep = C;
											L_total = L*N;
											C_total = C*N;
											N_tep = N;

											cout << "" << endl;
											cout << "Efficiency: "<< Efficiency << endl;
											cout << "Area: " << Actual_area*1000000 << endl;
											cout << "Ripple: " << Delta_v << endl;
											cout << "F_sw: "<< F_sw_tep << endl;
											cout << "W_sw_hi: "<< W_sw_hi_tep << endl;
											cout << "W_sw_low: "<< W_sw_low_tep << endl;
											cout << "L_total: "<< L_total << endl;
											cout << "C_total: "<< C_total << endl;
											cout << "N: "<< N_tep << endl;


											cout << "P_driver: " << P_driver*N << endl;
											cout << "P_control: " << P_control*N << endl;
											cout << "P_sw: " << P_sw*N << endl;
											cout << "P_ind: " << P_ind*N << endl;
										}
									}
									}

                }
							}
						}
        	}

        }
    }
}

exit(0);


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
L_step = L_step/10;
N_step = 2;



//Second Level Optimization
for(F_sw = F_min; F_sw <= F_max; F_sw = F_sw + F_sw_step)
{
		cout << "Level 2 Optimization: %" << 100*(F_sw-F_min)/(F_max-F_min) << endl;

    for(N = 1; N <= N_max; N = N + N_step)
    {
				if(N == 3)
				{
					N = N - 1;
				}

		for(W_sw_low = W_sw_low_min; W_sw_low <= W_sw_low_max; W_sw_low = W_sw_low + W_sw_step)
        {

			for(W_sw_hi = W_sw_hi_min; W_sw_hi <= W_sw_hi_max; W_sw_hi = W_sw_hi + W_sw_step)
        	{

					//for(C = C_min/N; C <= C_max/N; C = C + C_step/N)
					for(C = 0; C <= C_max/N; C = C + C_step/N)
					{
						for(L = 0; L <= L_max/N; L = L + L_step/N)
						//for(L = L_min/N; L <= L_max/N; L = L + L_step/N)
						{

							Actual_area = N * (Area_driver_controller + (W_sw_hi + W_sw_low)*Area_driver + L/L_density + C/C_density);
							if(Actual_area <= Area)
								{

									I_ind = I_load/N;
									Delta_i = (V_in-V_out)*D/(F_sw*L);
									Delta_i = Delta_i * N * (D-(floor(D*N)/N)) * ((1+floor(D*N))/N - D) / ((1-D)*D);
										//Delta_v = (Delta_i/(8*F_sw*C))*(0.25*(1/(D*(1-D))))*(1/(N*N));
										//Delta_v = (Delta_i/(8*F_sw*C))*(0.25*(1/(D*(1-D))))*(1/(N));
										//Delta_v = (Delta_i/(F_sw*C*N));
										Delta_v = (Delta_i/(F_sw*C));

									if(Delta_v <= Delta_v_range)
									{
										P_driver = ((C_bridge+C_driver)*(W_sw_hi+W_sw_low))*V_driver*F_sw;
										P_control = I_control*V_driver;
										P_sw = (D*R_sw/W_sw_hi + (1-D)*R_sw/W_sw_hi)*(I_ind*I_ind + (1/12)*Delta_i*Delta_i) + Leak_sw*(W_sw_hi+W_sw_low) + Switching_loss*(W_sw_hi+W_sw_low)*I_ind*F_sw;
										P_ind = R_ind * L * (I_ind*I_ind + (1/12)*Delta_i*Delta_i);
										Converter_power = (P_driver + P_control + P_sw + P_ind)*N;
										Efficiency_tep = P_load/(P_load + Converter_power);
										if(Efficiency_tep > Efficiency)
										{
											Efficiency = Efficiency_tep;
											F_sw_tep = F_sw;
											W_sw_hi_tep = W_sw_hi;
											W_sw_low_tep = W_sw_low;
											L_tep = L;
											C_tep = C;
											L_total = L*N;
											C_total = C*N;
											N_tep = N;

											cout << "" << endl;
											cout << "Efficiency: "<< Efficiency << endl;
											cout << "Area: " << Actual_area*1000000 << endl;
											cout << "Ripple: " << Delta_v << endl;
											cout << "F_sw: "<< F_sw_tep << endl;
											cout << "W_sw_hi: "<< W_sw_hi_tep << endl;
											cout << "W_sw_low: "<< W_sw_low_tep << endl;
											cout << "L_total: "<< L_total << endl;
											cout << "C_total: "<< C_total << endl;
											cout << "N: "<< N_tep << endl;


											cout << "P_driver: " << P_driver*N << endl;
											cout << "P_control: " << P_control*N << endl;
											cout << "P_sw: " << P_sw*N << endl;
											cout << "P_ind: " << P_ind*N << endl;
										}
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
    C_max = Min_Area*C_density;
    double L_step = (Min_Area/10)*L_density;
		L_max = Min_Area*L_density;
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
				for(W_sw_low = W_sw_low_min; W_sw_low <= W_sw_low_max; W_sw_low = W_sw_low + W_sw_step)
		        {

					for(W_sw_hi = W_sw_hi_min; W_sw_hi <= W_sw_hi_max; W_sw_hi = W_sw_hi + W_sw_step)
		        	{

							for(C = 0; C <= C_max/N; C = C + C_step/N)
							{

								for(L = 0; L <= L_max/N; L = L + L_step/N)
								{
									Actual_area = N * (Area_driver_controller + (W_sw_hi + W_sw_low)*Area_driver + L/L_density + C/C_density);
									if(Actual_area <= Min_Area)
										{
										I_ind = I_load/N;
										Delta_i = (V_in-V_out)*D/(F_sw*L);
										  Delta_i = Delta_i * N * (D-(floor(D*N)/N)) * ((1+floor(D*N))/N - D) / ((1-D)*D);
											Delta_v = (Delta_i/(F_sw*C));
											if(Delta_v <= Delta_v_range)
											{
												P_driver = ((C_bridge+C_driver)*(W_sw_hi+W_sw_low))*V_driver*F_sw;
												P_control = I_control*V_driver;
												P_sw = (D*R_sw/W_sw_hi + (1-D)*R_sw/W_sw_hi)*(I_ind*I_ind + (1/12)*Delta_i*Delta_i) + Leak_sw*(W_sw_hi+W_sw_low) + Switching_loss*I_ind*F_sw;
												P_ind = R_ind * L * (I_ind*I_ind + (1/12)*Delta_i*Delta_i);
												Converter_power = (P_driver + P_control + P_sw + P_ind)*N;
												Efficiency_tep = P_load/(P_load + Converter_power);
												if(Efficiency_tep > Acceptable_Efficiency)
												{
													Efficiency = Efficiency_tep;
													F_sw_tep = F_sw;
													W_sw_hi_tep = W_sw_hi;
													W_sw_low_tep = W_sw_low;
													L_tep = L;
													C_tep = C;
													L_total = L*N;
													C_total = C*N;
													N_tep = N;

													cout << "" << endl;
													cout << "Efficiency: "<< Efficiency << endl;
													cout << "Area: " << Actual_area*1000000 << endl;
													cout << "Ripple: " << Delta_v << endl;
													cout << "F_sw: "<< F_sw_tep << endl;
													cout << "W_sw_hi: "<< W_sw_hi_tep << endl;
													cout << "W_sw_low: "<< W_sw_low_tep << endl;
													cout << "L_total: "<< L_total << endl;
													cout << "C_total: "<< C_total << endl;
													cout << "N: "<< N_tep << endl;


													cout << "P_driver: " << P_driver*N << endl;
													cout << "P_control: " << P_control*N << endl;
													cout << "P_sw: " << P_sw*N << endl;
													cout << "P_ind: " << P_ind*N << endl;
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

  }

cout << "Sorry! We cannot find the min area you want. Try to change area boundary." << endl;

}

}
