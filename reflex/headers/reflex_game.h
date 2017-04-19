/*
 * reflex_struct.h
 * Header with all game logic related functions
 *
 *  Created on: 22-12-2016
 *      Author: embedded
 */

#ifndef REFLEX_GAME_H_
#define REFLEX_GAME_H_

#include "LPC17xx.h"
#include "lpc_types.h"

// size of dirs array
#define MAX_DIRS 5
// distance between dirs in pixels
#define DISTANCE 25
// dirs speed
#define SPEED 0.025

/**
 * struct for single moving char
 */
struct BlockDir {
	char *model;
	float x;
	uint8_t y;
	// flag checking if point was given
	uint16_t checkFlag;
};

/**
 * struct for accelerometer state
 */
struct Acc {
	int8_t x;
	int8_t y;
	int8_t z;
	int32_t xOff;
	int32_t yOff;
	int32_t zOff;
};

/**
 * Initialize values of dirs array
 */
void init_dirs(struct BlockDir dirs[]);

/**
 * Set random char (R, L, U, D)
 */
void set_rand_dir(struct BlockDir *dir);

/**
 * Handle screen crossing of dirs array - move them to begining
 */
void handle_cross(struct BlockDir dirs[]);

/**
 * Put dirs array on the screen
 */
void draw_dirs(struct BlockDir dirs[]);

/**
 * Decrement score if dir was ignored by player
 * @param dirs			dirs array
 * @param score 		current score
 */
void handle_no_input(struct BlockDir dirs[], uint8_t *score);

/**
 * Put game info on the screen
 * @param time 			elapsed game time
 * @param bufE 			buffor with data from eeprom with best score
 * @param bufStr 		buffor for int2str algorithm
 * @param score 		current score
 */
void draw_oled(uint32_t time, uint8_t *bufE, uint8_t *bufStr, uint8_t score);

/**
 * Update dirs array position
 * @param dirs			dirs array
 * @param dt 			delta time since last update
 */
void update_dirs(struct BlockDir dirs[], uint32_t dt);

/**
 * Handles input for proper incrementing the score
 * @param dirs			dirs array
 * @param joyState 		joystic state
 * @param score 		current score
 */
uint8_t handle_input(struct BlockDir dirs[], uint8_t joyState, uint8_t *score);

/**
 * Function used in handle_input to check if dir is proper for joystick input
 * @param dirs 			dirs array
 * @param dir 			char (U,D,R,L)
 * @retun 0 - 			if not correct inpuT,
 * 		  1 - 			if input is correct,
 * 		  2 - 			if correct second time and more
 */
uint8_t check_dirs(struct BlockDir dirs[], char *dir);

/**
 * Calculate floor modulo
 * @return floor modulo from num%mod
 */
uint8_t floor_mod(int8_t num, uint8_t mod);

/**
 * Read input form accelerometer
 * @param acc		accelermoeter state struct
 * @return 1 - 		if board was shaked strongly
 * 		   0 -		other cases
 */
uint8_t acc_input(struct Acc *acc);

/**
 * Algorithm for conterverting int to ASCII
 * @param value 	int to convert
 * @param pBuf 		buffor to get converted int
 * @param len 		length of number +1
 * @param base 		how many ascii chars to use, 2 means binary, 10 - decimal etc
 */
void int_to_string(int value, uint8_t* pBuf, uint32_t len, uint32_t base);

#endif /* REFLEX_GAME_H_ */
