#ifdef DJGPP
#include <pc.h>
#endif /* DJGPP */

#define HAVE_TSC

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <grx20.h>

#include "misc/pcx.h"
#include "misc/util.h"
#include "misc/debug.h"
#include "misc/cmdline.h"
#include "misc/profile.h"

#include "vc1541/lptdetct.h"

#define ATN_BIT         0
#define ATN             (1 << ATN_BIT)
#define CLOCK_BIT       1
#define CLOCK           (1 << CLOCK_BIT)
#define DATA_BIT        3
#define DATA            (1 << DATA_BIT)

static int _mhz = 200;

static char *text_1[] = {
	"Keys:",
        "",
        "    Home        : set displayed start time to zero",
        "    Cursor Right: increase displayed start time",
        "    Cursor Left : decrease displayed start time",
        "    Cursor Down : zoom in",
        "    Cursor Up   : zoom out",
        "",
        "    +           : move marker one pixel right",
        "    -           : move marker one pixel left",
        "    Page Up     : move marker 20 pixel right",
        "    Page Down   : move marker 20 pixel left",
        "",
        "    Space       : set displayed start time to marker position",
        "    n           : move marker to next low-high transition",
        "    s           : scan data byte",
        "",
        "    F1          : this help",
        "    F2          : short description of IEC command bytes",
        "    F3          : screen dump (IEC_xxxx.pcx)",
        "",
	"press a key...",
        NULL
};

static char *text_2[] = {
       	"IEC Command Byte (Command Mask 0xf0, Device Mask 0x0f):",
	"",
	"    IEC_LISTEN   = 0x20",
	"    IEC_UNLISTEN = 0x30",
	"    IEC_TALK     = 0x40",
	"    IEC_UNTALK   = 0x50",
	"",
	"",
	"IEC Secondary Address (Mode Mask 0xf0, Channel Mask 0x0f):",
	"",
	"    IEC_DATA     = 0x60",
	"    IEC_CLOSE    = 0xe0",
	"    IEC_OPEN     = 0xf0",
	"",
	"",
	"press a key...",
	NULL
};

static char *text_3[] = {
	"pcx file saved",
        "",
        "press a key...",
        NULL
};

static bool constrain_lpt(const char *name, const char *arg)
{
	char *endp;

	int val = strtol(arg, &endp, 0);

	if ((val >= 1) && (val <= 4) && (endp[0] == '\0')) return true;
	debug->form("* Warning: invalid argument (%s) for option '%s'!\n",
		    arg, name);
	debug->form("           ignoring this option.\n");
	debug->form("           valid arguments: 1 - 4.\n");
	return false;
}

static bool constrain_mhz(const char *name, const char *arg)
{
	char *endp;

	int val = strtol(arg, &endp, 0);

	if ((val >= 33) && (val <= 1000) && (endp[0] == '\0')) return true;
	debug->form("* Warning: invalid argument (%s) for option '%s'!\n",
		    arg, name);
	debug->form("           ignoring this option.\n");
	debug->form("           valid arguments: 33 - 1000.\n");
	return false;
}

CmdArguments CmdLineArgs::_arg[] = {
	/*
	 *  name,  has_arg,  is_given,  arg, constrain_func
	 */
	{ "-lpt",      true,    false, NULL, constrain_lpt },
	{ "-mhz",      true,    false, NULL, constrain_mhz },
	{ "-print",   false,    false, NULL, NULL },
	{ "-h",       false,    false, NULL, NULL },
	{ NULL,       false,    false, NULL, NULL }
};

#if 0
inline unsigned long long tsc_read(void)
{
	union {
        	unsigned long long l;
                struct {
                	unsigned int lo;
                        unsigned int hi;
                } lh;
	} val;

	__asm__ __volatile__ (
		"rdtsc\n\t"          /* .byte 0x0f, 0x31 */
		: "=a" (val.lh.lo),  /* EAX, 0 */
		  "=d" (val.lh.hi)   /* EDX, 1 */
		: /* no inputs */
		: "eax", "edx"
          );

        return val.l;
}
#endif

static const int NR_OF_PORTS = 6;
static const int port_address[NR_OF_PORTS]={
	0x3bc,
	0x378,
	0x278,
	0x268,
	0x27c,
	0x26c
};
static const char *modes[6]={
	"N/A",
	"SPP",
	"PS/2",
	"EPP",
	"EPPc",
	"ECP"
};
static const char *ecpM[9]={
	"no ECR found",
	"Standard Mode",
	"Byte Mode",
	"Parallel Port FIFO Mode",
	"ECP FIFO Mode",
	"EPP Mode",
	"Reserved",
	"FIFO Test Mode",
	"Configuration Mode"
};

