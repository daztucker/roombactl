/*
 * Copyright (c) 2012 Darren Tucker.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * roomba commands, data from
 * http://www.irobot.lv/uploaded_files/File/iRobot_Roomba_500_Open_Interface_Spec.pdf
 */

/* Initialisation commands */
#define ROOMBA_RESET	7	/* undocumented */
#define ROOMBA_START	128
#define ROOMBA_BAUD	129

/* Mode commands */
#define ROOMBA_CONTROL	130	/* deprecated? */
#define ROOMBA_SAFE	131
#define ROOMBA_FULL	132
#define ROOMBA_POWER	133

/* Cleaning commands */
#define ROOMBA_SPOT	134
#define ROOMBA_CLEAN	135
#define ROOMBA_MAX	136
#define ROOMBA_DOCK	143

/* Schedule commands */
#define ROOMBA_SCHEDULE	167
#define ROOMBA_SETTIME	168

/* Binary LEDs */
#define ROOMBA_LED	139
#define ROOMBA_LED_DEBRIS	0x1
#define ROOMBA_LED_SPOT		0x2
#define ROOMBA_LED_DOCK		0x4
#define ROOMBA_LED_CHECK	0x8
