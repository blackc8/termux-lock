#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "termux-auth.h"

#define _TA_HOME "/data/data/com.termux/files/home"
#define _TA_PREFIX "/data/data/com.termux/files/usr"

static bool show_motd = true;

// Detect a shell suitable for login and return
// path to its binary.
char *get_shell() {
    char *shell = NULL;
    struct stat st;

    if (lstat(_TA_HOME "/.termux/shell", &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            // If ~/.termux/shell points to executable.
            char link_target[PATH_MAX];
            ssize_t len = readlink(_TA_HOME "/.termux/shell", link_target, sizeof(link_target) - 1);

            if (len != -1) {
                link_target[len] = '\0';

                if (access(link_target, R_OK | X_OK) == 0) {
                    shell = link_target;
                }
            }
        } else if (S_ISREG(st.st_mode)) {
            // If ~/.termux/shell is regular executable file.
            // Some Termux users may prefer to launch a custom program
            // instead of shell.
            if (access(_TA_HOME "/.termux/shell", R_OK | X_OK) == 0) {
                shell = _TA_HOME "/.termux/shell";
            }
        } else {
            // If ~/.termux/shell has unexpected file type.
            fprintf(stderr, "Warning: ~/.termux/shell is not a regular file or symbolic link.\n");
        }
    } else {
        // No custom shell requested.
        // Using the default.

        char *default_shells[/* 2 */] = {
            _TA_PREFIX "/bin/bash",
            _TA_PREFIX "/bin/sh"
        };

        for (int i=0; i<2; i++) {
            if (access(default_shells[i], R_OK | X_OK) == 0) {
                shell = default_shells[i];
                break;
            }
        }
    }

    // If failed to pick any shell, use /system/bin/sh.
    if (!shell) {
        fprintf(stderr, "Warning: failed to detect login shell, using the /system/bin/sh instead.\n");
        shell = "/system/bin/sh";
    }

    setenv("SHELL", shell, 1);

    return strdup(shell);
}

// Setup the termux-exec hook if possible.
void prepare_termux_exec() {
    pid_t ch_pid, pid;
    int status;

    setenv("LD_PRELOAD", _TA_PREFIX "/lib/libtermux-exec.so", 1);

    ch_pid = fork();

    if (ch_pid == 0) {
        execl(_TA_PREFIX "/bin/sh", "sh", "-c", "true", NULL);
    } else if (ch_pid > 0) {
        pid = wait(&status);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                unsetenv("LD_PRELOAD");
            }
        }
    } else {
        unsetenv("LD_PRELOAD");
    }
}

// Print MOTD if needed and launch the login shell.
void init_login() {
    FILE *motd;

    if (show_motd && (motd = fopen(_TA_PREFIX "/etc/motd", "rb")) != NULL) {
        char *buf[256];
        int bytes_read;

        while ((bytes_read = fread(buf, 1, sizeof(buf), motd)) > 0) {
            fwrite(buf, 1, bytes_read, stdout);
        }

        fclose(motd);
    }

    char *shell = get_shell();
    char *shell_name = basename(shell);
    prepare_termux_exec();
    execl(shell, shell_name, "-l", NULL);
    free(shell);
    exit(1);
}

void show_usage(){
    puts("Usage: login [options]"
         "\n      -h, --help                   display help message"
         "\n      -e, --enable-password        enables login with password"
         "\n      -d, --disable-password       disables login with password"
         "\n\nAuthor: blackc8");
}

int main(int argc, char **argv) {
    int c;
    int fp;

    while(1){
        static struct option options[] = {
          {"help", no_argument, 0, 'h'},
          {"enable-password", no_argument, 0, 'e'},
          {"disable-password", no_argument, 0, 'd'},
          {0,0,0,0}
        };

        c = getopt_long(argc, argv, "hed", options, 0);
        if(c == -1)
            break;
        switch(c){
            case 'h':
                show_usage();
                exit(1);

            case 'e':
                if(access(_TA_HOME "/.termux/login_enabled",F_OK) != -1){
                 puts("Login with password already enabled");
                 exit(1);
                }

                fp = open(_TA_HOME "/.termux/login_enabled", O_RDWR|O_CREAT, 0777);
                if(fp == -1){
                 puts("Error enabling login with password");
                 exit(-1);
                }
                puts("Login with password successfully enabled");
                exit(1);
            case 'd':

                if(access(_TA_HOME "/.termux/login_enabled",F_OK) == -1){
                 puts("Login with password already disabled");
                 exit(1);
                }

                int del = remove(_TA_HOME "/.termux/login_enabled");
                if(del){
                 puts("Error disabling login with password");
                 exit(-1);
                }
                 puts("Login with password successfully disabled");
                 exit(1);
            case '?':
                /* getopt_long already printed an error message. */
                exit(1);
        }


    }


    if(access(_TA_HOME "/.termux/login_enabled",F_OK) == -1){ // login with password disabled
        init_login();
    }

    chdir(_TA_HOME);

    if (access(AUTH_HASH_FILE_PATH, R_OK) != 0) {
        // Allow passwordless login if no password is set
        // or unreadable.
        fprintf(stderr, "Warning: password is not set. Please set it with utility 'passwd'.\n");
        init_login();
    }

    if (access(_TA_HOME "/.hushlogin", F_OK) == 0 || getenv("TERMUX_HUSHLOGIN") != NULL) {
        show_motd = false;
        unsetenv("TERMUX_HUSHLOGIN");
    }
    puts("Login to your Termux.");

    for (int attempt=0; attempt<3; attempt++) {
        char *password = getpass("Password: ");
        if (!password) {
            puts("Failed to read password input.");
            continue;
        }

        if (termux_auth("termux", password)) {
            init_login();
        } else {
            puts("Invalid password.");
        }
    }

    return EXIT_FAILURE;
}
