#include "obstacle.h"
#include "unistd.h"

// Draw a single obstacle
void draw_obstacle(Obstacle* obstacle) {
    sys_draw_rectangle(obstacle->color, obstacle->x, obstacle->y, obstacle->width, obstacle->height);
}

// Check collision between ball and obstacle (rectangle)
int check_ball_obstacle_collision(Ball* ball, Obstacle* obstacle) {
    // Ball bounds
    uint64_t ball_left = ball->center_x - ball->radius;
    uint64_t ball_right = ball->center_x + ball->radius;
    uint64_t ball_top = ball->center_y - ball->radius;
    uint64_t ball_bottom = ball->center_y + ball->radius;
    
    // Obstacle bounds
    uint64_t obs_left = obstacle->x;
    uint64_t obs_right = obstacle->x + obstacle->width;
    uint64_t obs_top = obstacle->y;
    uint64_t obs_bottom = obstacle->y + obstacle->height;
    
    // Check if ball overlaps with obstacle
    return (ball_right > obs_left && ball_left < obs_right &&
            ball_bottom > obs_top && ball_top < obs_bottom);
}

// Check collision between player and obstacle (both rectangles)
int check_player_obstacle_collision(Player* player, Obstacle* obstacle) {
    // Player bounds
    uint64_t player_left = player->x;
    uint64_t player_right = player->x + player->size;
    uint64_t player_top = player->y;
    uint64_t player_bottom = player->y + player->size;
    
    // Obstacle bounds
    uint64_t obs_left = obstacle->x;
    uint64_t obs_right = obstacle->x + obstacle->width;
    uint64_t obs_top = obstacle->y;
    uint64_t obs_bottom = obstacle->y + obstacle->height;
    
    // Check if player overlaps with obstacle
    return (player_right > obs_left && player_left < obs_right &&
            player_bottom > obs_top && player_top < obs_bottom);
}

// Handle collision between ball and obstacle
void handle_ball_obstacle_collision(Ball* ball, Obstacle* obstacle) {
    if (!check_ball_obstacle_collision(ball, obstacle)) {
        return;
    }
    
    // Simple bounce - reverse velocity component based on which side was hit
    uint64_t ball_center_x = ball->center_x;
    uint64_t ball_center_y = ball->center_y;
    uint64_t obs_center_x = obstacle->x + obstacle->width / 2;
    uint64_t obs_center_y = obstacle->y + obstacle->height / 2;
    
    // Determine which side was hit
    int64_t dx = ball_center_x - obs_center_x;
    int64_t dy = ball_center_y - obs_center_y;
    
    if ((dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy)) {
        // Hit from left or right
        ball->vel_x = -ball->vel_x;
        
        // Push ball out of obstacle
        if (dx > 0) {
            ball->center_x = obstacle->x + obstacle->width + ball->radius;
        } else {
            ball->center_x = obstacle->x - ball->radius;
        }
    } else {
        // Hit from top or bottom
        ball->vel_y = -ball->vel_y;
        
        // Push ball out of obstacle
        if (dy > 0) {
            ball->center_y = obstacle->y + obstacle->height + ball->radius;
        } else {
            ball->center_y = obstacle->y - ball->radius;
        }
    }
    
    // Reduce velocity slightly on collision
    ball->vel_x = (ball->vel_x * 8) / 10;  // 80% of original velocity
    ball->vel_y = (ball->vel_y * 8) / 10;
} 