#ifndef INC_MAIN_CPU0_H_
#define INC_MAIN_CPU0_H_

// Communication channel with CPU1
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
// Delay line address
#define DLL_R_ADDR  0xF8006140
#define DLL_W_ADDR  0xF8006154

// Macros
#define REG_READ(addr) \
    ({int val;int a=addr; asm volatile ("ldr   %0,[%1]\n" : "=r"(val) : "r"(a)); val;})

#define REG_WRITE(addr,val) \
    ({int v = val; int a = addr; __asm volatile ("str  %1,[%0]\n" :: "r"(a),"r"(v)); v;})

// Function prototypes
int 		Calibrate(void);
int 		Fault_Piret_CPU1(char * user_input);
int 		Fault_Persistent_CPU1(char * user_input);
int 		Write_Test_CPU1(char * user_input);
int 		Read_Test_CPU1(char * user_input);
int 		Modify_Cache_State(int cacheState);
int 		Extract_Faulted_Cipher(uint32_t *cipherText, uint32_t * fcipherText, uint8_t nByte);
int 		Play_AES_CPU1(uint32_t * plainArray,uint32_t * cipherArray);
uint8_t * 	Read_DLL_State(uint32_t nSample);
int 		Faulted_Sbox_Matching(uint32_t * plainText,uint32_t * fcipherText,uint8_t * sboxIndex, uint8_t * sboxValue);
void 		Command_Helper(void);

// DLL Configuration
#define DLL_addr  0xF80061E0

// MASTER DLL 1
#define coarsemask1  0x7f
#define coarsepos1  13
#define finemask1  0x03
#define finepos1  11
// MASTER DLL 0
#define coarsemask0  0x7f
#define coarsepos0  4
#define finemask0  0x03
#define finepos0  2

//FPGA ADDR
#define FPGA_TRIGGER 0x43c00000
#define FPGA_DELAY 0x43c00004


#endif


/* INC_MAIN_CPU0_H_ */


