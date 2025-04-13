// This code provides Real-Time Multiplayer X-O-X (tic-tac-toe) server with poll and socketpair IPC.
// IPC = inter-process communication.
/*
 A concurrent(multithreaded) multiplayer tictactoe grid game. Here each player run in a separate child process and communicate with socketpair() bidirectional IPC. Sever-Client communication from all players with poll(), also checking win/draw conditions with valid move conditions after each move, and logs with print_output(). All IPC and MP is done in a binary-safe structs.*/

#include <stdio.h>      // Known Standard I/O, gives cdisplay and output print
#include <stdlib.h>     // Quit handling and memory dynamically Dynamic memory allocation, exit handling
#include <unistd.h>     // POSIX material --> fork, read, write, dup2, execvp from "How to use the execvp() function in C/C++" regular formatting for fork-exec on multi-process in C
//dup2 similar to dup, with difference on using fd specified in newfd.
#include <sys/socket.h> // duplex operation(bidirectional) piping in IPC, provides socketpair()
#include <sys/types.h>  // Defn for processes and types
#include <sys/wait.h>   // waitpid() to clean up children, takes the responses to parent.
#include <poll.h>       // poll() to listen to multiple FDs in real time, it is recommended in the recitation with wait-time.
#include <string.h>     // memset library include, at initiation
#include <fcntl.h>      // fcntl-file control library for adv fd flags
#include <signal.h>     // I got execvp errors for early quitting of one, of the players hence included signal handling specifically (SIGPIPE) to neglect err 141 = 128 + 13.
#include "game_structs.h"  // Given game struct for shared message and board format defn
#include "print_output.h"  // To log, again given.

#define PLAYERS_MAX_COUNT 8 // My computer has 8 cores M2, in mac's those cores correspond to threads. hence 8 threads. That's why 8 players.
#define MAXIMUM_GRID_SIZE 10 // for not to make computationally overloading amount of tic-tac-toe block.
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, 0, fd)  // IPC setup macro definition.

// Game configuration and runtime state variables
int width, height, streak, num_player;                 // Grid-dim and game-rule in input.txt
char player_symbols[PLAYERS_MAX_COUNT];                // Player Game Symbols (e.g. 'X', 'O', 'A',...)
char ***player_argvs;                                  // pointer to array of arrays for each players' execvp()
pid_t pids_childs[PLAYERS_MAX_COUNT];                  // Child PIDs stored for future use (such as reap.)
int player_fds[PLAYERS_MAX_COUNT];                     // Socketpair with each player, for server-side.
char game_board[MAXIMUM_GRID_SIZE][MAXIMUM_GRID_SIZE]; // State of the Game Grid
int counts_fill = 0;                                   // Mnemonic name, counts filled cells.

// vertical = height referenced as y or dy, hence; board[y == height][x == width]
// 4 direction, right-bottom-downdiag-updiag, starting from top-left clockwise.
int dx[4] = {1, 0, 1, 1};
int dy[4] = {0, 1, 1, -1};

// BC checker: Boundary conditions are predefined in the input.txt and this checks whether coordinates lies within these grid bounds I.E MAXIMUM_GRID_SIZE x MAXIMUM_GRID_SIZE. then return true.
int is_in_bounds(int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

// Winning line check based on input.txt from (x ,y) in four dir.
int check_win(int x, int y, char c) {
    for (int dir = 0; dir < 4; dir++) {
        int icout = 1;

        // Forward scan from current direction
        for (int step = 1; step < streak; step++) {
            int nx = x + step * dx[dir];
            int ny = y + step * dy[dir];
            if (!is_in_bounds(nx, ny) || game_board[ny][nx] != c) break;
            icout++;
        }

        // Backward scan in current direction
        for (int step = 1; step < streak; step++) {
            int nx = x - step * dx[dir];
            int ny = y - step * dy[dir];
            if (!is_in_bounds(nx, ny) || game_board[ny][nx] != c) break;
            icout++;
        }

        if (icout >= streak) return 1;  // If winning streak is detected return 1.
    }
    return 0;
}

// Display the current board state after every move, like the reference output.txt
void print_game_board() {
    printf("\nGame_Board Grid State:\n  ");
    for (int x = 0; x < width; x++) {
        printf("%d ", x);
    }
    printf("\n");
    for (int y = 0; y < height; y++) {
        printf("%d ", y);
        for (int x = 0; x < width; x++) {
            printf("%c ", game_board[y][x]);
        }
        printf("\n");
    }
    printf("\n");
    fflush(stdout); //flush all open output streams, to ensure write in file.
}

// Read stdin game config and player exec.
void input_parsing() {
    scanf("%d %d %d %d", &width, &height, &streak, &num_player);
    player_argvs = malloc(num_player * sizeof(char **));

    for (int i = 0; i < num_player; i++) {
        int arg_count;
        scanf(" %c %d", &player_symbols[i], &arg_count);
        player_argvs[i] = malloc((arg_count + 2) * sizeof(char *));

        for (int j = 0; j < arg_count + 1; j++) {
            player_argvs[i][j] = malloc(100); //allocate memory bytes
            scanf("%s", player_argvs[i][j]);
        }
        player_argvs[i][arg_count + 1] = NULL; // execvp() argument vector, terminating NULL.
    }
}

// Fork player processes and setup IPC using socketpair
void Player_setup() {
    for (int i = 0; i < num_player; i++) {
        int fd[2]; // fd[0] read, fd[1] write for parent child communication, unidirectional.
        // however, due to #define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, 0, fd), we make a
        // socketpair hence PIPE(fd) is with Macro:#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, 0, fd) makes full-duplex (bi-dir), communication channel between parent and child.
        PIPE(fd);  //
        player_fds[i] = fd[0]; // parent uses to communicate with player

        // forking a new process for the players.
        pid_t pid = fork(); // child gets fork value 0, parent other than 0, then we replace
        if (pid == 0) {
            // Child -> duplicates the file descriptor fd[1] to I/O.
            // redirection of stdin/stdout to sockets
            // comm happens at fd[1].
            dup2(fd[1], STDIN_FILENO);
            dup2(fd[1], STDOUT_FILENO);
            // fd[0] for parent use, hence; close.
            close(fd[0]);
            execvp(player_argvs[i][0], player_argvs[i]); // we replace child process with executable of player, which is actually the clone of parent process and assign here a new program to it.
            perror("execvp failed");
            exit(1); // if exec fails.
        } else {
            // Parent-> stores pids', closes children at the EOP (end of pipe.)
            pids_childs[i] = pid;
            // for child processes use. Hence, close for parent.
            close(fd[1]);
        }
    }
}

// Board state and move result info sent to the player moving (either success or fail.).
void result_sent(int fd, pid_t pid, int success) {
    sm feedback_resp = {RESULT, success, counts_fill}; // server message result response_delivery
    gd game_update[MAXIMUM_GRID_SIZE * MAXIMUM_GRID_SIZE]; // array of the board grid
    // Building the filled cell list (i.e non-empty game_board cells.)
    int index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (game_board[y][x] != '*') {
                // Fill in grid update data for each occupied cell
                game_update[index++] = (gd){{x, y}, game_board[y][x]}; // grid-data board update.
            }
        }
    }

    // Resulting message delivered/sent to player.
    write(fd, &feedback_resp, sizeof(feedback_resp));
    // Board state(list of filled cells) sent.
    write(fd, game_update, sizeof(gd) * counts_fill);

    smp out = {pid, &feedback_resp};
    // which is functionally the same as gd, just a different alias
    print_output(NULL, &out, (gu *)game_update, counts_fill); // grid update in print_output.h
    // Print the current board to console
    print_game_board();  // Game Board log at each step.
}

