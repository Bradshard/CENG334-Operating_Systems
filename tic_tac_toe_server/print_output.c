#include <stdio.h>
#include "print_output.h"

void print_output(cmp *client_msg, smp *server_msg, gu *grid_updates, int update_count) {
    // Print client message if provided
    if (client_msg != NULL) {
        printf("Client Message (PID: %d)\n", client_msg->process_id);
        if (client_msg->client_message->type == START) {
            printf("Type: START\n");
        } else if (client_msg->client_message->type == MARK) {
            printf("Type: MARK\n");
            printf("Position: (%d, %d)\n", 
                client_msg->client_message->position.x,
                client_msg->client_message->position.y);
        }
    }

    // Print server message if provided
    if (server_msg != NULL) {
        printf("Server Message (PID: %d)\n", server_msg->process_id);
        if (server_msg->server_message->type == END) {
            printf("Type: END\n");
        } else if (server_msg->server_message->type == RESULT) {
            printf("Type: RESULT\n");
            printf("Success: %d\n", server_msg->server_message->success);
            printf("Filled Count: %d\n", server_msg->server_message->filled_count);
        }
    }

    // Print grid updates if provided
    if (grid_updates != NULL && update_count > 0) {
        printf("Grid Updates:\n");
        for (int i = 0; i < update_count; i++) {
            printf("Position: (%d, %d), Character: %c\n",
                grid_updates[i].position.x,
                grid_updates[i].position.y,
                grid_updates[i].character);
        }
    }

    // Add a newline for better readability
    printf("\n");
    fflush(stdout);
} 