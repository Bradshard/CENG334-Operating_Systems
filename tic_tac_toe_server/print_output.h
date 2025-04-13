#ifndef PRINT_OUTPUT_H
#define PRINT_OUTPUT_H

#include "game_structs.h"

// Structure for printing client messages
typedef struct client_message_print {
    pid_t process_id;
    cm *client_message;
} cmp;

// Structure for printing server messages
typedef struct server_message_print {
    pid_t process_id;
    sm *server_message;
} smp;

// Structure for printing grid updates
typedef struct grid_update {
    coordinate position;
    char character;
} gu;

// Print output function prototype
void print_output(cmp *client_msg, smp *server_msg, gu *grid_updates, int update_count);

#endif // PRINT_OUTPUT_H 