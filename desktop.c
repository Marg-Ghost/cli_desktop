//
// Created by Admin on 23.08.2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define MAX_FOLDERS 60
#define MAX_NAME 256

char **get_folder(int *count);
char **get_folder_point(int *count);
char **get_file(int *count);

void init_env(struct termios *orig);

void deactivate_env(struct termios *orig);

int init_desktop(char **folders,char **folder_point,char **file, int count, int count_point, int count_file);

int main() {
    struct termios orig;
    init_env(&orig);
    int count = 0;
    int count_point = 0;
    int count_file = 0;
    char **folder = get_folder(&count);
    char **folder_point = get_folder_point(&count_point);
    char **file = get_file(&count_file);

    int selected = init_desktop(folder,folder_point,file, count, count_point, count_file);

    deactivate_env(&orig);

    fprintf(stderr, "\033[H\033[J");
    if (selected >= 0) {
        if (selected < count) {
            printf("DIR %s\n", folder[selected]);
        } else if (selected < count + count_point) {
            printf("DIR %s\n", folder_point[selected - count]);
        } else if (selected < count + count_point + count_file) {
            printf("FILE %s\n", file[selected - count - count_point]);
        }
    }

    // Cleanup Memory
    for (int i = 0; i < count; i++) free(folder[i]);
    free(folder);
    for (int i = 0; i < count_point; i++) free(folder_point[i]);
    free(folder_point);
    for (int i = 0; i < count_file; i++) free(file[i]);
    free(file);

    return 0;
}

char **get_folder(int *count) {
    DIR *dir = opendir(".");
    if (!dir) return NULL;

    char **folders = malloc(MAX_FOLDERS * sizeof(char *));
    *count = 0;

    struct dirent *entry;

    // 1. nur norm
    while ((entry = readdir(dir)) != NULL && *count < MAX_FOLDERS) {
        if (entry->d_type == DT_DIR) {
            if (entry->d_name[0] == '.') continue; // Versteckte Ordner skippen

            folders[*count] = malloc(MAX_NAME);
            strncpy(folders[*count], entry->d_name, MAX_NAME - 1);
            (*count)++;
        }
    }
    closedir(dir);
    return folders;
}
char **get_folder_point(int *count) {
    DIR *dir = opendir(".");
    if (!dir) return NULL;

    char **folders = malloc(MAX_FOLDERS * sizeof(char *));
    *count = 0;

    struct dirent *entry;

    // 2. nur norm
    while ((entry = readdir(dir)) != NULL && *count < MAX_FOLDERS) {
        if (entry->d_type == DT_DIR) {
            // '.' und '..' ignorieren
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            if (entry->d_name[0] == '.') {
                folders[*count] = malloc(MAX_NAME);
                strncpy(folders[*count], entry->d_name, MAX_NAME - 1);
                (*count)++;
            }
        }
    }
    closedir(dir);
    return folders;
}
char **get_file(int *count) {
    DIR *dir = opendir(".");
    if (!dir) return NULL;

    char **folders = malloc(MAX_FOLDERS * sizeof(char *));
    *count = 0;

    struct dirent *entry;

    // 1. nur norm
    while ((entry = readdir(dir)) != NULL && *count < MAX_FOLDERS) {
        if (entry->d_type != DT_DIR) {
            folders[*count] = malloc(MAX_NAME);
            strncpy(folders[*count], entry->d_name, MAX_NAME - 1);
            (*count)++;
        }
    }
    closedir(dir);
    return folders;
}



void init_env(struct termios *orig) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, orig);
    raw = *orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void deactivate_env(struct termios *orig) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig);
}

int init_desktop(char **folders,char **folder_point,char **file, int count, int count_point, int count_file) {
    int selected_folder = 0;
    int total_count = count + count_point + count_file;
    if (total_count == 0) return -1;
    //int current_idx = 0;
    while (1) {
        int current_idx = 0;
        fprintf(stderr,"\033[H\033[J");
        fprintf(stderr,"=== Select a Folder (up/down Arrows, enter to confirm, q to quit) ===\n\n");
        // 1. Normale Ordner
        for (int i = 0; i < count; i++, current_idx++) {
            if (current_idx == selected_folder) {
                fprintf(stderr, " > \033[7m %s/ \033[0m\n", folders[i]);
            } else {
                fprintf(stderr, "   %s/\n", folders[i]);
            }
        }

        // Trennlinie 1
        if (count_point > 0) fprintf(stderr, "============================\n");

        // 2. Versteckte Ordner
        for (int i = 0; i < count_point; i++, current_idx++) {
            if (current_idx == selected_folder) {
                fprintf(stderr, " > \033[7m %s/ \033[0m\n", folder_point[i]);
            } else {
                fprintf(stderr, "   %s/\n", folder_point[i]);
            }
        }

        // Trennlinie 2
        if (count_file > 0) fprintf(stderr, "============================\n");

        // 3. Dateien
        for (int i = 0; i < count_file; i++, current_idx++) {
            if (current_idx == selected_folder) {
                fprintf(stderr, " > \033[7m %s \033[0m\n", file[i]);
            } else {
                fprintf(stderr, "   %s\n", file[i]);
            }
        }

        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;
        //quitting
        if (c == 'q' || c == 'Q') {
            selected_folder = -1;
            break;
        }
        //confirm
        if (c == '\n' || c == '\r') {
            /*if (selected_folder >= 0) {
                chdir(folders[selected_folder]);
            }*/
            break;
        }
        //movement
        //! read == Queue: --> ! muss 3x read sonst bug
        //bei 'up' == [/033,[,A]
        // => raed immer erstes dann del
        if (c == '\033') {
            char key_queue[2];
            if (read(STDIN_FILENO, &key_queue[0], 1) > 0 && read(STDIN_FILENO, &key_queue[1], 1) > 0) {
                if (key_queue[0] == '[') {
                    if (key_queue[1] == 'A') {
                        //==up
                        //weil: count % count ist wieder 0 und somit scroll == endless
                        //aber: 1 % count ist immernoch 1
                        selected_folder = (selected_folder - 1 + count) % total_count;
                    } else if (key_queue[1] == 'B') {
                        selected_folder = (selected_folder + 1) % total_count;
                    }
                }
            }
        }
    }
    return selected_folder;
}
