#include "game.h"
#include "ball.h"
#include "player.h"
#include "hole.h"
#include "obstacle.h"
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rtc.h"
#include "stddef.h"
#define MENU_BG_COLOR 0x001122
#define OBSTACLE_COLOR 0x8B4513

// Commonly used colors
#define COLOR_WHITE      0xFFFFFF
#define COLOR_GOLD       0xFFD700
#define COLOR_LIGHT_BLUE 0x8888FF
#define COLOR_GREEN      0x00FF00
#define COLOR_RED        0xFF0000
#define COLOR_DARK_GRAY  0x222222
#define COLOR_BLACK      0x000000
#define COLOR_GRAY       0x888888
#define COLOR_YELLOW     0xFFFF00
#define COLOR_BLUE       0x4444FF
#define COLOR_CYAN       0x00FFFF
#define COLOR_LIGHT_RED  0xFF8888

// Increments for power color gradient. These values were tuned visually and are
// not considered magic numbers as they represent incremental color steps.
#define POWER_COLOR_INC1 0x002200
#define POWER_COLOR_INC2 0x003300
#define POWER_COLOR_INC3 0x005500
#define POWER_COLOR_ORANGE 0xFF4400
#define POWER_COLOR_YELLOW_BASE 0xFFAA00

// Game state variables
uint64_t canvas_color = 0x00AA00; // Green canvas background
uint64_t canvas_x, canvas_y, canvas_width, canvas_height;
int game_running = 1;
int hit_count = 0;       // Track number of hits
int game_completed = 0;  // Track if ball is in hole

// Movement control variables
static int last_movement_frame = 0;
static const int MOVEMENT_DELAY = 1; // Delay between movements (in frames) - reduced for smoother movement

// Frame counter for movement timing
static int frame_counter = 0;

// Multiplayer variables
int is_multiplayer = 0;
int current_player_turn = 1;  // 1 = Player1's turn, 2 = Player2's turn
int player1_hits = 0;
int player2_hits = 0;
int last_ball_toucher = 0; // 1 or 2, who touched the ball last
int player1_wins = 0;      // Count of levels won by player 1
int player2_wins = 0;      // Count of levels won by player 2

// Player instances
Player player1;
Player player2;

// Ball instances
Ball game_ball;

// Hole instance
Hole game_hole;

// Level system variables
Level* current_level = NULL;
int current_level_number = 1;
int total_score = 0;
int levels_completed = 0;

// Level obstacles definitions
Obstacle level2_obstacles[] = {
    {0, 0, 60, 120, OBSTACLE_COLOR} // Will be positioned relative to canvas
};

Obstacle level3_obstacles[] = {
    {0, 0, 40, 80, OBSTACLE_COLOR},  // Will be positioned relative to canvas
    {0, 0, 40, 60, OBSTACLE_COLOR},
    {0, 0, 60, 40, OBSTACLE_COLOR}
};

// Level definitions - positions will be calculated relative to canvas
Level game_levels[MAX_LEVELS] = {
    {
        .level_number = 1,
        .level_name = "First Steps",
        .player1_start_x = 0, .player1_start_y = 0,  // Will be set in load_level
        .player2_start_x = 0, .player2_start_y = 0,
        .ball_start_x = 0, .ball_start_y = 0,
        .hole_x = 0, .hole_y = 0, .hole_radius = 12,
        .obstacles = NULL,
        .obstacle_count = 0,
        .max_strokes = 5
    },
    {
        .level_number = 2,
        .level_name = "The Wall",
        .player1_start_x = 0, .player1_start_y = 0,
        .player2_start_x = 0, .player2_start_y = 0,
        .ball_start_x = 0, .ball_start_y = 0,
        .hole_x = 0, .hole_y = 0, .hole_radius = 12,
        .obstacles = level2_obstacles,
        .obstacle_count = 1,
        .max_strokes = 6
    },
    {
        .level_number = 3,
        .level_name = "Maze Runner",
        .player1_start_x = 0, .player1_start_y = 0,
        .player2_start_x = 0, .player2_start_y = 0,
        .ball_start_x = 0, .ball_start_y = 0,
        .hole_x = 0, .hole_y = 0, .hole_radius = 12,
        .obstacles = level3_obstacles,
        .obstacle_count = 3,
        .max_strokes = 8
    }
};

// User-level delay function using sys_get_ticks
// Based on Kernel/idt/time.c logic (approx. 18 ticks per second)
#define TICKS_PER_SECOND 18
void user_delay(uint64_t milliseconds) {
    uint64_t start_ticks = sys_get_ticks();
    uint64_t num_ticks_to_wait = (milliseconds * TICKS_PER_SECOND) / 1000;

    // Ensure a small delay for very short, non-zero millisecond values
    if (milliseconds > 0 && num_ticks_to_wait == 0) {
        num_ticks_to_wait = 1;
    }

    while (sys_get_ticks() - start_ticks < num_ticks_to_wait);
}

// Old get_instructions_position function removed - no longer needed

