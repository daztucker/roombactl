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
 * set roomba 530 for scheduled cleaning.
 * based on info from http://www.robotreviews.com/chat/viewtopic.php?t=9235
 * and http://www.irobot.lv/uploaded_files/File/iRobot_Roomba_500_Open_Interface_Spec.pdf
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "roomba.h"

static int roomba_needs_start = 1;
static int verbose = 0;

struct roomba_cmd {
	size_t len;
	unsigned char type;
	unsigned char *data;
};

static void command_send_simple(int, int);

struct roomba_cmd *
command_new(unsigned char type, size_t size)
{
	struct roomba_cmd *p;

	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("malloc");
		exit(1);
	}
	p->type = type;
	p->len = size;
	p->data = NULL;
	if (size > 0) {
		if ((p->data = malloc(size)) == NULL) {
			perror("malloc");
			exit(1);
		}
		memset(p->data, 0, size);
	}
	return p;
}

void
command_free(struct roomba_cmd *cmd)
{
	if (cmd->data != NULL)
		free(cmd->data);
	free(cmd);
}

static void
command_send(int fd, struct roomba_cmd *cmd)
{
	int res;
	size_t i, pos = 0, len = cmd->len + 1;;
	unsigned char *buf = malloc(len);

	/* send a start command if needed */
	if (roomba_needs_start && cmd->type != ROOMBA_START &&
	   cmd->type != ROOMBA_RESET) {
		command_send_simple(fd, ROOMBA_START);
		usleep(100000);
		roomba_needs_start = 0;
	}

	if (fd < 0) {
		fprintf(stderr, "No device specified");
		exit(1);
	}

	if (verbose) {
		printf("command %d", cmd->type);
		for (i = 0; i < cmd->len; i++)
			printf(" %d", cmd->data[i]);
		printf("\n");
	}
	buf[0] = cmd->type;
	memcpy(buf + 1, cmd->data, cmd->len);
	while (len > pos) {
		res = write(fd, buf + pos, len - pos);
		switch (res) {
		case -1:
			if (errno == EINTR || errno == EAGAIN)
				continue;
			perror("write");
			exit(1);
		default:
			pos += (size_t)res;
		}
	}
	free(buf);
}

static void
command_send_simple(int fd, int command)
{
	struct roomba_cmd *cmd = command_new(command, 0);

	command_send(fd, cmd);
	command_free(cmd);
}

static int
open_device(char *devicename)
{
	static int fd = -1;
	char cmd[1024];

	snprintf(cmd, sizeof cmd,
	    "stty raw clocal speed 115200 <%s >/dev/null", devicename);
	if (system(cmd) != 0) {
		perror("system");
		exit(1);
	}

	if (fd != -1)
		close(fd);
	printf("open %s\n", devicename);
	if ((fd = open(devicename,  O_RDWR)) == -1)
		perror("open");
	tcflush(fd, TCIOFLUSH);

#if 0
	{
		struct termios tio;

		/* speed to 115200 */
		tcgetattr(fd, &tio);
		tio.c_cflag = (CLOCAL | CREAD);
		cfsetispeed(&tio, B115200);
		cfsetospeed(&tio, B115200);
		tcsetattr(fd, TCSANOW, &tio);
	}
#endif

	return fd;
}

