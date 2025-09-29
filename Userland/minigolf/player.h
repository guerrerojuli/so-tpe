#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "ball.h"

// Player structure to encapsulate all player-related data
typedef struct {
    uint64_t x, y;              // Position
    uint64_t size;              // Player size
    uint64_t color;             // Player color
    uint64_t move_speed;        // Current movement speed
    char last_movement;         // Last movement direction ('w', 'a', 's', 'd', or 0)
    uint64_t last_movement_tick; // When last movement occurred
    int64_t vel_x, vel_y;       // Player velocity for realistic ball hitting
    int is_moving;              // Whether player moved this frame
    int64_t recent_vel_x, recent_vel_y; // Velocity from recent movement (persists briefly)
    uint64_t last_active_movement_tick;  // When player last actively moved
    uint64_t key_hold_start_tick;        // When current key sequence started being held
    int consecutive_movement_count;      // Number of consecutive movements in same direction
    char current_held_direction;         // Current direction being held ('w','a','s','d', or 0)
} Player;

// Player management functions
void init_player(Player* player, uint64_t x, uint64_t y, uint64_t size, uint64_t color);
void draw_player(Player* player);
void erase_player(Player* player);
void erase_player_at_position(uint64_t x, uint64_t y, uint64_t size);

// Player movement functions
int move_player_up(Player* player);
int move_player_down(Player* player);
int move_player_left(Player* player);
int move_player_right(Player* player);

// Collision detection
int check_collision(Player* player, Ball* ball);
int check_player_collision(Player* player1, Player* player2);
void hit_ball(Player* player, Ball* ball);

// Player-player collision detection
int check_player_player_collision(Player* p1, Player* p2);

#endif