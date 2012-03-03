

CC = arm-v4t-linux-uclibcgnueabi-gcc


.PHONY: all clean

all: disptest

disptest: main.c
	$(CC) -static -o $@ main.c

clean:
	rm -f disptest *~
