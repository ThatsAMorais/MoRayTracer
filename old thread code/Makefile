
vpath %.cpp src
vpath %.h	src

objects = Main.o FIFO.o

hw2_ex2 : $(objects)
	g++ -o hw2_ex2 $(objects) -lpthread

Main.o : Main.cpp FIFO.h
FIFO.o : FIFO.cpp FIFO.h