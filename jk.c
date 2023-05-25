
/*
Copyright (c) 2021, Stephen P. Shoecraft
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#include "mybmm.h"
#include "jk.h"

#define _getshort(p) (short)(*(p) | (*((p)+1) << 8))
#define _getint(p) (int)(*(p) | (*((p)+1) << 8) | (*((p)+2) << 16) | (*((p)+3) << 24))

static int jk_init(mybmm_config_t *conf) {
	return 0;
}

static void *jk_new(mybmm_config_t *conf, ...) {
	jk_session_t *s;
	va_list ap;
	mybmm_pack_t *pp;
	mybmm_module_t *tp;

	va_start(ap,conf);
	pp = va_arg(ap,mybmm_pack_t *);
	tp = va_arg(ap,mybmm_module_t *);
	va_end(ap);

	dprintf(3,"transport: %s\n", pp->transport);
	s = calloc(1,sizeof(*s));
	if (!s) {
		perror("jk_new: malloc");
		return 0;
	}
	/* Save a copy of the pack */
	s->pp = pp;
	s->tp = tp;
	dprintf(1,"pp->target: %s, pp->opts: %s\n", pp->target, pp->opts);
	s->tp_handle = tp->new(conf,pp->target,pp->opts);
	if (!s->tp_handle) {
		free(s);
		return 0;
	}
	return s;
}

static int jk_open(void *handle) {
	jk_session_t *s = handle;
	return s->tp->open(s->tp_handle);
}

static void _getvolts(mybmm_pack_t *pp, uint8_t *data) {
	int i,j;

//	bindump("getvolts",data,300);
	i = 6;
	for(j=0; j < 24; j++) {
		pp->cellvolt[j] = _getshort(&data[i]) / 1000.0;
		if (!pp->cellvolt[j]) break;
		dprintf(4,"cellvolt[%02d] = data[%02d] = %.3f\n", j, i, (_getshort(&data[i]) / 1000.0));
		i += 2;
	}
	pp->cells = j;
	dprintf(4,"cells: %d\n", pp->cells);
	pp->voltage = ((unsigned short)_getshort(&data[118])) / 1000.0;
	dprintf(1,"voltage: %.2f\n", pp->voltage);
//	dprintf(1,"data[126]: %d %d %04x\n", _getshort(&data[126]),(unsigned short)_getshort(&data[126]),(unsigned short)_getshort(&data[126]));
	pp->current = _getshort(&data[126]) / 1000.0;
	dprintf(4,"current: %.2f\n", pp->current);
	pp->ntemps = 2;
	pp->temps[0] = ((unsigned short)_getshort(&data[130])) / 10.0;
	pp->temps[1] = ((unsigned short)_getshort(&data[132])) / 10.0;
	/* Dont include mosfet temp ... ? */
//	pp->temps[2] = ((unsigned short)_getshort(&data[134])) / 10.0;
}

#if 0
static void _getinfo(jk_info_t *info, uint8_t *data) {
	int i,j,uptime;

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
//	unk = _getshort(&data[i]);
//	i += 2;
	/* Skip 0s */
	while(i < 300 && !data[i]) i++;
	/* Device */
	j=0;
	dprintf(1,"i: %x\n", i);
	while(i < 300 && data[i]) {
		info->device[j++] = data[i++];
		if (j >= sizeof(info->device)-1) break;
	}
	info->device[j] = 0;
	dprintf(1,"Device: %s\n", info->device);
}
#endif

#define GOT_RES 0x01
#define GOT_VOLT 0x02
#define GOT_INFO 0x04

static int getdata(mybmm_pack_t *pp, uint8_t *data, int bytes) {
	uint8_t sig[] = { 0x55,0xAA,0xEB,0x90 };
	int i,j,start,r;

	r = 0;
//	bindump("data",data,bytes);
	for(i=j=0; i < bytes; i++) {
		dprintf(6,"data[%d]: %02x, sig[%d]: %02x\n", i, data[i], j, sig[j]);
		if (data[i] == sig[j]) {
			if (j == 0) start = i;
			j++;
			if (j >= sizeof(sig) && (start + 300) < bytes) {
				dprintf(1,"found sig\n");
				if (data[i+1] == 1)  {
//					_getres(pp,&data[start]);
					r |= GOT_RES;
				} else if (data[i+1] == 2) {
					_getvolts(pp,&data[start]);
					r |= GOT_VOLT;
				} else if (data[i+1] == 3) {
//					_getinfo(pp,&data[start]);
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

static int jk_read(void *handle,...) {
	jk_session_t *s = handle;

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
		r = getdata(s->pp,data,bytes);
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
		r = getdata(s->pp,data,bytes);
		if (r & GOT_VOLT) break;
	}
	return (retries < 1 ? -1 : 0);
}

static int jk_close(void *handle) {
	jk_session_t *s = handle;
	return s->tp->close(s->tp_handle);
}

static int jk_control(void *handle,...) {
//	jk_session_t *s = handle;
	va_list ap;
	int op,action;

	va_start(ap, handle);
	op = va_arg(ap,int);
	dprintf(1,"op: %d\n", op);
	switch(op) {
	case MYBMM_CHARGE_CONTROL:
		action = va_arg(ap,int);
		dprintf(1,"action: %d\n", action);
		break;
	}
	va_end(ap);
	return 0;
}

mybmm_module_t jk_module = {
	MYBMM_MODTYPE_CELLMON,
	"jk",
	MYBMM_CHARGE_CONTROL | MYBMM_DISCHARGE_CONTROL | MYBMM_BALANCE_CONTROL,
	jk_init,			/* Init */
	jk_new,			/* New */
	jk_open,			/* Open */
	jk_read,			/* Read */
	0,				/* Write */
	jk_close,			/* Close */
	0,				/* Free */
	0,				/* Shutdown */
	jk_control,			/* Control */
	0,				/* Config */
};
