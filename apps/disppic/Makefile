

CC = arm-v4t-linux-uclibcgnueabi-gcc


.PHONY: all clean


all: disppic


disppic: main.c font_5x8.h
	$(CC) -static -o $@ main.c


font_5x8.h: font_5x8.txt
	./txt2header $< $@

run:
	./disppic Hello


clean:
	rm -f font.h disppic *~

