#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"

int main(int argc, char **argv){

	FILE * fp;
	struct timespec begin,end;
 	struct AES_ctx ctx;
 	uint32_t iTest = 0;
 	uint8_t iByte = 0;
 	uint8_t pt[16] = {0x86,0x20,0x91,0x40,0x39,0xf6,0x8c,0x7c,0x5a,0x33,0x50,0x9a,0xec,0xda,0x18,0x5a}; //-> cipher = 0
	uint8_t aes_key[16] = {0x7d,0xc4,0x04,0x63,0xee,0x8d,0x0d,0x11,0x56,0xda,0x16,0x8a,0x71,0x1d,0xef,0xba};
 	uint32_t nTest = atoi(argv[1]);
 	uint32_t nAttack = atoi(argv[2]);
 	uint8_t aes_pt[nAttack][16];
 	(void)argc;

	srand(0);

	//fill 
  	for(iTest = 0 ; iTest < nAttack ; iTest++)
 	{
 		if(iTest < nTest)
 		{	
	 		for(iByte = 0 ; iByte < 16 ; iByte++)
	 		{
				aes_pt[iTest][iByte] = pt[iByte];
	 		}
 		}
 		else
 		{
 			for(iByte = 0 ; iByte < 16 ; iByte++)
	 		{
				aes_pt[iTest][iByte] = (uint8_t)rand();
	 		}
 		}
 	}

 	//the section we try to fault
	clock_gettime(CLOCK_MONOTONIC_RAW,&begin);	
  	for(iTest = 0 ; iTest < nAttack ; iTest++)
 	{
 		AES_init_ctx(&ctx,aes_key);
 		AES_ECB_encrypt(&ctx,aes_pt[iTest]);
 	}
 	clock_gettime(CLOCK_MONOTONIC_RAW,&end);


 	//print faulted section
	printf("AES begin: %08.2f us\n\r",(double)(begin.tv_nsec)/1000);
 	printf("time spent: %08.2f us\n\r",(double)(end.tv_nsec-begin.tv_nsec)/1000);

 	//write results in file
 	fp = fopen("/home/ciphertexts","w+");

 	if(fp == NULL)
 	{
 		printf("Opening file error\n\r");
 		return 0;
 	}

	fprintf(fp,"\n\n\n\n *********** New Tiny AES Array ********* \n");

  	for(iTest = 0 ; iTest < nAttack ; iTest++)
 	{	
 		if(iTest < nTest)
 		{	
	 		printf("ciphertext: ");

		 	for(iByte  = 0 ; iByte  < 16 ; iByte ++)
		 	{
		 		printf("%02x",aes_pt[iTest][iByte]);
		 	}
	 		printf("\n\r");
	 	}

		//fprintf(fp,"ciphertext: ");
		for(iByte  = 0 ; iByte  < 16 ; iByte ++){
			fprintf(fp,"%02x",aes_pt[iTest][iByte]);
		}
		fprintf(fp,"\n");
 	}

 	fclose(fp);

 	return 1;
}