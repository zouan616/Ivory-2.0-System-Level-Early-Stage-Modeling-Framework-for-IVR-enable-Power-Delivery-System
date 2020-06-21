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

double Static_Switched_Capacitor(double Vin, double Vout, double P, double Area_buget, double ripple, double Efficiency_buget, int Optimization_Target);