// Display victory message
void show_victory_screen() {
    // Get screen dimensions using syscalls
    uint64_t screen_width = sys_get_screen_width();
    uint64_t screen_height = sys_get_screen_height();
    
    // Calculate relative positions
    uint64_t center_x = screen_width / 2;
    
    sys_clear_screen(MENU_BG_COLOR);
    
    if (is_multiplayer) {
        // Multiplayer victory screen - show final results after all levels
        
        // Title
        uint64_t title_x = center_x - (19 * 3 * 8) / 2; // "MULTIPLAYER RESULTS"
        uint64_t title_y = screen_height / 5;
        sys_put_text("MULTIPLAYER RESULTS", 19, COLOR_GOLD, title_x, title_y, 3);
        
        // Show levels won by each player
        char p1_wins_text[50];
        char p2_wins_text[50];
        char wins_str[10];
        
        strcpy(p1_wins_text, "Player 1 (White) won: ");
        itoa(player1_wins, wins_str);
        strcat(p1_wins_text, wins_str);
        strcat(p1_wins_text, " levels");
        
        strcpy(p2_wins_text, "Player 2 (Blue) won: ");
        itoa(player2_wins, wins_str);
        strcat(p2_wins_text, wins_str);
        strcat(p2_wins_text, " levels");
        
        uint64_t p1_x = center_x - (strlen(p1_wins_text) * 2 * 8) / 2;
        uint64_t p1_y = (screen_height * 35) / 100;
        sys_put_text(p1_wins_text, strlen(p1_wins_text), COLOR_WHITE, p1_x, p1_y, 2);
        
        uint64_t p2_x = center_x - (strlen(p2_wins_text) * 2 * 8) / 2;
        uint64_t p2_y = (screen_height * 45) / 100;
        sys_put_text(p2_wins_text, strlen(p2_wins_text), COLOR_LIGHT_BLUE, p2_x, p2_y, 2);
        
        // Determine overall winner
        char overall_winner[50];
        if (player1_wins > player2_wins) {
            strcpy(overall_winner, "PLAYER 1 WINS OVERALL!");
        } else if (player2_wins > player1_wins) {
            strcpy(overall_winner, "PLAYER 2 WINS OVERALL!");
    } else {
            strcpy(overall_winner, "IT'S A TIE!");
        }
        
        uint64_t winner_x = center_x - (strlen(overall_winner) * 2 * 8) / 2;
        uint64_t winner_y = (screen_height * 60) / 100;
        sys_put_text(overall_winner, strlen(overall_winner), COLOR_GREEN, winner_x, winner_y, 2);
        
    } else {
        // Singleplayer final victory screen (all levels completed)
        // Main title
        uint64_t title_x = center_x - (19 * 3 * 8) / 2; // "CONGRATULATIONS!" = 16 chars
        uint64_t title_y = screen_height / 5;
        sys_put_text("CONGRATULATIONS!", 16, COLOR_GOLD, title_x, title_y, 3);
        
        // Subtitle
        uint64_t subtitle_x = center_x - (31 * 2 * 8) / 2;
        uint64_t subtitle_y = (screen_height * 30) / 100;
        sys_put_text("You completed all 3 levels!", 28, COLOR_GREEN, subtitle_x, subtitle_y, 2);
        
        // Total score info
        char score_info[50];
        strcpy(score_info, "Total Score: ");
        char score_str[10];
        itoa(total_score, score_str);
        strcat(score_info, score_str);
        strcat(score_info, " hits");
        
        uint64_t score_x = center_x - (strlen(score_info) * 2 * 8) / 2;
        uint64_t score_y = (screen_height * 45) / 100;
        sys_put_text(score_info, strlen(score_info), COLOR_WHITE, score_x, score_y, 2);
        
        // Performance evaluation
        char performance[50];
        if (total_score <= 6) {
            strcpy(performance, "PERFECT! Amazing score!");
        } else if (total_score <= 12) {
            strcpy(performance, "Excellent performance!");
        } else if (total_score <= 18) {
            strcpy(performance, "Good job!");
        } else {
            strcpy(performance, "Keep practicing!");
        }
        
        uint64_t perf_x = center_x - (strlen(performance) * 1 * 8) / 2;
        uint64_t perf_y = (screen_height * 60) / 100;
        sys_put_text(performance, strlen(performance), COLOR_CYAN, perf_x, perf_y, 1);
    }
    
    // Instructions - centered, 70% from top
    uint64_t instructions_x = center_x - (32 * 1 * 8) / 2; // 32 chars * 1 font size * ~8 pixels per char / 2
    uint64_t instructions_y = (screen_height * 70) / 100;
    sys_put_text("Press any key to return to menu", 32, COLOR_WHITE, instructions_x, instructions_y, 1);
    
    // Render everything to screen
    sys_render_screen();
}

// Play victory sound effect - simplified to achievement melody only
void play_victory_sound() {
    // Achievement melody - ascending notes (C-E-G-C octave)
    sys_beep(523, 200);  // C5 - 200ms
    //user_delay(20);
    
    sys_beep(659, 200);  // E5 - 200ms
    //user_delay(20);
    
    sys_beep(784, 200);  // G5 - 200ms
    // user_delay(20);
    
    sys_beep(1047, 400); // C6 - 400ms (longer for emphasis)
}

