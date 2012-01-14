roombactl: roombactl.c roomba.h
	cc -g -O -o roombactl roombactl.c

clean:
	rm -f roombactl
