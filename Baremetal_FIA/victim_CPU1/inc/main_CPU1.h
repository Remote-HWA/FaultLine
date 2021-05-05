#ifndef INC_MAIN_CPU1_H_
#define INC_MAIN_CPU1_H_

// Communication channel with CPU0
#define SHARED_BASE_ADDR  0x7000000

#define MODE_ADDR (SHARED_BASE_ADDR+0x0)
	#define IDLE_MODE  		0
	#define AES_MODE        1
	#define CACHE_MODE      2
	#define READ_MODE       3
	#define WRITE_MODE      4

#define CONFIG_ADDR (SHARED_BASE_ADDR+0x4)
	#define CRYPT_WAIT  1
	#define CRYPT_READY 2
	#define CRYPT_START 3
	#define CRYPT_END	4
	#define CRYPT_STOP  5
	#define PLAIN_READY 6
	#define CIPHER_READY 7

	#define DATA_IDLE 1
	#define DATA_START 2
	#define DATA_READY 3
	#define DATA_END 4
	#define DATA_STOP 5

#define CACHE_ADDR (SHARED_BASE_ADDR+0xc)
	#define CACHE 0
	#define NOCACHE 1

#define FAULT_ADDR (SHARED_BASE_ADDR+0x10)

#define KEY_ADDR (SHARED_BASE_ADDR+0x100)
#define PLAIN_ADDR (SHARED_BASE_ADDR+0x200)
#define CIPHER_ADDR (SHARED_BASE_ADDR+0x300)
#define DATA_ADDR (SHARED_BASE_ADDR+0x400)

// Macros
#define REG_READ(addr) \
    ({int val;int a=addr; asm volatile ("ldr   %0,[%1]\n" : "=r"(val) : "r"(a)); val;})

#define REG_WRITE(addr,val) \
    ({int v = val; int a = addr; __asm volatile ("str  %1,[%0]\n" :: "r"(a),"r"(v)); v;})

// Function prototypes
int AES128_Tiny_Encrypt(void);
int Modify_Cache_State(void);
int Read_Memory_Array(void);
int Write_Memory_Array(void);

#endif /* INC_MAIN_CPU1_H_ */



