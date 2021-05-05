# ATTENTION ce makefile doit etre lancé avec la commande "make" 
# a partir du dossier dans lequel sont stockes les sources et includes 
# de votre projet

#Ici remplacer toto par le nom de l'executable que vous souhaitez creer
EXE = RSApenc.elf
#Ici remplacer les includes par ceux de votre projet
#Ici remplacer les sources par celles de votre projet
SRCS = RSApenc.c

ifeq ($(OS),Windows_NT)
	CC = C:\Xilinx\SDK\2018.1\gnu\aarch32\nt\gcc-arm-linux-gnueabi\bin\arm-linux-gnueabihf-gcc
else
	CC = /tools/Xilinx/SDK/2019.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-gcc
	#CC = gcc
endif

#LIBPATH = -L../cross-openssl/compiled-openssl/lib
LIBPATH = -L../compiled_1_1_0a/lib
LIBS = -lssl -lcrypto -lm -pthread -lpthread 
OBJS = $(SRCS:.c=.o)
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
CFLAGS =  -g -Wall -Wextra -I../compiled_1_1_0a/include

%.o: %.c
	$(CC) -c $(CFLAGS) $(SRCS) 
$(EXE): $(OBJS) 
	$(CC) $(LDFLAGS) $(OBJS)
	
$(OBJS): $(INCS)	

clean:
	rm -f $(OBJS) $(EXE)
	