// Bound movement functions that work with the current player
// Player 1 movement functions (WASD)
void move_player1_up() {
    // Check if it's player 1's turn and ball is not moving (for multiplayer)
    // In single player, also check if ball is moving to prevent movement while ball is in motion
    if ((is_multiplayer && (current_player_turn != 1 || is_ball_moving(&game_ball))) ||
        (!is_multiplayer && is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player1.x;
    uint64_t prev_y = player1.y;
    
    if (move_player_up(&player1)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player1.size);
        draw_player(&player1);
    }
}

void move_player1_down() {
    // Check if it's player 1's turn and ball is not moving (for multiplayer)
    // In single player, also check if ball is moving to prevent movement while ball is in motion
    if ((is_multiplayer && (current_player_turn != 1 || is_ball_moving(&game_ball))) ||
        (!is_multiplayer && is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player1.x;
    uint64_t prev_y = player1.y;
    
    if (move_player_down(&player1)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player1.size);
        draw_player(&player1);
    }
}

void move_player1_left() {
    // Check if it's player 1's turn and ball is not moving (for multiplayer)
    // In single player, also check if ball is moving to prevent movement while ball is in motion
    if ((is_multiplayer && (current_player_turn != 1 || is_ball_moving(&game_ball))) ||
        (!is_multiplayer && is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player1.x;
    uint64_t prev_y = player1.y;
    
    if (move_player_left(&player1)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player1.size);
        draw_player(&player1);
    }
}

void move_player1_right() {
    // Check if it's player 1's turn and ball is not moving (for multiplayer)
    // In single player, also check if ball is moving to prevent movement while ball is in motion
    if ((is_multiplayer && (current_player_turn != 1 || is_ball_moving(&game_ball))) ||
        (!is_multiplayer && is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player1.x;
    uint64_t prev_y = player1.y;
    
    if (move_player_right(&player1)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player1.size);
        draw_player(&player1);
    }
}

// Player 2 movement functions (Arrow keys)
void move_player2_up() {
    // Check if it's player 2's turn and ball is not moving
    if (is_multiplayer && (current_player_turn != 2 || is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player2.x;
    uint64_t prev_y = player2.y;
    
    if (move_player_up(&player2)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player2.size);
        draw_player(&player2);
    }
}

void move_player2_down() {
    // Check if it's player 2's turn and ball is not moving
    if (is_multiplayer && (current_player_turn != 2 || is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player2.x;
    uint64_t prev_y = player2.y;
    
    if (move_player_down(&player2)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player2.size);
        draw_player(&player2);
    }
}

void move_player2_left() {
    // Check if it's player 2's turn and ball is not moving
    if (is_multiplayer && (current_player_turn != 2 || is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player2.x;
    uint64_t prev_y = player2.y;
    
    if (move_player_left(&player2)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player2.size);
        draw_player(&player2);
    }
}

void move_player2_right() {
    // Check if it's player 2's turn and ball is not moving
    if (is_multiplayer && (current_player_turn != 2 || is_ball_moving(&game_ball))) {
        return;
    }
    
    uint64_t prev_x = player2.x;
    uint64_t prev_y = player2.y;
    
    if (move_player_right(&player2)) {
        // Erase old position intelligently and draw new position
        erase_player_at_position(prev_x, prev_y, player2.size);
        draw_player(&player2);
    }
}

// Multi-key movement function - handles all key combinations for both players
void handle_movement() {
    // Only allow movement when balls are not moving
    if (are_any_balls_moving()) {
        return;
    }
    
    // Check movement delay
    if (frame_counter - last_movement_frame < MOVEMENT_DELAY) {
        return;
    }
    
    // Check for key releases to reset key hold tracking
    
    // Reset Player 1 key hold if no relevant keys are pressed
    uint8_t w_pressed = sys_is_key_pressed(SCANCODE_W);
    uint8_t a_pressed = sys_is_key_pressed(SCANCODE_A);
    uint8_t s_pressed = sys_is_key_pressed(SCANCODE_S);
    uint8_t d_pressed = sys_is_key_pressed(SCANCODE_D);
    
    if (!w_pressed && !a_pressed && !s_pressed && !d_pressed) {
        // No keys pressed - reset key hold tracking for player 1
        player1.current_held_direction = 0;
        player1.consecutive_movement_count = 0;
        player1.key_hold_start_tick = 0;
    } else {
        // Keys are pressed - start timing if not already started
        if (player1.key_hold_start_tick == 0) {
            player1.key_hold_start_tick = sys_get_ticks();
        }
        
        // Check if we changed direction - if so, need to reset tracking in movement functions
        char current_direction = 0;
        if (w_pressed) current_direction = 'w';
        else if (s_pressed) current_direction = 's';
        else if (a_pressed) current_direction = 'a';
        else if (d_pressed) current_direction = 'd';
        
        // For diagonal movement, we'll let the movement functions handle the complexity
        if ((w_pressed && a_pressed) || (w_pressed && d_pressed) || 
            (s_pressed && a_pressed) || (s_pressed && d_pressed)) {
            // Diagonal - complex case handled in movement functions
        } else if (current_direction != player1.current_held_direction) {
            // Single direction changed - reset timing
            player1.key_hold_start_tick = sys_get_ticks();
            player1.current_held_direction = current_direction;
        }
    }
    
    // Handle Player 2 (Arrow keys) only in multiplayer
    uint8_t up_pressed = 0, down_pressed = 0, left_pressed = 0, right_pressed = 0;
    if (is_multiplayer) {
        up_pressed = sys_is_key_pressed(SCANCODE_UP_ARROW);
        down_pressed = sys_is_key_pressed(SCANCODE_DOWN_ARROW);
        left_pressed = sys_is_key_pressed(SCANCODE_LEFT_ARROW);
        right_pressed = sys_is_key_pressed(SCANCODE_RIGHT_ARROW);
        
        if (!up_pressed && !down_pressed && !left_pressed && !right_pressed) {
            // No keys pressed - reset key hold tracking for player 2  
            player2.current_held_direction = 0;
            player2.consecutive_movement_count = 0;
            player2.key_hold_start_tick = 0;
        } else {
            // Keys are pressed - start timing if not already started
            if (player2.key_hold_start_tick == 0) {
                player2.key_hold_start_tick = sys_get_ticks();
            }
            
            // Check direction changes
            char current_direction = 0;
            if (up_pressed) current_direction = 'u';
            else if (down_pressed) current_direction = 'd';
            else if (left_pressed) current_direction = 'l';
            else if (right_pressed) current_direction = 'r';
            
            // For diagonal movement, we'll let the movement functions handle the complexity
            if ((up_pressed && left_pressed) || (up_pressed && right_pressed) || 
                (down_pressed && left_pressed) || (down_pressed && right_pressed)) {
                // Diagonal - complex case handled in movement functions
            } else if (current_direction != player2.current_held_direction) {
                // Single direction changed - reset timing
                player2.key_hold_start_tick = sys_get_ticks();
                player2.current_held_direction = current_direction;
            }
        }
    }
    
    int any_movement = 0;
    
    // Reset player velocities by default (will be set in movement functions if they actually move)
    player1.vel_x = 0;
    player1.vel_y = 0;
    player1.is_moving = 0;
    
    if (is_multiplayer) {
        player2.vel_x = 0;
        player2.vel_y = 0;
        player2.is_moving = 0;
    }
    
    // Handle Player 1 movement (WASD)
    if (w_pressed || a_pressed || s_pressed || d_pressed) {
        uint64_t prev_x = player1.x;
        uint64_t prev_y = player1.y;
        int moved = 0;
        
        // Handle diagonal movement combinations for Player 1
        if (w_pressed && a_pressed) {
            int moved_up = move_player_up(&player1);
            int moved_left = move_player_left(&player1);
            if (moved_up || moved_left) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_up && moved_left) {
                    player1.vel_x = -player1.move_speed;
                    player1.vel_y = -player1.move_speed;
                    player1.recent_vel_x = -player1.move_speed;
                    player1.recent_vel_y = -player1.move_speed;
                } else if (moved_up) {
                    player1.vel_y = -player1.move_speed;
                    player1.recent_vel_y = -player1.move_speed;
                } else if (moved_left) {
                    player1.vel_x = -player1.move_speed;
                    player1.recent_vel_x = -player1.move_speed;
                }
                player1.last_active_movement_tick = sys_get_ticks();
            }
        } else if (w_pressed && d_pressed) {
            int moved_up = move_player_up(&player1);
            int moved_right = move_player_right(&player1);
            if (moved_up || moved_right) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_up && moved_right) {
                    player1.vel_x = player1.move_speed;
                    player1.vel_y = -player1.move_speed;
                    player1.recent_vel_x = player1.move_speed;
                    player1.recent_vel_y = -player1.move_speed;
                } else if (moved_up) {
                    player1.vel_y = -player1.move_speed;
                    player1.recent_vel_y = -player1.move_speed;
                } else if (moved_right) {
                    player1.vel_x = player1.move_speed;
                    player1.recent_vel_x = player1.move_speed;
                }
                player1.last_active_movement_tick = sys_get_ticks();
            }
        } else if (s_pressed && a_pressed) {
            int moved_down = move_player_down(&player1);
            int moved_left = move_player_left(&player1);
            if (moved_down || moved_left) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_down && moved_left) {
                    player1.vel_x = -player1.move_speed;
                    player1.vel_y = player1.move_speed;
                    player1.recent_vel_x = -player1.move_speed;
                    player1.recent_vel_y = player1.move_speed;
                } else if (moved_down) {
                    player1.vel_y = player1.move_speed;
                    player1.recent_vel_y = player1.move_speed;
                } else if (moved_left) {
                    player1.vel_x = -player1.move_speed;
                    player1.recent_vel_x = -player1.move_speed;
                }
                player1.last_active_movement_tick = sys_get_ticks();
            }
        } else if (s_pressed && d_pressed) {
            int moved_down = move_player_down(&player1);
            int moved_right = move_player_right(&player1);
            if (moved_down || moved_right) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_down && moved_right) {
                    player1.vel_x = player1.move_speed;
                    player1.vel_y = player1.move_speed;
                    player1.recent_vel_x = player1.move_speed;
                    player1.recent_vel_y = player1.move_speed;
                } else if (moved_down) {
                    player1.vel_y = player1.move_speed;
                    player1.recent_vel_y = player1.move_speed;
                } else if (moved_right) {
                    player1.vel_x = player1.move_speed;
                    player1.recent_vel_x = player1.move_speed;
                }
                player1.last_active_movement_tick = sys_get_ticks();
            }
        } else {
            // Single direction movement for Player 1
            if (w_pressed && move_player_up(&player1)) moved = 1;
            if (s_pressed && move_player_down(&player1)) moved = 1;
            if (a_pressed && move_player_left(&player1)) moved = 1;
            if (d_pressed && move_player_right(&player1)) moved = 1;
        }
        
        // Redraw Player 1 if moved
        if (moved) {
            erase_player_at_position(prev_x, prev_y, player1.size);
            draw_player(&player1);
            any_movement = 1;
        }
    }
    
    // Handle Player 2 movement (Arrow keys) only in multiplayer - FIXED: Removed turn restriction
    if (is_multiplayer) {
        uint64_t prev_x = player2.x;
        uint64_t prev_y = player2.y;
        int moved = 0;
        
        // Handle diagonal movement for Player 2
        if (up_pressed && left_pressed) {
            int moved_up = move_player_up(&player2);
            int moved_left = move_player_left(&player2);
            if (moved_up || moved_left) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_up && moved_left) {
                    player2.vel_x = -player2.move_speed;
                    player2.vel_y = -player2.move_speed;
                    player2.recent_vel_x = -player2.move_speed;
                    player2.recent_vel_y = -player2.move_speed;
                } else if (moved_up) {
                    player2.vel_y = -player2.move_speed;
                    player2.recent_vel_y = -player2.move_speed;
                } else if (moved_left) {
                    player2.vel_x = -player2.move_speed;
                    player2.recent_vel_x = -player2.move_speed;
                }
                player2.last_active_movement_tick = sys_get_ticks();
            }
        } else if (up_pressed && right_pressed) {
            int moved_up = move_player_up(&player2);
            int moved_right = move_player_right(&player2);
            if (moved_up || moved_right) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_up && moved_right) {
                    player2.vel_x = player2.move_speed;
                    player2.vel_y = -player2.move_speed;
                    player2.recent_vel_x = player2.move_speed;
                    player2.recent_vel_y = -player2.move_speed;
                } else if (moved_up) {
                    player2.vel_y = -player2.move_speed;
                    player2.recent_vel_y = -player2.move_speed;
                } else if (moved_right) {
                    player2.vel_x = player2.move_speed;
                    player2.recent_vel_x = player2.move_speed;
                }
                player2.last_active_movement_tick = sys_get_ticks();
            }
        } else if (down_pressed && left_pressed) {
            int moved_down = move_player_down(&player2);
            int moved_left = move_player_left(&player2);
            if (moved_down || moved_left) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_down && moved_left) {
                    player2.vel_x = -player2.move_speed;
                    player2.vel_y = player2.move_speed;
                    player2.recent_vel_x = -player2.move_speed;
                    player2.recent_vel_y = player2.move_speed;
                } else if (moved_down) {
                    player2.vel_y = player2.move_speed;
                    player2.recent_vel_y = player2.move_speed;
                } else if (moved_left) {
                    player2.vel_x = -player2.move_speed;
                    player2.recent_vel_x = -player2.move_speed;
                }
                player2.last_active_movement_tick = sys_get_ticks();
            }
        } else if (down_pressed && right_pressed) {
            int moved_down = move_player_down(&player2);
            int moved_right = move_player_right(&player2);
            if (moved_down || moved_right) {
                moved = 1;
                // Set velocity based on actual movements achieved
                if (moved_down && moved_right) {
                    player2.vel_x = player2.move_speed;
                    player2.vel_y = player2.move_speed;
                    player2.recent_vel_x = player2.move_speed;
                    player2.recent_vel_y = player2.move_speed;
                } else if (moved_down) {
                    player2.vel_y = player2.move_speed;
                    player2.recent_vel_y = player2.move_speed;
                } else if (moved_right) {
                    player2.vel_x = player2.move_speed;
                    player2.recent_vel_x = player2.move_speed;
                }
                player2.last_active_movement_tick = sys_get_ticks();
            }
        } else {
            // Single direction movement for Player 2
            if (up_pressed && move_player_up(&player2)) moved = 1;
            if (down_pressed && move_player_down(&player2)) moved = 1;
            if (left_pressed && move_player_left(&player2)) moved = 1;
            if (right_pressed && move_player_right(&player2)) moved = 1;
        }
        
        // Redraw Player 2 if moved
        if (moved) {
            erase_player_at_position(prev_x, prev_y, player2.size);
            draw_player(&player2);
            any_movement = 1;
        }
    }
    
    // Update movement timer if any player moved
    if (any_movement) {
        last_movement_frame = frame_counter;
    }
}

