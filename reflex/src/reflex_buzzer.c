/*
 * reflex_buzzer.c
 *
 *  Created on: 22-12-2016
 *      Author: embedded
 */

#include "reflex_buzzer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);

// notes peroid in us
uint32_t notes[] = { 2272, // A - 440 Hz
		2024, // B - 494 Hz
		3816, // C - 262 Hz
		3401, // D - 294 Hz
		3030, // E - 330 Hz
		2865, // F - 349 Hz
		2551, // G - 392 Hz
		1136, // a - 880 Hz
		1012, // b - 988 Hz
		1912, // c - 523 Hz
		1703, // d - 587 Hz
		1517, // e - 659 Hz
		1432, // f - 698 Hz
		1275, // g - 784 Hz
		};

// globals for timer1 irq handler (gpio sound)
uint8_t G_testFlag = 0;
uint32_t G_timePrev = 0;
uint32_t G_time = 0;
uint32_t G_dt = 0;
uint32_t G_elapsedDur = 0;
uint32_t G_note = 0;
uint32_t G_dur = 0;
uint32_t G_pause = 0;
volatile uint8_t *songGPIOptr = 0;

// globals for timer3 irq handler (DAC sound)
int soundWavSize = 0;
uint8_t loopFlag = 0;
volatile uint32_t cntWav = 0;
volatile const unsigned char *songDACptr = 0;

void TIMER1_IRQHandler(void) {
	uint32_t irFlags;
	PINSEL_CFG_Type PinCfg;

	G_testFlag++;

	/*
	 * "Writing a logic one to the corresponding IR bit will reset the
	 * interrupt. Writing a zero has no effect."
	 */
	irFlags = LPC_TIM1->IR;
	LPC_TIM1->IR = irFlags;

	if (G_elapsedDur == 0) {
		if (*songGPIOptr != '\0' && songGPIOptr != 0) {
			G_note = get_note(*songGPIOptr++);
			if (*songGPIOptr == '\0') {
				goto end;
			}
			G_dur = get_duration(*songGPIOptr++);
			if (*songGPIOptr == '\0') {
				goto end;
			}
			G_pause = get_pause(*songGPIOptr++);
			LPC_TIM1->MR0 = G_note / 2;
		} else {
			end: LPC_TIM1->MR0 = 50000;
			TIM_ResetCounter(LPC_TIM1);
			return;
		}

	}

	G_timePrev = G_time;
	G_time = LPC_TIM0->TC;
	G_dt = G_time - G_timePrev;
	G_elapsedDur += G_dt;

	// GPIO on P0.26
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	// tone
	if (G_testFlag == 1) {
		NOTE_PIN_HIGH();
	} else {
		NOTE_PIN_LOW();
		G_testFlag = 0;
	}

	// GO BACK TO AOUT
	PinCfg.Funcnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	if (G_elapsedDur >= G_dur) {
		G_elapsedDur = 0;
	}

	TIM_ResetCounter(LPC_TIM1);
}

void TIMER3_IRQHandler(void) {
	uint32_t irFlags;

	/*
	 * "Writing a logic one to the corresponding IR bit will reset the
	 * interrupt. Writing a zero has no effect."
	 */
	irFlags = LPC_TIM3->IR;
	LPC_TIM3->IR = irFlags;

	if (songDACptr == 0) {
		return;
	}

	DAC_UpdateValue(LPC_DAC, (uint32_t)(*songDACptr));

	cntWav++;
	songDACptr++;

	if (cntWav >= soundWavSize && loopFlag) {
		songDACptr -= cntWav;
		cntWav = 0;
	} else if (cntWav >= soundWavSize) {
		songDACptr = 0;
		cntWav = 0;
		return;
	}

	TIM_ResetCounter(LPC_TIM3);
}

void play_songGPIO(const char *song) {
	songGPIOptr = (uint8_t*) song;
}

void play_songDAC(const unsigned char *song, const int songSize, uint8_t loop) {
	loopFlag = loop;
	songDACptr = song;
	soundWavSize = songSize;
	TIM_ResetCounter(LPC_TIM3);
}

void speaker_init(void) {
	GPIO_SetDir(2, 1 << 0, 1);
	GPIO_SetDir(2, 1 << 1, 1);

	GPIO_SetDir(0, 1 << 27, 1);
	GPIO_SetDir(0, 1 << 28, 1);
	GPIO_SetDir(2, 1 << 13, 1);
	GPIO_SetDir(0, 1 << 26, 1);

	GPIO_ClearValue(0, 1 << 27); // LM4811-clk
	GPIO_ClearValue(0, 1 << 28); // LM4811-up/dn
	GPIO_ClearValue(2, 1 << 13); // LM4811-shutdn

	GPIO_SetValue(2, (1 << 0));
}

void dac_init(void) {
	PINSEL_CFG_Type PinCfg;

	/*
	 * Init DAC pin connect
	 * AOUT on P0.26
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/* init DAC structure to default
	 * Maximum	current is 700 uA
	 * First value to AOUT is 0
	 */
	DAC_Init(LPC_DAC);
}

uint32_t get_note(uint8_t ch) {
	if (ch >= 'A' && ch <= 'G') {
		return notes[ch - 'A'];
	}

	if (ch >= 'a' && ch <= 'g') {
		return notes[ch - 'a' + 7];
	}

	return 0;
}

uint32_t get_duration(uint8_t ch) {
	if (ch < '0' || ch > '9') {
		return 400;
	}

	if (ch == '0')
		return 100;

	// number of ms
	return (ch - '0') * 200;
}

uint32_t get_pause(uint8_t ch) {
	switch (ch) {
	case '+':
		return 0;
	case ',':
		return 5;
	case '.':
		return 20;
	case '_':
		return 30;
	default:
		return 5;
	}
}
