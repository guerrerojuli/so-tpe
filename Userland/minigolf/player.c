#include "player.h"
#include "game.h"
#include "obstacle.h"
#include "hole.h"
#include "unistd.h"
#include "rtc.h"

// ========== CONSTANTS ==========
#define PLAYER_MOVE_SPEED 2
#define MAX_BALL_VELOCITY 30
#define COLLISION_THRESHOLD 5
#define POWER_SCALING_THRESHOLD 25
#define MOVEMENT_TIME_WINDOW 3
#define MAX_POWER_MULTIPLIER 10
#define MOMENTUM_SCALING_FACTOR 3

// ========== TYPES ==========
typedef enum {
    MOVE_UP,
    MOVE_DOWN, 
    MOVE_LEFT,
    MOVE_RIGHT
} move_direction_t;

typedef struct {
    int obstacle_collision;
    int hole_collision;
    int player_collision;
} collision_result_t;

// ========== EXTERNAL DEPENDENCIES ==========
extern int is_multiplayer;
extern Ball game_ball;
extern Level* current_level;
extern Hole game_hole;
extern Player player1, player2;
extern int current_player_turn, player1_hits, player2_hits, last_ball_toucher;

// ========== PRIVATE HELPER FUNCTIONS ==========

// Calculate force multiplier based on key hold duration
static uint64_t calculate_force_multiplier(uint64_t key_hold_duration) {
    if (key_hold_duration >= POWER_SCALING_THRESHOLD) {
        return MAX_POWER_MULTIPLIER;
    } else if (key_hold_duration >= 2) {
        // Smooth scaling from 1x to 10x over 23 ticks
        if (key_hold_duration <= 12) {
            return 1 + ((key_hold_duration - 2) * 3) / 10;
        } else {
            return 4 + ((key_hold_duration - 12) * 6) / 13;
        }
    }
    return 1;
}

// Check all types of collisions for a player at given position
static collision_result_t check_all_collisions(Player* player, uint64_t prev_x, uint64_t prev_y) {
    collision_result_t result = {0, 0, 0};
    
    // Check obstacle collisions
    if (current_level && current_level->obstacles) {
        for (int i = 0; i < current_level->obstacle_count; i++) {
            if (check_player_obstacle_collision(player, &current_level->obstacles[i])) {
                result.obstacle_collision = 1;
                break;
            }
        }
    }
    
    // Check hole collision
    if (check_player_hole_collision(player, &game_hole)) {
        result.hole_collision = 1;
    }
    
    // Check player collision in multiplayer
    if (is_multiplayer) {
        Player* other_player = (player == &player1) ? &player2 : &player1;
        if (check_player_collision(player, other_player)) {
            result.player_collision = 1;
        }
    }
    
    return result;
}

// Update hit counts based on which player hit the ball
static void update_hit_counts(Player* player) {
    if (is_multiplayer) {
        if (player == &player1) {
            player1_hits++;
            last_ball_toucher = 1;
        } else if (player == &player2) {
            player2_hits++;
            last_ball_toucher = 2;
        }
    }
    hit_count++;
}

// Update UI after a ball hit
static void update_ui_after_hit(void) {
    extern void show_single_player_info(void);
    extern void show_both_players_info(void);
    
    if (is_multiplayer) {
        show_both_players_info();
    } else {
        show_single_player_info();
    }
}

// Apply momentum transfer and power scaling to ball
static void apply_momentum_transfer(Ball* ball, Player* player, int64_t hit_x, int64_t hit_y, uint64_t force_multiplier) {
    // Normalize direction for cleaner movement
    int64_t dir_x = 0, dir_y = 0;
    if (hit_x > COLLISION_THRESHOLD) dir_x = 1;
    else if (hit_x < -COLLISION_THRESHOLD) dir_x = -1;
    if (hit_y > COLLISION_THRESHOLD) dir_y = 1;
    else if (hit_y < -COLLISION_THRESHOLD) dir_y = -1;
    
    // Base force scales with hold duration
    int64_t base_force = COLLISION_FORCE * force_multiplier;
    int64_t new_vel_x = dir_x * base_force;
    int64_t new_vel_y = dir_y * base_force;
    
    // Add momentum transfer (also scales with force)
    new_vel_x += player->recent_vel_x * force_multiplier * MOMENTUM_SCALING_FACTOR;
    new_vel_y += player->recent_vel_y * force_multiplier * MOMENTUM_SCALING_FACTOR;
    
    // Add to current ball velocity
    ball->vel_x += new_vel_x;
    ball->vel_y += new_vel_y;
}