void quit_game() {
    game_running = 0;
}

void switch_player_turn() {
    if (!is_multiplayer) return;
    
    // Switch to the other player
    current_player_turn = (current_player_turn == 1) ? 2 : 1;
    
    // Update display to show new active player
    show_both_players_info();
}

void show_both_players_info() {
    if (!is_multiplayer) return;
    
    // Get screen dimensions using syscalls
    uint64_t screen_width = sys_get_screen_width();
    uint64_t screen_height = sys_get_screen_height();
    
    // ====== TITLE AT TOP ======
    char title[] = "MINIGOLF";
    uint64_t title_width = strlen(title) * 4 * 8;  // 4x scale
    uint64_t title_x = (screen_width - title_width) / 2;
    uint64_t title_y = 20;
    
    // Clear title area
    sys_draw_rectangle(COLOR_BLACK, title_x - 20, title_y - 10, title_width + 40, 50);
    sys_put_text(title, strlen(title), COLOR_GOLD, title_x, title_y, 4);  // Gold color, large scale
    
    // ====== PLAYER INFO (BOTTOM LEFT) ======
    uint64_t left_info_x = 20;
    uint64_t left_info_y = screen_height - 140;
    uint64_t left_info_width = 300;
    uint64_t left_info_height = 120;
    
    // Clear left info area
    sys_draw_rectangle(COLOR_DARK_GRAY, left_info_x, left_info_y, left_info_width, left_info_height);
    
    // Current turn indicator
    char turn_text[30];
    strcpy(turn_text, "TURN: Player ");
    char turn_num[5];
    itoa(current_player_turn, turn_num);
    strcat(turn_text, turn_num);
    
    uint64_t turn_color = (current_player_turn == 1) ? COLOR_WHITE : COLOR_LIGHT_BLUE;
    sys_put_text(turn_text, strlen(turn_text), turn_color, left_info_x + 10, left_info_y + 10, 2);
    
    // Player 1 info
    char hits_str[10];
    char p1_info[40];
    strcpy(p1_info, "P1 (White): ");
    itoa(player1_hits, hits_str);
    strcat(p1_info, hits_str);
    strcat(p1_info, " hits");
    
    uint64_t p1_color = (current_player_turn == 1) ? COLOR_YELLOW : COLOR_WHITE;
    sys_put_text(p1_info, strlen(p1_info), p1_color, left_info_x + 10, left_info_y + 40, 1);
    
    // Player 2 info
    char p2_info[40];
    strcpy(p2_info, "P2 (Blue):  ");
    itoa(player2_hits, hits_str);
    strcat(p2_info, hits_str);
    strcat(p2_info, " hits");
    
    uint64_t p2_color = (current_player_turn == 2) ? COLOR_YELLOW : COLOR_LIGHT_BLUE;
    sys_put_text(p2_info, strlen(p2_info), p2_color, left_info_x + 10, left_info_y + 60, 1);
    
    // Ball status
    char ball_status[30];
    if (is_ball_moving(&game_ball)) {
        strcpy(ball_status, "Ball moving - WAIT");
        sys_put_text(ball_status, strlen(ball_status), COLOR_RED, left_info_x + 10, left_info_y + 85, 1);
    } else {
        strcpy(ball_status, "Ball stopped - GO!");
        sys_put_text(ball_status, strlen(ball_status), COLOR_GREEN, left_info_x + 10, left_info_y + 85, 1);
    }
    
    // Power indicators
    if (!is_ball_moving(&game_ball)) {
        // Player 1 power indicator
        if (current_player_turn == 1 && player1.key_hold_start_tick > 0) {
            uint64_t current_ticks = sys_get_ticks();
            uint64_t key_hold_duration = current_ticks - player1.key_hold_start_tick;
            
            // Calculate power level (1-10) matching the enhanced system
            uint64_t power_level = 1;
            if (key_hold_duration >= 25) {
                power_level = 10;
            } else if (key_hold_duration >= 2) {
                if (key_hold_duration <= 12) {
                    power_level = 1 + ((key_hold_duration - 2) * 3) / 10;
                } else {
                    power_level = 4 + ((key_hold_duration - 12) * 6) / 13;
                }
            }
            
            char power_text[30];
            strcpy(power_text, "P1 Power: ");
            char power_str[10];
            itoa(power_level, power_str);
            strcat(power_text, power_str);
            strcat(power_text, "/10");
            
            // Enhanced color gradient: Red -> Orange -> Yellow -> Green
            uint64_t power_color;
            if (power_level <= 3) {
                power_color = COLOR_RED + (power_level * POWER_COLOR_INC1); // Red to orange
            } else if (power_level <= 6) {
                power_color = POWER_COLOR_ORANGE + ((power_level - 3) * POWER_COLOR_INC2); // Orange to yellow
            } else {
                power_color = POWER_COLOR_YELLOW_BASE + ((power_level - 6) * POWER_COLOR_INC3); // Yellow to green
            }
            sys_put_text(power_text, strlen(power_text), power_color, left_info_x + 10, left_info_y + 105, 1);
        }
        
        // Player 2 power indicator
        if (current_player_turn == 2 && player2.key_hold_start_tick > 0) {
            uint64_t current_ticks = sys_get_ticks();
            uint64_t key_hold_duration = current_ticks - player2.key_hold_start_tick;
            
            // Calculate power level (1-10) matching the enhanced system
            uint64_t power_level = 1;
            if (key_hold_duration >= 25) {
                power_level = 10;
            } else if (key_hold_duration >= 2) {
                if (key_hold_duration <= 12) {
                    power_level = 1 + ((key_hold_duration - 2) * 3) / 10;
                } else {
                    power_level = 4 + ((key_hold_duration - 12) * 6) / 13;
                }
            }
            
            char power_text[30];
            strcpy(power_text, "P2 Power: ");
            char power_str[10];
            itoa(power_level, power_str);
            strcat(power_text, power_str);
            strcat(power_text, "/10");
            
            // Enhanced color gradient: Red -> Orange -> Yellow -> Green
            uint64_t power_color;
            if (power_level <= 3) {
                power_color = COLOR_RED + (power_level * POWER_COLOR_INC1); // Red to orange
            } else if (power_level <= 6) {
                power_color = POWER_COLOR_ORANGE + ((power_level - 3) * POWER_COLOR_INC2); // Orange to yellow
            } else {
                power_color = POWER_COLOR_YELLOW_BASE + ((power_level - 6) * POWER_COLOR_INC3); // Yellow to green
            }
            sys_put_text(power_text, strlen(power_text), power_color, left_info_x + 10, left_info_y + 105, 1);
        }
    }
    
    // ====== KEYBINDS (BOTTOM RIGHT) ======
    uint64_t right_info_x = screen_width - 320;
    uint64_t right_info_y = screen_height - 140;
    uint64_t right_info_width = 300;
    uint64_t right_info_height = 120;
    
    // Clear right info area
    sys_draw_rectangle(COLOR_DARK_GRAY, right_info_x, right_info_y, right_info_width, right_info_height);
    
    // Keybind title
    sys_put_text("CONTROLS", 8, COLOR_GOLD, right_info_x + 10, right_info_y + 10, 2);
    
    // Player 1 controls
    sys_put_text("Player 1: WASD to move", 22, COLOR_WHITE, right_info_x + 10, right_info_y + 35, 1);
    
    // Player 2 controls  
    sys_put_text("Player 2: Arrow keys", 20, COLOR_LIGHT_BLUE, right_info_x + 10, right_info_y + 50, 1);
    
    // General controls
    sys_put_text("Hold key longer for power", 25, COLOR_WHITE, right_info_x + 10, right_info_y + 70, 1);
    
    // Game tip
    sys_put_text("Tip: Use fewer hits", 18, COLOR_GRAY, right_info_x + 10, right_info_y + 90, 1);
    sys_put_text("for a better score!", 19, COLOR_GRAY, right_info_x + 10, right_info_y + 105, 1);
}

