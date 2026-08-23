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

void init_env(struct termios *orig);

void deactivate_env(struct termios *orig);

int init_desktop(char **folders, int count, int *selected);

int main() {
    struct termios orig;
    init_env(&orig);
    int count = 0;
    char **folder = get_folder(&count);
    int selected = 0;
    init_desktop(folder, count, &selected);

    deactivate_env(&orig);

    if (selected >= 0) {
        printf("%s\n", folder[selected]);
    }

    //cleanup memory
    for (int i = 0; i < count; i++) {
        free(folder[i]);
    }
    free(folder);
    //printf("\033[H\033[J");
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

    // 2. nur hiddne
    rewinddir(dir); // Verzeichnis-Pointer zurück auf Anfang setzen
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

int init_desktop(char **folders, int count, int *selected) {
    int selected_folder = *selected;
    while (1) {
        printf("\033[H\033[J");
        printf("=== Select a Folder (up/down Arrows, enter to confirm, q to quit) ===\n\n");
        for (int i = 0; i < count; i++) {
            if (i == selected_folder) {
                printf(" > \033[7m %s \033[0m\n", folders[i]);
            } else {
                printf("   %s\n", folders[i]);
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
                        selected_folder = (selected_folder - 1 + count) % count;
                    } else if (key_queue[1] == 'B') {
                        selected_folder = (selected_folder + 1) % count;
                    }
                }
            }
        }
    }
    return selected_folder;
}
