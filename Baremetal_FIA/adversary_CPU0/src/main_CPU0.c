/*******************************************************************************
*               FaultLine 2020
*
* Name        : main_CPU0.c
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
#include "xil_io.h"
#include "xil_cache.h"
#include "main_CPU0.h"
#include "math.h"
#include "utility.h"
#include "aes.h"


uint8_t cacheState = 1;
uint32_t stress_min[4] = {25,25,25,25};
uint32_t stress_max[4] = {25,25,25,25};


int main(void)
{
	char * user_input;
	char * command = malloc(sizeof(char) * 32);
	init_perfcounters (1,0);

	// Print Hello Banner
	xil_printf("\n\r\n\r\n\r\n\r");
	xil_printf(" Welcome in FaultLine!\n\r");
	xil_printf("\n\rCPU#0: Welcome! I'm ready to attack!\n\r");

	// Wait for CPU1 ready
	HW_uDelay(400000);
	Command_Helper();

	xil_printf("\n\rEnter \"help\" to display the command helper\n\r");

	// State Machine
	do{
		xil_printf("FL>");
		command = GetString();
		user_input = strtok(command," ");
		print("\n\r");

		if((strcmp(user_input,"piret")==0) || (strcmp(user_input,"PIRET")==0))
		{
			Fault_Piret_CPU1(user_input);
		}
		else if((strcmp(user_input,"calib")==0) || (strcmp(user_input,"CALIB")==0))
		{
			if(cacheState){cacheState = Modify_Cache_State(0);} //disable data cache
			Calibrate();
		}
		else if((strcmp(user_input,"pfa")==0) || (strcmp(user_input,"PFA")==0))
		{
			Fault_Persistent_CPU1(user_input);
		}
		else if((strcmp(user_input,"rtest")==0) || (strcmp(user_input,"RTEST")==0))
		{
			if(cacheState){cacheState = Modify_Cache_State(0);} //disable data cache
			Read_Test_CPU1(user_input);
		}
		else if((strcmp(user_input,"wtest")==0) || (strcmp(user_input,"WTEST")==0))
		{
			Write_Test_CPU1(user_input);
		}
		else if((strcmp(user_input,"cache")==0) || (strcmp(user_input,"CACHE")==0))
		{
			user_input = strtok(NULL," ");
			int cacheState = (user_input == NULL)?0:int2int(user_input);
			if(cacheState){cacheState = Modify_Cache_State(cacheState);} //disable data cache
		}
		else if((strcmp(user_input,"help")==0) || (strcmp(user_input,"?")==0))
		{
			Command_Helper();
		}
		else
		{
			if(strcmp(command,"")!=0){
				xil_printf("Unknown Command %s\n\r",command);
			}
		}
	}
	while(1);

	return 0;
}

int Fault_Piret_CPU1(char * user_input)
{
	int tStress;
	int reg_val;
	int reg_val2;
	int tDelay;
	int iDelay = 0;
	int nDelay = 5;
	char * command = malloc(sizeof(char) * 32);
	int stepDelay = 0;
	int iByte = 0;
	int iFault = 0;
	int iWidth = 0;
	int tWidth = 0;
	int nTrace = 2000;
	int nFault = 1;
	int nByte = 4;
	int iTrace = 0;
	int iCounter = 0;
	//int i,j;
	uint8_t p_i;
	uint32_t plainText[4];
	uint32_t cipherText[4];
	uint32_t fcipherText[4];
	uint32_t val1,val2,val3;
	int DQ0vals[1] = { stress_min[0]};
	int DQ1vals[1] = { stress_min[1]};
	int DQ2vals[1] = { stress_min[2]};
	int DQ3vals[1] = { stress_min[3]};
	int delaySteps[10] = {1600,1610,1620, 1630,1640, 1650, 1660,1670, 1680,1690};
	int ** faultValues;

	faultValues = (int**)malloc(4*sizeof(int*));

	for(int i = 0 ; i < 4 ; i++){

		faultValues[i] = (int*)malloc(10*sizeof(int));

		for(int j = 0 ; j < 5 ; j++)
		{
			switch(i)
			{
			case 0:
				faultValues[i][j] = DQ0vals[j];
				break;
			case 1:
				faultValues[i][j] = DQ1vals[j];
				break;
			case 2:
				faultValues[i][j] = DQ2vals[j];
				break;
			case 3:
				faultValues[i][j] = DQ3vals[j];
				break;
			default:
				xil_printf("error");
				break;
			}
		}

	}

	// Prepare plaintext for transfer to CPU#1
	plainText[0] = 0x86209140;
	plainText[1] = 0x39f68c7c;
	plainText[2] = 0x5a33509a;
	plainText[3] = 0xecda185a;

	if(Xil_In32(CACHE_ADDR) == CACHE)
	{
		xil_printf("\n\rWarning!\n\rThis attack only works with cache memory disabled");
		xil_printf("\n\rDo you want to disable cache? (y/n)");
		command = GetString();
		if(strcmp(command,"y")==0)
		{
			Modify_Cache_State(0);
		}
	}

	xil_printf("\n\rStart Piret Fault Attack on CPU1");

	// Compute & Print AES results
	Play_AES_CPU1(plainText,cipherText);
	xil_printf("\n\rPrinting plaintext and correct ciphertext:");
	xil_printf("\n\rplain : %08x%08x%08x%08x",plainText[0],plainText[1],plainText[2],plainText[3]);
	xil_printf("\n\rcorrect : %08x%08x%08x%08x\n\r",cipherText[0],cipherText[1],cipherText[2],cipherText[3]);

	//Selecting CPU1 AES Mode
	HW_uDelay(10);
	REG_WRITE(CONFIG_ADDR,CRYPT_WAIT);
	REG_WRITE(MODE_ADDR,AES_MODE);

	reg_val2 = (((25 << 11)&0xFF800)| ((1 << 10)&0x400));

	for(iByte = 0 ; iByte < nByte ; iByte++)
	{
		xil_printf("\n\r\n\r********** Byte %d ************",iByte);

		for(iFault = 0 ; iFault < nFault ; iFault++)
		{
			xil_printf("\n\r");

			for(stepDelay = 0 ; stepDelay < nDelay ; stepDelay++)
			{
				tDelay = delaySteps[stepDelay];
				tStress = faultValues[iByte][iFault];
				reg_val = (((tStress << 11)&0xFF800)| ((1 << 10)&0x400));

				xil_printf("\n\rDQ%d, iFault: %d, delay: %d",iByte,tStress,tDelay);
				xil_printf("\n\rencrypt duration: %d",val3-val1);
				xil_printf("\n\rdelay duration: %d",val2-val1);
				// LOOP OVER iTrace NUMBER
				for(iTrace = 0 ; iTrace < nTrace ; iTrace++)
				{
					// Send plaintext to CPU#1
					for(p_i = 0; p_i < 4 ; p_i++){
						REG_WRITE(PLAIN_ADDR+p_i*4,plainText[p_i]);
					}
				    //xil_printf("\n\rplaintext : %08x%08x%08x%08x",plainText[0],plainText[1],plainText[2],plainText[3]);

					// Send plain ready to CPU#1
					REG_WRITE(CONFIG_ADDR,PLAIN_READY);

					// WAIT FOR CPU1 READY
					while(REG_READ(CONFIG_ADDR) != CRYPT_READY){}

					// LAUNCH CPU1 ENCRYPTION
					asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(val1));
					REG_WRITE(CONFIG_ADDR,CRYPT_START);

					//INIT DELAY
					for(iDelay = 0 ; iDelay < tDelay; iDelay++){}
					for(iDelay = 0 ; iDelay < (uint8_t)rand(); iDelay++){}

					asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(val2));

					//GLITCH
					*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_val ;  // set DLL glitch value
					for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
					*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_val2; // get back to DLL normal value

					//WAIT FOR CPU1 END OF ENCRYPTION
					while(REG_READ(CONFIG_ADDR) != CRYPT_END){}
					asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(val3));

					//SEND END TO CPU1
					if(iCounter < (nByte*nFault*nDelay*nTrace)-1)
					{
						REG_WRITE(CONFIG_ADDR,CRYPT_STOP);

						//WAIT FOR CPU1 CIPHERTEXT
						while(REG_READ(CONFIG_ADDR) != CIPHER_READY){}

						fcipherText[0] = REG_READ(CIPHER_ADDR);
						fcipherText[1] = REG_READ(CIPHER_ADDR+4);
						fcipherText[2] = REG_READ(CIPHER_ADDR+8);
						fcipherText[3] = REG_READ(CIPHER_ADDR+12);

						Extract_Faulted_Cipher(cipherText,fcipherText,4);
					}
					else
					{
						xil_printf("\n\rCPU1: Return to IDLE");
						HW_uDelay(100);
						REG_WRITE(MODE_ADDR,IDLE_MODE);
						REG_WRITE(CONFIG_ADDR,CRYPT_STOP);
					}

					iCounter++;
				}
			}
		}
	}

	xil_printf("\n\rend fault attack\n\r\n\r");
	return 1;
}

int Fault_Persistent_CPU1(char * user_input)
{
	//State Variables
	char * command = malloc(sizeof(char) * 32);
	int firsttry = 1;
	int faultdetected= 0;
	uint8_t p_i;
	uint8_t sboxValue = 0;
	uint8_t sboxIndex = 0;
	uint8_t success = 0;
	uint32_t iCipher = 0;
	uint32_t nCipher = 0;
	uint32_t val1,val2,val3;

	//Glitch Parameters
	uint8_t  iByte = 0;
	uint32_t iDelay = 0;
	uint32_t tDelay = 0;
	uint32_t iWidth = 0;
	uint32_t tWidth = 0;
	uint32_t tStress = 0;
	uint32_t nAttack = 25000;
	uint32_t dll_base;
	uint32_t dll_fault;

	//AES variables
	uint32_t plainText[4];
	uint32_t cipherText[4];
	uint32_t fcipherText[4];


	if(Xil_In32(CACHE_ADDR) == NOCACHE)
	{
		xil_printf("\n\rWarning!\n\rThis attack only works with cache memory enabled");
		xil_printf("\n\rDo you want to enable cache? (y/n)");
		command = GetString();
		if(strcmp(command,"y")==0)
		{
			Modify_Cache_State(1);
		}
	}

	xil_printf("\n\rStart Persistent Fault Attack on CP1");

	// Glitch Value
	user_input = strtok(NULL," ");
	tStress = (user_input == NULL)?stress_min[0]:int2int(user_input);
	dll_base = (((25 << 11)&0xFF800)| ((1 << 10)&0x400));
	dll_fault = (((tStress << 11)&0xFF800)| ((1 << 10)&0x400));
	xil_printf("\n\rGlitch Value: %d",tStress);

	// nCipher
	user_input = strtok(NULL," ");
	nCipher = (user_input == NULL)?5000:int2int(user_input);
	xil_printf("\n\rnCipher: %d",nCipher);
	uint32_t plainArray[nCipher][4];
	uint32_t cipherArray[nCipher][4];
	uint32_t fcipherArray[nCipher][4];

	// Initial delay (default 0)
	user_input = strtok(NULL," ");
	tDelay = (user_input == NULL)?0:int2int(user_input);
	xil_printf("\n\rInit Delay: %d",tDelay);

	for(iCipher = 0 ; iCipher < nCipher ; iCipher++)
	{
		plainArray[iCipher][0] = (uint32_t)rand();
		plainArray[iCipher][1] = (uint32_t)rand();
		plainArray[iCipher][2] = (uint32_t)rand();
		plainArray[iCipher][3] = (uint32_t)rand();
	}

	xil_printf("\n\r");

	for(iCipher = 0 ; iCipher < nCipher ; iCipher++)
	{
		Play_AES_CPU1(plainArray[iCipher],cipherArray[iCipher]);
	}

	//SELECT AES ENCRYPTION MODE
	REG_WRITE(MODE_ADDR,AES_MODE);
	xil_printf("\n\rMODE_ADDR: %08x",REG_READ(MODE_ADDR));

	for(int iAttack = 0 ; iAttack < nAttack ; iAttack++)
	{
		for(iByte = 0 ; iByte < 4 ; iByte++)
		{
			// Print attack progress
			if(iAttack%500 == 0)
			{
				xil_printf("\n\rnAttacks: %d/%d",iAttack,nAttack);
				xil_printf("\n\rencrypt duration: %d",val3-val1);
				xil_printf("\n\rdelay duration: %d",val2-val1);
			}

			// Prepare plaintext for transfer to CPU#1
			plainText[0] = 0x86209140;
			plainText[1] = 0x39f68c7c;
			plainText[2] = 0x5a33509a;
			plainText[3] = 0xecda185a;

			// Send plaintext to CPU#1
			for(p_i = 0; p_i < 4 ; p_i++){
				REG_WRITE(PLAIN_ADDR+p_i*4,plainText[p_i]);
			}

			//xil_printf("\n\rplaintext : %08x%08x%08x%08x",plainArray[0],plainArray[1],plainArray[2],plainArray[3]);

			// Send plain ready to CPU#1
			REG_WRITE(CONFIG_ADDR,PLAIN_READY);

			// WAIT FOR CPU1 READY
			while(REG_READ(CONFIG_ADDR) != CRYPT_READY){}

			// FLUSH CACHE
			//Xil_DCacheFlushRange(0x0600D728,256);//sbox addr aes1.c
			//Xil_DCacheFlushRange(0x06012d38,64);
			Xil_DCacheFlushRange(0x06010000,20000);//blind flush

			//Xil_ICacheInvalidateRange(0x06000a58, 44);

			//get perf counter
			asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(val1));

			// LAUNCH CPU1 ENCRYPTION
			REG_WRITE(CONFIG_ADDR,CRYPT_START);

			// INIT DELAY
			for(iDelay = 0 ; iDelay < tDelay; iDelay++){}

			// RANDOM DELAY
			for(iDelay = 0 ; iDelay < (uint8_t)rand(); iDelay++){}
			asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(val2));

			//GLITCH
			REG_WRITE(DLL_R_ADDR+4*iByte,dll_fault);  // set DLL glitch value
			for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
			REG_WRITE(DLL_R_ADDR+4*iByte,dll_base); // get back to DLL normal value

			//WAIT FOR CPU1 END OF ENCRYPTION
			while(REG_READ(CONFIG_ADDR) != CRYPT_END){}
			asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(val3));

			//SEND END TO CPU1
			if(iAttack < nAttack-1)
			{
				REG_WRITE(CONFIG_ADDR,CRYPT_STOP);
			}
			else
			{
				xil_printf("\n\rCPU1: Return to IDLE");
				HW_uDelay(100);
				REG_WRITE(MODE_ADDR,IDLE_MODE);
				REG_WRITE(CONFIG_ADDR,CRYPT_STOP);
				goto end;
			}

			//WAIT FOR CPU1 CIPHERTEXT
			while(REG_READ(CONFIG_ADDR) != CIPHER_READY){}

			// Get ciphertext
			if(firsttry)
			{
				cipherText[0] = REG_READ(CIPHER_ADDR);
				cipherText[1] = REG_READ(CIPHER_ADDR+4);
				cipherText[2] = REG_READ(CIPHER_ADDR+8);
				cipherText[3] = REG_READ(CIPHER_ADDR+12);
				xil_printf("\n\rcorrect cipher : %08x%08x%08x%08x",cipherText[0],cipherText[1],cipherText[2],cipherText[3]);
				firsttry = 0;
			}
			else
			{
				fcipherText[0] = REG_READ(CIPHER_ADDR);
				fcipherText[1] = REG_READ(CIPHER_ADDR+4);
				fcipherText[2] = REG_READ(CIPHER_ADDR+8);
				fcipherText[3] = REG_READ(CIPHER_ADDR+12);

				for(p_i = 0 ; p_i < 4 ; p_i++)
				{
					if(fcipherText[p_i] != cipherText[p_i])
					{
						xil_printf("\n\rfault detected : %08x%08x%08x%08x",fcipherText[0],fcipherText[1],fcipherText[2],fcipherText[3]);
						faultdetected = 1;
						break;
					}
				}

				if(faultdetected)
				{
					if(Faulted_Sbox_Matching(plainText,fcipherText,&sboxIndex,&sboxValue))
					{
						success = 1;
						goto end;
					}
					else
					{
						faultdetected =0;
					}
				}
			}
		}
	}

	end:

	if(success)
	{
		HW_uDelay(3000000);
		for(iCipher = 0 ; iCipher < nCipher ; iCipher++)
		{
			Play_AES_CPU1(plainArray[iCipher],fcipherArray[iCipher]);
			//xil_printf("\n\rplaintext : %08x%08x%08x%08x",plainArray[iCipher][0],plainArray[iCipher][1],plainArray[iCipher][2],plainArray[iCipher][3]);
			xil_printf("\n\rcciphertext : %08x%08x%08x%08x",cipherArray[iCipher][0],cipherArray[iCipher][1],cipherArray[iCipher][2],cipherArray[iCipher][3]);
			xil_printf("\n\rfciphertext : %08x%08x%08x%08x",fcipherArray[iCipher][0],fcipherArray[iCipher][1],fcipherArray[iCipher][2],fcipherArray[iCipher][3]);
		}
		xil_printf("PFA success: Save the log file and use PFA analysis to retrieve the key\n\r");
	}

	xil_printf("\n\rEnd fault attack\n\r\n\r");
	return 1;
}

int Write_Test_CPU1(char * user_input)
{
	return 0;
}

int Read_Test_CPU1(char * user_input)
{
	uint32_t reg_nominal;
	uint32_t reg_glitch;
	uint32_t tStress;
	uint32_t tWidth;
	uint32_t iWidth;
	uint32_t nDelay;
	uint32_t iDelay;
	uint32_t iTest;
	uint32_t nTest = 10000;
	uint32_t iByte;
	uint32_t direction;

	user_input = strtok(NULL," ");
	tStress = (user_input == NULL)?stress_min[0]:int2int(user_input);

	user_input = strtok(NULL," ");
	tWidth = (user_input == NULL)?10:int2int(user_input);

	user_input = strtok(NULL," ");
	iByte = (user_input == NULL)?0:int2int(user_input);

	user_input = strtok(NULL," ");
	direction = (user_input == NULL)?1:int2int(user_input);

	user_input = strtok(NULL," ");
	nDelay = (user_input == NULL)?0:int2int(user_input);

	if(direction){
		reg_nominal = (((25 << 11)&0xFF800)| ((1 << 10)&0x400));	}
	else{
		reg_nominal = (((50 << 11)&0xFF800)| ((1 << 10)&0x400));	}

	reg_glitch = (((tStress << 11)&0xFF800)| ((1 << 10)&0x400));

	HW_uDelay(10);
	REG_WRITE(CONFIG_ADDR,CRYPT_WAIT);
	REG_WRITE(MODE_ADDR,READ_MODE);

	xil_printf("\n\rStart Read Test on CPU1");
	xil_printf("\n\rtStress: %d",tStress);
	xil_printf("\n\rtWidth: %d",tWidth);
	xil_printf("\n\riByte: %d",iByte);
	xil_printf("\n\rdirection %d",direction);
	xil_printf("\n\rnDelay: %d",nDelay);

	for(iTest = 0 ; iTest < nTest ; iTest++)
	{
		while(REG_READ(CONFIG_ADDR) != DATA_IDLE){}

		REG_WRITE(CONFIG_ADDR,DATA_START);

		for(iDelay = 0 ; iDelay < nDelay ; iDelay++){}

		if(direction)
		{
			*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_glitch ;  // set DLL glitch value
			for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
			*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_nominal; // get back to DLL normal value
		}
		else
		{
			*(volatile uint32_t*)(DLL_W_ADDR+4*iByte) = reg_glitch ;  // set DLL glitch value
			for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
			*(volatile uint32_t*)(DLL_W_ADDR+4*iByte) = reg_nominal; // get back to DLL normal value
		}

		while(REG_READ(CONFIG_ADDR) != DATA_END){}

		if(iTest == (nTest-1)){REG_WRITE(MODE_ADDR,IDLE_MODE);}
		REG_WRITE(CONFIG_ADDR,DATA_STOP);
	}

	HW_uDelay(400000);
	REG_WRITE(CONFIG_ADDR,DATA_IDLE);
	xil_printf("\n\r");


	return 0;

}


int Calibrate(void)
{
	uint32_t reg_nominal = 0;
	uint32_t reg_glitch = 0;
	uint32_t tWidth = 10;
	uint32_t iWidth = 0;
	uint32_t nDelay = 0;
	uint32_t iDelay = 0;
	uint32_t iTest = 0;
	uint32_t nTest = 100;
	uint32_t iStress = 0;
	uint32_t iByte = 0;
	uint32_t direction = 1;
	uint32_t nFault = 0;

    //calibrate for reads
	for(iByte= 0 ;  iByte < 4 ;  iByte++)
	{
		REG_WRITE(FAULT_ADDR,0);

		for(iStress = 10 ;  iStress >= 0 ;  iStress--)
		{
			reg_nominal = (((25 << 11)&0xFF800)| ((1 << 10)&0x400));
			reg_glitch = (((iStress << 11)&0xFF800)| ((1 << 10)&0x400));

			HW_uDelay(10);
			REG_WRITE(CONFIG_ADDR,CRYPT_WAIT);
			REG_WRITE(MODE_ADDR,READ_MODE);

			xil_printf("\n\rStart Read Test on CPU1");
			xil_printf("\n\rtStress: %d",iStress);
			xil_printf("\n\rtWidth: %d",tWidth);
			xil_printf("\n\riByte: %d",iByte);
			xil_printf("\n\rdirection %d",direction);
			xil_printf("\n\rnDelay: %d",nDelay);

			for(iTest = 0 ; iTest < nTest ; iTest++)
			{
				while(REG_READ(CONFIG_ADDR) != DATA_IDLE){}

				REG_WRITE(CONFIG_ADDR,DATA_START);

				for(iDelay = 0 ; iDelay < nDelay ; iDelay++){}

				if(direction)
				{
					*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_glitch ;  // set DLL glitch value
					for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
					*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_nominal; // get back to DLL normal value
				}
				else
				{
					*(volatile uint32_t*)(DLL_W_ADDR+4*iByte) = reg_glitch ;  // set DLL glitch value
					for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
					*(volatile uint32_t*)(DLL_W_ADDR+4*iByte) = reg_nominal; // get back to DLL normal value
				}

				while(REG_READ(CONFIG_ADDR) != DATA_END){}

				nFault = REG_READ(FAULT_ADDR);
				//xil_printf("\n\rnFault: %d",nDelay);

				if(nFault > 0)
				{
					xil_printf("\n\r**********************************\n\r");
					xil_printf("Min stress found byte %d : %lu\n\r",iByte,iStress);
					xil_printf("**********************************\n\r");
					stress_min[iByte] = iStress;
					REG_WRITE(MODE_ADDR,IDLE_MODE);
					REG_WRITE(CONFIG_ADDR,DATA_IDLE);
					HW_uDelay(1000);
					REG_WRITE(CONFIG_ADDR,DATA_STOP);
					break;
				}

				REG_WRITE(CONFIG_ADDR,DATA_STOP);
			}

			xil_printf("\n\r");
			if(nFault > 0){break;}
		}

	}

    //calibrate for reads max
	for(iByte= 0 ;  iByte < 4 ;  iByte++)
	{
		REG_WRITE(FAULT_ADDR,0);

		for(iStress = 45 ;  iStress < 60 ;  iStress++)
		{
			reg_nominal = (((25 << 11)&0xFF800)| ((1 << 10)&0x400));
			reg_glitch = (((iStress << 11)&0xFF800)| ((1 << 10)&0x400));

			HW_uDelay(10);
			REG_WRITE(CONFIG_ADDR,CRYPT_WAIT);
			REG_WRITE(MODE_ADDR,READ_MODE);

			xil_printf("\n\rStart Read Test on CPU1");
			xil_printf("\n\rtStress: %d",iStress);
			xil_printf("\n\rtWidth: %d",tWidth);
			xil_printf("\n\riByte: %d",iByte);
			xil_printf("\n\rdirection %d",direction);
			xil_printf("\n\rnDelay: %d",nDelay);

			for(iTest = 0 ; iTest < nTest ; iTest++)
			{
				while(REG_READ(CONFIG_ADDR) != DATA_IDLE){}

				REG_WRITE(CONFIG_ADDR,DATA_START);

				for(iDelay = 0 ; iDelay < nDelay ; iDelay++){}

				if(direction)
				{
					*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_glitch ;  // set DLL glitch value
					for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
					*(volatile uint32_t*)(DLL_R_ADDR+4*iByte) = reg_nominal; // get back to DLL normal value
				}
				else
				{
					*(volatile uint32_t*)(DLL_W_ADDR+4*iByte) = reg_glitch ;  // set DLL glitch value
					for(iWidth = 0 ; iWidth < tWidth ; iWidth++){} // control glitch duration
					*(volatile uint32_t*)(DLL_W_ADDR+4*iByte) = reg_nominal; // get back to DLL normal value
				}

				while(REG_READ(CONFIG_ADDR) != DATA_END){}

				nFault = REG_READ(FAULT_ADDR);

				if(nFault > 0)
				{
					xil_printf("\n\r**********************************\n\r");
					xil_printf("Max stress found byte %d : %lu\n\r",iByte,iStress);
					xil_printf("**********************************\n\r");
					stress_max[iByte] = iStress;
					REG_WRITE(MODE_ADDR,IDLE_MODE);
					REG_WRITE(CONFIG_ADDR,DATA_IDLE);
					HW_uDelay(1000);
					REG_WRITE(CONFIG_ADDR,DATA_STOP);
					break;
				}

				REG_WRITE(CONFIG_ADDR,DATA_STOP);
			}

			xil_printf("\n\r");
			if(nFault > 0){break;}
		}
	}

	REG_WRITE(FAULT_ADDR,0);
	REG_WRITE(CONFIG_ADDR,DATA_IDLE);
	HW_uDelay(1000);
	REG_WRITE(CONFIG_ADDR,DATA_STOP);

	xil_printf("byte       | b0 | b1 | b2 | b3 |\n\r");
	xil_printf("min stress | %02d | %02d | %02d | %02d |\n\r",stress_min[0],stress_min[1],stress_min[2],stress_min[3]);
	xil_printf("max stress | %02d | %02d | %02d | %02d |\n\r",stress_max[0],stress_max[1],stress_max[2],stress_max[3]);
	return 0;

}

/****************************************************************************
 * Function: compute AES in CPU1
 * Description:
 ****************************************************************************/