static int findBIOSPort(int lpt)
{
	int addr = BIOSlpt2port(lpt);
	if (addr == 0) return -lpt;
	return addr;
}

static int findPort(int lpt) {
	int a, port;
	lptMode mode;

	if (lpt < 0) {
		debug->form("- trying LPT%d...\n", -lpt);
		port = findBIOSPort(-lpt);
		if (port < 0) {
			debug->form("* parallel port LPT%d is not available!\n",
				    -lpt);
			return 0;
		}
		debug->form("+ using parallel port at %03xh.\n", port);
		return port;
	} else if (lpt == 0) {
	        *debug << "- scanning parallel ports...\n";
		for(a = 0;a < NR_OF_PORTS;a++) {
	                port = port_address[a];

			mode = LPTmode(port);

	                if (mode == lptSPP) {
				outportb(port, 0x00);
				if (!(inportb(port + 1) & 0x08)) {
					// 1. test passed
					outportb(port, 0x01);
					if (inportb(port + 1) & 0x08) {
						// 2. test passed
						outportb(port, 0x00);
						if (!(inportb(port + 1) & 0x08)) {
							// 3. test passed
					                debug->form("+ using detected cable at %03xh.\n", port);
	                                                return port;
						}
					}
	                        }
			}
		}
               	*debug << "* no cable detected.\n";
	        return 0;
	} else {
		debug->form("+ using parallel port at %03xh.\n", port);
		return port;
	}
}

static const int BUFFER_SIZE = 65536;
static unsigned char *port_val;
static unsigned int   _idx;
static profile_value *tsc_val;

void readPort(int port)
{
	unsigned char val, v;

	outportb(port + 2, 0x04); // all IEC-lines to high

        *debug << "+ listening (ENTER to abort)..." << flush;

	val = inportb(port + 2) & 0x0b;
        port_val[0] = val;
        tsc_read(&tsc_val[0]);
	for (_idx = 1;_idx < BUFFER_SIZE;_idx++) {
        	do {
                	if (kbhit()) {
			        getchar();
                                _idx--;
			        tsc_val[0].ll = tsc_val[1].ll - _mhz * 50;
                		return;
                        }
	        	v = inportb(port + 2) & 0x0b;
                } while (val == v);
                port_val[_idx] = val = v;
                tsc_read(&tsc_val[_idx]);
        }

        tsc_val[0].ll = tsc_val[1].ll - _mhz * 50;
}

void detectProcessorSpeed(int mhz)
{
	profile_value p1, p2;
        int mhz1, mhz2;

	if (mhz != 0) {
		_mhz = mhz;
		debug->form("+ processor speed set to %dMHz.\n", mhz);
		return;
	}

	*debug << "- calibrating delay loop." << flush;
        sleep(1);

        *debug << "." << flush;
	tsc_read(&p1); sleep(1); tsc_read(&p2);
        mhz1 = (p2.ll - p1.ll) / 1000000;

        *debug << "." << flush;
	tsc_read(&p1); sleep(2); tsc_read(&p2);
        mhz2 = (p2.ll - p1.ll) / 1000000;

        _mhz = (5 * mhz2 - 4 * mhz1) / 6 + 1;

        *debug << " done.\n";
        *debug << "+ processor seems to run at " << dec << _mhz << "MHz.\n";
}

void print_info()
{
	float max_time = (tsc_val[_idx].ll - tsc_val[0].ll) / (_mhz * 1000);

	cout << endl;
        cout << "  buffer size: " << BUFFER_SIZE << endl;
	cout << "  samples:     " << _idx << endl;
        cout << "  time:        " << max_time << " ms" << endl << endl;
        cout << "  press ENTER" << flush;
        getch();
}

