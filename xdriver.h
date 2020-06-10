

/*           
 *         knotEd   by   John Mount
 *   Copyright (C) 1988-2004  John Mount (j@mzlabs.com)
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef xdriver_h  
#define xdriver_h  

/* some window and state information */
#define labx 0
#define top_gap 20
#define laby top_gap
#define bigradius 200.0
#define littleradius 8
#define pi2 6.28318530718
#define Pstring ":-)"
#define Bwidth 2

#define blackInk 0
#define whiteInk 1

/* visible variables */
extern int winx,winy;
extern int g_x,g_y;
extern int textX,textY;
extern char Imesg[],Pmes[3][strbuflen],HeadMes[],SepMes[],TailMes[];
extern int Xbegin[];

extern Display *X11_display;
extern int X11_screen;
extern Window theWindow,comWindow;
extern GC theGc,comGc;
extern XSizeHints size_hints;
extern unsigned long MyWhiteP,MyBlackP;
extern XFontStruct *the_font,*com_font;

extern int REVERSEcolors;
extern unsigned long COLORVEC[];
extern int ncolors;


/* externally callabel procs */
extern void x_label(char *string);
extern void x_circle(int x, int y, int radius);
extern void x_line(int x1, int y1,int x2, int y2);
extern void x_move(int x, int y);
extern void Cprint(char *s, int n);
extern void Cread(char *s, int n);
extern void Xheader(int n);
extern void Tline(int n, int mode);
extern void Tmenu();
extern void Cmenu();
extern void Aline(double x1, double y1, double x2, double y2);
extern void dotted(double x1, double y1, double x2, double y2,
		   int solid, int gap);
extern void dotFrom(struct point *PATH, struct cross *INTER, struct ktag *KNOTL,
		    struct kpt *p);
extern void arcFrom(struct point *PATH, struct cross *INTER, struct ktag *KNOTL,
		    struct kpt *p, int mode);
extern void plotKnot(struct point *PATH, struct cross *INTER, struct ktag  *KL);
extern void initGraphics();
extern void initWindows(int argc, char *argv[]);
extern void get_GC(Window win, GC *gc, XFontStruct *font_info);
extern void load_font(XFontStruct **font_info);
extern void windowSize(Window win, int *x, int *y);


#endif
