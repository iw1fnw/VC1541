#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <grx20.h>

static struct {
    unsigned char manufacturer;
    unsigned char version;
    unsigned char encoding;
    unsigned char bitsperpixel;
    short int x1, y1;
    short int x2, y2;
    short int hdpi;
    short int vdpi;
    unsigned char colormap[48];
    unsigned char reserved;
    unsigned char nplanes;
    short int bytesperline;
    short int paletteinfo;
    short int hscreensize;
    short  int vscreensize;
    unsigned char filler[54];
} pcx_header;

void encode(FILE *outfile);
void encode_scanline(FILE *outfile, int y);

int pcx_save(void)
{
	int a, x, y;
	int width, height;
	int colors;
	FILE *outfile;
	char buf[100];
	struct stat statbuf;
	unsigned char *pixels;
	unsigned char cols[] = {
		  0,   0,   0,
		255, 255, 255,
		  0, 170,   0,
		  0, 170, 170,
		170,   0,   0,
		170,   0, 170,
		170, 170,   0,
		170, 170, 170,
		 85,  85,  85,
		 85,  85, 255,
		 85, 255,  85,
		 85, 255, 255,
		255,  85,  85,
		255,  85, 255,
		255, 255,  85,
		255, 255, 255
	};

	outfile = NULL;
	for (a = 0;a <= 9999;a++) {
		sprintf(buf, "IEC_%04d.PCX", a);
		if (access(buf, F_OK) != 0) {
			outfile = fopen(buf, "wb");
			break;
		}
	}
	if (outfile == NULL) return 0;

	width  = GrScreenX();
	height = GrScreenY();

	pcx_header.manufacturer = 10;	// ZSoft .PCX
	pcx_header.version = 5;		// Version 2.5
	pcx_header.encoding = 1;	// .PCX run length encoding
	pcx_header.x1 = 0;
	pcx_header.y1 = 0;
	pcx_header.x2 = width - 1;
	pcx_header.y2 = height - 1;
	pcx_header.paletteinfo = 1;
	pcx_header.bitsperpixel = 8;
	pcx_header.nplanes = 1;
	pcx_header.bytesperline = width;
	pcx_header.hdpi = 300;
	pcx_header.vdpi = 300;
	pcx_header.reserved = 0;

	for (a = 0;a < 48;a++) {
 		pcx_header.colormap[a] = cols[a];
	}

	fwrite(&pcx_header, 128, 1, outfile);

	encode(outfile);

	fputc(0x0c, outfile);
	for(a = 0; a <= 255; a++) {
		fputc(GrColorInfo->ctable[a].r, outfile);
		fputc(GrColorInfo->ctable[a].g, outfile);
		fputc(GrColorInfo->ctable[a].b, outfile);
	}

	fclose(outfile);
	return 1;
}

void encode(FILE *outfile)
{
	int y;

	for (y = 0;y < GrScreenY();y++) {
		encode_scanline(outfile, y);
	}
}

void encode_scanline(FILE *outfile, int y)
{
	int x;
	int count;
	unsigned char old;
	unsigned char pixel;

	count = 0;
	for (x = 0;x < GrScreenX();x++) {
		pixel = GrPixel(x, y);
		if (count != 0) {
			if (old != pixel) {
				if ((count > 1) || (old >= 0xc0)) {
					fputc(0xc0 | count, outfile);
				}
				fputc(old, outfile);
				count = 0;
			} else {
				if (count == 15) {
					fputc(0xc0 | count, outfile);
					fputc(old, outfile);
					count = 0;
				}
			}
		}
		count++;
		old = pixel;
	}

	if ((count > 1) || (old >= 0xc0)) {
		fputc(0xc0 | count, outfile);
	}
	fputc(old, outfile);
}

