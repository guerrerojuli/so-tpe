#include "hole.h"

void init_hole(Hole* hole, uint64_t center_x, uint64_t center_y, uint64_t radius, uint64_t color) {
    hole->center_x = center_x;
    hole->center_y = center_y;
    hole->radius = radius;
    hole->color = color;
}

void draw_hole(Hole* hole) {
    sys_draw_circle(hole->color, hole->center_x, hole->center_y, hole->radius);
}

// Check if ball is in the hole
int check_ball_in_hole(Ball* ball, Hole* hole) {
    // Ball is in hole if ball center is within hole radius
    int64_t dx = (int64_t)ball->center_x - (int64_t)hole->center_x;
    int64_t dy = (int64_t)ball->center_y - (int64_t)hole->center_y;
    uint64_t distance_squared = dx * dx + dy * dy;
    return distance_squared <= (hole->radius * hole->radius);
}

// Check collision between player and hole
int check_player_hole_collision(Player* player, Hole* hole) {
    // Collision between rectangle (player) and circle (hole)
    // Find the closest point in the rectangle to the circle center
    uint64_t closest_x = hole->center_x;
    uint64_t closest_y = hole->center_y;
    
    // Clamp circle center to rectangle bounds
    if (hole->center_x < player->x) closest_x = player->x;
    else if (hole->center_x > player->x + player->size) closest_x = player->x + player->size;
    
    if (hole->center_y < player->y) closest_y = player->y;
    else if (hole->center_y > player->y + player->size) closest_y = player->y + player->size;
    
    // Calculate distance from circle center to closest point
    int64_t dx = (int64_t)hole->center_x - (int64_t)closest_x;
    int64_t dy = (int64_t)hole->center_y - (int64_t)closest_y;
    
    return (dx * dx + dy * dy) <= (hole->radius * hole->radius);
}