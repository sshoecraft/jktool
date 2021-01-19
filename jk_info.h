
#ifndef __JKINFO_H
#define __JKINFO_H

#include <stdint.h>
#include "jk.h"

struct jk_info {
	char model[32];
	char hwvers[16];
	char swvers[16];
	char uptime[24];
	char device[32];
	char pin[32];
	char num1[32];
	char num2[32];
	char pass[32];
	float voltage;
	float current;
	unsigned short protectbits;
	unsigned short state;
	unsigned char strings;			/* the number of battery strings */
	float cellvolt[32];			/* Cell voltages */
	float cellres[32];			/* Cell resistances */
	float cell_total;			/* sum of all cells */
	float cell_min;				/* lowest cell value */
	float cell_max;				/* highest cell value */
	float cell_diff;			/* diff from lowest to highest */
	float cell_avg;				/* avergae cell value */
	unsigned char probes;			/* the number of NTC (temp) probes */
	float temps[6];				/* Temps */
};
typedef struct jk_info jk_info_t;

#endif
