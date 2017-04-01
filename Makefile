roombactl: roombactl.c roomba.h
	cc -Wall -g -O -o roombactl roombactl.c

clean:
	rm -f roombactl
