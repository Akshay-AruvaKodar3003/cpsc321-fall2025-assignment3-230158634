/* 
Akshay Arulkrishnan
230158634
CPSC 321 - Assignment 3
31-10-2025
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// Thread function to test pthread setup
void* worker(void* arg) {
    (void)arg; 
    printf("[worker] pthread started\n");
    sleep(1); 
    return NULL;
}

// One process 
typedef struct {
    int pid;                 // process id
    int arrival_time;        // arrival time
    int burst_time;          // CPU burst time
    int start_time;          // when it actually starts
    int finish_time;         // when it completes
    int waiting_time;        // finish - arrival - burst
    int turnaround_time;     // finish - arrival
    int cpu_id;              // which CPU
    int done;            
} Proc;

// Overall simulation
typedef struct {
    Proc *plist;             // array of processes
    int total_procs;         // total count
} Sim;

static pthread_mutex_t sim_lock = PTHREAD_MUTEX_INITIALIZER;
static int global_time = 0;
static int total_finished = 0;
static Sim *shared_sim = NULL;

static void sim_init(Sim *s, Proc *p, int n) {
    s->plist = p;
    s->total_procs = n;
    global_time = 0;
    total_finished = 0;

    for (int i = 0; i < n; i++) {
        p[i].start_time = -1;
        p[i].finish_time = -1;
        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].cpu_id = -1;
        p[i].done = 0;
    }
}
// CPU worker (each core)
void* cpu_worker(void* arg) {
    int cpu_id = (int)(long)arg;
    int local_time = 0;

    while (1) {
        pthread_mutex_lock(&sim_lock);

        // all done
        if (total_finished >= shared_sim->total_procs) {
            pthread_mutex_unlock(&sim_lock);
            break;
        }

        // find shortest ready job
        int idx = -1;
        int min_burst = 1e9;
        for (int i = 0; i < shared_sim->total_procs; i++) {
            Proc *p = &shared_sim->plist[i];
            if (!p->done && p->arrival_time <= global_time) {
                if (p->burst_time < min_burst) {
                    min_burst = p->burst_time;
                    idx = i;
                }
            }
        }

        // none ready, CPU idle
        if (idx == -1) {
            pthread_mutex_unlock(&sim_lock);
            usleep(50000); 
            global_time++;
            continue;
        }

        // assign and update
        Proc *p = &shared_sim->plist[idx];
        p->cpu_id = cpu_id;
        p->start_time = global_time;
        global_time += p->burst_time;
        p->finish_time = global_time;
        p->turnaround_time = p->finish_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;
        p->done = 1;
        total_finished++;

        // Update Clocks
        local_time = p->finish_time;
        if (local_time > global_time)
            global_time = local_time;

        pthread_mutex_unlock(&sim_lock);

        // simulate CPU run
        usleep(p->burst_time * 10000);
    }
    return NULL;
}

// Multi-CPU SJF sim
static void run_sjf_multicpu(Sim *s) {
    shared_sim = s;
    global_time = 0;
    total_finished = 0;

    pthread_t cpu0, cpu1;
    pthread_create(&cpu0, NULL, cpu_worker, (void*)0);
    pthread_create(&cpu1, NULL, cpu_worker, (void*)1);

    pthread_join(cpu0, NULL);
    pthread_join(cpu1, NULL);
}

// Print Averages (Waiting and Turnaround Times)
static void print_avg(Sim *s) {
    double total_wait = 0, total_turn = 0;
    for (int i = 0; i < s->total_procs; i++) {
        total_wait += s->plist[i].waiting_time;
        total_turn += s->plist[i].turnaround_time;
    }
    printf("\nAverage Waiting Time: %.2f", total_wait / s->total_procs);
    printf("\nAverage Turnaround Time: %.2f\n", total_turn / s->total_procs);
}

// Main
int main(void) {
    printf("Assignment 3 running\n");

    // static arrays (from specifications)
    static const int N = 5;
    static const int arrival[]  = {0,1,2,3,4};
    static const int burst[]    = {10,5,8,6,3};

    Proc data[N];
    for (int i = 0; i < N; i++) {
        data[i].pid = i+1;
        data[i].arrival_time = arrival[i];
        data[i].burst_time = burst[i];
    }

    Sim sim;
    sim_init(&sim, data, N);

    run_sjf_multicpu(&sim);

    printf("\nProcess | Arrival | Burst | CPU | Wait | Turnaround\n");
    printf("-----------------------------------------------------\n");

    for (int i = 0; i < sim.total_procs; i++) {
        Proc *p = &sim.plist[i];

        printf("p%1d      | %7d | %5d | %3d | %4d | %10d\n",
               p->pid, p->arrival_time, p->burst_time,
               p->cpu_id, p->waiting_time, p->turnaround_time);
    }

    print_avg(&sim);
    return 0;
}