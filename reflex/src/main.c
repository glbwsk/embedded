/**
 * Reflex
 * A07
 * Lukasz Golebiewski 203882
 * Jakub Mielczarek 203943
 * Barbara Kajkowska 203898
 */

#include "LPC17xx.h"
#include "lpc_types.h"

#include "light.h"
#include "oled.h"
#include "eeprom.h"
#include "string.h"
#include "acc.h"
#include "joystick.h"

#include "reflex_buzzer.h"
#include "reflex_utils.h"
#include "reflex_game.h"

extern const char *songsGPIO[];
extern const unsigned char songWavBg[];
extern const int songWavBgSize;

int main(void) {
	// hardware init
	i2c_init();
	ssp_init();
	eeprom_init();
	oled_init();
	speaker_init();
	joystick_init();
	acc_init();
	dac_init();
	light_init();
	light_enable();
	light_setRange(LIGHT_RANGE_4000);
	oled_clearScreen(OLED_COLOR_BLACK);

	// EEPROM
	const uint16_t offset = 240;
	const uint8_t eWriteLen = 200;
	uint8_t bufE[eWriteLen];
	memset(bufE, 0, eWriteLen);
	//eeprom_write(0, offset, eWriteLen);
	eeprom_read(bufE, offset, eWriteLen);
	uint8_t bestScore = bufE[0];

	//joystick & acc
	struct Acc acc;
	uint8_t state = 0;
	acc_read(&acc.x, &acc.y, &acc.z);
	acc.xOff = 0 - acc.x;
	acc.yOff = 0 - acc.y;
	acc.zOff = 64 - acc.z;

	// timer0 for game time
	timer0_start();
	// timer1 for speaker tones irq
	timer1_start();
	// timer3 for speaker wav irq
	timer3_start();

	// game
	struct BlockDir dirs[MAX_DIRS];
	init_dirs(dirs);
	uint8_t bufStr[10];
	uint8_t score = 0;
	play_songDAC(songWavBg, songWavBgSize, 0);
	play_songGPIO(songsGPIO[0]);

	// time variables
	uint32_t time = 0;
	uint32_t timePrev = 0;
	uint32_t dt = 0;
	uint32_t elapsedTimeDirs = 0;
	uint32_t elapsedTimeInfo = 0;

	// main loop
	while (1) {
		timePrev = time;
		time = LPC_TIM0->TC;
		dt = time - timePrev;
		elapsedTimeDirs += dt;
		elapsedTimeInfo += dt;

		state = joystick_read();
		if (state != 0) {
			if (handle_input(dirs, state, &score) == 0) {
				if (bestScore < score) {
					bestScore = score;
					bufE[0] = (uint8_t) bestScore;
					play_songDAC(songWavBg, songWavBgSize, 0);
					eeprom_write(bufE, offset, eWriteLen);
				}
				score = 0;
				play_songGPIO(songsGPIO[3]);
				init_dirs(dirs);
			} else {
				play_songGPIO(songsGPIO[2]);
			}
		}

		handle_no_input(dirs, &score);

		if (acc_input(&acc) == 1) {
			score = 0;
			init_dirs(dirs);
		}

		update_dirs(dirs, dt);

		if (elapsedTimeDirs > 50) {
			draw_dirs(dirs);
			elapsedTimeDirs -= 50;
		}

		if (elapsedTimeInfo > 500) {
			draw_oled(time / 1000, bufE, bufStr, score);
			elapsedTimeInfo -= 500;
		}

	}
}