int Play_AES_CPU1(uint32_t * plainArray,uint32_t * cipherArray)
{
	//SELECT AES MODE
	REG_WRITE(MODE_ADDR,AES_MODE);

	// Send plainArray to CPU#1
	REG_WRITE(PLAIN_ADDR+0,plainArray[0]);
	REG_WRITE(PLAIN_ADDR+4,plainArray[1]);
	REG_WRITE(PLAIN_ADDR+8,plainArray[2]);
	REG_WRITE(PLAIN_ADDR+12,plainArray[3]);

	// Send plain ready to CPU#1
	REG_WRITE(CONFIG_ADDR,PLAIN_READY);

	// WAIT FOR CPU1 READY
	while(REG_READ(CONFIG_ADDR) != CRYPT_READY){}

	// LAUNCH CPU1 ENCRYPTION
	REG_WRITE(CONFIG_ADDR,CRYPT_START);

	//WAIT FOR CPU1 END OF ENCRYPTION
	while(REG_READ(CONFIG_ADDR) != CRYPT_END){}

	//SEND END TO CPU1
	REG_WRITE(MODE_ADDR,IDLE_MODE);
	REG_WRITE(CONFIG_ADDR,CRYPT_STOP);

	//WAIT FOR CPU1 CIPHERTEXT
	HW_uDelay(1);

	// Get ciphertext
	cipherArray[0] = REG_READ(CIPHER_ADDR);
	cipherArray[1] = REG_READ(CIPHER_ADDR+4);
	cipherArray[2] = REG_READ(CIPHER_ADDR+8);
	cipherArray[3] = REG_READ(CIPHER_ADDR+12);

	return 1;
}

