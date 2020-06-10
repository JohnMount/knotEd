

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



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include "struct.h"
#include "misc.h"
#include "xdriver.h"
#include "main.h"


  

/* some window and state information */
Window theWindow,comWindow,ic1;
int textX, textY,winx,winy;
int REVERSEcolors;

/* write a string to the current positon on the main window */
void x_label(char *string)
{
int i,j,l,s;
l = strlen(string);
i = 0;
while(i<l)
   {
   s = 0;
   for(j=i;(string[j]!=0)&&(string[j]!='\n');++j) 
      ++s;
   /*string[j] = 0;*/
   XDrawString(X11_display,theWindow,theGc,textX,textY,&(string[i]),s);
   i = j+1;
   textX = labx;
   textY += deltcom;
   }
}


/* draw a circle at x,y of radius radius on the main window */
void x_circle(int x, int y, int radius)
{
XDrawArc(X11_display,theWindow,theGc
   ,x-radius,y-radius,2*radius,2*radius,0,360*64);
}

/* draw a line on the main window */
void x_line(int x1, int y1,int x2, int y2) 
{
XDrawLine(X11_display,theWindow,theGc,x1,y1,x2,y2);
}


/* set the text printing position on the main window */
void x_move(int x, int y)
{
textX=x; textY=y;
}



/* print on the command window the string s at location n */
void Cprint(char *s, int n)
{
XDrawString(X11_display,comWindow,comGc,0,n,s,strlen(s));
}



/* read a string from the user, using the command window at posintion n */
/* use the passed in string pointer to do the dirty work */
void Cread(char *s, int n)
{
char temp[stlen];
int nc,start,c;
KeySym keysym;
XComposeStatus compose;
XEvent u;
nc = 0;
s[nc] = 0;
start = 0;
XClearArea(X11_display,comWindow,0,n-com_font->ascent,comx,deltay,False);
XDrawRectangle(X11_display,comWindow,comGc,start,n-com_font->ascent,5,deltay-1);
while(1)
   {
   XFlush(X11_display);
   XNextEvent(X11_display,&u);
   if(u.xany.window==comWindow)
      {
      if((u.type==Expose)||(u.type==KeyPress))
	 {
         if(u.type==Expose)
	    {
	    XSync(X11_display,0); /* catch other exposures */
	    Tmenu();
            }
         if(u.type==KeyPress)
	    {
	    c = XLookupString(&(u.xkey),temp,stlen,&keysym,&compose);
	    if((keysym == XK_Return)||(keysym == XK_KP_Enter)
	          ||(keysym == XK_Linefeed))
	       return;
            /* else */
	    if(((keysym == XK_BackSpace)||(keysym == XK_Delete))&&(nc>0))
	       --nc; 
            else
	       {
	       if((c==1)&&(temp[0]>=' ')&&(temp[0]<='~'))
		  {
	          s[nc] = temp[0];
	          ++nc;
		  }
	       }
            s[nc] = 0;
	    }
         /* show the user what he is typeing */
         XClearArea(X11_display,comWindow
	    ,0,n-com_font->ascent,comx,deltay,False);
         XDrawString(X11_display,comWindow,comGc,0,n,s,strlen(s));
	 start = XTextWidth(com_font,s,strlen(s));
	 XDrawRectangle(X11_display,comWindow,comGc
	    ,start,n-com_font->ascent,5,deltay-1);
	 }
      /* ignore the rest */
      }
   else
      {
      if((u.type==Expose)&&(u.xany.window==theWindow))
	  plotKnot(PATH,INTER,KNOTL); 
      /* else ignore */
      }
   }
}




/* header message storeage */
char Imesg[strbuflen];
char Pmes[3][strbuflen];
char HeadMes[strbuflen];
char SepMes[strbuflen];
char TailMes[strbuflen];
int Xbegin[4];



/* draw the nth portion of the main window command header */
void Xheader(int n)
{
XFillRectangle(X11_display,theWindow,theGc,Xbegin[n],laby-the_font->ascent
   ,Xbegin[n+1]-Xbegin[n],deltcom);
XSetForeground(X11_display,theGc,MyBlackP);
XDrawString(X11_display,theWindow,theGc,Xbegin[n],laby,Pmes[n],strlen(Pmes[n]));
XSetForeground(X11_display,theGc,MyWhiteP);
}


