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

static FILE *fp;
static const char* output_directory_name;

static u8 *get_wad(u32 len)
{
	u32 rounded_len;
	u8 *p;

	rounded_len = round_up(len, 0x40);
	p = (u8*)malloc(rounded_len);
	if (p == 0)
		fatal("malloc");
	if (len)
		if (fread(p, rounded_len, 1, fp) != 1)
			fatal("get_wad read, len = %x", len);

	return p;
}

static void do_app_file(u8 *app, u8 *tik, u8 *tmd)
{
	u8 title_key[16];
	u8 iv[16];
	u32 i;
	u8 *p;
	u32 len;
	u32 rounded_len;
	u32 num_contents;
	u32 cid;
	u16 index;
	u16 type;
	char name[17];
	FILE *fp;

	decrypt_title_key(tik, title_key);

	sprintf(name, "%016llx", be64(tmd + 0x018c));

	const char* directory_name = output_directory_name != NULL
		? output_directory_name
		: name;
#ifdef _MSC_VER
	mkdir(directory_name);
#else
	mkdir(directory_name, 0777);
#endif
	if (chdir(directory_name))
		fatal("chdir %s", directory_name);

	num_contents = be16(tmd + 0x01de);
	p = app;

	for (i = 0; i < num_contents; i++) {
		cid = be32(tmd + 0x01e4 + 0x24*i);
		index = be16(tmd + 0x01e8 + 0x24*i);
		type = be16(tmd + 0x01ea + 0x24*i);
		len = be64(tmd + 0x01ec + 0x24*i);
		rounded_len = round_up(len, 0x40);
		fprintf(stderr, "--- cid=%08x index=%04x type=%04x len=%08x\n",
		        cid, index, type, len);

		memset(iv, 0, sizeof iv);
		memcpy(iv, tmd + 0x01e8 + 0x24*i, 2);
		aes_cbc_dec(title_key, iv, p, rounded_len, p);

		sprintf(name, "%08x.app", index);
		fp = fopen(name, "wb");
		if (fp == 0)
			fatal("open %s", name);
		if (fwrite(p, len, 1, fp) != 1)
			fatal("write %s", name);
		fclose(fp);

		p += rounded_len;
	}

	if (chdir(".."))
		fatal("chdir ..");
}

static void do_install_wad(u8 *header)
{
	u32 header_len;
	u32 cert_len;
	u32 tik_len;
	u32 tmd_len;
	u32 app_len;
	u32 trailer_len;
	u8 *cert;
	u8 *tik;
	u8 *tmd;
	u8 *app;
	u8 *trailer;
	char name[25];

	header_len = be32(header);
	if (header_len != 0x20)
		fatal("bad install header length (%x)", header_len);

	cert_len = be32(header + 8);
	// 0 = be32(header + 0x0c);
	tik_len = be32(header + 0x10);
	tmd_len = be32(header + 0x14);
	app_len = be32(header + 0x18);
	trailer_len = be32(header + 0x1c);

	cert = get_wad(cert_len);
	tik = get_wad(tik_len);
	tmd = get_wad(tmd_len);
	app = get_wad(app_len);
	trailer = get_wad(trailer_len);
	
	// File Dump
	// Create/Select Folder
	sprintf(name, "%016llx", be64(tmd + 0x018c));
	const char* directory_name = output_directory_name != NULL
		? output_directory_name
		: name;
#ifdef _MSC_VER
	mkdir(directory_name);
#else
	mkdir(directory_name, 0777);
#endif
	if (chdir(directory_name))
		fatal("chdir %s", directory_name);
	// File Dump
	sprintf(name, "%016llx.cert", be64(tmd + 0x018c));
	FILE *cf = fopen(name, "wb");
	fwrite(cert, cert_len, 1, cf);
	fclose(cf);
	
	if (trailer_len>0) {
	sprintf(name, "%016llx.trailer", be64(tmd + 0x018c));
	cf = fopen(name, "wb");
	fwrite(trailer, trailer_len, 1, cf);
	fclose(cf);
	}
	sprintf(name, "%016llx.tmd", be64(tmd + 0x018c));
	cf = fopen(name, "wb");
	fwrite(tmd, tmd_len, 1, cf);
	fclose(cf);

	sprintf(name, "%016llx.tik", be64(tmd + 0x018c));
	cf = fopen(name, "wb");
	fwrite(tik, tik_len, 1, cf);
	fclose(cf);

	fprintf(stderr, "ticket:\n");
	hexdump(tik, tik_len);
	fprintf(stderr, "tmd:\n");
	dump_tmd(tmd);
	fprintf(stderr, "cert chain:\n");
	hexdump(cert, cert_len);

	/*printf("Normal sign check...\n");
	ret = check_cert_chain(tik, tik_len, cert, cert_len);
	if (ret)
		fprintf(stderr, "ticket cert failure (%d)\n", ret);

	ret = check_cert_chain(tmd, tmd_len, cert, cert_len);
	if (ret)
		fprintf(stderr, "tmd cert failure (%d)\n", ret);
	printf("Trucha sign check...\n");
	ret = check_cert_chain_trucha(tik, tik_len, cert, cert_len);
	if (ret)
		fprintf(stderr, "ticket cert failure (%d)\n", ret);

	ret = check_cert_chain_trucha(tmd, tmd_len, cert, cert_len);
	if (ret)
		fprintf(stderr, "tmd cert failure (%d)\n", ret);*/
	
	
	if (chdir(".."))
		fatal("chdir ..", name);
	do_app_file(app, tik, tmd);
}

static void do_wad(void)
{
	u8 header[0x80];
	u32 header_len;
	u32 header_type;

	if (fread(header, 0x40, 1, fp) != 1) {
		if (!feof(fp))
			fatal("reading wad header");
		else
			return;
	}
	header_len = be32(header);
	if (header_len >= 0x80)
		ERROR("wad header too big\n");
	if (header_len >= 0x40)
		if (fread(header + 0x40, 0x40, 1, fp) != 1)
			fatal("reading wad header (2)");
fprintf(stderr, "wad header:\n");
hexdump(header, header_len);

	header_type = be32(header + 4);
	switch (header_type) {
	case 0x49730000:
		do_install_wad(header);
		break;
	case 0x69620000:
		do_install_wad(header);
		break;
	default:
		fatal("unknown header type %08x", header_type);
	}
}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 4) {
		printf("USAGE: wadunpacker wad_file [directory_name] [common_key_path]\n");
		exit(-1);
	}
	fp = fopen(argv[1], "rb");
	if (!fp) {
		printf("Cannot open file %s.\n", argv[1]);
		exit(-1);
	}

	if (argc >= 3) {
		output_directory_name = argv[2];
	}

	if (argc >= 4) {
		load_common_key(argv[3]);
	}
	
	while (!feof(fp))
		do_wad();

	fclose(fp);

	return 0;
}
