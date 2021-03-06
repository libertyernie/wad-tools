// wad-tools
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
// Based on BFGR WadTools v0.39a by BFGR based on Zeventig by Segher

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tools.h"

#define ERROR(s) do { fprintf(stderr, s "\n"); exit(1); } while (0)

void printUsage(char *u) {
	printf("USAGE: %s ticket_file tmd_file cert_file file.wad [ options ]\n\n", u);
	printf("Options: -sign\t\tTrucha sign Ticket and TMD\n");
	printf("         -T\t\tTrucha sign Ticket\n");
	printf("         -M\t\tTrucha sign TMD\n");
	printf("         -i ABCD\tChange the title id\n");
	printf("         -k file.bin\tSet path to common-key\n");
	printf("         -w\t\tUses a compatible generic ticket.\n");
	printf("         -e\t\tDisable content encryption.\n\n");
	exit(-1);
}
 
int main(int argc, char **argv) {
	FILE *ftmd;
	FILE *ftik;
	FILE *fapp;
	FILE *fcert;
	FILE *ftrailer;
	u8 title_key[16];
	u8 *tmd;
	u8 *cert;
	u8 *trailer;
	u8 *tik;
	u8 *apps = NULL; //Encrypted files, with 64 bytes boundary
	u32 len_tmd;
	u32 len_tmd_nb;
	u32 len_tik;
	u32 len_tik_nb;
	u16 num_app;
	u32 len_cert;
	u32 len_cert_nb;
	u32 len_trailer;
	u32 len_trailer_nb;
	u64 len_apps = 0;
	char name[17];
	u8 hash[20];
	u16 i;
	u64 temp, temp2;
	u8 iv[16];
	u8 sign_tik = 0;
	u8 sign_tmd = 0;
	u8 sign_type = 1; // Sign type. if 1 watermark enabled. if 0 watermark disabled
	u8 encrypt_apps = 1;
	char *new_id = NULL;
	
	if (argc<5) {
		printUsage(argv[0]);
	}
	if (argc>5) {
		for(i=6;i<=argc;i++) {
			if (strcmp(argv[i-1], "-sign")==0) { // Trucha sign Ticket and TMD
				sign_tik = 1;
				sign_tmd = 1;
				printf("Trucha sign enabled\n");
			} else if (strcmp(argv[i-1], "-T")==0) { // Trucha sign Ticket
				sign_tik = 1;
				printf("Trucha sign enabled for Ticket\n");
			} else if (strcmp(argv[i-1], "-M")==0) { // Trucha Sign TMD
				sign_tmd = 1;
				printf("Trucha sign enabled for TMD\n");
			} else if (strcmp(argv[i-1], "-i")==0) { // Change title id
				if (strlen(argv[i])==4) {
					new_id = (char *)malloc(4);
					memset(new_id, 0, 4);
					strncpy(new_id, argv[i], 4);
					i++;
					printf("New Title ID: %s\n", new_id);
				} else {
					printUsage(argv[0]);
				}
			} else if (strcmp(argv[i-1], "-k")==0) { // Change common-key.bin path
				load_common_key(argv[i]);
			} else if (strcmp(argv[i-1], "-w")==0) { // Disable watermark
				sign_type = 0;
				printf("A generic ticket will be used.\n");
			} else if (strcmp(argv[i-1], "-e")==0) { // Disable .app encryption
				encrypt_apps = 0;
				printf("Contents will not be encrypted.\n");
			}
			fflush(stdout);
		}
	}
	//CERT
	printf("Cert... ");
	fcert = fopen(argv[3], "rb"); if (!fcert) {printf("Could not find cert_file (%s)\n", argv[3]); exit(-1);}
	temp = getfilesize(fcert);
	len_cert_nb = temp;
	len_cert = round_up(temp, 0x40);
	cert = (u8*)malloc(len_cert);
	memset(cert, 0, len_cert);
	fread(cert, temp, 1, fcert);
	fclose(fcert);
	printf("OK\n");
	//TRAILER
	printf("Trailer... ");
	ftrailer = fopen("00000000.app", "rb"); if (!ftrailer) {printf("Could not find trailer_file (00000000.app)\n"); exit(-1);}
	temp = getfilesize(ftrailer);
	len_trailer_nb = temp;
	len_trailer = round_up(temp, 0x40);
	trailer = (u8*)malloc(len_trailer);
	memset(trailer, 0, len_trailer);
	fread(trailer, temp, 1, ftrailer);
	fclose(ftrailer);
	printf("OK\n");
	//TMD
	printf("TMD... ");
	ftmd = fopen(argv[2], "rb"); if (!ftmd) {printf("Could not find tmd_file (%s)\n", argv[2]); exit(-1);}
	temp = getfilesize(ftmd);
	len_tmd_nb = temp;
	len_tmd = round_up(temp, 0x40);
	tmd = (u8*)malloc(len_tmd);
	memset(tmd, 0, len_tmd);
	fread(tmd, temp, 1, ftmd);
	num_app = be16(tmd + 0x01de);
	fclose(ftmd);
	printf("OK\n");
	printf("There are %d files described on the TMD.\n", num_app);
	// Ticket
	printf("Ticket... ");
	ftik = fopen(argv[1], "rb"); if (!ftik) {printf("Could not find ticket_file (%s)\n", argv[1]); exit(-1);}
	temp = getfilesize(ftik);
	len_tik = round_up(temp, 0x40);
	len_tik_nb = temp;
	tik = (u8*)malloc(len_tik);
	memset(tik, 0, len_tik);
	fread(tik, temp, 1, ftik);
	fclose(ftik);
	printf("OK\n");
	// Change title id if required
	if (new_id!=NULL) {
		memcpy(tmd + 0x0190, new_id, 4);
		memcpy(tik + 0x01E0, new_id, 4);
	}
	// Sign
	if (sign_tik) {
		printf("Signing Ticket... ");
		int ok = Ticket_resign(tik, len_tik_nb, sign_type);
		printf("result code %d\n", ok);
	}

	if(encrypt_apps) {
		// Get Title key
		decrypt_title_key(tik, title_key);
	}

	// Read app files
	for (i=0;i<num_app;i++) {
		sprintf(name, "%08x.app", i);
		printf("%s... ", name);
		fapp = fopen(name, "rb");
		if (!fapp) {
			printf("\nERROR: Could not find %s file.\n", name);
			printf("File TMD Description:\n");
			printf("File size: %lld bytes\n", be64(tmd + 0x01ec + 0x24*i));
			printf("File SHA Hash: 0x");
			printHashSHA(tmd + 0x01F4 + (0x24*i));
			printf("\n");
			exit(-1);
		}
		temp = getfilesize(fapp);
		temp2 = round_up(temp, 0x40);
		len_apps += temp2;
		apps = (u8 *)realloc(apps, len_apps);
		memset(apps+len_apps-temp2, 0, temp2);
		fread(apps+len_apps-temp2, temp, 1, fapp);
		fclose(fapp);

		if(encrypt_apps) {
			// SHA hash update
			sha(apps+len_apps-temp2, temp, hash);
			memcpy(tmd + 0x1F4 + (0x24*i), hash, 20);
			// File size update
			wbe64(tmd + 0x1EC + (0x24*i), temp);
			// Encrypt file
			memset(iv, 0, sizeof iv);
			memcpy(iv, tmd + 0x01e8 + 0x24*i, 2);
			aes_cbc_enc(title_key, iv, apps+len_apps-temp2, round_up(temp, 0x10), apps+len_apps-temp2);
		}
		printf("OK\n");
	}

	// Sign
	if (sign_tmd) {
		printf("Signing TMD... ");
		TMD_resign(tmd, len_tmd_nb);
		printf("OK\n");
	}
 	// Header
	printf("Header... ");
	u8 *header = (u8 *)malloc(0x40);
	memset(header, 0, 0x40);

	wbe32(header, 0x20); // Header size
	wbe32(header + 0x4, 0x49730000); // Header type
	wbe32(header + 0x8, len_cert_nb); // Cert length
	wbe32(header + 0xC, 0x00000000);
	wbe32(header + 0x10, len_tik_nb); // Ticket length
	wbe32(header + 0x14, len_tmd_nb); // TMD length
	wbe32(header + 0x18, len_apps); // APP length
	wbe32(header + 0x1C, len_trailer_nb); // Trailer length
	printf("OK\n");

	//Write final WAD file
	printf("Writing %s file... ", argv[4]);
	fapp = fopen(argv[4], "wb");
	if (!fapp) {
		printf("\nCould not open destination file.\n");
	}
	fwrite(header, 0x40, 1, fapp);
	fwrite(cert, len_cert, 1, fapp);
	fwrite(tik, len_tik, 1, fapp);
	fwrite(tmd, len_tmd, 1, fapp);
	fwrite(apps, len_apps, 1, fapp);
	fwrite(trailer, len_trailer, 1, fapp);
	fclose(fapp);
	printf("OK\n");
}