/****************************************************************************
 * Function: Extract Faulted Cipher Texts
 * Description:
 ****************************************************************************/
int Extract_Faulted_Cipher(uint32_t *cipherText, uint32_t * fcipherText, uint8_t nByte)
{

	uint8_t counterfault = 0;
	uint8_t bytefault = 0;

	for(uint8_t iWord = 0 ; iWord < 4 ; iWord++)
	{
		bytefault = 0;
		for(uint8_t iByte = 0 ; iByte < 4 ; iByte++)
		{
			if(((cipherText[iWord]>>(iByte*8)) & 0xff) != ((fcipherText[iWord]>>(iByte*8)) & 0xff))
			{
				counterfault++;
				bytefault++;
			}

			if(bytefault > 1)
			{
				//piret require 1 fault per word (discard all others)
				counterfault = 16;
				break;
			}
		}
	}

	if(counterfault == nByte)
	{
		xil_printf("\n\rrightfault : %08x%08x%08x%08x",fcipherText[0],fcipherText[1],fcipherText[2],fcipherText[3]);
	}
	else if(counterfault > 0)
	{
		xil_printf("\n\rwrongfault : %08x%08x%08x%08x",fcipherText[0],fcipherText[1],fcipherText[2],fcipherText[3]);
	}
	else
	{
		//print nothing (the cipher is not corrupted)
	}

	return 1;
}

