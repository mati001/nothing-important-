#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 256

FILE *open_file_read(char *filename)
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
void read_commend(FILE *file_data, FILE *file_result, int first, int second)
{

    char line[MAX_LINE_LENGTH];
    fseek(file_data, first, SEEK_SET);
    fgets(line, second - first, file_data);
    if (strlen(line) = second - first)
    {
        fprintf(file_result, "%s", line);
        printf("Read from %d to %d: %s", offset, second_offset, line);
    }

    else
    {
        printf("Error reading from %d to %d\n", offset, second_offset);
    }
}
void main_loop(FILE *file_request, FILE *file_data)
{
    char line[MAX_LINE_LENGTH + 20]; // the request might be +20 chars (probablt less, but i dont want to calculate)
    FILE *read_result = fopen("read_results.txt", "w");
    while (fgets(line, sizeof(line), file_request) != NULL)
    {
        char *commend, *first, *second;
        commend = strtok(line, " ");
        first = strtok(NULL, " ");
        second = strtok(NULL, " ");
        switch (*commend)
        {
        case 'R':
            read_commend(file_data, read_result, atoi(first), atoi(second)); // first - the offset, second - the second offset
            break;
        case 'W':
            write_commend(file_data, atoi(first), second); // first - the offset, second - the data to write
                                                           // Handle write
            break;
        case 'Q':
            return;
        default:
            printf("Unknown command: %s\n", commend);
            break;
        };
        // Process the line
        printf("Processing line: %s", line);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <data_file> <request_file>\n", argv[0]);
        return 1;
    }
    FILE *request = open_file_read(argv[2]);
    FILE *data = open_file_read(argv[1]);
    main_loop(request, data);

    /*
    loop
    do the request one by one
    read the request
    if the request is read than
    */

    return 0;
}