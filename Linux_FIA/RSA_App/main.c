#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <string.h>

int main(void)
{
	struct timespec begin,end;
	static char keyfile[]  = "/home/private.pem"; // generated using openssl openssl genrsa -out private.pem 2048
	char * message  = "hello!";
	char * encrypt;

	FILE * fp = fopen (keyfile, "r");
	if(fp == NULL){
		printf("error during file opening");
	}
	RSA *private_key = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
	fclose (fp);

	encrypt = malloc(RSA_size(private_key));
	
	clock_gettime(CLOCK_MONOTONIC_RAW,&begin);
	int encrypt_length = RSA_private_encrypt(strlen(message)+1,(unsigned char*)message,(unsigned char*)encrypt,private_key,RSA_PKCS1_PADDING);
	clock_gettime(CLOCK_MONOTONIC_RAW,&end);

	if(encrypt_length == -1)
	{
		printf("encryption error\n\r");
	}

	//print faulted section
	printf("RSA begin: %08.2f us\n\r",(double)(begin.tv_nsec)/1000);
	printf("RSA end: %08.2f us\n\r",(double)(end.tv_nsec)/1000);

	printf("Hex Signature = 0x");
	int i = 0;

	for(i = 0 ; i < encrypt_length ; i++)
	{
		printf("%02x",encrypt[i]);
	}

	printf("\n\r");

	 //write results in file
 	fp = fopen("/home/signed.txt","w+");

 	if(fp == NULL)
 	{
 		printf("Opening file error\n\r");
 		return 0;
 	}

	for(i = 0 ; i < encrypt_length ; i++)
	{
		fprintf(fp,"%02x",encrypt[i]);
	}

 	fclose(fp);
 	
 	RSA_free(private_key);
    
    return 0; 
}