// Game-End notification to child processes.
void end_report_msg() {
    for (int i = 0; i < num_player; i++) {
        sm end_msg = {END, 0, 0}; // server-message struct
        write(player_fds[i], &end_msg, sizeof(end_msg));

        smp end_log = {pids_childs[i], &end_msg}; //server message print given usage
        print_output(NULL, &end_log, NULL, 0); // again given.
    }
}

// Main game loop using poll() with 1ms timeout to ensure responsiveness and CPU efficiency
int main() {
    signal(SIGPIPE, SIG_IGN);  // Closed pipe check to prevent server crash when writing.

    input_parsing();                          // Game Grid and Player reading.
    memset(game_board, '*', sizeof(game_board));        // Board initialization with "*" marking, shows empty.
    Player_setup();                           // Player Process initiation (launch)

    struct pollfd fds[PLAYERS_MAX_COUNT];
    for (int i = 0; i < num_player; i++) {
        fds[i].fd = player_fds[i];            // File descriptor of pollfd struct
        fds[i].events = POLLIN;               // for readable input, requested events listen
    }

    int isFinished = 0; // Check finish, if true returns false at while.
    while (!isFinished) {
        // If game is not over, do this part.
        int ready = poll(fds, num_player, 1); // Poll call, check ready with 1 meaning 1ms timeout.
        // as stated in the recitation for responsive multi-client IO and CPU.

        if (ready > 0) {
            for (int i = 0; i < num_player; i++) {
                if (fds[i].revents & POLLIN) {
                    // returned event check.
                    cm msg;
                    ssize_t read_bytes = read(fds[i].fd, &msg, sizeof(msg));
                    //signed size_t.
                    if (read_bytes <= 0) continue; // disconnected

                    cmp in = {pids_childs[i], &msg};
                    print_output(&in, NULL, NULL, 0); // Logs delivered/incoming msg.

                    if (msg.type == START) {
                        // If checks player is ready, and here we send current board state.
                        result_sent(fds[i].fd, pids_childs[i], 0);
                    } else if (msg.type == MARK) {
                        int x = msg.position.x; // x position
                        int y = msg.position.y; // y position
                        char mark = player_symbols[i];

                        // Is Invalid move (O.O.Bounds or filled already.)
                        if (!is_in_bounds(x, y) || game_board[y][x] != '*') {
                            result_sent(fds[i].fd, pids_childs[i], 0);
                            continue;
                        }

                        // Is Valid move!
                        game_board[y][x] = mark;
                        counts_fill++;
                        result_sent(fds[i].fd, pids_childs[i], 1);

                        // Game-Ending condition check.
                        if (check_win(x, y, mark)) {
                            end_report_msg();
                            printf("Winner: Player%c\n", mark); // Winner given.
                            isFinished = 1;
                        } else if (counts_fill == width * height) {
                            end_report_msg();
                            printf("Draw\n"); // Game Draw.
                            isFinished = 1;
                        }
                    }
                }
            }
        }
    }

    // Wait all children and Reap the children. Kill/Clean part.
    for (int i = 0; i < num_player; i++) {
        waitpid(pids_childs[i], NULL, 0);
    }

    return 0;
}
