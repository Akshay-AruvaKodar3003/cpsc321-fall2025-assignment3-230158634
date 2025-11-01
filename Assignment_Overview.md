Akshay Arulkrishnan
230158634
CPSC - 321 Operating Systems
Assignment - 3

The goal of this assignment is to design and implement a CPU Scheduling Simulator in C that models how multiple CPUs handle processes using the Shortest Job First (SJF) Scheduling.

The simulator uses two threads (CPUs) that execute jobs concurrently while sharing a common ready queue, protected by a mutex lock to prevent race conditions. 
Each CPU selects the process with the shortest burst time among all processes that have already arrived.

The simulation reports each process’s:
     -> CPU assignment
     -> Arrival time and burst time
     -> Waiting time and turnaround time

At the end, it calculates and displays the average waiting time and average turnaround time across all processes.
