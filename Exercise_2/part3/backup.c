#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

void create_hard_link(const char *src, const char *dst);
void copy_symlink(const char *src, const char *dst);
void copy_directory(const char *src, const char *dst);
void make_absolute_path(const char *src, const char *dst);
bool directory_exists(const char *path);

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
    char *src_dir = realpath (argv[1], NULL);
    char *dest_dir = realpath (argv[2], NULL);

    copy_directory(src_dir, dest_dir);
    free(src_dir);
    free(dest_dir);
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
    char *target = malloc(256);
    int size = readlink(src, target, 256);
    target[size] = '\0'; // Ensure null termination
    symlink(target, dst);
    free(target);
    printf("Symlink created from %s to %s\n", src, dst);
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
        char src_path[1024];
        char dst_path[1024];
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