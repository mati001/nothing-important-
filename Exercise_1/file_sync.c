#include <stdio.h>
#include <unistd.h>  
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>



#define MAX_FILES 100
#define MAX_PATH 1024
#define MAX_NAME 256


bool file_exists_in_dir(DIR *dst, char *name);
void data_validation(int argv, char *argc[], char *current_dir);
void synchronize(char src_path[MAX_PATH], char dst_path[MAX_PATH]);
void copy_file(char src_path[MAX_PATH], char dst_path[MAX_PATH], char name[MAX_NAME]);
void synchronize_file(DIR *src, DIR *dst, char name[MAX_NAME], char src_path[MAX_PATH], char dst_path[MAX_PATH]);
void create_dir(char *dst);
bool check_diff(char src_path[MAX_PATH], char dst_path[MAX_PATH], char name[MAX_NAME]);
bool dst_newer(char src_path[MAX_PATH], char dst_path[MAX_PATH], char name[MAX_NAME]);
void remove_file(char dst_path[MAX_PATH], char name[MAX_NAME]);
// int skip_dot_files( struct dirent *entry);

// int skip_dot_files( struct dirent *entry) {
//     return strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0;
// }
bool file_exists_in_dir(DIR *dst, char *name) {
    struct dirent *entry;
    // Rewind in case the DIR* was already read
    rewinddir(dst);
    while ((entry = readdir(dst)) != NULL) {
        if (strcmp(entry->d_name, name) == 0) {
            return true;  // Found the file
        }
    }
    return false;  // Not found
}

void data_validation(int argv, char * argc[], char * current_dir){
    if (argv != 3) {
        printf("Usage: file_sync <source_directory> <destination_directory>\n");
        exit(1);
    }
    char *src = argc[1], *dst =argc[2];
    char src_full_path[MAX_PATH], dst_full_path[MAX_PATH];
    
    if(src[0]!='/')
        snprintf(src_full_path, sizeof(src_full_path), "%s/%s", current_dir, src);
    else
        strcpy(src_full_path,src);
    if(dst[0]!='/')
        snprintf(dst_full_path, sizeof(dst_full_path), "%s/%s", current_dir, dst);
    else
        strcpy(dst_full_path, dst);

    struct stat st;
    //check if src exist as dir
    if (!(stat(src_full_path, &st) == 0 && S_ISDIR(st.st_mode))) {
        printf("Error: Source directory '%s' does not exist.\n", src);
        exit(1);
    }
    if (!(stat(dst_full_path, &st) == 0 && S_ISDIR(st.st_mode))) {
        // need to open the directory - meaning create one
        create_dir(dst_full_path);
        printf("Created destination directory '%s'.\n", dst);
    }
}
bool next_file(DIR *src,char src_path[MAX_PATH],  const char prev[MAX_NAME], char next[MAX_NAME]) {
    struct dirent *entry;
    bool found = false;
    char best[MAX_NAME] = "";

    rewinddir(src);  // Always rewind before scanning
    while ((entry = readdir(src)) != NULL) {
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", src_path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode))
            continue;

        const char *name = entry->d_name;

        // Skip "." and ".."
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        // Skip if not greater than prev
        if (strcmp(name, prev) <= 0)
            continue;

        // Keep the smallest name greater than prev
        if (!found || strcmp(name, best) < 0) {
            strcpy(best, name);
            found = true;
        }
    }

    if (found) {
        strcpy(next, best);
    }

    return found;
}
void synchronize(char src_path[MAX_PATH], char dst_path[MAX_PATH]){
    DIR *src = opendir(src_path);
    if (src == NULL) {
        perror("Failed to open source directory");
        return;
    }

    DIR *dst = opendir(dst_path);
    if (dst == NULL) {
        perror("Failed to open destination directory");
        closedir(src);
        return;
    }    
    char* min=readdir(src)->d_name;
    struct dirent *entry;
    char prev[MAX_NAME] = "";  // the start is empty string
    char next[MAX_NAME];       // so i could know what is the next file
    // next_file(src, prev, next);

    int i=0;
    while (next_file(src,src_path, prev, next)) {
        synchronize_file(src, dst, next, src_path, dst_path);
        i++;
        if(i==MAX_FILES)
            break;
        strcpy(prev, next);
        rewinddir(src);           // each time i need to restart 
    }

    printf("Synchronization complete.\n");
    closedir(src);
    closedir(dst);
    
}
void copy_file(char src_path[MAX_PATH], char dst_path[MAX_PATH], char name[MAX_NAME]){
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }
    if (pid == 0){ //the child process 
        char full_src[MAX_PATH];
        snprintf(full_src, sizeof(full_src), "%s/%s", src_path, name);

        char *args[] = { "cp", full_src, dst_path, NULL };
        execvp("cp", args);
        perror("execvp failed"); // like Ayala told us, the only reason to get to this line is if exec faild
        exit(1);
    }
    else{ // meaning you are the father
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        } else {
            perror("Failed to copy file \n");
        }
    }    


}
void synchronize_file(DIR *src, DIR *dst, char name[MAX_NAME], char src_path[MAX_PATH], char dst_path[MAX_PATH]){
    if(file_exists_in_dir(dst, name)){
    // If the file exists in both directories:
        if (check_diff(src_path, dst_path, name)){ // meaning they are diffrent
            if (dst_newer(src_path, dst_path, name)){
                printf("File %s is newer in destination. Skipping...\n", name);
            }
            else{
                printf("File %s is newer in source. Updating...\n", name);
                remove_file(dst_path, name);
                copy_file(src_path, dst_path, name);
            }            
        }
        else { // they are the same 
            printf ("File %s is identical. Skipping...\n", name);
        }
    }else {
        printf("New file found: %s\n", name);
        copy_file(src_path, dst_path, name);
        char src_full_path[MAX_PATH], dst_full_path[MAX_PATH];
        
        char* current_dir = NULL;
        current_dir = getcwd(current_dir, MAX_PATH);
        if(!current_dir)
            perror("getcwd faild");
            
        if(src_path[0]!='/')
            snprintf(src_full_path, sizeof(src_full_path), "%s/%s", current_dir, src_path);
        else
            strcpy(src_full_path,src_path);
        if(dst_path[0]!='/')
            snprintf(dst_full_path, sizeof(dst_full_path), "%s/%s", current_dir, dst_path);
        else
            strcpy(dst_full_path,src_path);
        
        printf("Copied: %s/%s -> %s/%s\n", src_full_path, name, dst_full_path, name);
    }
}

