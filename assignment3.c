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
    int remaining_time;      // remaining time 
    int started;         
    int done;            
} Proc;

// Overall simulation
typedef struct {
    Proc *plist;             // array of processes
    int total_procs;         // total count
    int finished;            // how many done
    int current_time;        // simulation clock
    pthread_mutex_t lock;    // shared lock (for threads later)
} Sim;

static void sim_init(Sim *s, Proc *p, int n) {
    s->plist = p;
    s->total_procs = n;
    s->finished = 0;
    s->current_time = 0;
    pthread_mutex_init(&s->lock, NULL);

    for (int i = 0; i < n; i++) {
        s->plist[i].start_time = -1;
        s->plist[i].finish_time = -1;
        s->plist[i].waiting_time = 0;
        s->plist[i].turnaround_time = 0;
        s->plist[i].remaining_time  = s->plist[i].burst_time;
        s->plist[i].started = 0;
        s->plist[i].done = 0;
    }
}

// FCFS scheduler simulation
static void run_fcfs(Sim *s) {
    printf("\n[Sim] Running FCFS scheduling...\n");

    int time = 0;
    for (int i = 0; i < s->total_procs; i++) {
        Proc *p = &s->plist[i];

        if (time < p->arrival_time)
            time = p->arrival_time;

        p->start_time = time;
        p->finish_time = p->start_time + p->burst_time;
        p->turnaround_time = p->finish_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;

        time = p->finish_time;
        s->current_time = time;
        s->finished++;
    }
}

// SJB scheduler simulation
static void run_sjf(Sim *s) {
    printf("\n[Sim] Running SJF (non-preemptive) scheduling...\n");

    int time = 0, completed = 0;
    while (completed < s->total_procs) {
        int idx = -1;
        int min_burst = 1e9;

        for (int i = 0; i < s->total_procs; i++) {
            Proc *p = &s->plist[i];
            if (!p->done && p->arrival_time <= time) {
                if (p->burst_time < min_burst) {
                    min_burst = p->burst_time;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        Proc *p = &s->plist[idx];
        p->start_time = time;
        p->finish_time = time + p->burst_time;
        p->waiting_time = p->start_time - p->arrival_time;
        p->turnaround_time = p->finish_time - p->arrival_time;
        p->done = 1;
        time = p->finish_time;
        completed++;
    }
}

// Round Robin scheduler simulation
static void run_rr(Sim *s, int quantum) {
     printf("\n[Sim] Running Round Robin (q=%d)...\n", quantum);

    int time = 0, completed = 0;
    int queue[100];     // simple circular queue
    int front = 0, rear = 0;
    bool in_queue[100] = {false};

    // Enqueue any processes that have arrived at time 0
    for (int i = 0; i < s->total_procs; i++) {
        if (s->plist[i].arrival_time == 0) {
            queue[rear++] = i;
            in_queue[i] = true;
        }
    }

    while (completed < s->total_procs) {
        if (front == rear) {
            // no process ready, advance time
            time++;
            for (int i = 0; i < s->total_procs; i++) {
                if (!in_queue[i] && s->plist[i].arrival_time <= time && !s->plist[i].done) {
                    queue[rear++] = i;
                    in_queue[i] = true;
                }
            }
            continue;
        }

        int i = queue[front++]; // dequeue
        Proc *p = &s->plist[i];
        if (!p->started) {
            p->start_time = time;
            p->started = 1;
        }

        int exec = (p->remaining_time > quantum) ? quantum : p->remaining_time;
        p->remaining_time -= exec;
        time += exec;

        // enqueue new arrivals that came during this quantum
        for (int j = 0; j < s->total_procs; j++) {
            if (!in_queue[j] && s->plist[j].arrival_time <= time && !s->plist[j].done) {
                queue[rear++] = j;
                in_queue[j] = true;
            }
        }

        if (p->remaining_time == 0 && !p->done) {
            p->done = 1;
            p->finish_time = time;
            p->turnaround_time = p->finish_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            completed++;
        } else {
            // re-queue unfinished process
            queue[rear++] = i;
        }
    }
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


int main(void) {
    printf("Assignment 3 running\n");

    pthread_t t; // Just one thread 

    // Creating the thread 
    if (pthread_create(&t, NULL, worker, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    // Waiting for the thread to finish
    pthread_join(t, NULL);

    printf("[main] pthread join OK\n");

     // sample array
    Proc data[] = {
        {1, 0, 3, 0, 0, 0, 0, 0, 0, 0},
        {2, 1, 5, 0, 0, 0, 0, 0, 0, 0},
        {3, 2, 2, 0, 0, 0, 0, 0, 0, 0}
    };
    Sim sim;
    sim_init(&sim, data, (int)(sizeof(data)/sizeof(data[0])));

    printf("Loaded %d processes:\n", sim.total_procs);
    for (int i = 0; i < sim.total_procs; i++) {
        printf("  P%d arrival=%d burst=%d\n",
            sim.plist[i].pid,
            sim.plist[i].arrival_time,
            sim.plist[i].burst_time);
    }
    
    // FCFS
    run_fcfs(&sim);

    printf("\nPID | Arrival | Burst | Start | Finish | Wait | Turnaround\n");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < sim.total_procs; i++) {
        Proc *p = &sim.plist[i];
        printf("%3d | %7d | %5d | %5d | %6d | %4d | %10d\n",
            p->pid, p->arrival_time, p->burst_time,
            p->start_time, p->finish_time,
            p->waiting_time, p->turnaround_time);
    }
    print_avg(&sim);

     sim_init(&sim, data, (int)(sizeof(data)/sizeof(data[0])));

     // SJF
    run_sjf(&sim);

    printf("\nPID | Arrival | Burst | Start | Finish | Wait | Turnaround\n");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < sim.total_procs; i++) {
        Proc *p = &sim.plist[i];
        printf("%3d | %7d | %5d | %5d | %6d | %4d | %10d\n",
               p->pid, p->arrival_time, p->burst_time,
               p->start_time, p->finish_time,
               p->waiting_time, p->turnaround_time);
    }
    print_avg(&sim);

    sim_init(&sim, data, (int)(sizeof(data)/sizeof(data[0])));

    // Round Robin
    run_rr(&sim, 3);

    printf("\nPID | Arrival | Burst | Start | Finish | Wait | Turnaround\n");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < sim.total_procs; i++) {
        Proc *p = &sim.plist[i];
        printf("%3d | %7d | %5d | %5d | %6d | %4d | %10d\n",
            p->pid, p->arrival_time, p->burst_time,
            p->start_time, p->finish_time,
            p->waiting_time, p->turnaround_time);
    }
    print_avg(&sim);

    return 0;
}