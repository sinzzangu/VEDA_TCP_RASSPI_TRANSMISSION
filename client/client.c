#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

#define MAXDATASIZE 100

static volatile sig_atomic_t running = 1;
static int sockfd = -1;

/* ------------------------------------------------------------------ */
/*  Signal handling                                                    */
/* ------------------------------------------------------------------ */
static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

static void setup_signals(void) {
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    /* ignore all other catchable signals */
    struct sigaction sa_ign;
    memset(&sa_ign, 0, sizeof(sa_ign));
    sa_ign.sa_handler = SIG_IGN;
    sigemptyset(&sa_ign.sa_mask);

    sigaction(SIGQUIT, &sa_ign, NULL);
    sigaction(SIGHUP, &sa_ign, NULL);
    sigaction(SIGTERM, &sa_ign, NULL);
    sigaction(SIGPIPE, &sa_ign, NULL);
    sigaction(SIGTSTP, &sa_ign, NULL);
    sigaction(SIGTTIN, &sa_ign, NULL);
    sigaction(SIGTTOU, &sa_ign, NULL);
    sigaction(SIGUSR1, &sa_ign, NULL);
    sigaction(SIGUSR2, &sa_ign, NULL);
    sigaction(SIGALRM, &sa_ign, NULL);
}

static void cleanup(void) {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
    printf("\n[CLIENT] Connection closed. Goodbye.\n");
}

/* ------------------------------------------------------------------ */
/*  Send command, print server response                                */
/* ------------------------------------------------------------------ */
static void send_command(const char *cmd) {
    write(sockfd, cmd, strlen(cmd));

    char response[MAXDATASIZE];
    int n = recv(sockfd, response, MAXDATASIZE - 1, 0);
    if (n <= 0) {
        printf("[CLIENT] Server disconnected.\n");
        running = 0;
        return;
    }
    response[n] = '\0';
    printf("[SERVER] %s", response);
}

/* ------------------------------------------------------------------ */
/*  Input helper — returns -1 if SIGINT received                      */
/* ------------------------------------------------------------------ */
static int get_choice(int min, int max) {
    int choice;
    while (running) {
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n' && running)
                ;
            printf("  Invalid. Enter %d-%d: ", min, max);
            continue;
        }
        if (choice >= min && choice <= max)
            return choice;
        printf("  Invalid. Enter %d-%d: ", min, max);
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/*  Device sub-menus                                                   */
/* ------------------------------------------------------------------ */
static void led_menu(void) {
    while (running) {
        printf("\n===============================\n");
        printf("            LED MENU           \n");
        printf("===============================\n");
        printf("  1. ON\n");
        printf("  2. OFF\n");
        printf("  3. BRIGHTNESS\n");
        printf("  4. Back\n");
        printf("  Enter choice: ");

        int choice = get_choice(1, 4);
        if (choice == -1)
            return;

        if (choice == 1) {
            send_command("LED ON");
        } else if (choice == 2) {
            send_command("LED OFF");
        } else if (choice == 3) {
            printf("  Brightness [0=LOW | 1=MID | 2=HIGH]: ");
            int level = get_choice(0, 2);
            if (level == -1)
                return;
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "LED BRIGHT %d", level);
            send_command(cmd);
        } else {
            return;
        }
    }
}

static void buzzer_menu(void) {
    while (running) {
        printf("\n===============================\n");
        printf("          BUZZER MENU          \n");
        printf("===============================\n");
        printf("  1. ON\n");
        printf("  2. OFF\n");
        printf("  3. Back\n");
        printf("  Enter choice: ");

        int choice = get_choice(1, 3);
        if (choice == -1)
            return;

        if (choice == 1) {
            /* send BUZZER ON without blocking for response */
            write(sockfd, "BUZZER ON", 9);
            printf("  Playing... press any key to stop.\n");
            fflush(stdout);

            /* raw mode — detect keypress without waiting for Enter */
            struct termios old_tio, new_tio;
            tcgetattr(STDIN_FILENO, &old_tio);
            new_tio = old_tio;
            new_tio.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

            struct pollfd pfd[2];
            pfd[0].fd = STDIN_FILENO;
            pfd[0].events = POLLIN;
            pfd[1].fd = sockfd;
            pfd[1].events = POLLIN;

            char response[MAXDATASIZE];
            int n;
            int done = 0;

            while (!done && running) {
                if (poll(pfd, 2, -1) <= 0)
                    break;

                if (pfd[0].revents & POLLIN) {
                    /* user pressed a key — cancel the song */
                    char c;
                    read(STDIN_FILENO, &c, 1);
                    write(sockfd, "BUZZER OFF", 10);
                    /* recv the 200 OK from the cancel */
                    n = recv(sockfd, response, MAXDATASIZE - 1, 0);
                    if (n <= 0) {
                        running = 0;
                    } else {
                        response[n] = '\0';
                        printf("\n[SERVER] %s", response);
                    }
                    done = 1;
                } else if (pfd[1].revents & POLLIN) {
                    /* server responded — song finished naturally */
                    n = recv(sockfd, response, MAXDATASIZE - 1, 0);
                    if (n <= 0) {
                        running = 0;
                    } else {
                        response[n] = '\0';
                        printf("\n[SERVER] %s", response);
                    }
                    done = 1;
                }
            }

            /* restore terminal */
            tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

        } else if (choice == 2) {
            send_command("BUZZER OFF");
        } else {
            return;
        }
    }
}

