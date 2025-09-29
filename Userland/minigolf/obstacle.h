#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdint.h>
#include "ball.h"
#include "player.h"

// Obstacle structure for static boxes
typedef struct {
    uint64_t x, y;          // Position (top-left corner)
    uint64_t width, height; // Dimensions
    uint64_t color;         // Box color
} Obstacle;

// Obstacle management functions
void draw_obstacle(Obstacle* obstacle);

// Collision detection functions
int check_ball_obstacle_collision(Ball* ball, Obstacle* obstacle);
int check_player_obstacle_collision(Player* player, Obstacle* obstacle);

// Collision handling functions
void handle_ball_obstacle_collision(Ball* ball, Obstacle* obstacle);

#endif 