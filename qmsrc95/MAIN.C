/*  QMAP: Quake level viewer
 *
 *   main.c    Copyright 1997 Sean Barett
 *
 *   General setup control, main "sim" loop
 */

#include <conio.h>
#include <dos.h>
#include "bspfile.h"
#include "mode.h"
#include "3d.h"
#include "fix.h"
#include "scr.h"
//#include "char.h"
#include "tm.h"
#include "render.h"
#include "bsp.h"
#include "surface.h"
#include "poly.h"

#include <windows.h>

double chop_temp;

vector cam_loc, cam_vel, new_loc;
angvec cam_ang, cam_angvel;

char *scr_buf;
int   scr_row;

#define ANG_STEP  0x0080
#define VEL_STEP  0.5

char colormap[64][256];

void run_sim(void)
{
   vector temp;

   scr_buf = malloc(320*200);
   scr_row = 320;
   qmap_set_output(scr_buf, scr_row);

   for (;;) {

      // RENDER

      set_view_info(&cam_loc, &cam_ang);
      render_world(&cam_loc);
      blit(scr_buf);

      // UI


      // "PHYSICS"
      cam_ang.tx += cam_angvel.tx;
      cam_ang.ty += cam_angvel.ty;
      cam_ang.tz += cam_angvel.tz;
      set_view_info(&cam_loc, &cam_ang);

      temp.x = cam_vel.x; temp.y = 0; temp.z = 0;
      rotate_vec(&temp);
      cam_loc.x  += temp.x; cam_loc.y += temp.y; cam_loc.z += temp.z;

      temp.x = 0; temp.y = cam_vel.y; temp.z = 0;
      rotate_vec(&temp);
      cam_loc.x  += temp.x; cam_loc.y += temp.y; cam_loc.z += temp.z;

      temp.x = 0; temp.y = 0; temp.z = cam_vel.z;
      rotate_vec(&temp);
      cam_loc.x  += temp.x; cam_loc.y += temp.y; cam_loc.z += temp.z;
   }
}

void load_graphics(void)
{
   char pal[768];
   FILE *f;
   if ((f = fopen("palette.lmp", "rb")) == NULL)
      fatal("Couldn't read palette.lmp\n");
   fread(pal, 1, 768, f);
   fclose(f);
   set_pal(pal); 
   if ((f = fopen("colormap.lmp", "rb")) == NULL)
      fatal("Couldn't read colormap.lmp\n");
   fread(colormap, 256, 64, f);
   fclose(f);
}

int dosmain(char *map)
{
      LoadBSPFile(map);
      set_lores();
      load_graphics();
      init_cache();
      setup_default_point_list();

      run_sim();      
      set_text();
   return 0;
}