/* draw line n of the menu in color r */
void Tline(int n, int mode)
{
char ts[stlen];
if(mode==whiteInk)
   {
   XFillRectangle(X11_display,comWindow,comGc
      ,0,n*deltay+top_gap-com_font->ascent,comx,deltay);
   XSetForeground(X11_display,comGc,MyBlackP);
   }
else
   {
   XClearArea(X11_display,comWindow
      ,0,n*deltay+top_gap-com_font->ascent,comx,deltay,False);
   }
if(Xbool[n]==NULL)
   snprintf(ts,strbuflen,"%s",Xmes[n]);
else
   {
   if(*Xbool[n]==0)
      snprintf(ts,strbuflen,"%s  (is off)",Xmes[n]);
   else
      snprintf(ts,strbuflen,"%s [IS ON]",Xmes[n]);
   }
XDrawString(X11_display,comWindow,comGc,0,n*deltay+top_gap,ts,strlen(ts));
if(mode==whiteInk)
   XSetForeground(X11_display,comGc,MyWhiteP);
}



/* draw the entire command window */
void Tmenu()
{
int i;
XClearWindow(X11_display,comWindow);
for(i=0;i<Ncom;++i)
   Tline(i,blackInk);
if(last_sel>=0)
   Tline(last_sel,whiteInk);
}


/* clear out the dialog lines of the command menu */
void Cmenu()
{
XClearArea(X11_display,comWindow,0,f1-com_font->ascent,comx,3*deltay,False);
}



/* draw a line with arrows if necissary */
#define Awidth 5
int A_temp;
void Aline(double x1, double y1, double x2, double y2)
{
double dist,adjx,adjy;
Vline(x1,y1,x2,y2);
if(GLOBarrow&&(A_temp%LpA==0))
   {
   dist = dist2(x1,y1,x2,y2);
   if(dist!=0)
      {
      adjx = Awidth*(x2-x1)/dist;
      adjy = Awidth*(y2-y1)/dist;
      x1 = (x1+x2)/2;
      y1 = (y1+y2)/2;
      Vline(x1,y1,x1-adjy-adjx,y1+adjx-adjy);
      Vline(x1,y1,x1+adjy-adjx,y1-adjx-adjy);
      }
   }
A_temp = (A_temp+1)%LpA;
}


/* draw a dotted line with solid parts and gaps of the passed in sizes */
void dotted(double x1, double y1, double x2, double y2,
	    int solid, int gap)
{
int i;
double tt;
tt = dist2(x1,y1,x2,y2);
for(i=0;i<=(int)tt-solid;i=i+gap+solid)
   Vline(x1+(i/tt)*(x2-x1),y1+(i/tt)*(y2-y1)
    ,x1+((i+solid)/tt)*(x2-x1),y1+((i+solid)/tt)*(y2-y1));
}


/* use the knot data structures to draw a dotted line from the vertex */
/* or crossing described in *p to the next object in the knot in the */
/* givin inkColor */
void dotFrom(struct point *PATH, struct cross *INTER, struct ktag *KNOTL,
	     struct kpt *p)
{
double x1,x2,y1,y2;
struct kpt p1,p2;
p1 = *p;
p2 = *p;
Knext(PATH,INTER,&p2);
assignXY(PATH,INTER,&p1,&p2,&x1,&y1);
assignXY(PATH,INTER,&p2,&p1,&x2,&y2);
dotted(x1,y1,x2,y2,2,10);
}



