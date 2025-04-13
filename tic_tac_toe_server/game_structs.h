#ifndef GAME_STRUCTS_H
#define GAME_STRUCTS_H

#include <sys/types.h>

// Coordinate structure for grid positions
typedef struct coordinate {
    int x;
    int y;
} coordinate;

// Client message types
typedef enum client_message_type {
    START,
    MARK
} cmt;

// Client message structure
typedef struct client_message {
    cmt type;
    coordinate position;  // Used only for MARK message
} cm;

// Server message types
typedef enum server_message_type {
    END,
    RESULT
} smt;

// Server message structure
typedef struct server_message {
    smt type;
    int success;        // 1 if mark was successful, 0 otherwise
    int filled_count;   // Number of filled positions
} sm;

// Grid data structure for sending current game state
typedef struct grid_data {
    coordinate position;
    char character;
} gd;

#endif // GAME_STRUCTS_H 
