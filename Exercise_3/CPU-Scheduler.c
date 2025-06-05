#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#define MAX_PROCESSES 1000
#define MAX_NAME_LENGTH 50
#define MAX_DESCRIPTION_LENGTH 100
#define MAX_LINE_LENGTH 256
void Idle(int start, int end);
typedef struct
{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    int arrivalTime; // in seconds
    int burstTime;   // in seconds
    int priority;    // lower number means higher priority
    int endTime;     // in seconds
    int startTime;   // in seconds
    __pid_t pid;     // Process ID
} Process;

typedef struct
{
    Process processes[MAX_PROCESSES];
    int front;
    int rear;
    int size;
} queue;
float calculateAverageWaitingTime(Process process[], int processCount, int start)
{
    float totalWaitingTime = 0;
    for (int i = 0; i < processCount; i++)
    {
        if (process[i].endTime != -1) // Ensure the process has completed
        {
            int waitingTime = process[i].endTime - process[i].arrivalTime - process[i].burstTime;
            totalWaitingTime += waitingTime;
        }
        else
        {
            fprintf(stderr, "Process %d has not completed yet (endTime = -1)\n", i);
            return -1; // Return -1 to indicate an error if any process has not completed
        }
    }
    return totalWaitingTime / processCount; // Return the average waiting time
}

void printSchedulerHeader(const char *mode)
{
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : %s\n", mode);
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n");
}
void printSchedulerSummary(float avgWaitingTime)
{
    printf("──────────────────────────────────────────────\n");
    printf(">> Engine Status  : Completed\n");
    printf(">> Summary        :\n");
    printf("   └─ Average Waiting Time : %.2f time units\n", avgWaitingTime);
    printf(">> End of Report\n");
    printf("══════════════════════════════════════════════\n");
}
queue *createQueue()
{
    queue *q = (queue *)malloc(sizeof(queue));
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    return q;
}
void printProcess(Process *process);
void enqueue(queue *q, Process p)
{
    if (q->size < MAX_PROCESSES)
    {
        q->rear = (q->rear + 1) % MAX_PROCESSES;
        q->processes[q->rear] = p;
        q->size++;
    }
}
Process dequeue(queue *q)
{
    if (q->size > 0)
    {
        Process p = q->processes[q->front];
        q->front = (q->front + 1) % MAX_PROCESSES;
        q->size--;
        return p;
    }
    Process emptyProcess = {0}; // Return an empty process if queue is empty
    return emptyProcess;
}

/*

    Each line in the input CSV file represents a single process and follows this format:
    Name,Description,Arrival Time,Burst Time,Priority
    The fields are comma-separated and should appear in the specified order.
    Arrival Time, Burst Time, and Priority are integers representing time in seconds.
    Burst Time and Priority must be positive valuse.
    The Name field will not exceed 50 characters, and the Description field will not exceed 100 characters.
    You can assume that the file format is valid and correctly structured.
    You may assume there will be no more than 1000 processes, and each row in the CSV file will contain no more than 256 characters.
*/

