#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define NUM_GLADIATORS 4

int main()
{
    char *gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char *gladiator_files[NUM_GLADIATORS] = {"G1", "G2", "G3", "G4"};
    int pid, child_pid[NUM_GLADIATORS];
    for (int i = 0; i < NUM_GLADIATORS; i++)
    {

        pid = fork();
        if (pid == 0)
        { // this is a son process
            char glad[3];
            sprintf(glad, "%d", i + 1);
            // Child process
            execlp("./gladiator", "./gladiator", glad, NULL);
            perror("execlp failed");
            _exit(1); // Exit child process if execlp fails
        }
        else if (pid > 0)
        {
            child_pid[i] = pid; // Store the child's PID
        }
        else
        {
            // Fork failed
            perror("Fork failed");
            return 1;
        }
    }
    int status;
    pid_t finished_pid;
    for (int i = 0; i < NUM_GLADIATORS - 1; i++)
    {
        finished_pid = waitpid(-1, &status, 0);
        if (finished_pid == -1)
        {
            perror("waitpid failed");
            return 1;
        }
        for (int j = 0; j < NUM_GLADIATORS; j++)
        {
            if (child_pid[j] == finished_pid)
                child_pid[j] = 0;
        }
    }
    finished_pid = waitpid(-1, &status, 0);
    for (int j = 0; j < NUM_GLADIATORS; j++)
        if (child_pid[j] == finished_pid)
            printf("The gods have spoken, the winner of the tournament is %s!\n", gladiator_names[j]);
    return 0;
}