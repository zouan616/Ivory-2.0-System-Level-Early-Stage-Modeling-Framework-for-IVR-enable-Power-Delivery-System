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


#define true 1
#define false 0
//#define RTLD_LAZY	1	/* lazy function call binding */
//#define RTLD_NOW	2	/* immediate function call binding */
//#define RTLD_GLOBAL	4	/* symbols in this dlopen'ed obj are visible to other dlopen'ed objs */


/* callback functions used by ngspice */

funptr_t dlsym (void *, const char *);
funptr_t ngSpice_Init_handle = NULL;
funptr_t ngSpice_Command_handle = NULL;
funptr_t ngSpice_Circ_handle = NULL;
funptr_t ngSpice_CurPlot_handle = NULL;
funptr_t ngSpice_AllVecs_handle = NULL;
funptr_t ngSpice_GVI_handle = NULL;

  bool no_bg = true;
  bool not_yet = true;
  bool will_unload = false;
  int vecgetnumber = 0;
  double v2dat;
  static bool has_break = false;

  int i;
  char ** circarray;
  char **vecarray;

  int ng_getchar(char* outputreturn, int ident, void* userdata);
  int ng_getstat(char* outputreturn, int ident, void* userdata);
  int ng_thread_runs(bool noruns, int ident, void* userdata);
  int cieq(const char *p, const char *s);

  ControlledExit ng_exit;
  SendData ng_data;
  SendInitData ng_initdata;

  int *ret;




void * ngdllhandle = NULL;


void citcuit_simulator_instruction(const char* instruction)
{
ret = ((int * (*)(const char*)) ngSpice_Command_handle)(instruction);
}



void circuit_simulator_initialization(){




    //open handle
    const char *loadstring = "libngspice.so";
    char *errmsg = NULL;
    char *curplot;
    ngdllhandle = dlopen(loadstring, RTLD_LAZY);
    errmsg = dlerror();
    if (errmsg)
        printf("%s\n", errmsg);string temp;


    if (ngdllhandle)
       printf("ngspice dynamic link library loaded successfully\n");
    else {
       printf("ngspice dynamic link library not loaded !\n");
       exit(1);
    }

    ngSpice_Init_handle = dlsym(ngdllhandle, "ngSpice_Init");
    errmsg = dlerror();
    if (errmsg)
        cout << errmsg <<endl;
    ngSpice_Command_handle = dlsym(ngdllhandle, "ngSpice_Command");
    errmsg = dlerror();
    if (errmsg)
        cout << errmsg <<endl;
    ngSpice_CurPlot_handle = dlsym(ngdllhandle, "ngSpice_CurPlot");
    errmsg = dlerror();
    if (errmsg)
        cout << errmsg <<endl;
    ngSpice_AllVecs_handle = dlsym(ngdllhandle, "ngSpice_AllVecs");
    errmsg = dlerror();
    if (errmsg)
        cout << errmsg <<endl;
    ngSpice_GVI_handle = dlsym(ngdllhandle, "ngGet_Vec_Info");
    errmsg = dlerror();
    if (errmsg)
        cout << errmsg <<endl;



    ret = ((int * (*)(SendChar*, SendStat*, ControlledExit*, SendData*, SendInitData*, BGThreadRunning*, void*)) ngSpice_Init_handle)(ng_getchar, ng_getstat, ng_exit, NULL, ng_initdata, ng_thread_runs, NULL);

    //read in circuit



}


int ng_getchar(char* outputreturn, int ident, void* userdata)
{
    printf("%s\n", outputreturn);
    return 0;
}

/* Callback function called from bg thread in ngspice to transfer
   simulation status (type and progress in percent. */
int ng_getstat(char* outputreturn, int ident, void* userdata)
{
    printf("%s\n", outputreturn);
    return 0;
}

/* Callback function called from ngspice upon starting (returns true) or
  leaving (returns false) the bg thread. */
int ng_thread_runs(bool noruns, int ident, void* userdata)
{
    no_bg = noruns;
    if (noruns)
        printf("bg not running\n");
    else
        printf("bg running\n");

    return 0;
}

int ng_exit(int exitstatus, bool immediate, bool quitexit, int ident, void* userdata)
{

    if(quitexit) {
        printf("DNote: Returned from quit with exit status %d\n", exitstatus);
    }
    if(immediate) {
        printf("DNote: Unload ngspice\n");
        ((int * (*)(const char*)) ngSpice_Command_handle)("bg_pstop");

        dlclose(ngdllhandle);
    }

    else {
        printf("DNote: Prepare unloading ngspice\n");
        will_unload = true;
    }

    return exitstatus;

}


/* Callback function called from bg thread in ngspice once per accepted data point */
int ng_data(pvecvaluesall vdata, int numvecs, int ident, void* userdata)
{
    int *ret;

    v2dat = vdata->vecsa[vecgetnumber]->creal;
    if (!has_break && (v2dat > 0.5)) {
    /* using signal SIGTERM by sending to main thread, alterp() then is run from the main thread,
      (not on Windows though!)  */

        has_break = true;
    /* leave bg thread for a while to allow halting it from main */

        usleep (100000);

//        ret = ((int * (*)(char*)) ngSpice_Command_handle)("bg_halt");
    }
    return 0;
}

int ng_initdata(pvecinfoall intdata, int ident, void* userdata)
{
    int i;
    int vn = intdata->veccount;
    for (i = 0; i < vn; i++) {
        printf("Vector: %s\n", intdata->vecs[i]->vecname);
        /* find the location of V(2) */
        if (cieq(intdata->vecs[i]->vecname, "V(2)"))
            vecgetnumber = i;
    }
    return 0;
}

int cieq(const char *p, const char *s)
{
    while (*p) {
        if ((isupper(*p) ? tolower(*p) : *p) !=
            (isupper(*s) ? tolower(*s) : *s))
            return(false);
        p++;
        s++;
    }
    return (*s ? false : true);
}


int get_time()
{
  // Get the stop time
  citcuit_simulator_instruction("bg_halt");
  citcuit_simulator_instruction("print time[length(time)-1] >> time.txt");
  string time;
  const char* stop_time;
  string time_file_name;
  time_file_name = "time.txt";
  ifstream time_file_name_infile(time_file_name.c_str());
  getline(time_file_name_infile,time);
  time = time.erase(0,23);
  stop_time = time.c_str();
  FILE* fp = fopen("time.txt","w");
  fclose(fp);

  int int_time = (int)(atof(stop_time)*1e12);
  cout << "----------- before get time current time: "<< int_time << endl;
  if(int_time%2 == 1)
  {
    int_time = int_time + 1;
  }
  cout << "----------- get time current time: "<< int_time << endl;
  return int_time;
}
