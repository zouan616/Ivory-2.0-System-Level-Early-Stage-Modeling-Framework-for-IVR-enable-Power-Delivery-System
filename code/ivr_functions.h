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
#include "ivr.h"

void setperiod_load_ivr_Simulation(int T_load, int T_simulation);
float measure_voltage();
void start_simulation();