void print(void)
{
	int a, b;
        float abs_time;

        printf("                                   ATN        CLOCK        DATA     \n");
        for (a = 1;a < _idx;a++) {
        	int lines = 1;
        	unsigned char diff;
        	long long tsc_diff = tsc_val[a].ll - tsc_val[a - 1].ll;
                float tsc_time = tsc_diff / (float)_mhz;
                float abs_time = (tsc_val[a - 1].ll - tsc_val[0].ll) / (_mhz * 1000.0);
                char *base = "ns";

                if (tsc_time > 100) lines += 2;

		if (tsc_time > 1000) {
                	tsc_time /= 1000;
                        base = "ms";
                        lines += 2;
                }
                if (tsc_time > 5000) {
                	tsc_time = 0;
                        lines += 5;
                }

                for (b = 0;b < lines;b++) {
	                printf("                                ");
	        	printf((port_val[a - 1] & ATN)   ? "         |  " :  "|           ");
	                printf((port_val[a - 1] & CLOCK) ? "         |  " :  "|           ");
	                printf((port_val[a - 1] & DATA)  ? "         |  " :  "|           ");
	                printf("\n");
                        if (b == 3) {
		                printf("                                ");
		        	printf((port_val[a - 1] & ATN)   ? "   ATN   |  " :  "|  ATN      ");
		                printf((port_val[a - 1] & CLOCK) ? "  CLOCK  |  " :  "| CLOCK     ");
		                printf((port_val[a - 1] & DATA)  ? "   DATA  |  " :  "|  DATA     ");
		                printf("\n");
                        }
                }

        	// printf("%02x - ", port_val[a]);
                // printf("[%08x] - ", tsc_diff);
                printf("%12.4f ms - ", abs_time);
                if (tsc_time == 0) {
                	printf("          > 5s - ");
                } else {
                	printf(" %7.2f %s - ", tsc_time, base);
		}

                diff = port_val[a - 1] ^ port_val[a];
                printf((diff & ATN)   ? "+--------+  " :
                		((port_val[a] & ATN)   ? "         |  " :  "|           "));
                printf((diff & CLOCK) ? "+--------+  " :
                		((port_val[a] & CLOCK) ? "         |  " :  "|           "));
                printf((diff & DATA)  ? "+--------+  " :
                		((port_val[a] & DATA)  ? "         |  " :  "|           "));
                printf("\n");
        }
}

void help(char **text, int wait)
{
	int a, b, len;
        int width, height;

        width = 0;
        height = 0;
        for (a = 0;text[a] != NULL;a++) {
        	len = strlen(text[a]);
		b = GrFontStringWidth(&GrDefaultFont, text[a], len, GR_BYTE_TEXT);
                if (b > width) width = b;
		b = GrFontStringHeight(&GrDefaultFont, text[a], len, GR_BYTE_TEXT);
                if (b > height) height = b;
        }
        b = a;

	GrFilledBox(90, 90, 119 + width, 119 + b * height, GrBlack());
        for (a = 0;text[a] != NULL;a++) {
		GrTextXY(100, 100 + a * height, text[a], GrWhite(), GrBlack());
        }

        if (wait) getch();
}

char *dec_to_bin(int val)
{
	int a;
	static char buf[9]; /* !!! */

        for (a = 0;a < 8;a++) {
        	buf[a] = (val & (128 >> a)) ? '1' : '0';
        }

        return buf;
}

