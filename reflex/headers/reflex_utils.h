/*
 * reflex_timers.h
 * Header with all utility functions used to run the program, initalizing timers and buses
 *
 *  Created on: 22-12-2016
 *      Author: embedded
 */

#ifndef REFLEX_UTILS_H_
#define REFLEX_UTILS_H_

#include "LPC17xx.h"
#include "lpc_types.h"

/**
 * Initalize and start timer0
 * prescaled to 1ms, no interrupts
 * used for game time
 */
void timer0_start(void);

/**
 * Initalize and start timer1
 * prescaled to 1us, interrupt on match enabled
 * used for gpio songs for buzzer
 */
void timer1_start(void);

/**
 * Wait function
 * @param time 			TC value to stop waiting
 * @param intTimeUs 	us to increment TC
 */
void timer2_wait(uint32_t time, uint32_t intTimeUs);

/**
 * Initalize and start timer3
 * prescaled to 1us, interrupt on match enabled
 * interrupt frequency is 8khz when playing song
 * used for DAC songs for buzzer
 */
void timer3_start(void);

/**
 * Initialize SSP pins
 */
void ssp_init(void);

/**
 * Initlialize I2C pins
 */
void i2c_init(void);

// +function oled_clearPages in oled.h

#endif /* REFLEX_UTILS_H_ */
