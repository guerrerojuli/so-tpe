#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "ball.h"
#include "player.h"
#include "hole.h"
#include "obstacle.h"

// Scancodes for WASD and Q keys
#define SCANCODE_Q 16
#define SCANCODE_W 17
#define SCANCODE_A 30
#define SCANCODE_S 31
#define SCANCODE_D 32

// Scancodes for arrow keys (Player 2)
#define SCANCODE_UP_ARROW    72
#define SCANCODE_DOWN_ARROW  80
#define SCANCODE_LEFT_ARROW  75
#define SCANCODE_RIGHT_ARROW 77

// Physics constants - keep incoming branch values for obstacles
#define FRICTION_FACTOR 2
#define COLLISION_FORCE 3
#define MIN_VELOCITY 1

// Level system constants
#define MAX_LEVELS 3

// Level structure
typedef struct {
    int level_number;
    char* level_name;
    
    // Starting positions
    uint64_t player1_start_x, player1_start_y;
    uint64_t player2_start_x, player2_start_y;
    uint64_t ball_start_x, ball_start_y;
    
    // Hole configuration
    uint64_t hole_x, hole_y, hole_radius;
    
    // Obstacles
    Obstacle* obstacles;
    int obstacle_count;
    
    // Difficulty
    int max_strokes;
} Level;

// Game state variables
extern uint64_t canvas_color;
extern uint64_t canvas_x, canvas_y, canvas_width, canvas_height;
extern int game_running;
extern int hit_count;
extern int game_completed;

// Level system variables
extern Level* current_level;
extern int current_level_number;
extern int total_score;
extern int levels_completed;

// Multiplayer variables
extern int is_multiplayer;
extern int current_player_turn;  // 1 = Player1's turn, 2 = Player2's turn
extern int player1_hits;
extern int player2_hits;
extern int last_ball_toucher;  // 1 or 2, who touched the ball last
extern int player1_wins;       // Count of levels won by player 1
extern int player2_wins;       // Count of levels won by player 2

// Game instances
extern Player player1;
extern Player player2;
extern Ball game_ball;
extern Hole game_hole;

// Game control functions - Player 1 (WASD)
void move_player1_up(void);
void move_player1_down(void);
void move_player1_left(void);
void move_player1_right(void);

// Game control functions - Player 2 (Arrows)
void move_player2_up(void);
void move_player2_down(void);
void move_player2_left(void);
void move_player2_right(void);

void quit_game(void);

// Multiplayer functions
void show_both_players_info(void);
void show_single_player_info(void);
void switch_player_turn(void);

// Level system functions
void load_level(int level_num);
void draw_obstacles(Level* level);
void show_level_complete_screen(void);
void advance_to_next_level(void);
int is_last_level(void);

// Game flow functions
void start_singleplayer(void);
void start_multiplayer(void);
void show_menu(void);
void show_victory_screen(void);
void play_victory_sound(void);

// Utility functions
void user_delay(uint64_t milliseconds);
int are_any_balls_moving(void);
void run_game_loop(void);

#endif