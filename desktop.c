//
// Created by Admin on 23.08.2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>


#define MAX_FOLDERS 60
#define MAX_NAME 256
char resource_root[PATH_MAX];

void init_resource_root(void);
void print_file(const char *filename);
char **get_folder(int *count);
char **get_folder_point(int *count);
char **get_file(int *count);

void init_env(struct termios *orig);

void deactivate_env(struct termios *orig);

int init_desktop(char **folders,char **folder_point,char **file, int count, int count_point, int count_file, char *action_out, char *editor_out);

int main() {
    init_resource_root();
    struct termios orig;
    init_env(&orig);
    int count = 0;
    int count_point = 0;
    int count_file = 0;
    char **folder = get_folder(&count);
    char **folder_point = get_folder_point(&count_point);
    char **file = get_file(&count_file);
    //interactions
    char action = 0; // 0/r | D/del | Q/quit
    char editor_out[16] = "nano";
    int selected = init_desktop(folder,folder_point,file, count, count_point, count_file, &action, editor_out);

    deactivate_env(&orig);
    fprintf(stderr, "\033[H\033[J");

    const char *path = NULL;
    if (selected >= 0) {
        if (selected < count) {
            // 1. Ordner
            path = folder[selected];
        } else if (selected < count + count_file) {
            // 2. Dateien
            path = file[selected - count];
        } else if (selected < count + count_file + count_point) {
            // 3. . Ordner
            path = folder_point[selected - count - count_file];
        }
    }
    if (action == 'Q' || !path) {
        printf("QUIT\n");
    } else {
        printf("%c\n%s\n%s\n", action, editor_out, path);
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
            if (strcmp(entry->d_name, ".") == 0) continue;

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

void init_resource_root(void) {
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1); //path current programm
    if (len == -1) {
        strcpy(resource_root, ".");
        return;
    }
    exe_path[len] = '\0';
    strncpy(resource_root, dirname(exe_path), PATH_MAX - 1);
}

//actual main interaction point
int init_desktop(char **folders,char **folder_point,char **file, int count, int count_point, int count_file, char *action_out, char *editor_out) {
    int selected_folder = 0;
    int total_count = count + count_point + count_file;
    if (total_count == 0) return -1;

    int help = 0;

    int light_desktop = 1;
    int total_destktops = 3;

    int current_editor = 0;
    int all_editor = 2;
    char *editorlist[] = {"nano", "vim"};

    //int current_idx = 0;
    while (1) {
        int current_idx = 0;
        fprintf(stderr,"\033[H\033[J");
        if (light_desktop == 1) {
            print_file ("/home/marg_ghost/settings/desktop/ressources/Ghost.txt");
        }else if (light_desktop == 2) {
            print_file ("/home/marg_ghost/settings/desktop/ressources/Geist.txt");
        }else if (light_desktop == 0) {
            print_file ("/home/marg_ghost/settings/desktop/ressources/text.txt");
        }
        // 1. Normale Ordner
        if (count > 0) fprintf(stderr, "| Folder |============================\n");
        for (int i = 0; i < count; i++, current_idx++) {
            if (current_idx == selected_folder) {
                fprintf(stderr, " > \033[7m %s/ \033[0m\n", folders[i]);
            } else {
                fprintf(stderr, "   %s/\n", folders[i]);
            }
        }

        // 2. Dateien
        if (count_file > 0) fprintf(stderr, "| File |============================\n");
        if (count_file > 0) fprintf(stderr, "Editor : \033[7m %s/ \033[0m\n", editorlist[current_editor] );
        for (int i = 0; i < count_file; i++, current_idx++) {
            if (current_idx == selected_folder) {
                fprintf(stderr, " > \033[7m %s \033[0m\n", file[i]);
            } else {
                fprintf(stderr, "   %s\n", file[i]);
            }
        }

        // 3.
        if (count_point > 0) fprintf(stderr, "| .Folder |============================\n");
        for (int i = 0; i < count_point; i++, current_idx++) {
            if (current_idx == selected_folder) {
                fprintf(stderr, " > \033[7m %s/ \033[0m\n", folder_point[i]);
            } else {
                fprintf(stderr, "   %s/\n", folder_point[i]);
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
            *action_out = 'O';
            strcpy(editor_out, editorlist[current_editor]);
            break;
        }
        // HELP
        if (c == '1') {
            help = 1;
        }
        while (help == 1) {
            fprintf(stderr,"\033[H\033[J");
            print_file ("/home/marg_ghost/settings/desktop/ressources/help.txt");
            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0) break;
            if (c == '1') {
                help = 0;
            }
        }
        //switch desktop
        if (c == '2') {
            light_desktop = (light_desktop + 1) % total_destktops;
        }
        if (c == '3') {
            current_editor = (current_editor + 1) % all_editor;
        }

        if (c == 'x' || c == 'X') {
            *action_out = 'D';
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
                        selected_folder = (selected_folder - 1 + total_count) % total_count;
                    } else if (key_queue[1] == 'B') {
                        selected_folder = (selected_folder + 1) % total_count;
                    }
                }
            }
        }
    }
    return selected_folder;
}

void print_file(const char *filepath) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return;
    if (chdir(resource_root) != 0) return;
    FILE *file = fopen(filepath, "r");
    if (file) {
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), file)) {
            fputs(buffer, stderr);
        }
        fclose(file);
    }
    chdir(cwd);
}