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


class IVR
{
public:
IVR();
float measure_voltage(const char* measure_voltage_instruction, const char* file_name);
const char* IVR_Update_Instruction(const char* ivr_num, float update_voltage);
float Trans_Fun_Calculation(float input_voltage);

private:
//The initial value of IVR Output
float Delta_voltage;

};
