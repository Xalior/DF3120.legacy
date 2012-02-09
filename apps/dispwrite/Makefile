

CC = arm-v4t-linux-uclibcgnueabi-gcc


.PHONY: all clean


all: dispwrite


dispwrite: main.c font.h
	$(CC) -static -o $@ main.c


font.h: font.txt
	./txt2header $< $@

run:
	./dispwrite Hello


clean:
	rm -f font.h dispwrite *~

