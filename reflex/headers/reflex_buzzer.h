/*
 * reflex_buzzer.h
 * Header with all song playing related functions including
 * timer interrupts handlers, DAC initalization, speaker
 *
 *  Created on: 22-12-2016
 *      Author: embedded
 */

#ifndef REFLEX_BUZZER_H_
#define REFLEX_BUZZER_H_

#include "LPC17xx.h"
#include "lpc_types.h"

/**
 * IRQ for GPIO buzzer intterrupts
 */
void TIMER1_IRQHandler(void);

/**
 * IRQ for DAC buzzer innterrupts
 */
void TIMER3_IRQHandler(void);

/**
 * Play song using GPIO with innterrupts
 * @param song 			array of notes to play
 */
void play_songGPIO(const char *song);

/**
 * Play song using DAC with innterrupts
 * @param song 			8bit WAV array of tones
 * @param songSize 		size of array of tones
 * @param loop 			loop flag, set 1 to loop song
 */
void play_songDAC(const unsigned char *song, const int songSize, uint8_t loop);

/**
 * Initialize buzzer
 */
void speaker_init(void);

/**
 * Initalize DAC
 */
void dac_init(void);

/**
 * Get note form GPIO song array
 * @param 				ch char from song array
 * @return 				1000000us/noteFrequency
 */
uint32_t get_note(uint8_t ch);

/**
 * Get note form GPIO song array
 * @param ch 			char from song array
 * @return 				ms of pause
 */
uint32_t get_pause(uint8_t ch);

/**
 * Get note form GPIO song array
 * @param ch 			char from song array
 * @return 				ms of duration
 */
uint32_t get_duration(uint8_t ch);

#endif /* REFLEX_BUZZER_H_ */
