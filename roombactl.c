/*
 * set roomba 530 for scheduled cleaning.
 * based on info from http://www.robotreviews.com/chat/viewtopic.php?t=9235
 * and http://www.irobot.lv/uploaded_files/File/iRobot_Roomba_500_Open_Interface_Spec.pdf
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "roomba.h"

static roomba_needs_start = 1;

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
	size_t i;

	/* send a start command if needed */
	if (roomba_needs_start && cmd->type != ROOMBA_START &&
	   cmd->type != ROOMBA_RESET) {
		command_send_simple(fd, ROOMBA_START);
		roomba_needs_start = 0;
	}

	if (fd < 0) {
		fprintf(stderr, "No device specified");
		exit(1);
	}

	printf("command %d", cmd->type);
	for (i = 0; i < cmd->len; i++)
		printf(" %d", cmd->data[i]);
	printf("\n");
	write(fd, cmd->data, cmd->len);
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

	if (fd != -1)
		close(fd);
	printf("open %s\n", devicename);
	if ((fd = open(devicename,  O_RDWR)) == -1)
		perror("open");
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
		if ((i = sscanf(p, "%3s:%d:%d", &dayname, &hour, &minute)) != 3) {
			printf("day %s\n", dayname);
			fprintf(stderr, "incomplete schedule spec '%s' found %d\n", p, i);
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
	char *p, name[1024];
	int i, value;
	struct roomba_cmd *cmd = command_new(ROOMBA_LED, 3);

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
		} else {
			fprintf(stderr, "unknown LED '%s'\n", name);
			exit(1);
		}
	}
	printf("\n");
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
	printf("set time: day %d %02d:%02d\n", tm->tm_wday, tm->tm_hour, tm->tm_min);
	cmd->data[0] = tm->tm_wday;
	cmd->data[1] = tm->tm_hour;
	cmd->data[2] = tm->tm_min;
	command_send(fd, cmd);
}

int
main(int argc, char **argv)
{
	int opt, fd = -1, do_clean, do_reset;
	char *devicename = NULL;
	struct roomba_cmd *prg_sched = NULL;

	if ((devicename = getenv("ROOMBA_DEVICE")) != NULL) {
		fd = open_device(devicename);
	}

	while ((opt = getopt(argc, argv, "cd:l:prs:t")) != -1) {
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
		}
	}
	close(fd);
}