void show_single_player_info() {
    if (is_multiplayer) return;
    
    // Get screen dimensions using syscalls
    uint64_t screen_width = sys_get_screen_width();
    uint64_t screen_height = sys_get_screen_height();
    
    // ====== TITLE AT TOP ======
    char title[] = "MINIGOLF";
    uint64_t title_width = strlen(title) * 4 * 8;  // 4x scale
    uint64_t title_x = (screen_width - title_width) / 2;
    uint64_t title_y = 20;
    
    // Clear title area
    sys_draw_rectangle(COLOR_BLACK, title_x - 20, title_y - 10, title_width + 40, 50);
    sys_put_text(title, strlen(title), COLOR_GOLD, title_x, title_y, 4);  // Gold color, large scale
    
    // ====== LEVEL INFO (BOTTOM LEFT) ======
    uint64_t left_info_x = 20;
    uint64_t left_info_y = screen_height - 140;
    uint64_t left_info_width = 300;
    uint64_t left_info_height = 120;
    
    // Clear left info area
    sys_draw_rectangle(COLOR_DARK_GRAY, left_info_x, left_info_y, left_info_width, left_info_height);
    
    // Level info
    char level_text[50];
    strcpy(level_text, "Level ");
    char level_num[5];
    itoa(current_level_number, level_num);
    strcat(level_text, level_num);
    strcat(level_text, ": ");
    if (current_level) {
        strcat(level_text, current_level->level_name);
    }
    
    sys_put_text(level_text, strlen(level_text), COLOR_GOLD, left_info_x + 10, left_info_y + 10, 2);
    
    // Hits info
    char hits_info[40];
    strcpy(hits_info, "Hits: ");
    char hits_str[10];
    itoa(hit_count, hits_str);
    strcat(hits_info, hits_str);
    
    sys_put_text(hits_info, strlen(hits_info), COLOR_WHITE, left_info_x + 10, left_info_y + 40, 1);
    
    // Total score
    char score_info[40];
    strcpy(score_info, "Total Score: ");
    char score_str[10];
    itoa(total_score + hit_count, score_str);
    strcat(score_info, score_str);
    
    sys_put_text(score_info, strlen(score_info), COLOR_WHITE, left_info_x + 10, left_info_y + 60, 1);
    
    // Ball status
    char ball_status[30];
    if (is_ball_moving(&game_ball)) {
        strcpy(ball_status, "Ball moving - WAIT");
        sys_put_text(ball_status, strlen(ball_status), COLOR_RED, left_info_x + 10, left_info_y + 85, 1);
    } else {
        strcpy(ball_status, "Ball stopped - GO!");
        sys_put_text(ball_status, strlen(ball_status), COLOR_GREEN, left_info_x + 10, left_info_y + 85, 1);
    }
    
    // Power indicator for singleplayer
    if (!is_ball_moving(&game_ball) && player1.key_hold_start_tick > 0) {
        uint64_t current_ticks = sys_get_ticks();
        uint64_t key_hold_duration = current_ticks - player1.key_hold_start_tick;
        
        // Calculate power level (1-10) matching the enhanced system
        uint64_t power_level = 1;
        if (key_hold_duration >= 25) {
            power_level = 10;
        } else if (key_hold_duration >= 2) {
            if (key_hold_duration <= 12) {
                power_level = 1 + ((key_hold_duration - 2) * 3) / 10;
            } else {
                power_level = 4 + ((key_hold_duration - 12) * 6) / 13;
            }
        }
        
        char power_text[30];
        strcpy(power_text, "Power: ");
        char power_str[10];
        itoa(power_level, power_str);
        strcat(power_text, power_str);
        strcat(power_text, "/10");
        
        // Enhanced color gradient: Red -> Orange -> Yellow -> Green
        uint64_t power_color;
        if (power_level <= 3) {
            power_color = COLOR_RED + (power_level * POWER_COLOR_INC1); // Red to orange
        } else if (power_level <= 6) {
            power_color = POWER_COLOR_ORANGE + ((power_level - 3) * POWER_COLOR_INC2); // Orange to yellow
    } else {
            power_color = POWER_COLOR_YELLOW_BASE + ((power_level - 6) * POWER_COLOR_INC3); // Yellow to green
        }
        sys_put_text(power_text, strlen(power_text), power_color, left_info_x + 10, left_info_y + 105, 1);
    }
    
    // ====== KEYBINDS (BOTTOM RIGHT) ======
    uint64_t right_info_x = screen_width - 320;
    uint64_t right_info_y = screen_height - 140;
    uint64_t right_info_width = 300;
    uint64_t right_info_height = 120;
    
    // Clear right info area
    sys_draw_rectangle(COLOR_DARK_GRAY, right_info_x, right_info_y, right_info_width, right_info_height);
    
    // Keybind title
    sys_put_text("CONTROLS", 8, COLOR_GOLD, right_info_x + 10, right_info_y + 10, 2);
    
    // Movement controls
    sys_put_text("WASD: Move player", 17, COLOR_WHITE, right_info_x + 10, right_info_y + 35, 1);
    sys_put_text("Hold key longer for power", 25, COLOR_WHITE, right_info_x + 10, right_info_y + 50, 1);
      sys_put_text("Q: Quit game", 12, COLOR_LIGHT_RED, right_info_x + 10, right_info_y + 70, 1);
  
  // Game tip
  sys_put_text("Tip: Use fewer hits", 18, COLOR_GRAY, right_info_x + 10, right_info_y + 90, 1);
  sys_put_text("for a better score!", 19, COLOR_GRAY, right_info_x + 10, right_info_y + 105, 1);
}

