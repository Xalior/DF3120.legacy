CFLAGS = -I../../minifs/build-df3120/staging/usr/include
PNG_CFLAGS = -I../../minifs/build-df3120/staging/usr/include
LDFLAGS = -L/u/GIT/CEVIN/DF3120/minifs/build-df3120/staging/usr/lib -lz -lpng14

CC = arm-v4t-linux-uclibcgnueabi-gcc


.PHONY: all clean


all: disppic


disppic: main.c 
	$(CC) -o $@ main.c $(CFLAGS) $(PNG_CFLAGS) $(LDFLAGS)

run:
	./disppic TEST.png

clean:
	rm -f disppic *~

