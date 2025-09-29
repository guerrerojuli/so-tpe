#ifndef HOLE_H
#define HOLE_H

#include <stdint.h>
#include "ball.h"
#include "player.h"

// Hole structure for the target
typedef struct {
    uint64_t center_x, center_y;    // Center position
    uint64_t radius;                // Hole radius
    uint64_t color;                 // Hole color (usually black)
} Hole;

// Hole management functions
void init_hole(Hole* hole, uint64_t center_x, uint64_t center_y, uint64_t radius, uint64_t color);
void draw_hole(Hole* hole);

// Collision detection
int check_ball_in_hole(Ball* ball, Hole* hole);
int check_player_hole_collision(Player* player, Hole* hole);

#endif