void display(void)
{
        int a;
        int key;
        int col;
        int col_scan;
        int x0, x1, inc, t, div;
        int scale = _mhz * 3;
        int marker_x;
        int move_marker;
        int scan_data;
        int scan_data_value;
        int width, height;
        int atn0, data0, clock0;
        int atn1, data1, clock1;
        unsigned long long start;
	GrLineOption line_opts;
	float time;
        char buf[1024];
        char *t_str[] = {
        	"æs", "ms", "s"
        };
        char *command[16] = {
        	"??? (0x00) ",
                "??? (0x10) ",
                "LISTEN (0x20), Device ",
                "UNLISTEN (0x30), Device ",
                "TALK (0x40), Device ",
                "UNTALK (0x50), Device ",
                "DATA (0x60), Channel ",
                "??? (0x70) ",
                "??? (0x80) ",
                "??? (0x90) ",
                "??? (0xA0) ",
                "??? (0xB0) ",
                "??? (0xC0) ",
                "??? (0xD0) ",
                "CLOSE (0xE0), Channel ",
                "OPEN (0xF0), Channel ",
        };

        GrSetMode(GR_width_height_color_graphics, 1024, 768, 16);

	width  = GrScreenX();
	height = GrScreenY();

	marker_x = 0;
        start = tsc_val[0].ll;
        col = GrAllocColor(255, 0, 0);
        col_scan = GrAllocColor(255, 0, 255);
        line_opts.lno_color = GrAllocColor(0, 0, 255);
        line_opts.lno_width = 1;
        line_opts.lno_pattlen = 2;
        line_opts.lno_dashpat = (unsigned char *)"\5\5";

        move_marker = 0;
	scan_data = 0;
	scan_data_value = 0xff;

        do {
		GrFilledBox(0, 0, width - 1, height - 1, GrWhite());

              	/*
                 *  draw grid
                 */
                a = 0;
                t = 0;
                x0 = 0;
                inc = 100;
                div = 1;
                while ((inc * _mhz) / scale < 100) {
                	inc *= 2;
                        if ((inc / div) > 1000) {
                        	t++;
                        	div += 1000;
                        }
                }
	        while (x0 < GrScreenX()) {
                	GrCustomLine(x0, 0, x0, GrScreenY() - 1, &line_opts);
	                sprintf(buf, "%d%s", a / div, t_str[t]);
			GrTextXY(x0 + 4, GrScreenY() - 30, buf, GrBlack(), GrWhite());
                        a += inc;
                        x0 = (a * _mhz) / scale;
                }

                /*
                 *  draw data
                 */
		a = 0;
                x0 = 0;

                while (start > tsc_val[a + 1].ll) a++;

		atn0   = (port_val[a] & ATN)   != 0;
	        data0  = (port_val[a] & DATA)  != 0;
		clock0 = (port_val[a] & CLOCK) != 0;

                x1 = (tsc_val[a + 1].ll - start) / scale;

	        while (x1 < GrScreenX()) {
                	if (kbhit()) break;

                        if ((move_marker == 1) && (marker_x < x1)) {
                        	move_marker = 2;
                        }

                        if ((scan_data == 1) && (marker_x < x1)) {
                        	scan_data = 2;
                        }

			atn1   = (port_val[a + 1] & ATN)   != 0;
		        data1  = (port_val[a + 1] & DATA)  != 0;
			clock1 = (port_val[a + 1] & CLOCK) != 0;

                        if ((clock0 == 1) && (clock1 == 0)) {
	                        if (move_marker == 2) {
	                        	move_marker = 0;
	                                marker_x = x1;
	                        }

	                        if ((scan_data >= 2) && (scan_data <= 9)) {
                                        scan_data_value >>= 1;
                                        scan_data_value |= (data0 & 1) << 7;
                                        GrLine(x1, 270, x1, 390, col_scan);
                                        sprintf(buf, "%d", (~data0) & 1);
			                GrTextXY(x1 + 4, 270, buf, col_scan, GrWhite());
                                        sprintf(buf, "%d", scan_data - 2);
			                GrTextXY(x1 + 4, 390, buf, col_scan, GrWhite());
	                        	scan_data++;
	                        }
                        }

	               	GrLine(x0, 100 + 80 * atn0,   x1, 100 + 80 * atn0,   GrBlack());
	               	GrLine(x0, 300 + 80 * data0,  x1, 300 + 80 * data0,  GrBlack());
	               	GrLine(x0, 500 + 80 * clock0, x1, 500 + 80 * clock0, GrBlack());

	               	GrLine(x1, 100 + 80 * atn0,   x1, 100 + 80 * atn1,   GrBlack());
	               	GrLine(x1, 300 + 80 * data0,  x1, 300 + 80 * data1,  GrBlack());
	               	GrLine(x1, 500 + 80 * clock0, x1, 500 + 80 * clock1, GrBlack());

	                x0     = x1;
	                atn0   = atn1;
	                data0  = data1;
	                clock0 = clock1;

			a++;
                        x1 += (tsc_val[a + 1].ll - tsc_val[a].ll) / scale;
	        }

                move_marker = 0;
                scan_data = 0;

               	GrLine(x0, 100 + 80 * atn0,   GrScreenX() - 1, 100 + 80 * atn0,   GrBlack());
               	GrLine(x0, 300 + 80 * data0,  GrScreenX() - 1, 300 + 80 * data0,  GrBlack());
               	GrLine(x0, 500 + 80 * clock0, GrScreenX() - 1, 500 + 80 * clock0, GrBlack());

                GrTextXY(10, 190, "ATN",   GrBlack(), GrWhite());
                GrTextXY(10, 390, "DATA",  GrBlack(), GrWhite());
                GrTextXY(10, 590, "CLOCK", GrBlack(), GrWhite());

                sprintf(buf, "Samples     : %d out of %d", _idx, BUFFER_SIZE);
                GrTextXY(10, 10, buf, GrBlack(), GrWhite());
		time = (float)(start - tsc_val[0].ll) / _mhz;
                t = 0;
                while (time > 1000.0) {
                	time /= 1000.0;
                        t++;
                }
                sprintf(buf, "Start Time  : %.3f%s", time, t_str[t]);
		GrTextXY(10, 30, buf, GrBlack(), GrWhite());
                sprintf(buf, "Scanned Data: 0x%02X (%dd, %sb) [%s%d]",
                	~scan_data_value & 0xff,
                        ~scan_data_value & 0xff,
                        dec_to_bin(~scan_data_value & 0xff),
			command[(~scan_data_value & 0xf0) >> 4],
                        ~scan_data_value & 0x0f);
                GrTextXY(10, 50, buf, GrBlack(), GrWhite());

		GrLine(marker_x, 0, marker_x, height - 1, col);

                key = getch();

                switch (key) {
                case '+':
                	marker_x++;
                        if (marker_x >= width) marker_x = width - 1;
                        break;
                case '-':
                	marker_x--;
                        if (marker_x < 0) marker_x = 0;
                        break;
                case ' ':
                	start += marker_x * scale;
                        marker_x = 0;
                        break;
                case 'n':
                	move_marker = 1;
                        break;
                case 's':
                	scan_data = 1;
			scan_data_value = 0;
                        break;
                case 0:
                	key = getch();
                        switch (key) {
                        case 0x3b: /* F1 */
                        	help(text_1, 1);
                                break;
                        case 0x3c: /* F2 */
                        	help(text_2, 1);
                                break;
			case 0x3d: /* F3 */
				pcx_save();
                                help(text_3, 1);
                                break;
                        case 0x47: /* HOME */
                        	start = tsc_val[0].ll;
                                break;
                        case 0x4d: /* RIGHT */
                        	start += 50 * scale;
                                break;
                        case 0x4b: /* LEFT */
                        	start -= 50 * scale;
                                if (start < tsc_val[0].ll) start = tsc_val[0].ll;
                                break;
                        case 0x48: /* UP */
                        	scale += _mhz;
                        	break;
                        case 0x50: /* DOWN */
                        	scale -= _mhz;
                                if (scale < _mhz) scale = _mhz;
                        	break;
                        case 0x49: /* PG UP */
	                	marker_x += 20;
	                        if (marker_x >= width) marker_x = width - 1;
                                break;
                        case 0x51: /* PG DOWN */
        	        	marker_x -= 20;
	                        if (marker_x < 0) marker_x = 0;
                        }
                }

	} while (key != 0x1b);
}

