#include <time.h>
#include "hi_treem.c"

int main(argc, argv)

    int argc;
char *argv[];

{
    PreprocData *pd = walloc(1,sizeof(PreprocData));
    //no mode 2, so it is just randomly picking up children
    pd->P = 500; // P% percent of total passes in mode 1, the remaining in mode 2, 300 is mode 3, 400 is mode 4, 500 for mode 5
    pd->total = 4; //number of total passes
    // pd->LEVEL_NUM = 5, and target con
    pd->K = 5;


    loadGraphData(pd); //load graph data from standard input

    struct timespec time_start={0,0},time_end={0,0};

    clock_gettime(CLOCK_REALTIME,&time_start);
    initPreprocData(pd); //init data structure
    clock_gettime(CLOCK_REALTIME,&time_end);

    double tm1 = (10e9*time_end.tv_sec +time_end.tv_nsec - 10e9*time_start.tv_sec - time_start.tv_nsec)/10e9;

    clock_gettime(CLOCK_REALTIME,&time_start);
    preProc(pd); // preproc by traversing the graph for pd->total times
    clock_gettime(CLOCK_REALTIME,&time_end);
    double tm2 = (10e9*time_end.tv_sec +time_end.tv_nsec - 10e9*time_start.tv_sec - time_start.tv_nsec)/10e9;


    clock_gettime(CLOCK_REALTIME,&time_start);
    buildMCT(pd);
    clock_gettime(CLOCK_REALTIME,&time_end);
    double tm3 = (10e9*time_end.tv_sec +time_end.tv_nsec - 10e9*time_start.tv_sec - time_start.tv_nsec)/10e9;;

    //calcuRandomPairs(1,pd); // randomly choose 100 node pairs and calcu their min-cut and output
  
    printf("the final total time: %lf (%lf %lf %lf)\n",(tm1+tm2+tm3),tm1,tm2,tm3);
    // calcuRandomPairs(1000,pd); // randomly choose 100 node pairs and calcu their min-cut and output
  
    exit(0);

}