// Check if any ball is moving (used by movement restriction system)
int are_any_balls_moving() {
    return is_ball_moving(&game_ball);
}

void show_menu() {
  // Get screen dimensions using syscalls
  uint64_t screen_width = sys_get_screen_width();
  uint64_t screen_height = sys_get_screen_height();
  
  // Clear screen with dark blue background
  sys_clear_screen(MENU_BG_COLOR);
  
  // Calculate relative positions based on screen dimensions
  uint64_t center_x = screen_width / 2;
  
  // Title with large font size (3) - centered horizontally, 10% from top
  uint64_t title_x = center_x - (8 * 3 * 8) / 2; // 8 chars * 3 font size * ~8 pixels per char / 2
  uint64_t title_y = screen_height / 10;
  sys_put_text("MINIGOLF", 8, COLOR_WHITE, title_x, title_y, 3);
  
  // Menu title with medium font size (2) - centered, 20% from top
  uint64_t menu_title_x = center_x - (9 * 2 * 8) / 2; // 9 chars * 2 font size * ~8 pixels per char / 2
  uint64_t menu_title_y = screen_height / 5;
  sys_put_text("Main Menu", 9, COLOR_WHITE, menu_title_x, menu_title_y, 2);
  
  // Menu options with medium font size (2) - centered, starting at 45% from top
  uint64_t options_start_y = (screen_height * 45) / 100;
  uint64_t option_spacing = (screen_height * 8) / 100; // 8% of screen height between options
  
  // Option 1: Singleplayer
  uint64_t option1_x = center_x - (15 * 2 * 8) / 2; // 15 chars * 2 font size * ~8 pixels per char / 2
  sys_put_text("1. Singleplayer", 15, COLOR_GREEN, option1_x, options_start_y, 2);
  
  // Option 2: Multiplayer
  uint64_t option2_x = center_x - (14 * 2 * 8) / 2; // 14 chars * 2 font size * ~8 pixels per char / 2
  sys_put_text("2. Multiplayer", 14, COLOR_GREEN, option2_x, options_start_y + option_spacing, 2);
  
  // Option 3: Exit
  uint64_t option3_x = center_x - (7 * 2 * 8) / 2; // 7 chars * 2 font size * ~8 pixels per char / 2
  sys_put_text("3. Exit", 7, COLOR_RED, option3_x, options_start_y + (2 * option_spacing), 2);
  
  // Instructions with small font size (1) - centered, 80% from top
  uint64_t instructions_x = center_x - (24 * 1 * 8) / 2; // 24 chars * 1 font size * ~8 pixels per char / 2
  uint64_t instructions_y = (screen_height * 80) / 100;
  sys_put_text("Enter your choice (1-3) ", 24, COLOR_WHITE, instructions_x, instructions_y, 1);
  
  // Render everything to screen
  sys_render_screen();
}