/****************************************************************************
 * Function: Modify Cache State
 * Description: Disable or enable cache memory (L1 and L2) on both processors
 ****************************************************************************/
int Modify_Cache_State(int cacheState)
{
	xil_printf("\n\r******** CACHE ENABLE/DISABLE MODE *******");

	if(cacheState)
	{
		xil_printf("\n\rEnabling data cache CPU1");
		HW_uDelay(100);

		//ENABLE CACHE CPU1
		REG_WRITE(CACHE_ADDR,CACHE);

		//SELECT CACHE MODE
		REG_WRITE(MODE_ADDR,CACHE_MODE);
		HW_uDelay(1000000);

		//ENABLE CACHE CPU0
		xil_printf("\n\rEnabling data cache CPU0");
		Xil_DCacheEnable();
		HW_uDelay(1000000);

		//SELECT IDLE MODE
		REG_WRITE(MODE_ADDR,IDLE_MODE);
		HW_uDelay(10000);
	}
	else
	{
		xil_printf("\n\rDisabling data cache CPU1");
		HW_uDelay(100);

		//DISABLE CACHE CPU1
		REG_WRITE(CACHE_ADDR,NOCACHE);

		//SELECT CACHE MODE
		REG_WRITE(MODE_ADDR,CACHE_MODE);
		HW_uDelay(1000000);

		//DISABLE CACHE CPU0
		xil_printf("\n\rDisabling data cache CPU0");
		Xil_DCacheDisable();
		HW_uDelay(1000000);

		//SELECT IDLE MODE
		REG_WRITE(MODE_ADDR,IDLE_MODE);
		HW_uDelay(10000);
	}
	xil_printf("\n\r\n\r");

	return cacheState;
}