/*

To simulate the execution time of each process, use the alarm system call with the process's burst time. This will pause the process for the specified duration, effectively mimicking actual CPU work.
Use alarm similarly to simulate idle time when no processes are ready to run — representing the CPU waiting for the next arrival.

*/
void sortbyArrivalTime(Process *process, int processCount)
{
    // Simple bubble sort to sort processes by arrival time
    for (int i = 0; i < processCount - 1; i++)
    {
        for (int j = 0; j < processCount - i - 1; j++)
        {
            if (process[j].arrivalTime > process[j + 1].arrivalTime)
            {
                Process temp = process[j];
                process[j] = process[j + 1];
                process[j + 1] = temp;
            }
        }
    }
}
void signal_handler_process(int signum)
{
    // do nothing
}
void runProcess(Process *p, int currentTime)
{
    // Register signal handler for SIGALRM
    signal(SIGALRM, signal_handler_process);
    struct sigaction sa;
    sa.sa_handler = signal_handler_process;
    sa.sa_flags = 0;          // No special flags
    sigemptyset(&sa.sa_mask); // Initialize the signal mask to empty

    sigaction(SIGALRM, &sa, NULL);
    // Set an alarm for the specified time (in seconds)
    alarm(p->burstTime);

    // Simulate process execution
    p->startTime = currentTime;              // Set the start time of the process
    pause();                                 // Wait for a signal to be received
    p->endTime = currentTime + p->burstTime; // Set the end time of the process
    printProcess(p);
}
void rrScheduler(Process process[], int processCount, int timeQuantum);
void fcfsScheduler(Process process[], int processCount);
void sjfScheduler(Process process[], int processCount);
void priorityScheduler(Process process[], int processCount);
void processLine(char *line, Process *process)
{
    char *token = strtok(line, ",");
    if (token != NULL)
    {
        strncpy(process->name, token, MAX_NAME_LENGTH);
        process->name[MAX_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        // process->name[strlen(token)] = '\0'; // Ensure null termination
    }

    token = strtok(NULL, ",");
    if (token != NULL)
    {
        strncpy(process->description, token, MAX_DESCRIPTION_LENGTH);
        process->description[MAX_DESCRIPTION_LENGTH - 1] = '\0'; // Ensure null termination
    }

    token = strtok(NULL, ",");
    if (token != NULL)
    {
        process->arrivalTime = atoi(token);
    }

    token = strtok(NULL, ",");
    if (token != NULL)
    {
        process->burstTime = atoi(token);
    }

    token = strtok(NULL, ",");
    if (token != NULL)
    {
        process->priority = atoi(token);
    }
    process->endTime = -1; // Initialize end time to -1 to indicate it hasn't been set yet
}
void initliazeAllProcesses(Process *processes, int *count, char *processesCsvFilePath)
{
    int processCount = 0;
    // open the file
    FILE *file = fopen(processesCsvFilePath, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    // Read the file line by line
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    do
    {
        read = getline(&line, &len, file);
        if (read == -1)
        {
            break; // End of file reached
        }
        char *lineCopy = strdup(line); // Make a separate copy
        if (!lineCopy)
        {
            perror("strdup failed");
            exit(EXIT_FAILURE);
        }
        processLine(lineCopy, &processes[processCount]);
        free(lineCopy); // Free the copy after processing
        processCount++;
    } while (processCount < MAX_PROCESSES);

    *count = processCount;
    if (line)
        free(line); // Free the line buffer
    fclose(file);
}

void runCPUScheduler(char *processesCsvFilePath, int timeQuantum)
{
    Process processes[MAX_PROCESSES];
    int processCount = 0;
    initliazeAllProcesses(processes, &processCount, processesCsvFilePath);

    fcfsScheduler(processes, processCount);
    sjfScheduler(processes, processCount);
    priorityScheduler(processes, processCount);
    rrScheduler(processes, processCount, timeQuantum);
}

void rrScheduler(Process process[], int processCount, int timeQuantum)
{
    /*
    need a ready queue
    after every second need to update the ready queue
    need to keep track of the remaining burst time for each process (allready in the struct, just need to update it)
       └─ Total Turnaround Time : 13 time units
    */
}

void fcfsScheduler(Process process[], int processCount)
{
    printSchedulerHeader("FCFS");
    queue *readyQueue = createQueue();
    sortbyArrivalTime(process, processCount); // Sort processes by arrival time before scheduling
    int currentTime = 0;                      // Initialize current time to 0
    int processLeft = processCount;           // Keep track of the number of processes left to run
    for (int i = 0; i < processCount; i++)
    {
        if (process[i].arrivalTime > currentTime)
        {
            Idle(currentTime, process[i].arrivalTime);
            currentTime = process[i].arrivalTime; // Update current time to the arrival time of the next process
        }
        __pid_t pid = fork(); // Create a new process
        if (pid < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) // process pov
        {
            // Set the process ID in the struct
            process[i].pid = getpid();
            // Run the process for its burst time
            runProcess(&process[i], currentTime);
            // Set the end time for the process
            exit(0); // Exit child process after running
        }
        else // schoudler pov
        {
            wait(NULL); // Wait for the child process to finish
            process[i].endTime = currentTime + process[i].burstTime;
            currentTime += process[i].burstTime; // Update current time after the process finishes
        }
    }
    float avg = calculateAverageWaitingTime(process, processCount, 0);
    printSchedulerSummary(avg);
}
void sjfScheduler(Process process[], int processCount)
{
    /*
        need to sort the process by burst time
        need to keep track of the remaining burst time for each process (allready in the struct, just need to update it)
        need to keep track on the arrival of new processes ->linked list after the first sort? min heap?
        need to keep track of     Average Waiting Time
    */
}

void priorityScheduler(Process process[], int processCount)
{
    /*
    need to have a min heap to hold the processes in the order of their priority
    every time
    need to sort the process by priority
    need to keep track of the remaining burst time for each process (allready in the struct, just need to update it)
    need to keep track on the arrival of new processes ->linked list after the first sort? min heap?
    need to keep track of        └─ Average Waiting Time : 2.00 time units
    */
}

// TODO: impliment a sorting method to sort the processes by arrival time
// TODO: impliment a sorting method to sort the processes by burst time, need to be stable
// TODO: impliment a sorting method to sort the processes by priority, need to be stable

void Idle(int start, int end)
{
    signal(SIGALRM, signal_handler_process);
    struct sigaction sa;
    sa.sa_handler = signal_handler_process;
    sa.sa_flags = 0;          // No special flags
    sigemptyset(&sa.sa_mask); // Initialize the signal mask to empty

    sigaction(SIGALRM, &sa, NULL);
    // Set an alarm for the specified time (in seconds)
    int burstTime = end - start; // Calculate the idle time
    alarm(burstTime);
    pause(); // Wait for a signal to be received

    printf("%d → %d: Idle.\n", start, end);
}
void printProcess(Process *process)
{
    printf("%d → %d: %s Running %s.\n", process->startTime, process->endTime, process->name, process->description);
}
void printProcesses(Process *processes, int processCount)
{
    for (int i = 0; i < processCount; i++)
    {
        printf("--------------------------------------------------\n");
        printf("Process %d:\n", i + 1);
        printProcess(&processes[i]);
        printf("--------------------------------------------------\n");
    }
}
int main()
{
    char processesCsvFilePath[] = "processes1.csv"; // Path to the CSV file containing process information
    int timeQuantum = 2;                            // Time quantum for the Round Robin scheduler
    runCPUScheduler(processesCsvFilePath, timeQuantum);
}