void start_singleplayer() {
  // Get screen dimensions using syscalls
  uint64_t screen_width = sys_get_screen_width();
  uint64_t screen_height = sys_get_screen_height();
  
  // Calculate canvas dimensions (80% width, 60% height for UI space)
  canvas_width = (screen_width * 80) / 100;
  canvas_height = (screen_height * 60) / 100;
  
  // Calculate canvas position (centered)
  canvas_x = (screen_width - canvas_width) / 2;
  canvas_y = (screen_height - canvas_height) / 2;
  
  // Reset level system state
  current_level_number = 1;
  total_score = 0;
  levels_completed = 0;
  
  // Load first level
  load_level(1);
  
  // In singleplayer, only player1 is used
  
  // Reset game state
  is_multiplayer = 0;
  game_running = 1;
  game_completed = 0;
  hit_count = 0;
  
  // Clear screen with outside color
  sys_clear_screen(COLOR_DARK_GRAY);
  
  // Draw the green canvas using a single rectangle
  sys_draw_rectangle(canvas_color, canvas_x, canvas_y, canvas_width, canvas_height);
  
  // Draw hole first (so it appears behind the ball if they overlap)
  draw_hole(&game_hole);
  
  // Draw obstacles
  draw_obstacles(current_level);
  
  // Draw initial player and ball (singleplayer uses player1)
  draw_player(&player1);
  draw_ball(&game_ball);
  
  // Show single player info with new UI
  show_single_player_info();
  
  // Render everything to screen
  sys_render_screen();
  
  // Only bind quit key - movement is now handled by multi-key system  
  sys_bind(SCANCODE_Q, quit_game);
  
  // Use unified game loop
  run_game_loop();
  
  // Unbind quit key when game ends
  sys_unbind(SCANCODE_Q);
  
  // Show appropriate end screen
  while (game_completed) {
    if (is_last_level()) {
      // Show final victory screen
      total_score += hit_count;
    show_victory_screen();
    getchar();
      break;
    } else {
      // Show level complete screen and advance
      show_level_complete_screen();
      getchar();
      advance_to_next_level();
      
      // Continue with next level
      sys_clear_screen(COLOR_DARK_GRAY);
      sys_draw_rectangle(canvas_color, canvas_x, canvas_y, canvas_width, canvas_height);
      
      // Draw hole first
      draw_hole(&game_hole);
      
      // Draw player and ball (singleplayer uses player1)
      draw_player(&player1);
      draw_ball(&game_ball);
      
      // Show single player info for new level
      show_single_player_info();
      
      // Only bind quit key - movement is now handled by multi-key system  
      sys_bind(SCANCODE_Q, quit_game);
      
      // Continue game loop with UI updates
      run_game_loop();
      
      // Unbind keys when level ends
      sys_unbind(SCANCODE_Q);
    }
  } 
}

// Check if any ball is moving in multiplayer
// This function is no longer needed - we only have one ball
// Use is_ball_moving(&game_ball) directly instead

