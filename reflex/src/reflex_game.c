/*
 * reflex_game.c
 *
 *  Created on: 22-12-2016
 *      Author: embedded
 */

#include "reflex_game.h"
#include <stdlib.h>
#include "oled.h"
#include "acc.h"
#include "joystick.h"

uint8_t bufStr[10];

uint8_t acc_input(struct Acc *acc) {
	acc_read(&acc->x, &acc->y, &acc->z);
	acc->x = acc->x + acc->xOff;
	acc->y = acc->y + acc->yOff;
	acc->z = acc->z + acc->zOff;

	if (acc->x * acc->y * acc->z > 120000 || acc->x * acc->y * acc->z < -120000) {
		return 1;
	}

	return 0;
}

void set_rand_dir(struct BlockDir *dir) {
	uint8_t newDir = rand() % 4;
	dir->checkFlag = 0;

	switch (newDir) {
	case 0:
		dir->model = (char*) "U";
		break;
	case 1:
		dir->model = (char*) "D";
		break;
	case 2:
		dir->model = (char*) "L";
		break;
	case 3:
		dir->model = (char*) "R";
		break;
	}
}

void handle_cross(struct BlockDir dirs[]) {
	uint8_t i;
	for (i = 0; i < MAX_DIRS; i++) {
		if (dirs[i].x <= 1) {
			// TODO:
			uint8_t lastDir = floor_mod(i - 1, MAX_DIRS);
			set_rand_dir(&dirs[i]);
			dirs[i].x = dirs[lastDir].x + DISTANCE;
		}
	}
}

uint8_t floor_mod(int8_t num, uint8_t mod) {
	return ((num % mod) + mod) % mod;
}

void update_dirs(struct BlockDir dirs[], uint32_t dt) {
	uint8_t i;
	for (i = 0; i < MAX_DIRS; i++) {
		dirs[i].x -= dt * SPEED;
		handle_cross(dirs);
	}
}

void init_dirs(struct BlockDir dirs[]) {
	uint8_t i;
	for (i = 0; i < MAX_DIRS; i++) {
		set_rand_dir(&dirs[i]);
		dirs[i].x = OLED_DISPLAY_WIDTH + i * DISTANCE;
		dirs[i].y = 40;
		dirs[i].checkFlag = 0;
	}
}

uint8_t handle_input(struct BlockDir dirs[], uint8_t joyState, uint8_t *score) {
	uint8_t flag;

	if ((joyState & JOYSTICK_UP) != 0) {
		flag = check_dirs(dirs, "U");
		if (flag == 1) {
			(*score)++;
		}
		if (flag > 0) {
			return 1;
		}
	}

	if ((joyState & JOYSTICK_DOWN) != 0) {
		flag = check_dirs(dirs, "D");
		if (flag == 1) {
			(*score)++;
		}
		if (flag > 0) {
			return 1;
		}
	}

	if ((joyState & JOYSTICK_LEFT) != 0) {
		flag = check_dirs(dirs, "L");
		if (flag == 1) {
			(*score)++;
		}
		if (flag > 0) {
			return 1;
		}
	}

	if ((joyState & JOYSTICK_RIGHT) != 0) {
		flag = check_dirs(dirs, "R");
		if (flag == 1) {
			(*score)++;
		}
		if (flag > 0) {
			return 1;
		}
	}

	return 0;
}

void handle_no_input(struct BlockDir dirs[], uint8_t *score) {
	uint8_t i;
	for (i = 0; i < MAX_DIRS; i++) {
		if (dirs[i].x < 44 && dirs[i].checkFlag == 0) {
			if (*score > 0) {
				(*score)--;
			}
			dirs[i].checkFlag = 1;
		}
	}
}

uint8_t check_dirs(struct BlockDir dirs[], char *dir) {
	uint8_t i;
	for (i = 0; i < MAX_DIRS; i++) {
		if (dirs[i].x > 44 && dirs[i].x < 51 && dirs[i].model == dir) {
			dirs[i].checkFlag += 1;
			if (dirs[i].checkFlag == 1) {
				return 1;
			}
			return 2;
		}
	}
	return 0;
}

void draw_dirs(struct BlockDir dirs[]) {
	oled_clearPages(OLED_COLOR_BLACK, 0xB5, 0xB6);
	uint8_t i;
	for (i = 0; i < MAX_DIRS; i++) {
		oled_putString(dirs[i].x, dirs[i].y, (uint8_t*) dirs[i].model,
				OLED_COLOR_WHITE, OLED_COLOR_BLACK);
	}
}

void draw_oled(uint32_t time, uint8_t *bufE, uint8_t *bufStr, uint8_t score) {
	oled_clearPages(OLED_COLOR_BLACK, 0xB0, 0xB1);

	// timer
	int_to_string(time, bufStr, 10, 10);
	oled_putString(1, 1, bufStr, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	// eeprom data
	int_to_string(bufE[0], bufStr, 10, 10);
	oled_putString(40, 1, bufStr, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	// current score
	int_to_string(score, bufStr, 10, 10);
	oled_putString(80, 1, bufStr, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

	// pointer where to click
	oled_putString(45, 50, (uint8_t*) "^", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
}

void int_to_string(int value, uint8_t* pBuf, uint32_t len, uint32_t base) {
	static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
	int pos = 0;
	int tmpValue = value;

	// the buffer must not be null and at least have a length of 2 to handle one
	// digit and null-terminator
	if (pBuf == NULL || len < 2) {
		return;
	}

	// a valid base cannot be less than 2 or larger than 36
	// a base value of 2 means binary representation. A value of 1 would mean only zeros
	// a base larger than 36 can only be used if a larger alphabet were used.
	if (base < 2 || base > 36) {
		return;
	}

	// negative value
	if (value < 0) {
		tmpValue = -tmpValue;
		value = -value;
		pBuf[pos++] = '-';
	}

	// calculate the required length of the buffer
	do {
		pos++;
		tmpValue /= base;
	} while (tmpValue > 0);

	if (pos > len) {
		// the len parameter is invalid.
		return;
	}

	pBuf[pos] = '\0';

	do {
		pBuf[--pos] = pAscii[value % base];
		value /= base;
	} while (value > 0);

	return;
}
