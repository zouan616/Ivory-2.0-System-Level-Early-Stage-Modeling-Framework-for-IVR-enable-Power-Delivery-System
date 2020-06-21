#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include "sharedspice.h"
#include <dlfcn.h> /* to load libraries*/
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "circuit_simulator.h"
#include "ivr_functions.h"
#include "static_buck.h"
#include "static_switched_capacitor.h"


using namespace std;

extern int *ret;


int main()
{
  //----------------Static Optimization-----------------
  Static_Buck(double Vin, double Vout, double P, double Area_buget, double ripple, double Efficiency_buget, int Optimization_Target 0 for efficiency 1 for area)
  Static_Buck(1.8, 1, 15, 400, 0.01, 0.8, 0);

  // Static_Switched_Capacitor(double Vin, double Vout, double P, double Area_buget, double ripple, double Efficiency_buget, int Optimization_Target 0 for efficiency 1 for area)
  //Static_Switched_Capacitor(3.3, 1, 20, 200, 0.01, 0.7, 0);



  //----------------Dynamic Optimization---------------
  //circuit_simulator_initialization();
  //read in circuit
  //citcuit_simulator_instruction("OnChipPDN.cir");
  //set load period and simulation time in ps
  //setperiod_load_ivr_Simulation(1400, 3000000);
  /start_simulation();

  return 0;
}