/*
IEC_COMMAND  = 0xf0;
IEC_DEVICE   = 0x0f;
IEC_UNTALK   = 0x50;
IEC_UNLISTEN = 0x30;
IEC_LISTEN   = 0x20;
IEC_TALK     = 0x40;
IEC_MODE     = 0xf0;
IEC_ADDRESS  = 0x0f;
IEC_OPEN     = 0xf0;
IEC_CLOSE    = 0xe0;
IEC_DATA     = 0x60;
*/

int main(int argc, char **argv)
{
	int port, mhz, lpt;

        if (sizeof(unsigned long long) != 8) {
        	*debug << "* sizeof(unsigned long long) != 8" << endl;
                return 1;
        }

        CmdLineArgs *args = new CmdLineArgs(&argc, &argv);

	if (args->isGiven("-h")) {
		*debug << endl
		       << "usage: " << args->programName() << " [option]" << endl
		       << endl
		       << "  -print :  no graphics, only text output" << endl
		       << "  -lpt x :  use parallel port LPTx" << endl
		       << "  -mhz x :  don't detect processor speed, set it to x" << endl
		       << "  -h     :  this help text" << endl
		       << endl
		       << "In graphic mode use F1 to display help."
 		       << endl;
		return 0;
	}

        mhz = args->getIntValue("-mhz");
	lpt = args->getIntValue("-lpt");
	port = findPort(-lpt);

	if (port == 0) {
		*debug << "* no usable parallel port available." << endl;
		*debug << "* exitting..." << endl;
                return 1;
        }

	detectProcessorSpeed(mhz);

        tsc_val  = (profile_value *)malloc(BUFFER_SIZE * sizeof(profile_value));
        port_val = (unsigned char *)malloc(BUFFER_SIZE * sizeof(*port_val));

#if 0
        int k;
        do {
        	k = getch();
                printf("%02x - '%c'\n", k, k);
        } while (k != 0x1b);
#endif

        readPort(port);

        print_info();

        if (args->isGiven("-print")) {
		print();
        } else {
		GrSetDriver(NULL);
		if(GrCurrentVideoDriver() == NULL) {
			*debug << "* no graphics driver found" << endl << endl;
			print();
		} else {
		        display();
                }
	}

	return 0;
}

