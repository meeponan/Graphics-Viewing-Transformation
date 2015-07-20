CC = gcc
NAME = liu_xiuwen
LIBS = -lglut32 -lglu32 -lopengl32
CFLAGS = -O3

LIBS =  -L/usr/X11R6/lib/ -O2 -lglut -lGLU -lGL -lXmu -lXt -lSM -lICE -lXext -lX11 -lXi -lXext -lX11 -lm

lab2: lab2.o SSD_util.o
	$(CC) -o lab2 lab2.o SSD_util.o $(LIBS)
lab2_extra: lab2_extra.o SSD_util.o
	$(CC) -o lab2_extra lab2_extra.o SSD_util.o $(LIBS)
.c.o: 
	$(CC)  $(CFLAGS) -c  $(COPT) $<
tar:
	tar cvfz lab2_$(NAME).tar.gz *.c *.h 
	ls -l lab2_$(NAME).tar.gz
run:
	./lab2 lab2_scene1.ssd
	./lab2 lab2_scene2.ssd
	./lab2 lab2_scene3.ssd
	./lab2 lab2_scene4.ssd

clean:
	rm  *.o