static void seg_menu(void) {
    while (running) {
        printf("\n===============================\n");
        printf("        7-SEGMENT MENU         \n");
        printf("===============================\n");
        printf("  1. DISPLAY digit\n");
        printf("  2. CLEAR\n");
        printf("  3. COUNTDOWN\n");
        printf("  4. Back\n");
        printf("  Enter choice: ");

        int choice = get_choice(1, 4);
        if (choice == -1)
            return;

        if (choice == 1) {
            printf("  Digit [0-9]: ");
            int digit = get_choice(0, 9);
            if (digit == -1)
                return;
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "SEG DISPLAY %d", digit);
            send_command(cmd);
        } else if (choice == 2) {
            send_command("SEG CLEAR");
        } else if (choice == 3) {
            printf("  Start from [0-9]: ");
            int start = get_choice(0, 9);
            if (start == -1)
                return;
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "SEG COUNTDOWN %d", start);
            send_command(cmd);
        } else {
            return;
        }
    }
}

static void cds_menu(void) {
    while (running) {
        printf("\n===============================\n");
        printf("           CDS MENU            \n");
        printf("===============================\n");
        printf("  1. READ\n");
        printf("  2. Back\n");
        printf("  Enter choice: ");

        int choice = get_choice(1, 2);
        if (choice == -1)
            return;

        if (choice == 1) {
            send_command("CDS READ");
        } else {
            return;
        }
    }
}

static void manual_menu(void) {
    while (running) {
        printf("\n===============================\n");
        printf("         MANUAL MODE           \n");
        printf("===============================\n");
        printf("  1. LED\n");
        printf("  2. BUZZER\n");
        printf("  3. 7-SEGMENT\n");
        printf("  4. CDS SENSOR\n");
        printf("  5. Back\n");
        printf("  Enter choice: ");

        int choice = get_choice(1, 5);
        if (choice == -1)
            return;

        if (choice == 1)
            led_menu();
        else if (choice == 2)
            buzzer_menu();
        else if (choice == 3)
            seg_menu();
        else if (choice == 4)
            cds_menu();
        else
            return;
    }
}

/* ------------------------------------------------------------------ */
/*  Main menu                                                          */
/* ------------------------------------------------------------------ */
static void print_main_menu(void) {
    printf("\n===============================\n");
    printf("        LIGHT CONTROLLER       \n");
    printf("===============================\n");
    printf("  1. Manual Mode\n");
    printf("  2. Automatic Mode\n");
    printf("  3. Quit\n");
    printf("===============================\n");
    printf("  Enter choice: ");
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: client <hostname>\n");
        return 1;
    }

    setup_signals();
    atexit(cleanup);

    /* connect to server */
    struct hostent *he = gethostbyname(argv[1]);
    if (!he) {
        perror("gethostbyname");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    memset(&(server_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 1;
    }

    printf("[CLIENT] Connected to %s\n", inet_ntoa(server_addr.sin_addr));

    /* main menu loop */
    while (running) {
        print_main_menu();

        int choice = get_choice(1, 3);
        if (choice == -1)
            break;

        if (choice == 1) {

            manual_menu();
        } else if (choice == 2) {
            printf("[CLIENT] Reading sensor...\n");
            send_command("CDS AUTO");
        } else {
            break;
        }
    }

    return 0;
}