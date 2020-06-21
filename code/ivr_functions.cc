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

using namespace std;

int simulation_time;
int Load_Period;

//This function sets the load period, ivr period, and simulation time
void setperiod_load_ivr_Simulation(int T_load, int T_simulation)
{
  simulation_time = T_simulation;
  Load_Period = T_load;

  string a,c;
  //set load update time
  for(int k = 1; k< T_simulation/T_load; k++)
  {
  a = "stop when time = ";
  c = "p";
  int tep;
  tep = T_load*k;    //curent load update time in ps.
  stringstream ss;
  ss << tep;
  string b = ss.str();
  a+=b;
  a+=c;
  const char* stop_command = a.c_str();

  citcuit_simulator_instruction(stop_command);

  }

}


void start_simulation()
{
  string powertrace = "GPU_Power.txt";
  ifstream infile(powertrace.c_str());
  string Current_Load[24]; // The GPU Power
  //float Delta_voltage1 = 0;


  citcuit_simulator_instruction("bg_run");
  int current_time;
  while(true)
  {
    usleep (10000);
    current_time = get_time();
    cout << "-----------current time: "<< current_time << endl;

    //finish simulation if it reaches the finish time and output note voltage
    if(current_time >= simulation_time)
    {
      //Stop back thread simulaton
      citcuit_simulator_instruction("bg_stop");
      //Set output file format
      citcuit_simulator_instruction("set filetype=ascii");
      citcuit_simulator_instruction("wrdata SM1 V(1000000001)");
      citcuit_simulator_instruction("wrdata SM2 V(1000006001)");
      citcuit_simulator_instruction("wrdata SM3 V(1002000001)");
      citcuit_simulator_instruction("wrdata SM4 V(1002006001)");

      printf("Simulation finish!!!!!!!!!\n");
      exit(1);
    }

    //Update Current_Load
    if(current_time%Load_Period == 0)
    {
    cout << "-------------Update Current Load-------------" << endl;
    // Read in the power trace
          string temp;
          getline(infile,temp);
          char s[1024];
          int m;
          for( m=0;m<temp.length();m++)
          {
              s[m] = temp[m];
          }
          const char *d = " ,*";
          char *p;
          p = strtok(s,d);
          int outputloadnum = 0;

          while(outputloadnum < 24)
          {
              //printf("%s\n",p);
              double a = atof(p);

              stringstream resistor_string;
              resistor_string << a;


              Current_Load[outputloadnum] = resistor_string.str();

              outputloadnum = outputloadnum + 1;
              p=strtok(NULL,d);
          }


        string alter[24];
        // SM1
        alter[0] = "alter r1000000001_1000000002 = ";
        alter[1] = "alter r1000002001_1000002002 = ";
        alter[2] = "alter r1000004001_1000004002 = ";
        alter[3] = "alter r1002000001_1002000002 = ";
        alter[4] = "alter r1002002001_1002002002 = ";
        alter[5] = "alter r1002004001_1002004002 = ";
        // SM2
        alter[6] = "alter r1000006001_1000006002 = ";
        alter[7] = "alter r1000008001_1000008002 = ";
        alter[8] = "alter r1000010001_1000010002 = ";
        alter[9] = "alter r1002006001_1002006002 = ";
        alter[10] = "alter r1002008001_1002008002 = ";
        alter[11] = "alter r1002010001_1002010002 = ";
        // SM3
        alter[12] = "alter r1004000001_1004000002 = ";
        alter[13] = "alter r1004002001_1004002002 = ";
        alter[14] = "alter r1004004001_1004004002 = ";
        alter[15] = "alter r1006000001_1006000002 = ";
        alter[16] = "alter r1006002001_1006002002 = ";
        alter[17] = "alter r1006004001_1006004002 = ";
        // SM4
        alter[18] = "alter r1004006001_1004006002 = ";
        alter[19] = "alter r1004008001_1004008002 = ";
        alter[20] = "alter r1004010001_1004010002 = ";
        alter[21] = "alter r1006006001_1006006002 = ";
        alter[22] = "alter r1006008001_1006008002 = ";
        alter[23] = "alter r1006010001_1006010002 = ";

        //string cc = "A";

        stringstream load;



        for (int n = 0; n < 24; n++)
        {
        string bb = Current_Load[n];
        string aa = alter[n];
        aa+=bb;
        //aa+=cc;
        const char* alter = aa.c_str();
        cout << n << ": " << alter << endl;

        //ret = ((int * (*)(const char*)) ngSpice_Command_handle)(alter);
        citcuit_simulator_instruction(alter);
        }
    cout << "-------------Finish Updating Current Load-------------" << endl;
    }




    citcuit_simulator_instruction("bg_resume");

  }

}