// Unified movement function for all directions
static int move_player(Player* player, move_direction_t direction) {
    // Don't allow movement if ball is moving
    if (are_any_balls_moving()) {
        return 0;
    }
    
    // Set consistent move speed
    player->move_speed = PLAYER_MOVE_SPEED;
    
    // Store previous position
    uint64_t prev_x = player->x;
    uint64_t prev_y = player->y;
    
    // Apply movement based on direction
    switch (direction) {
        case MOVE_UP:
            if (player->y >= canvas_y + player->move_speed) {
                player->y -= player->move_speed;
            } else {
                player->y = canvas_y;
            }
            break;
        case MOVE_DOWN:
            if (player->y + player->size + player->move_speed <= canvas_y + canvas_height) {
                player->y += player->move_speed;
            } else {
                player->y = canvas_y + canvas_height - player->size;
            }
            break;
        case MOVE_LEFT:
            if (player->x >= canvas_x + player->move_speed) {
                player->x -= player->move_speed;
            } else {
                player->x = canvas_x;
            }
            break;
        case MOVE_RIGHT:
            if (player->x + player->size + player->move_speed <= canvas_x + canvas_width) {
                player->x += player->move_speed;
            } else {
                player->x = canvas_x + canvas_width - player->size;
            }
            break;
    }
    
    // Check for collisions
    collision_result_t collisions = check_all_collisions(player, prev_x, prev_y);
    
    if (collisions.obstacle_collision || collisions.hole_collision || collisions.player_collision) {
        // Revert movement
        player->x = prev_x;
        player->y = prev_y;
        return 0;
    }
    
    // Update velocity tracking for hit system
    switch (direction) {
        case MOVE_UP:
            player->recent_vel_y = -1;
            player->recent_vel_x = 0;
            player->vel_y = -1;
            player->vel_x = 0;
            break;
        case MOVE_DOWN:
            player->recent_vel_y = 1;
            player->recent_vel_x = 0;
            player->vel_y = 1;
            player->vel_x = 0;
            break;
        case MOVE_LEFT:
            player->recent_vel_x = -1;
            player->recent_vel_y = 0;
            player->vel_x = -1;
            player->vel_y = 0;
            break;
        case MOVE_RIGHT:
            player->recent_vel_x = 1;
            player->recent_vel_y = 0;
            player->vel_x = 1;
            player->vel_y = 0;
            break;
    }
    
    player->last_active_movement_tick = sys_get_ticks();
    player->is_moving = 1;
    
    return 1;
}

// ========== PUBLIC API FUNCTIONS ==========

void init_player(Player* player, uint64_t x, uint64_t y, uint64_t size, uint64_t color) {
    player->x = x;
    player->y = y;
    player->size = size;
    player->color = color;
    player->move_speed = 1;
    player->last_movement = 0;
    player->last_movement_tick = 0;
    player->vel_x = 0;
    player->vel_y = 0;
    player->is_moving = 0;
    player->recent_vel_x = 0;
    player->recent_vel_y = 0;
    player->last_active_movement_tick = 0;
    player->key_hold_start_tick = 0;
    player->consecutive_movement_count = 0;
    player->current_held_direction = 0;
}

void draw_player(Player* player) {
    sys_draw_square(player->color, player->x, player->y, player->size);
}

void erase_player(Player* player) {
    erase_player_at_position(player->x, player->y, player->size);
}

// Intelligent erase function that preserves the hole
void erase_player_at_position(uint64_t x, uint64_t y, uint64_t size) {
    // Ya no es necesario verificar el hoyo porque los jugadores no pueden pisarlo
    sys_draw_square(canvas_color, x, y, size);
}

