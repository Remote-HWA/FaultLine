EXE = simple_glitcher.elf
SRCS = simple_glitcher.c

ifeq ($(OS),Windows_NT)
	CC = C:\Xilinx\Vitis\2020.2\gnu\aarch32\nt\gcc-arm-linux-gnueabi\bin\arm-linux-gnueabihf-gcc
else
	CC = /tools/Xilinx/SDK/2019.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-gcc
endif

LIBPATH =
LIBS = -lm 
OBJS = $(SRCS:.c=.o)
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
CFLAGS =  -g -Wall -Wextra 

%.o: %.c
	$(CC) -c $(CFLAGS) $(SRCS) 
$(EXE): $(OBJS) 
	$(CC) $(LDFLAGS) $(OBJS)
	
$(OBJS): $(INCS)	

clean:
	rm -f $(OBJS) $(EXE)
	