void create_dir(char *dst)
{
    pid_t pid = fork();
    if(pid<0){
        perror("fork failed");
        exit(1);
    }
    if (pid==0){// meaning this son
        char *args[] = { "mkdir", "-p", dst, NULL };
        execvp("mkdir", args);
        perror("execvp failed");  // If execvp fails
        exit(1);
    }
    else{ // meaning you are the father
        int status;
        waitpid(pid, &status, 0);
    }    

}

bool check_diff(char src_path[MAX_PATH], char dst_path[MAX_PATH], char name[MAX_NAME])
{
    char full_src[MAX_PATH], full_dst[MAX_PATH];
    snprintf(full_src, sizeof(full_src), "%s/%s", src_path, name);
    snprintf(full_dst, sizeof(full_dst), "%s/%s", dst_path, name);
    pid_t pid = fork();
    if(pid<0){
        perror("fork failed");
        exit(1);
    }
    if (pid==0){// meaning this son
     // Prepare the args for diff command
     char *args[] = { "diff", "-q", full_src, full_dst, NULL };
        
     // Redirect output and error to /dev/null
     int null_fd = open("/dev/null", O_WRONLY);
     if (null_fd < 0) {
         perror("Failed to open /dev/null");
         exit(1);
     }

     // Redirect stdout and stderr to /dev/null
     dup2(null_fd, STDOUT_FILENO);
     dup2(null_fd, STDERR_FILENO);
     close(null_fd);

     // Execute the diff command
     execvp("diff", args);
     perror("execvp failed");  // If execvp fails
     exit(1);



        // char *args[] = { "diff", "-q", full_src, full_dst, ">", "/dev/null" , "2>&1" ,  NULL };        
        // Redirect output and error to /dev/null
        execvp("diff", args);  // Execute the diff command
        perror("execvp failed");  // If execvp fails
        exit(1);
    }
    else{ // meaning you are the father
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return false;
        } else {
            return true;
        }
    }    

    return false;

}

bool dst_newer(char src_path[MAX_PATH], char dst_path[MAX_PATH], char name[MAX_NAME])
{
    char full_src[MAX_PATH];
    char full_dst[MAX_PATH];

    snprintf(full_src, sizeof(full_src), "%s/%s", src_path, name);
    snprintf(full_dst, sizeof(full_dst), "%s/%s", dst_path, name);

    struct stat src_stat, dst_stat;

    if (stat(full_src, &src_stat) != 0) {
        perror("stat failed on source file");
        return false; // Fail-safe: assume src is not newer
    }

    if (stat(full_dst, &dst_stat) != 0) {
        perror("stat failed on destination file");
        return false; // Fail-safe: assume dst is not newer
    }

    return dst_stat.st_mtime > src_stat.st_mtime;
}

void remove_file(char dst_path[MAX_PATH], char name[MAX_NAME])
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    if (pid == 0){ //the child process 
        // change the directory to dst
        //after that use rm commend 
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dst_path, name);

        char *args[] = { "rm", "-f", full_path, NULL };
        execvp("rm", args);
        perror("execvp failed"); // like Ayala told us, the only reason to get to this line is if exec faild
        exit(1);
    }
    else{ // meaning you are the father
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            ;
        } else {
            perror("Failed to delete file \n");
        }
    }    
}

int main(int argv, char * argc[]){
    char* current_dir = NULL;
    current_dir = getcwd(current_dir, MAX_PATH);
    if(!current_dir)
        perror("getcwd faild");
    printf("Current working directory: %s\n", current_dir);
    data_validation(argv, argc, current_dir);
    // printf("all good so far \n");
    char *src = argc[1], *dst =argc[2];
    char src_full_path[MAX_PATH], dst_full_path[MAX_PATH];
    
    if(src[0]!='/')
        snprintf(src_full_path, sizeof(src_full_path), "%s/%s", current_dir, src);
    else
        strcpy(src_full_path,src);
    if(dst[0]!='/')
        snprintf(dst_full_path, sizeof(dst_full_path), "%s/%s", current_dir, dst);
    else
        strcpy(dst_full_path,src);

    printf("Synchronizing from %s to %s\n", src_full_path, dst_full_path);
    synchronize(src, dst);
    chdir(current_dir); // here is a chdir commend as you requsted

    return 0;
}