#include <stdio.h>
#define NUM_GLADIATORS 4



int main(){
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_files[NUM_GLADIATORS] = {"G1", "G2", "G3", "G4"};
    int pid;
    for (int i = 0; i < NUM_GLADIATORS; i++) {
        pid =fork();
        if (pid == 0) {
            // Child process
            printf("Gladiator %s (PID: %d) is ready to fight!\n", gladiator_names[i], getpid());
            execlp("./gladiator", gladiator_files[i], NULL);
            perror("execlp failed");
            _exit(1); // Exit child process if execlp fails
        } else if (pid > 0){

        }
        else {
            // Fork failed
            perror("Fork failed");
            return 1;
        }
        
    
    
    }
    printf("The gods have spoken, the winner of the tournament is [gladiator_name]!\n");

    return 0;
}