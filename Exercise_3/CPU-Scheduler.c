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
typedef int (*ProcessCompareFn)(const Process *, const Process *);
typedef struct
{
    Process data[MAX_PROCESSES]; // Array to hold the processes
    int size;
    ProcessCompareFn compare;
} MinHeap;
void rrScheduler(Process process[], int processCount, int timeQuantum);
void printProcess(Process *process);
void runCPUScheduler(char *processesCsvFilePath, int timeQuantum);
void swap(Process *a, Process *b)
{
    Process temp = *a;
    *a = *b;
    *b = temp;
}

void initMinHeap(MinHeap *heap, int (*compare)(const Process *, const Process *))
{
    heap->size = 0;
    heap->compare = compare;
}
void heapifyUp(MinHeap *heap, int index)
{
    if (index == 0)
        return;
    int parent = (index - 1) / 2;
    if (heap->compare(&heap->data[index], &heap->data[parent]) < 0)
    {
        Process temp = heap->data[index];
        heap->data[index] = heap->data[parent];
        heap->data[parent] = temp;
        heapifyUp(heap, parent);
    }
}
void heapifyDown(MinHeap *heap, int index)
{
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int smallest = index;

    if (left < heap->size && heap->compare(&heap->data[left], &heap->data[smallest]) < 0)
        smallest = left;
    if (right < heap->size && heap->compare(&heap->data[right], &heap->data[smallest]) < 0)
        smallest = right;

    if (smallest != index)
    {
        Process temp = heap->data[index];
        heap->data[index] = heap->data[smallest];
        heap->data[smallest] = temp;
        heapifyDown(heap, smallest);
    }
}

void insertProcess(MinHeap *heap, Process proc)
{
    heap->data[heap->size] = proc;
    heapifyUp(heap, heap->size);
    heap->size++;
}
Process removeMin(MinHeap *heap)
{
    if (heap->size == 0)
    {
        fprintf(stderr, "Heap is empty\n");
        exit(EXIT_FAILURE);
    }

    Process min = heap->data[0];
    heap->data[0] = heap->data[--heap->size];
    heapifyDown(heap, 0);
    return min;
}
int compareByPriority(const Process *a, const Process *b)
{
    return a->priority - b->priority;
}

int compareByBurstTime(const Process *a, const Process *b)
{
    return a->burstTime - b->burstTime;
}
int compareByArrivalTime(const Process *a, const Process *b)
{
    return a->arrivalTime - b->arrivalTime;
}
void printSchedulerHeader(const char *mode)
{
    printf("══════════════════════════════════════════════\n");
    printf(">> Scheduler Mode : %s\n", mode);
    printf(">> Engine Status  : Initialized\n");
    printf("──────────────────────────────────────────────\n\n");
}
void printSchedulerSummary(float avgWaitingTime)
{
    printf("\n");
    printf("──────────────────────────────────────────────\n");
    printf(">> Engine Status  : Completed\n");
    printf(">> Summary        :\n");
    printf("   └─ Average Waiting Time : %.2f time units\n", avgWaitingTime);
    printf(">> End of Report\n");
    printf("══════════════════════════════════════════════\n\n");
}
queue *createQueue()
{
    queue *q = (queue *)malloc(sizeof(queue));
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    return q;
}
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

void rrScheduler(Process process[], int processCount, int timeQuantum)
{
    /*
    need a ready queue
    after every second need to update the ready queue
    need to keep track of the remaining burst time for each process (allready in the struct, just need to update it)
       └─ Total Turnaround Time : 13 time units
    */
}
void first_type_scheduler(Process process[], int processCount, ProcessCompareFn compare, char *mode)
{
    printSchedulerHeader(mode);
    sortbyArrivalTime(process, processCount); // Sort processes by arrival time before scheduling
    MinHeap minHeap;
    initMinHeap(&minHeap, compare); // Initialize the min heap with the burst time comparison function
    int currentTime = 0;                       // Initialize current time to 0
    int processLeft = processCount;            // Keep track of the number of processes left to run
    int currentProcessIndex = 0; // Index to track the next process to be added to the min heap
    float waitingTime = 0; // Variable to accumulate waiting time for average calculation
    while (processLeft > 0)
    {
        while (currentProcessIndex < processCount && process[currentProcessIndex].arrivalTime <= currentTime)
        {
            // Add all processes that have arrived by the current time to the min heap
            insertProcess(&minHeap, process[currentProcessIndex]);
            currentProcessIndex++;
        }        
        if (minHeap.size == 0) // No processes are ready to run
        {
            Idle(currentTime, process[currentProcessIndex].arrivalTime); // Idle for 1 second
            currentTime = process[currentProcessIndex].arrivalTime; // Update current time to the arrival time of the next process
            continue;
        }

        // Get the process with the shortest burst time from the min heap
        Process p = removeMin(&minHeap);
        __pid_t pid = fork(); // Create a new process
        if (pid < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) // Child process
        {
            runProcess(&p, currentTime); // Run the process for its burst time
            exit(0);                     // Exit child process after running
        }
        else // Scheduler pov
        {
            wait(NULL);                            // Wait for the child process to finish
            p.endTime = currentTime + p.burstTime; // Set the end time for the process
            processLeft--;                         // Decrease the number of processes left to run
            currentTime += p.burstTime;            // Update current time after the process finishes
            waitingTime += (currentTime - p.arrivalTime - p.burstTime); // Calculate waiting time
        }
    }
    printSchedulerSummary(waitingTime / processCount); // Print the average waiting time
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

void runCPUScheduler(char *processesCsvFilePath, int timeQuantum)
{
    Process processes[MAX_PROCESSES];
    int processCount = 0;
    initliazeAllProcesses(processes, &processCount, processesCsvFilePath);

    first_type_scheduler(processes, processCount, compareByArrivalTime, "FCFS");
    first_type_scheduler(processes, processCount, compareByBurstTime, "SJF");
    first_type_scheduler(processes, processCount, compareByPriority, "Priority");
    
    rrScheduler(processes, processCount, timeQuantum);
}