/****************************************************************************
 * Function: Test 256 * 256 sbox values
 * Description: test if a faulted cipher is due to unique sbox modification
 ****************************************************************************/
int Faulted_Sbox_Matching(uint32_t * plainText,uint32_t * fcipherText,uint8_t * sboxIndex, uint8_t * sboxValue)
{
	struct AES_ctx ctx;
	uint8_t match = 0;
	uint8_t p_i = 0;
	uint8_t ptArray[16];
	uint32_t cipherText[4];
	uint8_t keyArray[16] = {0x7d,0xc4,0x04,0x63,0xee,0x8d,0x0d,0x11,0x56,0xda,0x16,0x8a,0x71,0x1d,0xef,0xba};
	//xil_printf("\n\rfaulted[0] = 0x%08x", fcipherText[p_i]);

	// Perform the AES key expension
	AES_init_ctx(&ctx,keyArray);

	// Compute AES with broken SBOX values
	for(uint16_t iSBOX = 0 ; iSBOX < 256; iSBOX++)
	{
		for(uint16_t vSBOX = 0 ; vSBOX < 256 ; vSBOX++)
		{

			// Convert 32-bit plainText to 8-bit
			for(p_i = 0 ; p_i < 4 ; p_i++)
			{
				ptArray[p_i*4+3] = (uint8_t)((plainText[p_i] & 0x000000ff)>>0);
				ptArray[p_i*4+2] = (uint8_t)((plainText[p_i] & 0x0000ff00)>>8);
				ptArray[p_i*4+1] = (uint8_t)((plainText[p_i] & 0x00ff0000)>>16);
				ptArray[p_i*4+0] = (uint8_t)((plainText[p_i] & 0xff000000)>>24);
			}

			// Compute broken AES
			AES_ECB_encrypt(&ctx, ptArray,iSBOX, vSBOX);


			// Convert 8-bit cipherText to 32-bit
			for(p_i = 0 ; p_i < 4 ; p_i++)
			{
				cipherText[p_i] = (((uint32_t)(ptArray[p_i*4+0]) << 24) & 0xff000000) |
							 	 (((uint32_t)(ptArray[p_i*4+1]) << 16) & 0x00ff0000) |
								 (((uint32_t)(ptArray[p_i*4+2]) << 8)  & 0x0000ff00) |
								 (((uint32_t)(ptArray[p_i*4+3]) << 0)  & 0x000000ff);
			}

			//compare dram faulted with sbox faulted
			for(p_i = 0 ; p_i < 4 ; p_i++)
			{
				if(cipherText[p_i] == fcipherText[p_i])
				{
					//xil_printf("\n\r0x%08x = 0x%08x",cipherText[p_i], fcipherText[p_i]);
					match = 1;
				}
				else
				{
					match = 0;
					break;
				}
			}

			//on success return sbox value and index
			if(match)
			{
				*sboxIndex = iSBOX;
				*sboxValue = vSBOX;
				xil_printf("\n\r********* SUCCESS *********");
				xil_printf("\n\rfaulted sbox index: %d",*sboxIndex);
				xil_printf("\n\rfaulted sbox value: 0x%02x",*sboxValue);
				xil_printf("\n\r****************************\n\r");
				return 1;
			}
		}
	}


	return 0;
}


