#ifndef BALL_H
#define BALL_H

#include <stdint.h>

// Ball structure for game physics
typedef struct {
    uint64_t center_x, center_y;    // Center position
    int64_t vel_x, vel_y;           // Velocity (can be negative)
    uint64_t radius;                // Ball radius
    uint64_t color;                 // Ball color
    uint64_t friction;              // Friction factor (higher = more friction)
} Ball;

// Ball management functions
void init_ball(Ball* ball, uint64_t center_x, uint64_t center_y, uint64_t radius, uint64_t color);
void draw_ball(Ball* ball);
void erase_ball(Ball* ball);
void erase_ball_at_position(uint64_t center_x, uint64_t center_y, uint64_t radius);
int is_ball_moving(Ball* ball);
void update_ball(Ball* ball, int owner_player_num);

#endif