/*******************************************************************************
*               FaultLine 2020
*
* Name        : main_CPU1.c
*
* Description : Piret and Persistent dual core fault attack
*
*
* Author      :
*
* Toolchain   : Xilinx ARM v7 GNU Toolchain
*
* Host        : Zynq XC7Z010-CLG400
*
*
*******************************************************************************
*
* Version :     1.0
*               31/08/2020 : 1.0 - Initial version
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xil_cache.h"
#include "xil_io.h"
#include "main_CPU1.h"
#include "math.h"
#include "utility.h"
#include "aes.h"


// AES DECLARATION
struct AES_ctx ctx;
uint8_t ptArray[16];
uint8_t ctArray[16];
uint8_t keyArray[16] = {0x7d,0xc4,0x04,0x63,0xee,0x8d,0x0d,0x11,0x56,0xda,0x16,0x8a,0x71,0x1d,0xef,0xba};
uint32_t nFault = 0;

int main(void)
{
	uint8_t ack_work = 0;
	uint8_t ack_cache = 0;

	xil_printf("\n\rCPU#1: Welcome! I'm ready to encrypt!\n\r");

	// Perform the Tiny AES key expension
	AES_init_ctx(&ctx,keyArray);
	REG_WRITE(MODE_ADDR,IDLE_MODE);
	REG_WRITE(CONFIG_ADDR,CRYPT_WAIT);
	REG_WRITE(CACHE_ADDR,CACHE);

	do
	{
		switch(REG_READ(MODE_ADDR))
		{
			/************* IDLE Mode ************/
			case(IDLE_MODE):

				if(ack_work)
				{
					ack_work = 0;
					//xil_printf("\n\rnFault: %d\n\r",nFault);
					nFault = 0;
				}

				if(ack_cache)
				{
					ack_cache = 0;
				}

				break;

			/************* AES encryption Mode ************/
			case(AES_MODE):

				ack_work = 1;
				AES128_Tiny_Encrypt();
				break;

			/********** Cache Enable/Disable Mode *********/
			case(CACHE_MODE):

				if(ack_cache==0)
				{
					Modify_Cache_State();
					ack_cache = 1;
				}
				break;

			/******** Read data Mode **********/
			case(READ_MODE):
				ack_work = 1;
				Read_Memory_Array();
				break;

			/**************** Default *******************/
			default:
				printf("\n\r Warning: Mode is not correct");
				break;
		}
	}
	while(1);

	return 0;
}

int AES128_Tiny_Encrypt(void)
{
	uint8_t c_i, p_i;

	// Wait for CPU#0 plaintext
	while(REG_READ(CONFIG_ADDR) != PLAIN_READY){}

	// When the plaintext is ready, read it
	for(p_i = 0 ; p_i < 4 ; p_i++)
	{
		ptArray[p_i*4+3] = (uint8_t)((REG_READ(PLAIN_ADDR+p_i*4) & 0x000000ff)>>0);
		ptArray[p_i*4+2] = (uint8_t)((REG_READ(PLAIN_ADDR+p_i*4) & 0x0000ff00)>>8);
		ptArray[p_i*4+1] = (uint8_t)((REG_READ(PLAIN_ADDR+p_i*4) & 0x00ff0000)>>16);
		ptArray[p_i*4+0] = (uint8_t)((REG_READ(PLAIN_ADDR+p_i*4) & 0xff000000)>>24);
	}

	//SEND READY TO CPU 0
	REG_WRITE(CONFIG_ADDR,CRYPT_READY);

	//WAIT CPU0 SYNCHRO
	while(REG_READ(CONFIG_ADDR) != CRYPT_START){}

	//TINY AES ENCRYPT
	AES_ECB_encrypt(&ctx,ptArray);

	//SEND CIPHERTEXT TO CPU0
	for(c_i = 0; c_i < 4 ; c_i++){
		REG_WRITE(CIPHER_ADDR+c_i*4,((ptArray[c_i*4+0]<<24) & 0xff000000) | ((ptArray[c_i*4+1]<<16) & 0x00ff0000) | ((ptArray[c_i*4+2]<<8) & 0x00ff00) | ((ptArray[c_i*4+3]<<0) & 0x000000ff));
	}

	//SEND SYNCHRO TO CPU#0
	REG_WRITE(CONFIG_ADDR,CRYPT_END);

	//WAIT CPU0 END
	while(REG_READ(CONFIG_ADDR) != CRYPT_STOP){}

	// Send ciphertext ready to CPU#0
	REG_WRITE(CONFIG_ADDR,CIPHER_READY);

	return 1;
}

int Modify_Cache_State(void)
{
	if(REG_READ(CACHE_ADDR)==NOCACHE)
	{
		Xil_DCacheDisable();
		HW_uDelay(100000);
	}
	else
	{
		Xil_DCacheEnable();
		HW_uDelay(100000);
	}

	REG_WRITE(MODE_ADDR,IDLE_MODE);
	REG_WRITE(CONFIG_ADDR,CRYPT_WAIT);
	HW_uDelay(1000);

	return 1;
}

int Read_Memory_Array(void)
{
	uint32_t nTest = 100;
	uint32_t iTest;
	uint32_t writeArray[nTest];
	uint32_t readArray[nTest] ;

	for(iTest = 0 ; iTest < nTest ; iTest++)
	{
		if(iTest % 2 == 0)
		{
			writeArray[iTest] = 0x00000000;
		}
		else
		{
			writeArray[iTest] = 0xFFFFFFFF;
		}
	}

	//SEND READY TO CPU 0
	REG_WRITE(CONFIG_ADDR,DATA_IDLE);

	//WAIT CPU0 SYNCHRO
	while(REG_READ(CONFIG_ADDR) != DATA_START){}

	for(int iTest = 0 ; iTest < nTest ; iTest++)
	{
		readArray[iTest] = writeArray[iTest];
	}

	for( iTest = 0 ; iTest < nTest ; iTest++)
	{
		if(readArray[iTest] != writeArray[iTest])
		{
			xil_printf("\n\r%d %08x->%08x",iTest,writeArray[iTest],readArray[iTest]);
			nFault++;
			REG_WRITE(FAULT_ADDR,nFault);
		}
	}



	//SEND SYNCHRO TO CPU#0
	REG_WRITE(CONFIG_ADDR,DATA_END);

	//WAIT CPU0 END
	while(REG_READ(CONFIG_ADDR) != DATA_STOP){}



	return 0;


}


