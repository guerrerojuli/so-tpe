#include "ball.h"
#include "game.h"
#include "obstacle.h"
#include "unistd.h"

extern int is_multiplayer;

// Forward declaration for hit_ball function from player.c
void hit_ball(Player* player, Ball* ball);

void init_ball(Ball* ball, uint64_t center_x, uint64_t center_y, uint64_t radius, uint64_t color) {
    ball->center_x = center_x;
    ball->center_y = center_y;
    ball->vel_x = 0;
    ball->vel_y = 0;
    ball->radius = radius;
    ball->color = color;
    ball->friction = FRICTION_FACTOR;
}

void draw_ball(Ball* ball) {
    sys_draw_circle(ball->color, ball->center_x, ball->center_y, ball->radius);
}

// Intelligent erase function that preserves the hole and obstacles
void erase_ball_at_position(uint64_t center_x, uint64_t center_y, uint64_t radius) {
    // Calculate distance between ball and hole centers
    int64_t hole_dx = (int64_t)center_x - (int64_t)game_hole.center_x;
    int64_t hole_dy = (int64_t)center_y - (int64_t)game_hole.center_y;
    uint64_t distance_squared = hole_dx * hole_dx + hole_dy * hole_dy;
    uint64_t combined_radius = radius + game_hole.radius;
    
    // If ball and hole don't overlap, simple erase using sys_draw_circle
    if (distance_squared >= combined_radius * combined_radius) {
        sys_draw_circle(canvas_color, center_x, center_y, radius);
    } else {
        // They overlap, so first erase with canvas color then redraw hole
        sys_draw_circle(canvas_color, center_x, center_y, radius);
        draw_hole(&game_hole);
    }
    
    // Redraw obstacles if they exist (for compatibility with advanced features)
    if (current_level) {
        draw_obstacles(current_level);
    }
}

// Smart erase function for ball that preserves the hole
void erase_ball(Ball* ball) {
    erase_ball_at_position(ball->center_x, ball->center_y, ball->radius);
}

int is_ball_moving(Ball* ball) {
    return (ball->vel_x != 0 || ball->vel_y != 0);
}

// Update ball physics
void update_ball(Ball* ball, int owner_player_num) {
    uint64_t prev_x = ball->center_x;
    uint64_t prev_y = ball->center_y;
    
    int64_t new_x = (int64_t)ball->center_x + ball->vel_x;
    int64_t new_y = (int64_t)ball->center_y + ball->vel_y;
    
    if (new_x < 0) new_x = 0;
    if (new_y < 0) new_y = 0;
    
    ball->center_x = (uint64_t)new_x;
    ball->center_y = (uint64_t)new_y;
    
    // Boundary collision with canvas walls
    if (ball->center_x <= canvas_x + ball->radius) {
        ball->center_x = canvas_x + ball->radius;
        ball->vel_x = -ball->vel_x;
    } else if (ball->center_x >= canvas_x + canvas_width - ball->radius) {
        ball->center_x = canvas_x + canvas_width - ball->radius;
        ball->vel_x = -ball->vel_x;
    }
    
    if (ball->center_y <= canvas_y + ball->radius) {
        ball->center_y = canvas_y + ball->radius;
        ball->vel_y = -ball->vel_y;
    } else if (ball->center_y >= canvas_y + canvas_height - ball->radius) {
        ball->center_y = canvas_y + canvas_height - ball->radius;
        ball->vel_y = -ball->vel_y;
    }
    
    // Check collisions with obstacles
    if (current_level && current_level->obstacles) {
        for (int i = 0; i < current_level->obstacle_count; i++) {
            handle_ball_obstacle_collision(ball, &current_level->obstacles[i]);
        }
    }
    
    // Apply friction
    if (ball->vel_x != 0 || ball->vel_y != 0) {
        int64_t abs_vel_x = ball->vel_x < 0 ? -ball->vel_x : ball->vel_x;
        int64_t abs_vel_y = ball->vel_y < 0 ? -ball->vel_y : ball->vel_y;
        int64_t total_vel = abs_vel_x + abs_vel_y;
        
        if (total_vel <= FRICTION_FACTOR) {
            ball->vel_x = 0;
            ball->vel_y = 0;
        } else {
            if (ball->vel_x > 0) ball->vel_x--;
            else if (ball->vel_x < 0) ball->vel_x++;
            if (ball->vel_y > 0) ball->vel_y--;
            else if (ball->vel_y < 0) ball->vel_y++;
            if ((ball->vel_x == 0 && abs_vel_y > 2 && abs_vel_x > 0) ||
                (ball->vel_y == 0 && abs_vel_x > 2 && abs_vel_y > 0)) {
                if (ball->vel_x > 0) ball->vel_x--;
                else if (ball->vel_x < 0) ball->vel_x++;
                if (ball->vel_y > 0) ball->vel_y--;
                else if (ball->vel_y < 0) ball->vel_y++;
            }
        }
    }
    
    // Check if either player overlapped with the previous ball position
    extern Player player1, player2;
    int player1_overlaps_prev = !(player1.x + player1.size <= prev_x - ball->radius || 
                                  prev_x + ball->radius <= player1.x ||
                                  player1.y + player1.size <= prev_y - ball->radius ||
                                  prev_y + ball->radius <= player1.y);
    
    int player2_overlaps_prev = !(player2.x + player2.size <= prev_x - ball->radius || 
                                  prev_x + ball->radius <= player2.x ||
                                  player2.y + player2.size <= prev_y - ball->radius ||
                                  prev_y + ball->radius <= player2.y);
    
    erase_ball_at_position(prev_x, prev_y, ball->radius);
    
    // Redraw players if they overlapped with previous ball position
    if (player1_overlaps_prev) {
        draw_player(&player1);
    }
    if (player2_overlaps_prev) {
        draw_player(&player2);
    }
    
    draw_ball(ball);
    
    // Redraw players if they collide with new ball position
    if (check_collision(&player1, ball)) {
        hit_ball(&player1, ball);  // Handle collision - transfer momentum
        draw_player(&player1);
    }
    if (check_collision(&player2, ball)) {
        hit_ball(&player2, ball);  // Handle collision - transfer momentum  
        draw_player(&player2);
    }
    
    if (check_ball_in_hole(ball, &game_hole)) {
        extern int last_ball_toucher;
        extern int player1_wins, player2_wins;
        if (is_multiplayer && last_ball_toucher > 0) {
            // Multiplayer: winner is who touched the ball last
            if (last_ball_toucher == 1) {
                player1_wins++;
            } else {
                player2_wins++;
            }
        }
        game_completed = 1;
        game_running = 0;
        play_victory_sound();
    }
}