static int
lookup_day(char *day)
{
	int i;
	char *days[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat", NULL};

	for (i = 0; i < 7; i ++)
		if (strcasecmp(days[i], day) == 0)
			return i;
	return -1;
}

void
set_schedule(int fd, char *schedule)
{
	int i, day, hour, minute;
	char *p, dayname[4];
	/* command, daymask, sun(hour,min), mon(hour,min), tue(hour,min) ... */
	unsigned char daymask = 0;
	struct roomba_cmd *cmd;

	cmd = command_new(ROOMBA_SCHEDULE, 15);

	for (p = schedule;  ; p = NULL) {
		if ((p = strtok(p, ", ")) == NULL)
			break;
		if ((i = sscanf(p, "%3s:%d:%d", dayname, &hour, &minute)) != 3) {
			printf("day %s\n", dayname);
			fprintf(stderr,
			    "incomplete schedule spec '%s' found %d\n", p, i);
			exit(1);
		}
		if ((day = lookup_day(dayname)) == -1) {
			fprintf(stderr, "unknown day '%s'\n", dayname);
			exit(1);
		}
		daymask = 1;
		daymask <<= day;
		cmd->data[0] |= daymask;
		cmd->data[1+day*2] = hour;
		cmd->data[2+day*2] = minute;
	}
	command_send(fd, cmd);
	command_free(cmd);
}

/*
 * led string looks like:
 * check,dock,spot,debris,colour:[0-255],intensity:[0-255]
 */
void
set_led(int fd, char *spec)
{
	char *p;
	struct roomba_cmd *cmd = command_new(ROOMBA_LED, 3);
	int colour, intensity;

	printf("leds:");
	for (p = spec;  ; p = NULL) {
		if ((p = strtok(p, ", ")) == NULL)
			break;
		printf(" %s", p);
		if (strcmp(p, "debris") == 0) {
			cmd->data[0] |= ROOMBA_LED_DEBRIS;
		} else if (strcmp(p, "spot") == 0) {
			cmd->data[0] |= ROOMBA_LED_SPOT;
		} else if (strcmp(p, "dock") == 0) {
			cmd->data[0] |= ROOMBA_LED_DOCK;
		} else if (strcmp(p, "check") == 0) {
			cmd->data[0] |= ROOMBA_LED_CHECK;
		} else if (sscanf(p, "colour:%d", &colour) == 1 ||
			    sscanf(p, "color:%d", &colour) == 1) {
			if (colour < 0 || colour > 255) {
				errno = ERANGE;
				perror("colour");
				exit(1);
			}
			cmd->data[1] = colour;
		} else if (sscanf(p, "intensity:%d", &intensity) == 1) {
			if (intensity < 0 || intensity > 255) {
				errno = ERANGE;
				perror("intensity");
				exit(1);
			}
			cmd->data[2] = intensity;
		} else {
			fprintf(stderr, "unknown LED '%s'\n", p);
			exit(1);
		}
	}
	printf("\n");
	/* LEDs can only be controlled in Safe or Full mode */
	command_send_simple(fd, ROOMBA_FULL);
	usleep(100000);
	/* Now send LED command */
	command_send(fd, cmd);
	command_free(cmd);
}

void
set_time(int fd)
{
	time_t now;
	struct tm *tm;
	struct roomba_cmd *cmd = command_new(ROOMBA_SETTIME, 3);

	time(&now);
	tm = localtime(&now);
	printf("set time: day %d %02d:%02d\n", tm->tm_wday, tm->tm_hour,
	    tm->tm_min);
	cmd->data[0] = tm->tm_wday;
	cmd->data[1] = tm->tm_hour;
	cmd->data[2] = tm->tm_min;
	command_send(fd, cmd);
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: roombactl -d device [-cprtv] [-l led] [-s schedule]\n\n"
	    "\t-c (clean) -p (poweroff) -r (reset) -t (set time) "
		"-v (verbose)\n"
	    "\t-d /dev/ttyUSB0\n"
	    "\t-l [check,dock,spot,debris,colour:[0-255],intensity:[0-255]]\n"
	    "\t-s [day:HH:MM[,day:HH:MM]...]\n\n"
	 );
	exit(1);
}

int
main(int argc, char **argv)
{
	int opt, fd = -1;
	char *devicename = NULL;

	if (argc == 1)
		usage();

	if ((devicename = getenv("ROOMBA_DEVICE")) != NULL) {
		fd = open_device(devicename);
	}

	while ((opt = getopt(argc, argv, "cd:l:prs:tv")) != -1) {
		switch (opt) {
		case 'c':
			command_send_simple(fd, ROOMBA_CLEAN);
			break;
		case 'd':
			fd = open_device(optarg);
			break;
		case 'l':
			set_led(fd, optarg);
			break;
		case 'p':
			command_send_simple(fd, ROOMBA_POWER);
			break;
		case 'r':
			command_send_simple(fd, ROOMBA_RESET);
			roomba_needs_start = 1;
			break;
		case 's':
			set_time(fd);
			set_schedule(fd, optarg);
			break;
		case 't':
			set_time(fd);
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage();
			break;
		}
	}
	close(fd);
	exit(0);
}