// Integer square root function (Newton's method)
uint64_t isqrt(uint64_t n) {
    if (n == 0) return 0;
    uint64_t x = n;
    uint64_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

// Ball-ball collision function removed - only one ball now

void start_multiplayer() {
  // Get screen dimensions using syscalls
  uint64_t screen_width = sys_get_screen_width();
  uint64_t screen_height = sys_get_screen_height();
  
  // Calculate canvas dimensions (80% width, 60% height for UI space)
  canvas_width = (screen_width * 80) / 100;
  canvas_height = (screen_height * 60) / 100;
  
  // Calculate canvas position (centered)
  canvas_x = (screen_width - canvas_width) / 2;
  canvas_y = (screen_height - canvas_height) / 2;
  
  // Reset level system state
  current_level_number = 1;
  total_score = 0;
  levels_completed = 0;
  
  // Load first level
  load_level(1);
  
  // Set multiplayer mode and reset state
  is_multiplayer = 1;
  current_player_turn = 1;  // Player 1 starts
  game_running = 1;
  game_completed = 0;
  hit_count = 0;
  player1_hits = 0;
  player2_hits = 0;
  last_ball_toucher = 0;  // No one has touched the ball yet
  
  // Clear screen with outside color
  sys_clear_screen(COLOR_DARK_GRAY);
  
  // Draw the green canvas using a single rectangle
  sys_draw_rectangle(canvas_color, canvas_x, canvas_y, canvas_width, canvas_height);
  
  // Old text instructions removed - now using new UI panels
  
  // Draw hole first (so it appears behind the ball if they overlap)
  draw_hole(&game_hole);
  
  // Draw obstacles
  draw_obstacles(current_level);
  
  // Draw both players and the ball
  draw_player(&player1);
  draw_player(&player2);
  draw_ball(&game_ball);
  
  // Show both players info with new UI
  show_both_players_info();
  
  // Render everything to screen
  sys_render_screen();
  
  // Only bind quit key - movement is now handled by multi-key system
  sys_bind(SCANCODE_Q, quit_game);
  
  // Use unified game loop
  run_game_loop();
  
  // Unbind quit key when game ends
  sys_unbind(SCANCODE_Q);
  
  // Reset multiplayer mode
  is_multiplayer = 0;
  
  // Show appropriate end screen
  while (game_completed) {
    if (is_last_level()) {
      // Show final victory screen
      total_score += hit_count;
      show_victory_screen();
      getchar();
      break;
    } else {
      // Show level complete screen and advance
      show_level_complete_screen();
      getchar();
      advance_to_next_level();
      
      // Continue with next level for multiplayer
      sys_clear_screen(COLOR_DARK_GRAY);
      sys_draw_rectangle(canvas_color, canvas_x, canvas_y, canvas_width, canvas_height);
      
      // Draw hole first
      draw_hole(&game_hole);
      
      // Draw obstacles
      draw_obstacles(current_level);
      
      // Draw both players and the ball
      draw_player(&player1);
      draw_player(&player2);
      draw_ball(&game_ball);
      
      // Show both players info for new level  
      show_both_players_info();
      
      // Only bind quit key - movement is now handled by multi-key system
      sys_bind(SCANCODE_Q, quit_game);
      
      // Set multiplayer mode back
      is_multiplayer = 1;
      
      // Continue game loop with turn system
      run_game_loop();
      
      // Unbind keys when level ends
      sys_unbind(SCANCODE_Q);
    }
  }
}

// ============= LEVEL SYSTEM FUNCTIONS =============

// Load a specific level and set up all positions
void load_level(int level_num) {
    if (level_num < 1 || level_num > MAX_LEVELS) {
        return;
    }
    
    current_level = &game_levels[level_num - 1];
    current_level_number = level_num;
    
    // Calculate positions relative to canvas
    switch (level_num) {
        case 1: // First Steps - simple straight shot
            current_level->player1_start_x = canvas_x + 30;
            current_level->player1_start_y = canvas_y + canvas_height/2 - 30;
            current_level->player2_start_x = canvas_x + 30;
            current_level->player2_start_y = canvas_y + canvas_height/2 + 30;
            current_level->ball_start_x = canvas_x + canvas_width/3;
            current_level->ball_start_y = canvas_y + canvas_height/2;
            current_level->hole_x = canvas_x + canvas_width - 50;
            current_level->hole_y = canvas_y + canvas_height/2;
            break;
            
        case 2: // The Wall - obstacle in the middle
            current_level->player1_start_x = canvas_x + 30;
            current_level->player1_start_y = canvas_y + canvas_height/2 - 30;
            current_level->player2_start_x = canvas_x + 30;
            current_level->player2_start_y = canvas_y + canvas_height/2 + 30;
            current_level->ball_start_x = canvas_x + canvas_width/3;
            current_level->ball_start_y = canvas_y + canvas_height/2;
            current_level->hole_x = canvas_x + canvas_width - 50;
            current_level->hole_y = canvas_y + canvas_height/2;
            
            // Position the wall obstacle
            level2_obstacles[0].x = canvas_x + canvas_width/2 - 30;
            level2_obstacles[0].y = canvas_y + canvas_height/2 - 60;
            break;
            
        case 3: // Maze Runner - multiple obstacles
            current_level->player1_start_x = canvas_x + 30;
            current_level->player1_start_y = canvas_y + 50;
            current_level->player2_start_x = canvas_x + 30;
            current_level->player2_start_y = canvas_y + 100;
            current_level->ball_start_x = canvas_x + 150;
            current_level->ball_start_y = canvas_y + 75;
            current_level->hole_x = canvas_x + canvas_width - 50;
            current_level->hole_y = canvas_y + canvas_height - 50;
            
            // Position the maze obstacles
            level3_obstacles[0].x = canvas_x + canvas_width/4;
            level3_obstacles[0].y = canvas_y + 100;
            level3_obstacles[1].x = canvas_x + canvas_width/2;
            level3_obstacles[1].y = canvas_y + canvas_height - 120;
            level3_obstacles[2].x = canvas_x + canvas_width*3/4 - 20;
            level3_obstacles[2].y = canvas_y + 80;
            break;
    }
    
    // Initialize game objects with level positions
    init_player(&player1, current_level->player1_start_x, current_level->player1_start_y, 20, COLOR_WHITE);
    init_player(&player2, current_level->player2_start_x, current_level->player2_start_y, 20, COLOR_BLUE);
    init_ball(&game_ball, current_level->ball_start_x, current_level->ball_start_y, 8, COLOR_RED);
    init_hole(&game_hole, current_level->hole_x, current_level->hole_y, current_level->hole_radius, COLOR_BLACK);
}

// Draw all obstacles for the current level
void draw_obstacles(Level* level) {
    if (!level || !level->obstacles) {
        return;
    }
    
    for (int i = 0; i < level->obstacle_count; i++) {
        draw_obstacle(&level->obstacles[i]);
    }
}

// Show level completion screen
void show_level_complete_screen(void) {
    uint64_t screen_width = sys_get_screen_width();
    uint64_t screen_height = sys_get_screen_height();
    uint64_t center_x = screen_width / 2;
    
    sys_clear_screen(MENU_BG_COLOR);
    
    // Level completed title
    char title[50];
    strcpy(title, "Level ");
    char level_str[10];
    itoa(current_level_number, level_str);
    strcat(title, level_str);
    strcat(title, " Complete!");
    
    uint64_t title_x = center_x - (strlen(title) * 3 * 8) / 2;
    uint64_t title_y = screen_height / 5;
    sys_put_text(title, strlen(title), COLOR_GOLD, title_x, title_y, 3);
    
    // Level name
    uint64_t name_x = center_x - (strlen(current_level->level_name) * 2 * 8) / 2;
    uint64_t name_y = (screen_height * 30) / 100;
    sys_put_text(current_level->level_name, strlen(current_level->level_name), COLOR_GREEN, name_x, name_y, 2);
    
    // Hits info
    char hits_info[50];
    strcpy(hits_info, "Completed in ");
    char hits_str[10];
    itoa(hit_count, hits_str);
    strcat(hits_info, hits_str);
    strcat(hits_info, " hits");
    
    uint64_t hits_x = center_x - (strlen(hits_info) * 1 * 8) / 2;
    uint64_t hits_y = (screen_height * 45) / 100;
    sys_put_text(hits_info, strlen(hits_info), COLOR_WHITE, hits_x, hits_y, 1);
    
    // Total score
    char score_info[50];
    strcpy(score_info, "Total Score: ");
    itoa(total_score, hits_str);
    strcat(score_info, hits_str);
    
    uint64_t score_x = center_x - (strlen(score_info) * 1 * 8) / 2;
    uint64_t score_y = (screen_height * 55) / 100;
    sys_put_text(score_info, strlen(score_info), COLOR_WHITE, score_x, score_y, 1);
    
    // Instructions
    char* instruction;
    if (is_last_level()) {
        instruction = "Press any key to see final results";
    } else {
        instruction = "Press any key to continue to next level";
    }
    
      uint64_t inst_x = center_x - (strlen(instruction) * 1 * 8) / 2;
  uint64_t inst_y = (screen_height * 75) / 100;
  sys_put_text(instruction, strlen(instruction), COLOR_WHITE, inst_x, inst_y, 1);
  
  // Render everything to screen
  sys_render_screen();
}

// Check if this is the last level
int is_last_level(void) {
    return current_level_number >= MAX_LEVELS;
}

// Advance to the next level
void advance_to_next_level(void) {
    if (!is_last_level()) {
        total_score += hit_count;
        levels_completed++;
        load_level(current_level_number + 1);
        
        // Reset game state for new level
        game_running = 1;
        game_completed = 0;
        hit_count = 0;
        if (is_multiplayer) {
            current_player_turn = 1;  // Player 1 starts each level
            player1_hits = 0;
            player2_hits = 0;
            last_ball_toucher = 0;  // Reset last toucher for new level
        }
  }
}

// Unified game loop that handles both singleplayer and multiplayer
void run_game_loop() {
  int tick = sys_get_ticks();
  int last_ball_moving = 0;  // Track previous ball movement state to detect transitions
  int first_frame = 1;       // Flag to ensure UI is shown on first frame
  
  while (game_running && !game_completed) {
    // Increment frame counter for movement timing
    frame_counter++;
    
    // Handle multi-key movement
    handle_movement();
    
    // Render frame after movement updates
    sys_render_screen();
    
    // Update ball physics
    if (sys_get_ticks() - tick >= 1) {
      update_ball(&game_ball, 0);
      tick = sys_get_ticks();
      
      // Check if ball movement state changed
      int ball_moving = is_ball_moving(&game_ball);
      
      if (is_multiplayer) {
        // Multiplayer: Switch turns when ball transitions from moving to stopped
      if (last_ball_moving && !ball_moving) {
          switch_player_turn();
        } else if (last_ball_moving != ball_moving || first_frame) {
          // Update UI when ball status changes or on first frame
          show_both_players_info();
        }
      } else {
        // Singleplayer: Update UI when ball status changes or on first frame
        if (last_ball_moving != ball_moving || first_frame) {
          show_single_player_info();
        }
      }
      
      last_ball_moving = ball_moving;
      first_frame = 0;  // Clear first frame flag
    }

    // Wait a bit to control game speed
    for (volatile int i = 0; i < 50000; i++);
  } 
}