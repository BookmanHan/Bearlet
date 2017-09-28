LIBS = -lafcpu
LIB_PATHS = -L/usr/lib
INCLUDES = -I/usr/include
CC = icc $(COMPILER_OPTIONS)
CVer = Release

ifeq ($(CVer), Release)
	COMPILER_OPTIONS = -std=c++11 -g -O3 -xHost -mkl=parallel -qopenmp
else
	COMPILER_OPTIONS = -std=c++11 -g -mkl -qopenmp
endif

all: ./Bearlet/Bearlet.cpp Makefile
		$(CC) ./Bearlet/Bearlet.cpp -o Bearlet.app $(INCLUDES) $(LIBS) $(LIB_PATHS)
		./Bearlet.app