/* use the knot data structures to draw a line or spline arc from the vertex */
/* or crossing described in *p to the next object in the knot in the */
/* givnin inkColor */
/* the spline is either a paramiterized Hermetian curve or a non-parameterized*/
/* Hermentian curve.  Control points may be missed but crossings must be hit */
void arcFrom(struct point *PATH, struct cross *INTER, struct ktag *KNOTL,
	     struct kpt *p, int mode)
{
double t,ay0,ay1,ay2,ay3,ax0,ax1,ax2,ax3,xt,yt,tt,s,c,dy1,dy2,ox2,oy2;
struct kpt p0,p1,p2,p3;
double x1,y1,x2,y2,x0,y0,x3,y3,xa,ya,xb,yb;
int i,knot;
char sc[3],tc[20];

if(p->wstruct==onCross)
   knot = PATH[INTER[p->node].toPoint[p->level]].knot;
else
   knot = PATH[p->node].knot;
if(GLOBoutmode==devX)
   {
   if(mode==whiteInk)
      XSetForeground(X11_display,theGc,MyBlackP);
   else
      XSetForeground(X11_display,theGc,COLORVEC[knot%ncolors]); 
   }
p1 = *p;
p2 = *p;
Knext(PATH,INTER,&p2);
if(GLOBspline)
   {
   p0 = *p;
   Kprev(PATH,INTER,&p0);
   p3 = *p;
   Knext(PATH,INTER,&p3);
   Knext(PATH,INTER,&p3);
   }
assignXY(PATH,INTER,&p1,&p2,&x1,&y1);
assignXY(PATH,INTER,&p2,&p1,&x2,&y2);
#ifdef DEBUG
if(p1.wstruct==onPath)
   {
   (void)snprintf(tc,strbuflen,"p%d",p1.node);
   Vlabel(PATH[p1.node].x,PATH[p1.node].y,tc);
   }
else
   {
   (void)snprintf(tc,strbuflen,"c%d",p1.node);
   Vlabel(INTER[p1.node].x,INTER[p1.node].y,tc);
   }
#endif
if(p1.wstruct==onPath)
   {
   if(GLOBmpoint)
      {
      Vline(PATH[p->node].x-3,PATH[p->node].y+3
         ,PATH[p->node].x+3,PATH[p->node].y+3);
      Vline(PATH[p->node].x-3,PATH[p->node].y-3
         ,PATH[p->node].x+3,PATH[p->node].y-3);
      Vline(PATH[p->node].x-3,PATH[p->node].y-3,PATH[p->node].x-3
         ,PATH[p->node].y+3);
      Vline(PATH[p->node].x+3,PATH[p->node].y-3,PATH[p->node].x+3
         ,PATH[p->node].y+3);
      }
   }
else
   {
   if((GLOBsign)||(GLOBtag&&name_valid))
      {
      if(GLOBsign)
         {
         if(Csign(PATH,INTER,p1.node)<0)
            sc[0] = '-';
         else
            sc[0] = '+';
         sc[1] = 0;
         }         
      else
         sc[0] = 0;
      if(GLOBtag&&name_valid)
         (void)snprintf(tc,strbuflen,"%s%d",sc,INTER[p1.node].tag);
      else
         (void)snprintf(tc,strbuflen,"%s",sc);
      Vlabel(INTER[p1.node].x,INTER[p1.node].y,tc);
      }
   }
if(!GLOBspline)
   Aline(x1,y1,x2,y2);
else 
   {
   if(GLOBghost)
      dotted(x1,y1,x2,y2,2,10);
   assignXY(PATH,INTER,&p0,&p1,&x0,&y0);
   assignXY(PATH,INTER,&p3,&p2,&x3,&y3);

   if(Bparm==0.0)
      {
      x0 = x0 - x1;
      y0 = y0 - y1;
      ox2 = x2;
      oy2 = y2;
      x2 = x2 - x1;
      y2 = y2 - y1;
      x3 = x3 - x1;
      y3 = y3 - y1;
      tt = dist2(0.0,0.0,x2,y2);
      if(tt==0)
	 goto arcExit; /* coincdent points, draw nothing */
      /* else */
      c = x2/tt;
      s = y2/tt;
      rotate(&x0,&y0,-s,c);  /* rotate system */
      rotate(&x2,&y2,-s,c);
      rotate(&x3,&y3,-s,c);
      dy1 = dslope(x0,y0,0.0,0.0,x2,y2);
      dy2 = dslope(0.0,0.0,x2,y2,x3,y3); 
      ay1 = dy1;
      ay2 = -(dy2+2*dy1)/x2;
      ay3 = (dy2+dy1)/(x2*x2);
   
      xa = x1;
      ya = y1;
      for(i=1;i<Bnarcs;++i)
         {
         t = ((double)i)/(double)Bnarcs;
         xt = t*x2;
         yt = ((ay3*xt+ay2)*xt+ay1)*xt;
         rotate(&xt,&yt,s,c);
         xt = xt + x1;
         yt = yt + y1;
         if(i==(Bnarcs/2))
            Aline(xa,ya,xt,yt);
         else
            Vline(xa,ya,xt,yt);
         xa = xt;
         ya = yt;
         }
      if(Bnarcs<=1)
         Aline(xa,ya,ox2,oy2);   /* force matchup */
      else
         Vline(xa,ya,ox2,oy2);
      }
   else
      { 
      if ((p1.wstruct==onCross)&&(p2.wstruct==onCross)) 
         {
         Aline(x1,y1,x2,y2);
         }
      else 
         {
         if (p1.wstruct==onCross) 
            {
            ax0=x1;
            ax1=x2-x1;
            ax2= -((3*Bparm-1)*x3+(4-6*Bparm)*x2+(3*Bparm-3)*x1)/2.0;
            ax3=Bparm*x3+(1-2*Bparm)*x2+(Bparm-1)*x1;
            ay0=y1;
            ay1=y2-y1;
            ay2= -((3*Bparm-1)*y3+(4-6*Bparm)*y2+(3*Bparm-3)*y1)/2.0;
            ay3=Bparm*y3+(1-2*Bparm)*y2+(Bparm-1)*y1;
            }
         else 
            {
            if (p2.wstruct==onCross) 
               {
               ax0= -((Bparm-1)*x2-2*Bparm*x1+(Bparm-1)*x0)/2.0;
               ax1=x2-x0;
               ax2=((3*Bparm-3)*x2+(2-6*Bparm)*x1+(3*Bparm+1)*x0)/2.0;
               ax3=(1-Bparm)*x2+(2*Bparm-1)*x1-Bparm*x0; 
               ay0= -((Bparm-1)*y2-2*Bparm*y1+(Bparm-1)*y0)/2.0;
               ay1=y2-y0;
               ay2=((3*Bparm-3)*y2+(2-6*Bparm)*y1+(3*Bparm+1)*y0)/2.0;
               ay3=(1-Bparm)*y2+(2*Bparm-1)*y1-Bparm*y0; 
               }
            else 
               {
               ax0= -((Bparm-1)*x2-2*Bparm*x1+(Bparm-1)*x0)/2.0;
               ax1=x2-x0;
          ax2=(-3*Bparm*x3+x3+(9*Bparm-7)*x2+(5-9*Bparm)*x1+(3*Bparm+1)*x0)/2.0;
               ax3=Bparm*x3+(2-3*Bparm)*x2+(3*Bparm-2)*x1-Bparm*x0; 
               ay0= -((Bparm-1)*y2-2*Bparm*y1+(Bparm-1)*y0)/2.0;
               ay1=y2-y0;
          ay2=(-3*Bparm*y3+y3+(9*Bparm-7)*y2+(5-9*Bparm)*y1+(3*Bparm+1)*y0)/2.0;
               ay3=Bparm*y3+(2-3*Bparm)*y2+(3*Bparm-2)*y1-Bparm*y0; 
               }
            }
         xa = ax0;   /* force curve segments to match up */
         ya = ay0;
         for(i=1;i<=Bnarcs;++i)
            {
	    t = ((double)i)/(double)Bnarcs;
	    xb = ((ax3*t+ax2)*t+ax1)*t+ax0;
	    yb = ((ay3*t+ay2)*t+ay1)*t+ay0;
	    if(i==(Bnarcs/2))
	       Aline(xa,ya,xb,yb);
	    else
	       Vline(xa,ya,xb,yb);
	    xa = xb;
	    ya = yb;
	    }
         }
      }
   }
#ifdef DEBUG
XSync(X11_display,1);
XSync(X11_display,1);
#endif
arcExit:
if(GLOBoutmode==devX)
   XSetForeground(X11_display,theGc,MyWhiteP);
}



