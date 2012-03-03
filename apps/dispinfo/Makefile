CC = arm-v4t-linux-uclibcgnueabi-gcc

.PHONY: all clean

all: dispinfo

dispinfo: main.c
	$(CC) -static -o $@ main.c

run:
	./dispinfo

clean:
	rm -f dispinfo *~

