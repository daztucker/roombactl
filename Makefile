roombactl: roombactl.c
	cc -g -O -o roombactl roombactl.c

clean:
	rm -f roombactl