/* draw all of the knot components on the screen */
void plotKnot(struct point *PATH, struct cross *INTER, struct ktag  *KL)
{
struct kpt p;
int knot;
char tc[strbuflen];
A_temp = 0;
/*XDefineCursor(theWindow,wait_curs);*/
/*XDefineCursor(comWindow,wait_curs);*/
XFlush(X11_display);
for(knot=1;knot<=KL[0].n;++knot)
   {
   Pfirst(&p,KL[knot].n);
   if(GLOBkl)
      {
      (void)snprintf(tc,strbuflen,"%d",KL[knot].l1);
      Vlabel(PATH[p.node].x,PATH[p.node].y,tc);
      }
   while(Kvalid(&p))
      {
      arcFrom(PATH,INTER,KL,&p,blackInk);
      Knext(PATH,INTER,&p);
      }
   }
/*XDefineCursor(theWindow,curs);*/
/*XDefineCursor(comWindow,com_curs);*/
XSync(X11_display,1);
}


Display *X11_display;
int X11_screen;
XFontStruct *the_font,*com_font;
GC theGc,comGc;
XSizeHints size_hints;
unsigned long MyWhiteP,MyBlackP;
Colormap cmap;
#define max_colors 6
unsigned long COLORVEC[max_colors+1];
int ncolors;
static char *cname[] = {"red", "green", "blue", "violet", "orange", "brown"};


