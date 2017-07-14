/* QMAP: Quake level viewer
 *
 *   mode.c    Copyright 1997 Sean Barett
 *
 *   General screen functions (set graphics
 *   mode, blit to framebuffer, set palette)
 */

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "s.h"
#include "mode.h"

char *framebuffer = (char *) 0xA0000;

extern int *pixel;
char palette[768];


#include <windows.h> // For RGB macro
void blit(char *src)
{
	for (int i = 0; i < 320 * 200; i++)
	{
		// src is index into 256 color pallete
		int index = src[i];

		// pallete is 768 bytes of 256 different RGB values
		pixel[i] = RGB(palette[index * 3 + 2], palette[index * 3 + 1], palette[index * 3 + 0]);
	}
}



void set_pal(uchar *pal)
{
   int i;
//   outp(0x3c8, 0);
   for (i = 0; i < 768; ++i)
	   palette[i] = pal[i];
      //outp(0x3c9, pal[i] >> 2);
}

#pragma aux set_mode = \
    "  int  10h" \
    parm [eax];

void set_mode(int mode)
{

}

bool graphics=0;
void set_lores(void)
{
   graphics = 1;
   set_mode(0x13);
}
   
void set_text(void)
{
   if (graphics) {
      set_mode(0x3);
      graphics = 0;
   }
}

void fatal_error(char *message, char *file, int line)
{
   set_text();
   printf("Error (%s line %d): %s\n", file, line, message);
   exit(1);
}
