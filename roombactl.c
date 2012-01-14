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

struct roomba_cmd {
	size_t len;
	unsigned char type;
	unsigned char *data;
};

struct roomba_cmd *
command_new(unsigned char type, size_t size)
{
	struct roomba_cmd *p;

	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("malloc");
		exit(1);
	}
	p->len = size;
	if ((p->data = malloc(size)) == NULL) {
		perror("malloc");
		exit(1);
	}
	memset(p->data, 0, size);
	p->type = type;
	return p;
}

static void
command_send(int fd, struct roomba_cmd *cmd)
{
	size_t i;

	printf("command %d", cmd->type);
	for (i = 0; i < cmd->len; i++)
		printf(" %d", cmd->data[i]);
	printf("\n");
	write(fd, cmd->data, cmd->len);
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

struct roomba_cmd *
set_schedule(char *schedule)
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
	return cmd;
}

int
main(int argc, char **argv)
{
	int opt, fd = -1;
	time_t now;
	struct tm *tm;
	char *devicename;
	unsigned char prg_reset = {7};
	unsigned char prg_wake[] = {128};
	unsigned char prg_power[] = {133};
	struct roomba_cmd *prg_time = NULL;
	struct roomba_cmd *prg_sched = NULL;

	if ((devicename = getenv("ROOMBA_DEVICE")) != NULL)
		fd = open_device(devicename);

	while ((opt = getopt(argc, argv, "d:s:")) != -1) {
		switch (opt) {
		case 'd':
			fd = open_device(optarg);
			break;
		case 's':
			prg_sched = set_schedule(optarg);
			break;
		}
	}

	write(fd, prg_wake, sizeof(prg_wake));

	if (prg_sched != NULL) {
		time(&now);
		tm = localtime(&now);
		printf("set time: day %d %02d:%02d\n", tm->tm_wday, tm->tm_hour, tm->tm_min);
		prg_time = command_new(ROOMBA_SETTIME, 3);
		prg_time->data[0] = tm->tm_wday;
		prg_time->data[1] = tm->tm_hour;
		prg_time->data[2] = tm->tm_min;
		command_send(fd, prg_time);
		command_send(fd, prg_sched);
	}
	write(fd, prg_power, sizeof(prg_power));
	close(fd);
}