// Check collision between player and ball
int check_collision(Player* player, Ball* ball) {
    // Collision between rectangle (player) and circle (ball)
    // Find the closest point in the rectangle to the circle center
    uint64_t closest_x = ball->center_x;
    uint64_t closest_y = ball->center_y;
    
    // Clamp circle center to rectangle bounds
    if (ball->center_x < player->x) closest_x = player->x;
    else if (ball->center_x > player->x + player->size) closest_x = player->x + player->size;
    
    if (ball->center_y < player->y) closest_y = player->y;
    else if (ball->center_y > player->y + player->size) closest_y = player->y + player->size;
    
    // Calculate distance from circle center to closest point
    int64_t dx = (int64_t)ball->center_x - (int64_t)closest_x;
    int64_t dy = (int64_t)ball->center_y - (int64_t)closest_y;
    
    return (dx * dx + dy * dy) <= (ball->radius * ball->radius);
}

// Check collision between two players (rectangle to rectangle)
int check_player_collision(Player* player1, Player* player2) {
    // Check if rectangles overlap
    return !(player1->x + player1->size <= player2->x ||  // player1 is to the left of player2
             player2->x + player2->size <= player1->x ||  // player2 is to the left of player1
             player1->y + player1->size <= player2->y ||  // player1 is above player2
             player2->y + player2->size <= player1->y);   // player2 is above player1
}

void hit_ball(Player* player, Ball* ball) {
    // In multiplayer, only allow hitting if it's this player's turn
    if (is_multiplayer) {
        int player_number = (player == &player1) ? 1 : 2;
        if (player_number != current_player_turn) {
            return; // Not this player's turn - atraviesa la pelota
        }
    }
    
    // Update hit counts and UI
    update_hit_counts(player);
    update_ui_after_hit();
    
    // Calculate hit direction (from player center to ball center)
    int64_t player_center_x = player->x + player->size / 2;
    int64_t player_center_y = player->y + player->size / 2;
    int64_t hit_x = ball->center_x - player_center_x;
    int64_t hit_y = ball->center_y - player_center_y;
    
    if (hit_x == 0 && hit_y == 0) {
        return; // No direction to hit
    }
    
    // Calculate force based on key hold duration and movement timing
    uint64_t current_ticks = sys_get_ticks();
    uint64_t time_since_movement = current_ticks - player->last_active_movement_tick;
    uint64_t key_hold_duration = current_ticks - player->key_hold_start_tick;
    
    // Apply power scaling if movement was recent and player is holding keys
    if (time_since_movement <= MOVEMENT_TIME_WINDOW && player->key_hold_start_tick > 0) {
        uint64_t force_multiplier = calculate_force_multiplier(key_hold_duration);
        apply_momentum_transfer(ball, player, hit_x, hit_y, force_multiplier);
    } else {
        // No recent movement or key hold - minimal force with clean directions
        if (hit_x > COLLISION_THRESHOLD) ball->vel_x += COLLISION_THRESHOLD;
        else if (hit_x < -COLLISION_THRESHOLD) ball->vel_x -= COLLISION_THRESHOLD;
        
        if (hit_y > COLLISION_THRESHOLD) ball->vel_y += COLLISION_THRESHOLD;
        else if (hit_y < -COLLISION_THRESHOLD) ball->vel_y -= COLLISION_THRESHOLD;
    }
    
    // Apply velocity cap
    if (ball->vel_x > MAX_BALL_VELOCITY) ball->vel_x = MAX_BALL_VELOCITY;
    if (ball->vel_x < -MAX_BALL_VELOCITY) ball->vel_x = -MAX_BALL_VELOCITY;
    if (ball->vel_y > MAX_BALL_VELOCITY) ball->vel_y = MAX_BALL_VELOCITY;
    if (ball->vel_y < -MAX_BALL_VELOCITY) ball->vel_y = -MAX_BALL_VELOCITY;
    
    // Reset key hold tracking after hitting the ball
    player->key_hold_start_tick = 0;
}

// ========== MOVEMENT FUNCTIONS ==========

int move_player_up(Player* player) {
    return move_player(player, MOVE_UP);
}

int move_player_down(Player* player) {
    return move_player(player, MOVE_DOWN);
}

int move_player_left(Player* player) {
    return move_player(player, MOVE_LEFT);
}

int move_player_right(Player* player) {
    return move_player(player, MOVE_RIGHT);
}

// ========== COLLISION FUNCTIONS ==========

int check_player_player_collision(Player* p1, Player* p2) {
    return !(p1->x + p1->size <= p2->x || 
             p2->x + p2->size <= p1->x || 
             p1->y + p1->size <= p2->y || 
             p2->y + p2->size <= p1->y);
}