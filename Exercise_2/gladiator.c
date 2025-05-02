#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define NUM_GLADIATORS 4

FILE *openfile(char *filename);

void initiliaze_opponents(int opponents[], int gladiator)
{
    char filename[20];                       // Buffer to hold the filename
    sprintf(filename, "G%d.txt", gladiator); // Format the string
    FILE *file = openfile(filename);
    // Use the file (e.g., read data)
    char line[256];
    fgets(line, sizeof(line), file);
    char *token = strtok(line, ",");
    token = strtok(NULL, ","); // the second one is the attack
    for (int i = 0; i < NUM_GLADIATORS - 1; i++)
    {
        token = strtok(NULL, ",");
        opponents[i] = atoi(token); // Convert string to integer
    }
    fclose(file);
}

FILE *openfile(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file ");
        perror(filename);
        exit(EXIT_FAILURE);
    }
    return file;
}
int get_health(int gladiator)
{
    char filename[20];                       // Buffer to hold the filename
    sprintf(filename, "G%d.txt", gladiator); // Format the string
    FILE *file = openfile(filename);
    // Use the file (e.g., read data)
    char line[256];
    fgets(line, sizeof(line), file);
    char *token = strtok(line, ",");
    int health = atoi(token); // Convert string to integer
    fclose(file);
    return health;
}
int get_opponent_attack(int opponent)
{
    char filename[20];                      // Buffer to hold the filename
    sprintf(filename, "G%d.txt", opponent); // Format the string
    FILE *file = openfile(filename);
    // Use the file (e.g., read data)
    char line[256];
    fgets(line, sizeof(line), file);
    char *token = strtok(line, ",");
    token = strtok(NULL, ","); // the second one is the attack
    int attack = atoi(token);  // Convert string to integer
    fclose(file);
    return attack;
}
void fight(int glad, int opponents[], FILE *logFile)
{
    // Simulate fighting logic here
    int health = get_health(glad);
    // printf("Gladiator %d has %d health\n", glad, health);
    while (health >= 0)
    {
        for (int i = 0; i < 3; i++)
        {
            int opponent_attack = get_opponent_attack(opponents[i]);
            fprintf(logFile, "Facing opponent %d... Taking %d damage\n", opponents[i], opponent_attack);
            health -= opponent_attack;
            if (health > 0)
            {
                fprintf(logFile, "Are you not entertained? Remaining health: %d\n", health);
            }
            else
            {
                fprintf(logFile, "The gladiator has fallen... Final health: %d\n", health);
                break;
            }
        }
    }
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <gladiator_name>\n", argv[0]);
        return 1;
    }
    char *gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char *gladiator_files[NUM_GLADIATORS] = {"G1", "G2", "G3", "G4"};
    int gladiator_index = atoi(argv[1]);
    char *gladiator_name = gladiator_names[gladiator_index - 1];
    int opponents[NUM_GLADIATORS - 1];
    initiliaze_opponents(opponents, gladiator_index);
    // Simulate fighting logic here
    char filename[30];
    sprintf(filename, "%s_log_file.txt", gladiator_name);
    FILE *logFile = fopen(filename, "w"); // Open the log file for writing and create it
    fprintf(logFile, "Gladiator process started. %d:\n", getpid());
    fight(gladiator_index, opponents, logFile);
    return 0;
}