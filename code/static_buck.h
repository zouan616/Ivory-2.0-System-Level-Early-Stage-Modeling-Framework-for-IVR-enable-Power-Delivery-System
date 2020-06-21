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

double Read_in_Technology(int n);
void Static_Buck(double Vin, double Vout, double P, double Area_buget, double ripple, double Efficiency_buget, int Optimization_Target);