/****************************************************************************
 * Function: Read delay line state
 * Description: returns sample array
 ****************************************************************************/
uint8_t * Read_DLL_State(uint32_t nSample)
{
	uint8_t * dataArray;
	uint32_t DLL_reg;

	dataArray = (uint8_t*)malloc(sizeof(uint8_t)*nSample);

	for (int iSample = 0; iSample < nSample; iSample++)
	{
		DLL_reg = Xil_In32(DLL_addr);
		dataArray[iSample] = (uint8_t)((DLL_reg>>finepos1)&finemask1) +  ((uint8_t)((DLL_reg>>coarsepos1)&coarsemask1)*4) -100;
	    //dataArray[iSample] = (uint8_t)((DLL_reg>>finepos0)&finemask0) + ((uint8_t)((DLL_reg>>coarsepos0)&coarsemask0)*4) -100;
	}

	return dataArray;
}

 /****************************************************************************
  * Function: command helper
  * Description:
  ****************************************************************************/
 void Command_Helper(void)
 {
 	xil_printf("\n\rCommand Helper:");
 	xil_printf("\n\r--------------------------------------------------------------------------");
 	xil_printf("\n\r| cmd   |                   Parameters                 |  Description     |");
 	xil_printf("\n\r|-------------------------------------------------------------------------|");
 	xil_printf("\n\r| help  |                                              |  Command Help    |");
 	xil_printf("\n\r| calib |                                              |  Calibrate       |");
 	xil_printf("\n\r| rtest | <stress> <width> <byte> <direction> <delay>  |  Mem read attack |");
 	xil_printf("\n\r| PFA   | <stress>                                     |  PFA  Attack     |");
 	xil_printf("\n\r| piret |                                              |  Piret Attack    |");
 	xil_printf("\n\r| cache | <state> (1: enable, 0:disable)               |  Cache state     |");
 	xil_printf("\n\r--------------------------------------------------------------------------");
 	xil_printf("\n\r\n\rexample : \"PFA 1\"\n\r\n\r");
 }