/* minumum screen set up*/
void initGraphics()
{
char t[stlen],display_name[stlen],*p;

p = getenv("DISPLAY");
strncpy(display_name,p,stlen);
if( (X11_display=XOpenDisplay(display_name))==NULL)
   {
   (void)snprintf(t,strbuflen,"Couldn't open display %s\n",display_name);
   panic_nosave(t);
   }
X11_screen = DefaultScreen(X11_display);
load_font(&com_font);
deltay = com_font->ascent + com_font->descent;
load_font(&the_font);
deltcom = the_font->ascent + the_font->descent;
}

/* get stuff going now that we know more */
void initWindows(int argc, char *argv[])
{
int depth,error,i;
XColor exact_def;
XSizeHints hints;

if(REVERSEcolors)
   {
   MyBlackP = WhitePixel(X11_display,X11_screen);
   MyWhiteP = BlackPixel(X11_display,X11_screen);
   }
else
   {
   MyWhiteP = WhitePixel(X11_display,X11_screen);
   MyBlackP = BlackPixel(X11_display,X11_screen);
   }
depth = DisplayPlanes(X11_display,X11_screen);
cmap = DefaultColormap(X11_display,X11_screen);
COLORVEC[0] = MyWhiteP;
if(depth==1)
   ncolors = 1;
else
   {
   error = 0;
   for(i=0;(i<max_colors)&&(!error);++i)
      {
      if(!XParseColor(X11_display,cmap,cname[i],&exact_def))
	 error = 1;
      else
	 {
	 if(!XAllocColor(X11_display,cmap,&exact_def))
	    error = 1;
         else
	    COLORVEC[i+1] = exact_def.pixel;
         }
      }
   ncolors = i+1; 
   }
theWindow = XCreateSimpleWindow(X11_display,RootWindow(X11_display,X11_screen)
   ,comx+2*Bwidth,0,winx,winy,Bwidth,MyWhiteP,MyBlackP);
comWindow = XCreateSimpleWindow(X11_display,RootWindow(X11_display,X11_screen)
   ,0,0,comx,comy,Bwidth,MyWhiteP,MyBlackP);

get_GC(theWindow,&theGc,the_font);
XSetLineAttributes(X11_display,theGc,GLOBfast,LineSolid,CapRound,JoinRound);
hints.flags = 0;  /* no hints */
XSetStandardProperties(X11_display,theWindow,"knotEd","knotEd",None,argv,argc
   ,&hints);
get_GC(comWindow,&comGc,com_font);
XSetLineAttributes(X11_display,comGc,0,LineSolid,CapRound,JoinRound);
hints.flags = 0;  /* no hints */
XSetStandardProperties(X11_display,comWindow,"kmenu","kmenu",None,argv,argc
   ,&hints);

/* set up what we hear */
XSelectInput(X11_display,theWindow
   ,ExposureMask | KeyPressMask | ButtonPressMask);
XSelectInput(X11_display ,comWindow
   ,ExposureMask | ButtonPressMask | KeyPressMask | PointerMotionMask);

/* add size hints and icon stuff later */
XMapWindow(X11_display,theWindow);
XMapWindow(X11_display,comWindow);
}


void get_GC(Window win, GC *gc, XFontStruct *font_info)
{
unsigned long valuemask;
XGCValues values;

valuemask = 0;
*gc = XCreateGC(X11_display,win,valuemask,&values);
XSetFont(X11_display,*gc,font_info->fid);
XSetForeground(X11_display,*gc,MyWhiteP);
XSetClipMask(X11_display,*gc,None);
}

void load_font(XFontStruct **font_info)
{
char fontname[stlen];
strncpy(fontname,"9x15",stlen);
if((*font_info = XLoadQueryFont(X11_display,fontname))==NULL)
   panic_nosave("could not load font");
}

void windowSize(Window win, int *x, int *y)
{
}
