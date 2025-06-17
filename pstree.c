/*
    Sorry for a mess of a code, just did it during the deadline day :D
    To add functionality to the code just type: ./pstree (flag1) (flag2) (arg1) (arg2)
    There are 5 flags: 
    -p (print pid) (ex: ./pstree -p -> prints out pstree with pid)
    -a (print all active process in proc), 
    -g (print pgid)
    -n (print specific pid as root) (ex: ./pstree -n 2 -> prints the pstree of process with pid 2)
    -s (sort the process alphabetically instead of by pid)
    Note: It is possible to combine most of the flags, example: ./pstree -p -a (prints all active process and print pid alongside it).
    Note that -a -n is mutually exclusive and the program will take the first flag that appears
    If there is no further argument, the program will print the default output.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#define SIZE 1024
#define CHAR_SIZE 256
struct process {
    char *name;
    int pid;
    int ppid;
    int pgid;
    struct process *parent_process;
    struct process **child_process;
    int child_num;
};

int compare_process_name(const void *a, const void *b) {
    struct process *proc1 = *(struct process **)a;
    struct process *proc2 = *(struct process **)b;
    return strcmp(proc1->name, proc2->name);
}

void extract_sub(char *src, int start, int end, char *dest) {
    if (start < 0 || end < start || end >= strlen(src)) {
        dest[0] = '\0';
        return;
    }
    int len = end - start + 1;
    strncpy(dest, src + start, len);
    dest[len - 1] = '\0';
}

void metadata(const char *stat, struct process *proc) {
    FILE *file = fopen(stat, "r");
    if (file == NULL) {
        perror("Could not open file");
        return;
    }
    
    int count = 0;
    char buffer[SIZE];
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        count++;
        char word[CHAR_SIZE];
        
        if (count == 1) {
            extract_sub(buffer, 6, strlen(buffer) - 1, word);
            proc->name = malloc(strlen(word) + 1);
            if (proc->name != NULL) {
                strcpy(proc->name, word);
            } else {
                perror("Failed to allocate memory for process name");
            }
        }
        if (count == 7) {
            extract_sub(buffer, 5, strlen(buffer) - 1, word);
            proc->ppid = atoi(word);
        }
        if (count == 15){
            extract_sub(buffer, 8, strlen(buffer) - 1, word);
            int a;
            sscanf(word, "%d", &a);
            proc->pgid = a;
            break;
        }
    }
    fclose(file);
}

int dir_process(const char *dir) {
    if (*dir == 0) {
        return 0;
    }
    while (*dir) {
        if (!isdigit((unsigned char)*dir)) return 0;
        dir++;
    }
    return 1;
}

void add_child(struct process *child, struct process *parent) {
    parent->child_num++;
    parent->child_process = realloc(parent->child_process, parent->child_num * sizeof(struct process*));
    if (parent->child_process == NULL) {
        printf("Failed to allocate\n");
        exit(1);
    }
    parent->child_process[parent->child_num - 1] = child;
}

void find_parent(struct process **arr, struct process *child, int low, int high) {
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid]->pid == child->ppid) {
            child->parent_process = arr[mid];
            add_child(child, arr[mid]);
            return;
        } else if (arr[mid]->pid < child->ppid) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    child->parent_process = NULL;
}

void print_tree(struct process *process, const char *prefix, int is_last, int sort, int pid_print, int pgid_print) {
    if (process == NULL) return;
    printf("%s%s%s", prefix, is_last ? "└─" : "├─", process->name);
    if (pid_print)
    {
        printf("(%d)", process->pid);
    }
    if (pgid_print)
    {
        printf("(pgid:%d)", process->pgid);
    }
    printf("\n");
    if (sort)
        qsort(process->child_process, process->child_num, sizeof(struct process*), compare_process_name);
    size_t prefix_len = strlen(prefix);
    size_t new_prefix_len = prefix_len + 4;
    char *new_prefix = malloc(new_prefix_len + 1);
    if (new_prefix == NULL) {
        perror("Failed to allocate memory for new prefix");
        exit(EXIT_FAILURE);
    }
    strcpy(new_prefix, prefix);
    strcat(new_prefix, is_last ? "    " : "│   ");
    for (int i = 0; i < process->child_num; i++) {
        print_tree(process->child_process[i], new_prefix, i == process->child_num - 1, sort, pid_print, pgid_print);
    }
    free(new_prefix);
}

int isnumber(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    struct process **arr;
    arr = malloc(SIZE * sizeof(struct process*));
    int i = 0;
    if (arr == NULL) {
        printf("Array not allocated\n");
        return 1;
    }
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_DIR && strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0 && dir_process(dir->d_name)) {
                char statusFile[SIZE];
                snprintf(statusFile, sizeof(statusFile), "/proc/%s/status", dir->d_name);
                struct process *curr_process = malloc(sizeof(struct process));
                if (curr_process == NULL) {
                    printf("Failed to allocate memory for process\n");
                    continue;
                }
                curr_process->child_num = 0;
                curr_process->child_process = NULL;
                curr_process->pid = atoi(dir->d_name);
                metadata(statusFile, curr_process);
                arr[i] = curr_process;
                i++;
            }
        }
        closedir(d);
    } else {
        perror("Could not open /proc directory");
    }
    for (int t = 0; t < i; t++) {
            find_parent(arr, arr[t], 0, i - 1);
        }
    // DEFAULT FEATURE IF NO ARGUMENT FOR FLAGS ARE GIVEN
    if (argc == 1)
    {
        for (int t = 0; t < i; t++) {
            if (arr[t]->pid == 1) {
                print_tree(arr[t], "", 1, 0, 0, 0);
                break;
            }
        }
    }
    else
    {
        // FLAGS TO DETERMINE PRINT OUTPUT
        int pgid_flag = 0;
        int pid_flag = 0;
        int all_flag = 0;
        int proc_flag = 0;
        int sort_flag = 0;
        int pid_find;
        
        for (int i = 0; i < argc; i++)
        {
            if (!strcmp("-p", argv[i]))
            {
                pid_flag = 1;
            }
            if (!strcmp("-a", argv[i]) && !proc_flag)
            {
                all_flag = 1;
            }
            if (!strcmp("-n", argv[i]) && !all_flag)
            {
                proc_flag = 1;
            }
            if (!strcmp("-s", argv[i]))
            {
                sort_flag = 1;
            }
            if (!strcmp("-g", argv[i]))
            {
                pgid_flag = 1;
            }
            if (proc_flag && isnumber(argv[i]))
            {
                pid_find = atoi(argv[i]);
            }
        }
        if (all_flag)
        {
            for (int t = 0; t < i; t++) {
                if (arr[t]->ppid == NULL) {
                    print_tree(arr[t], "", 1, sort_flag, pid_flag, pgid_flag);
                }
            }  
        }
        else if (proc_flag)
        {
            for (int t = 0; t < i; t++) {
                if (arr[t]->pid == pid_find) {
                    print_tree(arr[t], "", 1, sort_flag, pid_flag, pgid_flag);
                }
            } 
        }
        else // REVERT TO DEFAULT
        {
            for (int t = 0; t < i; t++) {
                if (arr[t]->pid == 1) {
                    print_tree(arr[t], "", 1, sort_flag, pid_flag, pgid_flag);
                }
            } 
        }
    }


    // for (int t = 0; t < i; t++) {
    //     free(arr[t]->name);
    //     for (int j = 0; j < arr[t]->child_num; j++) {
    //         free(arr[t]->child_process[j]);
    //     }
    //     free(arr[t]->child_process);
    //     free(arr[t]);
    // }
    // free(arr);

    return 0;
}