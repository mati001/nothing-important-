#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#define MAX_LINE_LENGTH 10000

FILE *open_file(char *filename, char *flag)
{
    FILE *file = fopen(filename, flag);
    if (file == NULL)
    {
        perror("Error opening file ");
        perror(filename);
        exit(EXIT_FAILURE);
    }
    return file;
}
bool exist(const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd != -1)
    {
        close(fd); // file exists, clean up
        return true;
    }
    return false; // open failed: file doesn't exist or permission denied
}
void read_commend(FILE *file_data, FILE *file_result, int first, int second)
{
    char line[MAX_LINE_LENGTH] = {'\0'};
    int length = second - first + 1;
    fseek(file_data, first, SEEK_SET);
    size_t bytes_read = fread(line, sizeof(char), length, file_data);
    if (bytes_read == length)
    {
        fprintf(file_result, "%s\n", line);
        printf("Read from %d to %d: %s\n", first, second, line);
        printf("read result: %s\n", line);
    }
    else
    {
        printf("Error reading from %d to %d, the file isnt long enougth\n", first, second);
    }
}
void write_commend(FILE *file_data, int offset, char *str)
{
    fseek(file_data, 0, SEEK_END);
    long end = ftell(file_data);
    if (offset < 0 || offset > end) {
        printf("Error: invalid offset %d, file size is %ld\n", offset, end);
        return;
    }
    long length = end - offset;         // Calculate how much to read
    fseek(file_data, offset, SEEK_SET); // Return to the start position again
    char tail[MAX_LINE_LENGTH] = {'\0'};
    fread(tail, 1, length, file_data); // Read the data
    tail[length] = '\0';
    printf("tail: %s\n", tail);
    fseek(file_data, offset, SEEK_SET);
    fwrite(str, sizeof(char), strlen(str), file_data);
    fwrite(tail, sizeof(char), strlen(tail), file_data);
    printf("Write to %d: %s\n", offset, str);
}
void main_loop(char *data_name, char *request_name)
{
    FILE *file_request = open_file(request_name, "r");
    FILE *file_data = open_file(data_name, "r+");
    char line[MAX_LINE_LENGTH + 20]; // the request might be +20 chars (probablt less, but i dont want to calculate)
    int fd = open("read_results.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE *read_result = fdopen(fd, "w");
    while (fgets(line, sizeof(line), file_request) != NULL)
    {
        printf("Processing line: %s ", line);
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
            second[strlen(second) - 1] = '\0';             // remove the new line character
            write_commend(file_data, atoi(first), second); // first - the offset, second - the data to write
            break;
        case 'Q':
            return;
        default:
            printf("Unknown command: %s\n", commend);
            break;
        };
        // Process the line
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <data_file> <request_file>\n", argv[0]);
        return 1;
    }
    if (!exist(argv[1]))
    {
        perror("data.txt");
        return 1;
    }
    if (!exist(argv[2]))
    {
        perror("request.txt");
        return 1;
    }
    main_loop(argv[1], argv[2]);
    
    return 0;
}