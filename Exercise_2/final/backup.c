#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#define MAX_PATH_LENGTH 4096 // i dont know what the OS is so i set it to 4096
void create_hard_link(const char *src, const char *dst);
void copy_symlink(const char *src, const char *dst);
void copy_directory(const char *src, const char *dst);
void make_absolute_path(const char *relative, char *absolute);
bool directory_exists(const char *path);
void find_start(const char *src, const char *dst, char *start);
void initilaize(const char *src, const char *dst, char *start, char *end, char *src_file_name, char *dst_file_name);

void initilaize(const char *src, const char *dst, char *start, char *end, char *src_file_name, char *dst_file_name)
{
    int last_slash = 0;
    int i = 0;
    while (src[i] && dst[i] && src[i] == dst[i])
    {
        start[i] = src[i];
        if (src[i] == '/')
            last_slash = i;
        i++;
    }
    start[last_slash] = '/';      // Truncate to last matching slash
    start[last_slash + 1] = '\0'; // Truncate to last matching slash
    i = last_slash + 1;
    for (int k = 0; src[i] != '\0' && src[i] != '/'; i++, k++)
    {
        src_file_name[k] = src[i];
    }
    for (int j = 0; src[i] != '\0'; i++, j++)
    {
        end[j] = src[i];
    }
    for (int k = 0, j = last_slash + 1; dst[j] != '\0' && dst[j] != '/'; j++, k++)
    {
        dst_file_name[k] = dst[j];
    }

    printf("\n\n\n\n");
    printf("src: %s\n", src);
    printf("dst: %s\n", dst);
    printf("start: %s\n", start);
    printf("end: %s\n", end);
    printf("src_file_name: %s\n", src_file_name);
    printf("dst_file_name: %s\n", dst_file_name);
    printf("\n\n\n\n");
}
void make_absolute_path(const char *relative, char *absolute)
{
    if (relative[0] == '/')
    {
        // Already an absolute path
        strcpy(absolute, relative);
        return;
    }
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        exit(1);
    }
    size_t size = strlen(cwd) + strlen(relative) + 2; // +2 for '/' and '\0'
    snprintf(absolute, size, "%s/%s", cwd, relative);
}

bool directory_exists(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
    {
        // stat failed — file doesn't exist or some error
        return false;
    }
    return S_ISDIR(statbuf.st_mode); // true if it's a directory
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage %s <source_directory> <backup_directory>\n", argv[0]);
        return 1;
    }
    if (!directory_exists(argv[1]))
    {
        perror("src dir");
        exit(1);
    }
    if (directory_exists(argv[2]))
    {
        perror("backup dir");
        exit(1);
    }
    char src_dir[MAX_PATH_LENGTH] = {'\0'}, dest_dir[MAX_PATH_LENGTH] = {'\0'};
    make_absolute_path(argv[1], src_dir);
    make_absolute_path(argv[2], dest_dir);
    printf("Source directory: %s\n", src_dir);
    printf("Destination directory: %s\n", dest_dir);
    copy_directory(src_dir, dest_dir);
    printf("Backup completed from %s to %s\n", src_dir, dest_dir);

    return 0;
}

void create_hard_link(const char *src, const char *dst)
{
    link(src, dst);
    printf("Hard link created from %s to %s\n", src, dst);
}
void copy_symlink(const char *src, const char *dst)
{
    char start[MAX_PATH_LENGTH] = {'\0'}, src_name[MAX_PATH_LENGTH] = {'\0'}, dst_name[MAX_PATH_LENGTH] = {'\0'}, end[MAX_PATH_LENGTH] = {'\0'};
    // initilaize(src, dst, start, mid, end);
    char target[MAX_PATH_LENGTH];
    int size = readlink(src, target, MAX_PATH_LENGTH);
    target[size] = '\0'; // Ensure null termination
    printf("Symlink target: %s\n", target);
    if (*(target) == '/')
    {
        initilaize(target, dst, start, end, src_name, dst_name);
        char *absolute_path = malloc(strlen(start) + strlen(dst_name) + strlen(end) + 3); // +3 for '/' and '\0'
        snprintf(absolute_path, strlen(start) + strlen(dst_name) + strlen(end) + 3, "%s%s%s", start, dst_name, end);
        symlink(absolute_path, dst);
        printf("Symlink created from %s to %s\n", absolute_path, dst);
        free(absolute_path);
    }
    else
    {
        symlink(target, dst);
        printf("Symlink created from %s to %s\n", target, dst);
    }
}

void copy_directory(const char *src, const char *dst)
{
    struct stat st;
    if (mkdir(dst, 0755) == -1)
    {
        perror("mkdir");
        exit(1);
    }
    DIR *dir = opendir(src);
    if (dir == NULL)
    {
        perror("opendir");
        exit(1);
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue; // Skip . and ..
        char src_path[MAX_PATH_LENGTH];
        char dst_path[MAX_PATH_LENGTH];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);
        lstat(src_path, &st);
        if (S_ISLNK(st.st_mode))
        {
            copy_symlink(src_path, dst_path);
            continue;
        }
        else if (S_ISREG(st.st_mode))
        {
            create_hard_link(src_path, dst_path);
            continue;
        }
        else if (S_ISDIR(st.st_mode))
        {
            printf("Copying directory %s to %s\n", src_path, dst_path);
            copy_directory(src_path, dst_path);
            continue;
        }
    }
    closedir(dir);
}