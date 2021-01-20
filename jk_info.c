
/*
Copyright (c) 2021, Stephen P. Shoecraft
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#include <unistd.h>
#include <stdint.h>
#include "jk_info.h"
#include "debug.h"
#include "utils.h"

#define _getshort(p) (short)(*(p) | (*((p)+1) << 8))
#define _getint(p) (int)(*(p) | (*((p)+1) << 8) | (*((p)+2) << 16) | (*((p)+3) << 24))

static void _getvolts(jk_info_t *info, uint8_t *data) {
	int i,j;

	if (debug >= 5) bindump("getvolts",data,300);

	i = 6;
	for(j=0; j < 24; j++) {
		info->cellvolt[j] = _getshort(&data[i]) / 1000.0;
		if (!info->cellvolt[j]) break;
		dprintf(1,"cellvolt[%02d] = data[%02d] = %.3f\n", j, i, (_getshort(&data[i]) / 1000.0));
		i += 2;
	}
	i = 64;
	for(j=0; j < 24; j++) {
		info->cellres[j] = _getshort(&data[i]) / 1000.0;
		if (!info->cellres[j]) break;
		dprintf(1,"cellres[%02d] = data[%02d] = %.3f\n", j, i, (_getshort(&data[i]) / 1000.0));
		i += 2;
	}
	info->strings = j;
	dprintf(1,"srings: %d\n", info->strings);
	info->voltage = ((unsigned short)_getshort(&data[118])) / 1000.0;
	dprintf(1,"voltage: %.2f\n", info->voltage);
	info->current = _getshort(&data[126]) / 1000.0;
	dprintf(1,"current: %.2f\n", info->current);
	info->probes = 3;
	dprintf(1,"probes: %d\n", info->probes);
	info->temps[0] = ((unsigned short)_getshort(&data[130])) / 10.0;
	dprintf(1,"temp[0]: %.1f\n", info->temps[0]);
	info->temps[1] = ((unsigned short)_getshort(&data[132])) / 10.0;
	dprintf(1,"temp[1]: %.1f\n", info->temps[1]);
	info->temps[2] = ((unsigned short)_getshort(&data[134])) / 10.0;
	dprintf(1,"temp[2]: %.1f\n", info->temps[2]);
}

static void _getinfo(jk_info_t *info, uint8_t *data) {
	int i,j,uptime,unk;

	if (debug >= 5) bindump("info",data,300);

	j=0;
	/* Model */
	for(i=6; i < 300 && data[i]; i++) {
		info->model[j++] = data[i];
		if (j >= sizeof(info->model)-1) break;
	}
	info->model[j] = 0;
	dprintf(1,"Model: %s\n", info->model);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	/* HWVers */
	dprintf(1,"i: %x\n", i);
	j=0;
	while(i < 300 && data[i]) {
		info->hwvers[j++] = data[i++];
		if (j >= sizeof(info->hwvers)-1) break;
	}
	info->hwvers[j] = 0;
	dprintf(1,"HWVers: %s\n", info->hwvers);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	/* SWVers */
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->swvers[j++] = data[i++];
		if (j >= sizeof(info->swvers)-1) break;
	}
	info->swvers[j] = 0;
	dprintf(1,"SWVers: %s\n", info->swvers);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	dprintf(1,"i: %x\n", i);
	uptime = _getint(&data[i]);
	dprintf(1,"uptime: %04x\n", uptime);
	i += 4;
	unk = _getint(&data[i]);
	dprintf(1,"unk: %04x\n", unk);
	i += 4;
	/* Device */
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->device[j++] = data[i++];
		if (j >= sizeof(info->device)-1) break;
	}
	info->device[j] = 0;
	dprintf(1,"Device: %s\n", info->device);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	dprintf(1,"i: %x\n", i);
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->pin[j++] = data[i++];
		if (j >= sizeof(info->pin)-1) break;
	}
	info->pin[j] = 0;
	dprintf(1,"Pin: %s\n", info->pin);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	dprintf(1,"i: %x\n", i);
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->num1[j++] = data[i++];
		if (j >= sizeof(info->num1)-1) break;
	}
	info->num1[j] = 0;
	dprintf(1,"Number: %s\n", info->num1);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	dprintf(1,"i: %x\n", i);
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->num2[j++] = data[i++];
		if (j >= sizeof(info->num2)-1) break;
	}
	info->num2[j] = 0;
	dprintf(1,"Another Number: %s\n", info->num2);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	dprintf(1,"i: %x\n", i);
	/* LOL skip "Input User data */
	while(i < 300 && data[i]) i++;
	dprintf(1,"i: %x\n", i);
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	dprintf(1,"i: %x\n", i);
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->pass[j++] = data[i++];
		if (j >= sizeof(info->pass)-1) break;
	}
	info->pass[j] = 0;
	dprintf(1,"passcode: %s\n", info->pass);
}

#define GOT_RES 0x01
#define GOT_VOLT 0x02
#define GOT_INFO 0x04

static int getdata(jk_info_t *info, uint8_t *data, int bytes) {
	uint8_t sig[] = { 0x55,0xAA,0xEB,0x90 };
	int i,j,start,r;

	r = 0;
//	bindump("data",data,bytes);
	for(i=j=0; i < bytes; i++) {
		dprintf(4,"data[%d]: %02x, sig[%d]: %02x\n", i, data[i], j, sig[j]);
		if (data[i] == sig[j]) {
			if (j == 0) start = i;
			j++;
			if (j >= sizeof(sig) && (start + 300) < bytes) {
				dprintf(1,"found sig\n");
				if (data[i+1] == 1)  {
//					_getres(info,&data[start]);
					r |= GOT_RES;
				} else if (data[i+1] == 2) {
					_getvolts(info,&data[start]);
					r |= GOT_VOLT;
				} else if (data[i+1] == 3) {
					_getinfo(info,&data[start]);
					r |= GOT_INFO;
				}
				i += 300;
				j = 0;
			}
		}
		if (r & GOT_VOLT) break;
	}
	dprintf(4,"returning: %d\n", r);
	return r;
}

int jk_get_info(jk_session_t *s, jk_info_t *info) {
	unsigned char getInfo[] =     { 0xaa,0x55,0x90,0xeb,0x97,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11 };
	unsigned char getCellInfo[] = { 0xaa,0x55,0x90,0xeb,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10 };
	uint8_t data[2048];
	int bytes,r,retries;

	/* Have to getInfo before can getCellInfo ... */
	retries=5;
	while(retries--) {
		bytes = s->tp->write(s->tp_handle,getInfo,sizeof(getInfo));
		dprintf(4,"bytes: %d\n", bytes);
		if (bytes < 0) return -1;
		bytes = s->tp->read(s->tp_handle,data,sizeof(data));
		dprintf(4,"bytes: %d\n", bytes);
		if (bytes < 0) return -1;
		r = getdata(info,data,bytes);
		if (r & GOT_INFO) break;
		sleep(1);
	}
	retries=5;
	bytes = s->tp->write(s->tp_handle,getCellInfo,sizeof(getCellInfo));
	dprintf(4,"bytes: %d\n", bytes);
	if (bytes < 0) return -1;
	while(retries--) {
		bytes = s->tp->read(s->tp_handle,data,sizeof(data));
		dprintf(4,"bytes: %d\n", bytes);
		r = getdata(info,data,bytes);
		if (r & GOT_VOLT) break;
	}
	return (retries < 1 ? -1 : 0);
}
