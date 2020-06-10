

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
#include <signal.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "struct.h"
#include "misc.h"
#include "xdriver.h"
#include "main.h"


/* knot theory program */


/* some global nasty type variables */
#define GPROX 0.9
int GLOBspline,GLOBarrow,GLOBoutmode,name_valid,GLOBkl,ISOTOPY_LOCK,GLOBfast;
int GLOBCrad,Bnarcs,LpA,GLOBmpoint,GLOBsign,GLOBtag,GLOBghost;
struct spst soft_spot;
double Bparm,GLOBTScale;
struct point *PATH,*OLDPATH,*UPATH,*TPATH;
struct cross *INTER,*OLDINTER,*UINTER,*TCROS;
struct ktag *KNOTL,*OLDKNOTL,*UKNOTL,*TKNTL;
struct ktag K_0;
char Xmes[Ncoms][strbuflen];
int *Xbool[Ncoms];
int Ncom,comx,comy,f1,f2,f3,last_sel,deltay,deltcom,PANICING,MEMREADY;
char version_stamp[strbuflen];
int maxlink,maxcross,maxpoint,*Atag,*Btag,*Ctag;






/* act on a mouse command in the main window */
/* yes,  I know this proc is MUCH too long */
#define Dsense 10
void interact(XEvent u)
{
  int but,p,nx,ny,i,p2,fch,okay;
  struct kpt dp,d2;
  double x,y;

  /* we know it was a button press event by now */
  but = u.xbutton.button;
  switch(but)
    {
    case 2:   /* mes 2 middle button */
      /* rip knot */
      Xheader(1);
      /* get nodes to cut */
      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
      p = Inode((double)u.xbutton.x,(double)u.xbutton.y
		,KNOTL,PATH,INTER);
      if(p!=-1)
        {
	  /*XDefineCursor(theWindow,rip_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  x_circle((int)PATH[p].x,(int)PATH[p].y,7);
	  x = PATH[PATH[p].toPoint].x;
	  y = PATH[PATH[p].toPoint].y;
	  i = (int)dist2((double)PATH[p].x
			 ,(double)PATH[p].y,(double)x,(double)y);
	  if(i==0)
	    i = 1;
	  x = PATH[p].x + (12*(x-PATH[p].x))/i;
	  y = PATH[p].y + (12*(y-PATH[p].y))/i;
	  x_line((int)x-4,(int)y+4,(int)x+4,(int)y-4);
	  x_line((int)x+4,(int)y+4,(int)x-4,(int)y-4);
	  get_xy(&nx,&ny,f1);
	  x = (double)nx;
	  y = (double)ny;
	  p2 = Inode(x,y,KNOTL,PATH,INTER);
	  if((p2!=-1)&&(PATH[p].knot==PATH[p2].knot))
	    {
	      Kbackup(PATH,INTER,KNOTL,UPATH,UINTER,UKNOTL);
	      /*XDefineCursor(theWindow,needle_curs);*/
	      XSync(X11_display,1);
	      name_valid = 0;
	      x_circle((int)PATH[p2].x,(int)PATH[p2].y,5); 
	      /* cut the knot */
	      Pfirst(&dp,p);
	      do {
		arcFrom(PATH,INTER,KNOTL,&dp,whiteInk);
		dotFrom(PATH,INTER,KNOTL,&dp);
		Knext(PATH,INTER,&dp);
              }while((dp.wstruct!=onPath)||(dp.node!=p2));
	      soft_spot.state = initialSoft;
	      soft_spot.P1 = p;
	      soft_spot.P2 = p2;
	      soft_spot.lcross = -1;
	      soft_spot.level = levHigh;
	      Pfirst(&dp,p);
	      do {
		if((dp.wstruct==onCross)
		   &&(INTER[dp.node].touch==cross_key(INTER)))
		  softDitchCross(PATH,INTER,dp.node,&soft_spot);
		Knext(UPATH,UINTER,&dp);
              }while((dp.wstruct!=onPath)||(dp.node!=p2));
	      for(i=PATH[p].toPoint;i!=p2;i=PATH[i].toPoint)
		{
		  if(PATH[i].fromPoint!=-1)
		    PATH[PATH[i].fromPoint].toPoint = -1;
		  if(PATH[i].toPoint!=-1)
		    PATH[PATH[i].toPoint].fromPoint = -1;
		  PATH[i].touch = point_key(PATH)-1;
		}
	      /* get nodes not deleted */
	      x_circle((int)PATH[p2].x,(int)PATH[p2].y,Dsense); 
	      x = PATH[p2].x;
	      y = PATH[p2].y;
	      /* get new points */
	      if(p2==p)
		fch = 1;
	      else
		fch = 0;
	      do {
		get_xy(&nx,&ny,f1);
		if((nx!=-1)
		   &&((nx!=(int)PATH[p].x)||(ny!=(int)PATH[p].y)))
		  {
		    if(dist2(x,y,(double)nx,(double)ny)>Dsense)
		      {
			fch = 0;
			x_line((int)PATH[p].x,(int)PATH[p].y,nx,ny);
			i = newpt(PATH);
			PATH[i].knot = PATH[p].knot;
			PATH[i].x = (double)nx;
			PATH[i].y = (double)ny;
			PATH[i].fromPoint = p;
			PATH[p].toPoint = i;
			p = i;
		      }
		    else
		      {
			/* if(fch)
			   XFeep(0);*/
		      }
		  }
		else
		  { 
		    /*XFeep(0); */
		    if(nx==-1)
		      {
			Kbackup(OLDPATH,OLDINTER,OLDKNOTL,PATH,INTER,KNOTL);
			goto abort_point;  
		      }
		  }
	      }while(((dist2(x,y,(double)nx,(double)ny)>Dsense)
		      ||(fch))&&(nx!=-1));
	      /* finish up the knot */
	      /* insure 4 points */
	      while(Npath(PATH,p2))
                {
		  i = newpt(PATH);
		  PATH[i].knot = PATH[p].knot;
		  PATH[i].x = (PATH[p].x+PATH[p2].x)/2;
		  PATH[i].y = (PATH[p].y+PATH[p2].y)/2;
		  PATH[i].fromPoint = p;
		  PATH[p].toPoint = i;
		  p = i;
                }
	      /* relink knot */
	      PATH[p].toPoint = p2;
	      PATH[p2].fromPoint = p;
	      /* check if we deleted the KNOT vertex */
	      if(PATH[KNOTL[PATH[p2].knot].n].touch!=point_key(PATH))
                KNOTL[PATH[p2].knot].n = p2;
	      {
                if(!find_cross(PATH,INTER,KNOTL
			       ,UPATH,UINTER,UKNOTL,&soft_spot))
		  Kbackup(OLDPATH,OLDINTER,OLDKNOTL,PATH,INTER,KNOTL);
	      }
	    }
          else
	    {
	      Kbackup(OLDPATH,OLDINTER,OLDKNOTL,PATH,INTER,KNOTL);
	      /*XFeep(0);*/
	    }
	abort_point:
          XClearWindow(X11_display,theWindow);
          plotKnot(PATH,INTER,KNOTL);
	}
      /*else
	XFeep(0);*/
      break;
    case 1:   /* mes 1 left button */
      Xheader(0);
      /* pull string */
      p = node((double)u.xbutton.x,(double)u.xbutton.y,KNOTL,PATH);
      if(p!=-1)
	{
          /*XDefineCursor(theWindow,pul_curs);*/
          /*XDefineCursor(comWindow,intr_curs);*/
          XSync(X11_display,1);
          x_circle((int)PATH[p].x,(int)PATH[p].y,7);
          get_xy(&nx,&ny,f1);
          x = (double)nx;
          y = (double)ny;
          if(nx!=-1)
	    {
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      soft_spot.state = initialSoft;
	      soft_spot.P1 = PATH[p].fromPoint;
	      soft_spot.P2 = PATH[p].toPoint;
	      soft_spot.lcross = -1;
	      soft_spot.level = levHigh;
	      Pfirst(&dp,PATH[p].fromPoint);
	      while((dp.wstruct!=onPath)
		    ||(dp.node!=PATH[p].toPoint))
                {
		  if(dp.wstruct==onCross)
		    softDitchCross(PATH,INTER,dp.node,&soft_spot);
		  Knext(OLDPATH,OLDINTER,&dp);
                }
	      name_valid = 0;
	      PATH[p].x = x;
	      PATH[p].y = y;
	      (void)find_cross(PATH,INTER,KNOTL
			       ,OLDPATH,OLDINTER,OLDKNOTL,&soft_spot);
	    }
          /*else
	    XFeep(0);*/
          XClearWindow(X11_display,theWindow);
          plotKnot(PATH,INTER,KNOTL);
	}
      /*else
	XFeep(0);*/
      break;
    case 3:   /* mes 3 right button */
      /* flip string */
      Xheader(2);
      p = Kcross(u.xbutton.x,u.xbutton.y,KNOTL,PATH,INTER);
      if(p!=-1)
	{
          if(ISOTOPY_LOCK)
	    {
	      okay = 0;
	      Cfirst(&dp,p,levLow);
	      do {
                Knext(PATH,INTER,&dp);
	      }while(Kvalid(&dp)&&(dp.wstruct!=onCross));
	      if(Kvalid(&dp)&&(dp.wstruct==onCross)
		 &&(dp.node==p)&&(dp.level!=levLow))
                okay = 1;
	      else
                {
		  Cfirst(&dp,p,levHigh);
		  do {
		    Knext(PATH,INTER,&dp); 
		  }while(Kvalid(&dp)&&(dp.wstruct!=onCross));
		  if(Kvalid(&dp)&&(dp.wstruct==onCross)
		     &&(dp.node==p)&&(dp.level!=levHigh))
		    okay = 1;
                } 
	    } 
          if((!ISOTOPY_LOCK)||(okay))
	    {
	      /*XWarpMouse(theWindow,(int)INTER[p].x,(int)INTER[p].y);*/
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL); 
	      for(i=levLow;i<=levHigh;++i)
                {
		  Cfirst(&dp,p,i);
		  Cfirst(&d2,p,i);
		  Knext(PATH,INTER,&d2);
		  Kprev(PATH,INTER,&dp);
		  while((dp.wstruct!=d2.wstruct)||(dp.node!=d2.node))
		    {
		      arcFrom(PATH,INTER,KNOTL,&dp,whiteInk);
		      Knext(PATH,INTER,&dp);
		    }
                }
	      flipCross(PATH,INTER,p);
	      for(i=levLow;i<=levHigh;++i)
                {
		  Cfirst(&dp,p,i);
		  Cfirst(&d2,p,i);
		  Knext(PATH,INTER,&d2);
		  Kprev(PATH,INTER,&dp);
		  while((dp.wstruct!=d2.wstruct)||(dp.node!=d2.node))
		    {
		      arcFrom(PATH,INTER,KNOTL,&dp,blackInk);
		      Knext(PATH,INTER,&dp);
		    }
                }
	    }
          /*else
	    XFeep(0);*/
	}
      /*else
	XFeep(0);*/
      break;
    default:
      panic("Bad button");
    }
  XClearArea(X11_display,theWindow,0,laby-the_font->ascent
	     ,winx,deltcom,False);
  x_move(labx,laby);
  x_label(Imesg);
  Cmenu();
  /*XDefineCursor(theWindow,curs);*/
  /*XDefineCursor(comWindow,com_curs);*/
  XSync(X11_display,1);
  return;
}



/* set up the user menu */
int com_list[strbuflen];
void Init_men()
{
  int i;
  for(i=0;i<strbuflen;++i)
    com_list[i] = -1;
  Ncom = 0;
  deltay = com_font->ascent + com_font->descent;
  Senter("(q) QUIT",(int *)NULL);
  Senter("(c) CLEAR KNOT",(int *)NULL);
  Senter("(r) READ KNOT",(int *)NULL);
  Senter("(s) save knot",(int *)NULL);
  Senter("([) write knot string",(int *)NULL);
  Senter("(o) write braid word",(int *)NULL);
  Senter("(e) READ BRAID WORD",(int *)NULL);
#ifdef tek4014
  Senter("(t) tek plot knot",(int *)NULL);
#endif
#ifdef PIC
  Senter("(t) troffable output",(int *)NULL);
#endif
  Senter("(k) adjust hard scale",(int *)NULL);
  Senter("(j) adjust curve draw",(int *)NULL);
  Senter("(\') adjust line width",(int *)NULL);
  Senter("(g) ghost line",&GLOBghost);
  Senter("(p) adjust spline",(int *)NULL);
  Senter("(b) spline",&GLOBspline);
  Senter("(v) vertex drawing",&GLOBmpoint);
  Senter("(y) adjust arrow density",(int *)NULL);
  Senter("(a) arrow drawing",&GLOBarrow);
  Senter("(h) REVERSE KNOT",(int *)NULL);
  Senter("(1) ADD KNOT+",(int *)NULL);
  Senter("(2) ADD KNOT-",(int *)NULL);
  Senter("(d) DELETE KNOT",(int *)NULL);
  Senter("(,) MOVE KNOT",(int *)NULL);
  Senter("(m) MOVE ALL",(int *)NULL);
  Senter("(x) ZOOM IN",(int *)NULL);
  Senter("(z) ZOOM OUT",(int *)NULL);
  Senter("(n) NORMAL VIEW",(int *)NULL);
  Senter("(i) knot labeling",&GLOBkl);
  Senter("(-) sign display",&GLOBsign);
  Senter("(f) cross tagging",&GLOBtag);
  Senter("(w) adjust cross width",(int *)NULL);
  Senter("(l) isotopy lock",&ISOTOPY_LOCK);
  Senter("(;) \"A\" CHANNEL CUT",(int *)NULL);
  Senter("(=) PUSH OFF",(int *)NULL);
  Senter("(u) UNDO",(int *)NULL);
  Senter("(.) refresh",(int *)NULL);
  f1 = deltay*Ncom + top_gap;
  f2 = f1 + deltay;
  f3 = f2 + deltay;
  comy = f3 + deltay;
}



/* set up the main winodw header */
void Init_st()
{
  int sep_size;
  (void)snprintf(Pmes[0],strbuflen,"%s",mes1);
  (void)snprintf(Pmes[1],strbuflen,"%s",mes2);
  (void)snprintf(Pmes[2],strbuflen,"%s",mes3);
  (void)snprintf(HeadMes,strbuflen,"do knot (");
  (void)snprintf(SepMes,strbuflen," | ");
  (void)snprintf(TailMes,strbuflen,")");
  (void)snprintf(Imesg,strbuflen,"%s%s%s%s%s%s%s",HeadMes,Pmes[0],SepMes,Pmes[1],SepMes
		,Pmes[2],TailMes);
  sep_size = XTextWidth(the_font,SepMes,strlen(SepMes));
  Xbegin[0] = XTextWidth(the_font,HeadMes,strlen(HeadMes)) + labx;
  Xbegin[1] = Xbegin[0] + XTextWidth(the_font,Pmes[0],strlen(Pmes[0])) + sep_size;
  Xbegin[2] = Xbegin[1] + XTextWidth(the_font,Pmes[1],strlen(Pmes[1])) + sep_size;
  Xbegin[3] = Xbegin[2] + XTextWidth(the_font,Pmes[2],strlen(Pmes[2])) 
    + XTextWidth(the_font,TailMes,strlen(TailMes));
}


/* save a knot to a file, the method is quick and dirty fwrites and */
/* IS MACHINE DEPENDENT,  If somebody wants to write a nice grammar based */
/* method :-) */
void save_knot(FILE *CUR_OUT, struct point *PATH,
	       struct cross *INTER, struct ktag *KNOTL)
{
  int knot;
  struct kpt p;
  Ltag(PATH,INTER,1);
  (void)fwrite((char *)KNOTL,sizeof(struct ktag),KNOTL[0].n+1,CUR_OUT);
  for(knot=1;knot<=KNOTL[0].n;++knot)
    {
      Pfirst(&p,KNOTL[knot].n);
      while(Kvalid(&p))
	{
	  if(p.wstruct==onPath)
	    {
	      PATH[p.node].knot = knot;   /* make sure drawing is okay */
	      (void)fwrite((char *)&p.node,sizeof(int),1,CUR_OUT);
	      (void)fwrite((char *)&p.wstruct,sizeof(int),1,CUR_OUT);
	      (void)fwrite((char *)&(PATH[p.node]),sizeof(struct point),1,CUR_OUT);
	    }
	  else
	    {  /* write each crossing but once */
	      if(INTER[p.node].tag==1)
		{
		  (void)fwrite((char *)&p.node,sizeof(int),1,CUR_OUT);
		  (void)fwrite((char *)&p.wstruct,sizeof(int),1,CUR_OUT);
		  (void)fwrite((char *)&(INTER[p.node]),sizeof(struct cross),1,CUR_OUT);
		  INTER[p.node].tag = 0;
		}
	    }
	  Knext(PATH,INTER,&p);
	}
    }
  (void)fclose(CUR_OUT);
}

/* read a knot back in from a file */
/* the insisting that records go in absolute locations costs us a lot of */
/* features hear (like reading one knot into another- though this can be */
/* worked out) but it is made up for in the savings in find_cross */
/* also the knot must often be read in with the same memory configuration as */
/* it was written out */
void read_knot(FILE *CUR_IN, struct point *PATH,
	       struct cross *INTER, struct ktag *KNOTL)
{
  int j,l;
  dispose_PATH(PATH);
  dispose_INTER(INTER);
  (void)fread((char *)&(KNOTL[0]),sizeof(struct ktag),1,CUR_IN);
  if(KNOTL[0].n>=maxlink-1)
    panic("too many links");
  (void)fread((char *)&(KNOTL[1]),sizeof(struct ktag),KNOTL[0].n,CUR_IN);
  while(!feof(CUR_IN))
    {
      (void)fread((char *)&j,sizeof(int),1,CUR_IN);
      (void)fread((char *)&l,sizeof(int),1,CUR_IN);
      if(l==onPath)
	{
	  if(j>=maxpoint)
	    panic("out of range point");
	  (void)fread((char *)&(PATH[j]),sizeof(struct point),1,CUR_IN);
	  PATH[j].touch = point_key(PATH);
	}
      else
	{
	  if(j>=maxcross)
	    panic("out of range crossing");
	  (void)fread((char *)&(INTER[j]),sizeof(struct cross),1,CUR_IN);
	  INTER[j].touch = cross_key(INTER);
	}
    }
  (void)fclose(CUR_IN);
}


int ABS(int x)
{
  if(x<0)
    return(-x);
  /*else*/
  return(x);
}

extern void DeCo(struct point *PATH,struct cross *INTER,struct ktag *KNOTL);

/* take in a braid word form a file and draw it on the screen */
void CreateBraid(struct point *PATH, struct cross *INTER,
		 struct ktag *KNOTL, FILE *CUR_IN)
{
  int Anext,sign,max,i,p,last,p2,c,j;
  double dx,dy,xrad,yrad,x,y;
  Anext = 0;
  max = 0;
  clearKnot(PATH,INTER,KNOTL);
  while(!feof(CUR_IN))   /* read in the braid word */
    {
      do {
	c = getc(CUR_IN);
      }while(c==' ');
      sign = 1;
      while((c=='+')||(c=='-'))
	{
	  if(c=='-')
	    sign = -sign;
	  c = getc(CUR_IN);
	}
      Atag[Anext] = 0;
      while((c>='0')&&(c<='9'))
	{
	  Atag[Anext] = 10*Atag[Anext] + c - '0';
	  c = getc(CUR_IN);
	} 
      if(Atag[Anext]>max)
	max = Atag[Anext];
      Atag[Anext] = Atag[Anext]*sign;
      if(Atag[Anext]!=0)
	++Anext;
    }
  yrad = .25*(winy-deltcom);
  xrad = .25*winx;
  if(Anext==0)
    return;
  dx = ((winx/2.0)-xrad)/(max+1);
  dy = 2*yrad/Anext;
  /* put string in place (get crossings right on next pass) */
  KNOTL[0].n = 1;
  KNOTL[1] = K_0;
  KNOTL[1].n = newpt(PATH);
  last = KNOTL[1].n;
  PATH[last].knot = 1;
  PATH[last].x = winx/2.0 + xrad/2;
  PATH[last].y = winy/2.0 + yrad + deltcom;
  PATH[last].tag = 0;
  i = 0;
  do {
    if(i!=0)
      {
	p = newpt(PATH);
	PATH[p].knot = 1;
	PATH[p].x = winx/2.0 + (xrad+i*dx);
	PATH[p].y = winy/2.0 + (yrad+i*dy) + deltcom;
	PATH[p].fromPoint = last;
	PATH[p].tag = 0;
	PATH[last].toPoint = p;
	last = p;
      }
    p = newpt(PATH);                     /* insert merry-go round points */
    PATH[p].knot = 1;
    PATH[p].x = winx/2.0 - (xrad+i*dx);
    PATH[p].y = winy/2.0 + (yrad+i*dy) + deltcom;
    PATH[p].fromPoint = last;
    PATH[p].tag = 0;
    PATH[last].toPoint = p;
    last = p;
    p = newpt(PATH);
    PATH[p].knot = 1;
    PATH[p].x = winx/2.0 - (xrad+i*dx);
    PATH[p].y = winy/2.0 - (yrad+i*dy) + deltcom;
    PATH[p].fromPoint = last;
    PATH[p].tag = 0;
    PATH[last].toPoint = p;
    last = p;
    p = newpt(PATH);
    PATH[p].knot = 1;
    PATH[p].x = winx/2.0 + (xrad+i*dx);
    PATH[p].y = winy/2.0 - (yrad+i*dy) + deltcom;
    PATH[p].fromPoint = last;
    PATH[p].tag = 0;
    PATH[last].toPoint = p;
    last = p;
    if(i!=0)
      {
	p = newpt(PATH);
	PATH[p].knot = 1;
	PATH[p].x = winx/2.0 + (xrad+i*dx);
	PATH[p].y = winy/2.0 - yrad + deltcom;
	PATH[p].fromPoint = last;
	PATH[p].tag = 0;
	PATH[last].toPoint = p;
	last = p;
      }
    j = 0;
    for(j=0;j<Anext;++j)
      {
	p = newpt(PATH);
	PATH[p].tag = 0;
	if(ABS(Atag[j])-i==0)
	  {
	    i = i-1;
	    Btag[j] = p;
	    PATH[p].tag = j;
	  }
	else
	  if(ABS(Atag[j])-i==1)
            {
	      i = i+1;
	      Ctag[j] = p;
	      PATH[p].tag = j;
            }
	PATH[p].knot = 1;
	PATH[p].x = winx/2.0 + (xrad+i*dx);
	PATH[p].y = winy/2.0 - yrad + j*dy + dy + deltcom;
	PATH[p].fromPoint = last;
	PATH[last].toPoint = p;
	last = p;
      }
  }while(i!=0);
  PATH[last].toPoint = KNOTL[1].n;
  PATH[KNOTL[1].n].fromPoint = last;
  /* insert the damn crossings */
  for(j=0;j<Anext;++j)
    {
      p = PATH[Btag[j]].fromPoint;
      p2 = PATH[Ctag[j]].fromPoint;
      if(ldisj(PATH[p].x,PATH[p].y
	       ,PATH[PATH[p].toPoint].x,PATH[PATH[p].toPoint].y
	       ,PATH[p2].x,PATH[p2].y
	       ,PATH[PATH[p2].toPoint].x,PATH[PATH[p2].toPoint].y,&x,&y))
	panic("error in braid reconstruction");
      c = ncross(INTER);
      INTER[c].x = x;
      INTER[c].y = y;
      insert_cross(PATH,INTER,p,c,levLow);
      insert_cross(PATH,INTER,p2,c,levHigh);
      if(Csign(PATH,INTER,c)*Atag[j]<0)
	flipCross(PATH,INTER,c);
      PATH[p].tag = 0;
    }
  DeCo(PATH,INTER,KNOTL);
}


/* remov all colinear control points */
/* can not be used as a general routine as it uses absolute distances */
void DeCo(struct point *PATH,struct cross *INTER,struct ktag *KNOTL)
{
  int k,p1,p2,p3,iseg;
  double x1,y1,x2,y2;
  struct kpt pt1,pt2,pt0;
  for(k=1;k<=KNOTL[0].n;++k)
    {
      iseg = KNOTL[k].n;
      p1 = iseg;
      do {
	PATH[p1].tag = 1;
	p1 = PATH[p1].toPoint;
      }while(p1!=iseg);
      while(PATH[iseg].tag==1)
	{
	  p1 = iseg;           /* pick initial segment */
	  x1 = PATH[p1].x;
	  y1 = PATH[p1].y;
	  PATH[p1].tag = 0;
	  p2 = PATH[p1].toPoint; 
	  x2 = PATH[p2].x;
	  y2 = PATH[p2].y;
	  p3 = PATH[p1].fromPoint;     /* extend backwards */
	  while(Ldist(PATH[p3].x,PATH[p3].y,x1,y1,x2,y2)<=2)
	    {
	      PATH[p1].tag = 0;
	      p1 = p3;
	      x1 = PATH[p1].x;
	      y1 = PATH[p1].y;
	      p3 = PATH[p1].fromPoint; 
	    }
	  p3 = PATH[p2].toPoint;      /* extend forwards */
	  while(Ldist(PATH[p3].x,PATH[p3].y,x1,y1,x2,y2)<=2)
	    { 
	      PATH[p2].tag = 0;
	      p2 = p3;
	      x2 = PATH[p2].x;
	      y2 = PATH[p2].y;
	      p3 = PATH[p2].toPoint;
	    }
	  if(p2!=PATH[p1].toPoint)   /* remove some points! */
	    {
	      PATH[p1].tag = 0;
	      Pfirst(&pt1,p1);
	      pt0 = pt1;
	      Knext(PATH,INTER,&pt1);
	      while((pt1.node!=p2)||(pt1.wstruct!=onPath))
		{
		  pt2 = pt1;
		  Knext(PATH,INTER,&pt2);
		  if(pt1.wstruct==onCross)
		    {
		      if(pt0.wstruct==onCross)
			{
			  INTER[pt0.node].toCross[pt0.level] = pt1.node;
			  INTER[pt0.node].toLev[pt0.level] = pt1.level;
			  INTER[pt1.node].fromCross[pt1.level] = pt0.node;
			  INTER[pt1.node].fromLev[pt1.level] = pt0.level;
			}
		      if(PATH[p1].clist==-1)
			{
			  PATH[p1].clist = pt1.node;
			  PATH[p1].toLev = pt1.level;
			}
		      pt0 = pt1;
		      INTER[pt1.node].toPoint[pt1.level] = p2;
		      INTER[pt1.node].fromPoint[pt1.level] = p1;
		    }
		  else
		    {
		      PATH[pt1.node].touch -= 1;
		      if(KNOTL[k].n==pt1.node)
			KNOTL[k].n = p1;
		    }
		  pt1 = pt2;
		}
	      PATH[p1].toPoint = p2;
	      PATH[p2].fromPoint = p1;
	    }
	  iseg = p2;
	}
    }
}



int safecat(char *grow, char *data, int len) {
  int dlen;
  if(len<=0) {
    return len;
  }
  if(data==NULL) {
    return len;
  }
  dlen = strlen(data);
  if(dlen<=0) {
    return;
  }
  strncat(grow,data,len);
  len -= dlen;
  return len;
}

/* make a knot string */
void make_string(struct point *PATH, struct cross *INTER,
		 struct ktag *KNOTL, char *s, int len)
{
  char temp[strbuflen];
  int k,knot;
  struct kpt p;
  s[0] = 0;
  len = safecat(s,"[\n",len);
  Ltag(PATH,INTER,0);
  k = 1;
  for(knot=1;knot<=KNOTL[0].n;++knot)
    {
      Pfirst(&p,KNOTL[knot].n);
      len = safecat(s,"   [",len);
      while(Kvalid(&p))
	{
	  if(p.wstruct==onCross)
	    {
	      if(INTER[p.node].tag==0)
		{
		  INTER[p.node].tag = k;  
		  ++k;
		  if(p.level==levLow)
		    (void)snprintf(temp,strbuflen," -%d ",INTER[p.node].tag);
		  else
		    (void)snprintf(temp,strbuflen," +%d ",INTER[p.node].tag);
		}
	      else
		{
		  if(Csign(PATH,INTER,p.node)<0)
		    (void)snprintf(temp,strbuflen," -%d ",INTER[p.node].tag);
		  else
		    (void)snprintf(temp,strbuflen," +%d ",INTER[p.node].tag);
		}
	      len = safecat(s,temp,len);
	    }
	  Knext(PATH,INTER,&p);
	}
      len = safecat(s,"]\n",len);
    }
  len = safecat(s,"]\n",len);
}



/* forget a crossing in favor of the points l and m */
/* for use by "A" channel cut only */
void forgetCross(struct cross *INTER,
		 int k,int lev,int l,int m,
		 struct cross *OLDINTER)
{
  int i,j,x;
  j = OLDINTER[k].toLev[lev];
  i = OLDINTER[k].toCross[lev]; 
  if(i!=-1)
    {
      INTER[i].fromCross[j] = -1;
      while(i!=-1)
	{
	  INTER[i].fromPoint[j] = l;
	  x = OLDINTER[i].toLev[j];
	  i = OLDINTER[i].toCross[j];
	  j = x; 
	}
    }
  j = OLDINTER[k].fromLev[lev];
  i = OLDINTER[k].fromCross[lev];
  if(i!=-1)
    {
      INTER[i].toCross[j] = -1;
      while(i!=-1)
	{
	  INTER[i].toPoint[j] = m;
	  x = OLDINTER[i].fromLev[j];
	  i = OLDINTER[i].fromCross[j];
	  j = x;
	}
    }
}


char siglist[][32] = 
  { "ZERO", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGIOT"
    , "SIGEMT", "SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV", "SIGSYS", "SIGPIPE"
    , "SIGALRM", "SIGTERM", "SIGURG", "SIGSTOP", "SIGTSTP", "SIGCONT"
    , "SIGCLD", "SIGTTIN", "SIGTTOU", "SIGIO", "SIGXCPU", "SIGXFSZ"
    , "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGLOST", "SIGUSR1", "SIGUSR2" };

/* catch a siganl and scream bloody murder */
void punt_handler(int sig) 
{
  char *str;
  if((sig>=0)&&(sig<32)) {
    panic(siglist[sig]);
  }
  /* intentionally leak this string */
  str = (char *)malloc(strbuflen);
  snprintf(str,strbuflen,"unkown signal %d",sig);
  panic(str);
}

/* catch X errors */
int mySoftError(Display *disp,XErrorEvent *myerr)
{
  char s[stlen];
  XGetErrorText(disp,myerr->error_code,s,stlen);
  panic(s);
}

int myHardError(Display *disp)
{
  panic("Fatal X error");
}



/* set up the memory */
void initMem()
{
  if((PATH = (struct point *)calloc((unsigned)maxpoint,sizeof(struct point)))
     ==NULL)
    panic_nosave("not enough memory");
  if((TPATH = (struct point *)calloc((unsigned)maxpoint,sizeof(struct point)))
     ==NULL)
    panic_nosave("not enough memory");
  if((UPATH = (struct point *)calloc((unsigned)maxpoint,sizeof(struct point)))
     ==NULL)
    panic_nosave("not enough memory");
  if((OLDPATH = (struct point *)calloc((unsigned)maxpoint,sizeof(struct point)))
     ==NULL)
    panic_nosave("not enough memory");
  if((KNOTL = (struct ktag *)calloc((unsigned)maxlink,sizeof(struct ktag)))
     ==NULL)
    panic_nosave("not enough memory");
  if((TKNTL = (struct ktag *)calloc((unsigned)maxlink,sizeof(struct ktag)))
     ==NULL)
    panic_nosave("not enough memory");
  if((UKNOTL = (struct ktag *)calloc((unsigned)maxlink,sizeof(struct ktag)))
     ==NULL)
    panic_nosave("not enough memory");
  if((OLDKNOTL = (struct ktag *)calloc((unsigned)maxlink,sizeof(struct ktag)))
     ==NULL)
    panic_nosave("not enough memory");
  if((INTER = (struct cross *)calloc((unsigned)maxcross,sizeof(struct cross)))
     ==NULL)
    panic_nosave("not enough memory");
  if((OLDINTER = (struct cross *)calloc((unsigned)maxcross,sizeof(struct cross)))
     ==NULL)
    panic_nosave("not enough memory");
  if((UINTER = (struct cross *)calloc((unsigned)maxcross,sizeof(struct cross)))
     ==NULL)
    panic_nosave("not enough memory");
  if((TCROS = (struct cross *)calloc((unsigned)maxcross,sizeof(struct cross)))
     ==NULL)
    panic_nosave("not enough memory");
  if((Atag = (int *)calloc((unsigned)maxcross,sizeof(int)))==NULL)
    panic_nosave("not enough memory");
  if((Btag = (int *)calloc((unsigned)maxcross,sizeof(int)))==NULL)
    panic_nosave("not enough memory");
  if((Ctag = (int *)calloc((unsigned)maxcross,sizeof(int)))==NULL)
    panic_nosave("not enough memory");
}



/* zoom centering at (dx,dy) using scaling factor scale */
void Zoom(struct point *PATH, struct cross *INTER,
	  struct ktag *KNOTL,
	  double dx, double dy, double scale)
{
  int k;
  struct kpt p;
  /* rescale and reposition */
  for(k=1;k<=KNOTL[0].n;++k)
    {
      Pfirst(&p,KNOTL[k].n);
      while(Kvalid(&p))
	{
	  if(p.wstruct==onPath)
	    {
	      PATH[p.node].x = (PATH[p.node].x-dx)*scale + winx/2.0;
	      PATH[p.node].y = (PATH[p.node].y-dy)*scale + winy/2.0
		+ deltcom;
	    }
	  else
	    {
	      if((INTER[p.node].touch==cross_key(INTER))
		 &&(p.level==levLow))
		{
		  INTER[p.node].x = (INTER[p.node].x-dx)*scale 
		    + winx/2.0;
		  INTER[p.node].y = (INTER[p.node].y-dy)*scale 
		    + winy/2.0 + deltcom;
		}
	    }
	  Knext(PATH,INTER,&p);
	}
    }
}


/* get the rectangle that contains a given knot */
void Ksize(struct point *PATH, struct cross *INTER,
	   struct ktag *KNOTL,
	   int *np,
	   double *mnx, double *mny, double *mxx, double *mxy)
{
  int n,k;
  struct kpt p;
  double minx,miny,maxx,maxy;
  n = 0;
  minx = 0;
  miny = 0;
  maxx = 0;
  maxy = 0;
  for(k=1;k<=KNOTL[0].n;++k)   /* get size */
    {
      Pfirst(&p,KNOTL[k].n);
      while(Kvalid(&p))
	{
	  if(p.wstruct==onPath)
	    {
	      if((PATH[p.node].x<minx)||(n==0))
		minx = PATH[p.node].x;
	      if((PATH[p.node].y<miny)||(n==0))
		miny = PATH[p.node].y;
	      if((PATH[p.node].x>maxx)||(n==0))
		maxx = PATH[p.node].x;
	      if((PATH[p.node].y>maxy)||(n==0))
		maxy = PATH[p.node].y;
	      ++n;
	    }
	  else
	    {
	      if((INTER[p.node].touch==cross_key(INTER))
		 &&(p.level==levLow))
		{
		  if((INTER[p.node].x<minx)||(n==0))
		    minx = INTER[p.node].x;
		  if((INTER[p.node].y<miny)||(n==0))
		    miny = INTER[p.node].y;
		  if((INTER[p.node].x>maxx)||(n==0))
		    maxx = INTER[p.node].x;
		  if((INTER[p.node].y>maxy)||(n==0))
		    maxy = INTER[p.node].y;
		  ++n;
		}
	    }
	  Knext(PATH,INTER,&p);
	}
    }
  *np = n;
  *mnx = minx;
  *mny = miny;
  *mxx = maxx;
  *mxy = maxy;
}




/* set the veiw point such that all of the knot is on the screen with a border*/
void Nveiw(struct point *PATH, struct cross *INTER, struct ktag *KNOTL,
	   struct point *OLDPATH, struct cross *OLDINTER, struct ktag *OLDKNOTL)
{
  double minx,miny,maxx,maxy,dx,dy,scale;
  int n;
  windowSize(theWindow,&winy,&winx);
  Ksize(PATH,INTER,KNOTL,&n,&minx,&miny,&maxx,&maxy);
  if(n!=0)
    {
      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
      dx = (maxx+minx)/2;
      dy = (maxy+miny)/2;
      if(MAX(maxx-minx,maxy-miny)<=0)
	scale = 1;
      else
	{
	  scale = scale_border*MIN((double)winx/(maxx-minx)
				   ,(double)(winy-delta)/(maxy-miny));
	}
      Zoom(PATH,INTER,KNOTL,dx,dy,scale);
    }
}



#define minX 500
#define minY 300
extern void make_braid(char *inst, char *outst,int outlen);

void make_banner()
{
  char s[stlen];
  x_move(labx,laby);
  x_label(version_stamp);
  (void)snprintf(s,strbuflen,"Hello %s",log_name);
  x_label(s);
  x_label("-------------------");
  x_label("     This is a maintinance update of the  1988 version.");
  x_label(" Email me at: j@mzlabs.com .");
}


/* initailizers MUST be called in the giver order */
main(int argc, char *argv[])
{
  FILE *CUR_IN,*CUR_OUT;
  KeySym keysym;
  XComposeStatus compose;
  int i,done,k,l,j,m,x,y,x2,y2,c,next_sel,keep_banner,key_was_legal;
  char s[big_stlen],t[stlen],stout[big_stlen];
  XEvent report;
  struct kpt p,p2;
  double minx,maxx,miny,maxy,dx,dy,scale;

  keep_banner = 1;
  PANICING = 0;
  MEMREADY = 0;
  for(k=1;k<=31;++k)  /* set up signals */
    if((k!=26)&&(k!=SIGCLD)) 
      (void)signal(k,punt_handler);
  userId();  /*  get user's name  and set corret ownership */
  (void)snprintf(version_stamp,strbuflen,"           knotEd version 1.6x by John Mount (c)");
  (void)fprintf(stderr,"%s\n",version_stamp);
  (void)fflush(stderr);
  winx = 600;
  winy = 600;
  comx = 200;
  comy = 600;
  initGraphics();
  /* trap X errors */
  (void)XSetErrorHandler(mySoftError);
  (void)XSetIOErrorHandler(myHardError);
  Init_men();
  K_0.n = 0;
  K_0.l1 = 1;
  K_0.l2 = 0;
  Bnarcs = 10;
  LpA = 4;
  GLOBkl = 0;
  GLOBspline = 0;
  GLOBfast = 0;
  GLOBmpoint = 0;
  GLOBsign = 0;
  name_valid = 0;
  ISOTOPY_LOCK = 0;
  GLOBghost = 0;
  GLOBtag = 0;
  GLOBarrow = 0;
  GLOBoutmode = devX;
#ifdef tek4014
  GLOBTScale = 5.0;
#endif
#ifdef PIC
  GLOBTScale = 0.005;
#endif
  Bparm = 0.8;
  GLOBCrad = 12;
  CUR_IN = stdin;
  last_sel = -1;
  done = 0;
  maxpoint = 1000;
  maxcross = 1000;
  maxlink = 200;
  REVERSEcolors = False;

  i = 1;
  while((argc-i>0)&&(argv[i][0]=='-'))
    {
      switch(argv[i][1])
	{
	case 'r':
	  REVERSEcolors = True;
	  break;
	case 'p':
          maxpoint = atoi(&(argv[i][2]));
          break;
	case 'c':
          maxcross = atoi(&(argv[i][2]));
          break;
	case 'l':
          maxcross = atoi(&(argv[i][2]));
          break; 
	default:
	  panic_nosave("bad flag (knotEd [-p#] [-c#] [-l#] [knotname])");
	}
      ++i;
    }
  initWindows(argc,argv);
  initMem();
  dispose_PATH(PATH);
  dispose_INTER(INTER);
  clearKnot(PATH,INTER,KNOTL);
  if(argc-i>0)
    {
      (void)snprintf(s,strbuflen,"%s.knot",argv[i]);
      if((CUR_IN=fopen(s,"r"))!=NULL)
	read_knot(CUR_IN,PATH,INTER,KNOTL); 
      else
	(void)fprintf(stderr,"couldn't open file %s\n",s);
    }
  MEMREADY = 1;
  Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
  /*XDefineCursor(comWindow,com_curs); */
  /*XDefineCursor(theWindow,curs); */
  Tmenu();
  Init_st();
  plotKnot(PATH,INTER,KNOTL);
  make_banner();

  while(!done)
    {
      c = 0;
      while(c==0)
	{
	  XNextEvent(X11_display,&report);
	  if(report.xany.window==comWindow)
	    {
	      switch(report.type)
		{
		case MotionNotify:
		  x = report.xmotion.x;
		  y = report.xmotion.y;
		  next_sel = (y-top_gap+com_font->ascent)/deltcom;
		  if(next_sel!=last_sel)
		    {
		      if(last_sel>=0)
			Tline(last_sel,blackInk);
		      if((next_sel<0)||(next_sel>=Ncom))
			last_sel = -1;
		      else
			last_sel = next_sel;
		      if(last_sel>=0)
			Tline(last_sel,whiteInk);
		    }
		  break;
		case Expose:
		  Tmenu();
		  break;
		case ButtonPress:
		  if(last_sel>=0)
		    c = Xmes[last_sel][1];
		  break;
		default:
		  break;
		}
	    }
	  else
	    {   
	      if(report.xany.window==theWindow)
		{
		  switch(report.type)
		    {
		    case ButtonPress:
		      interact(report);
		      c = 0;
		      break;
		    case Expose:
		      c = '.';
		      break;
		    case KeyPress:
		      i = XLookupString((XKeyEvent*)&report,s,stlen,&keysym,&compose);
		      if(i==1)
			{
			  c = s[0];
			  if((c>=0)&&(c<=255)
			     &&(com_list[c]!=-1)&&(last_sel!=com_list[c]))
			    {
			      if(last_sel>=0)
				Tline(last_sel,blackInk);
			      last_sel = com_list[c];
			      Tline(last_sel,whiteInk);
			    }
			}
		      break;
		    default:
		      break;
		    }
		}
	      /* icon refresh could go here */
	    }
	}
      /* WAY too big switch statement */
      key_was_legal = 1;
      switch(c)
	{
	case ';':
	  Cprint("\"A\" channel cut",f1);
	  /*XDefineCursor(theWindow,acut_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  get_xy(&x,&y,f2);
	  k = Kcross(x,y,KNOTL,PATH,INTER);
	  if(k!=-1)
	    {
	      name_valid = 0;
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      l = newpt(PATH);
	      m = newpt(PATH); 
	      PATH[l].knot = PATH[INTER[k].toPoint[levLow]].knot;
	      PATH[m].knot = PATH[INTER[k].toPoint[levHigh]].knot;
	      PATH[l].x = ( GPROX*INTER[k].x 
			    + ((1-GPROX)/2.0)*PATH[INTER[k].toPoint[levLow]].x
			    + ((1-GPROX)/2.0)*PATH[INTER[k].fromPoint[levHigh]].x );
	      PATH[l].y = ( GPROX*INTER[k].y 
			    + ((1-GPROX)/2.0)*PATH[INTER[k].toPoint[levLow]].y
			    + ((1-GPROX)/2.0)*PATH[INTER[k].fromPoint[levHigh]].y );
	      PATH[m].x = ( GPROX*INTER[k].x 
			    + ((1-GPROX)/2.0)*PATH[INTER[k].toPoint[levHigh]].x
			    + ((1-GPROX)/2.0)*PATH[INTER[k].fromPoint[levLow]].x );
	      PATH[m].y = ( GPROX*INTER[k].y 
			    + ((1-GPROX)/2.0)*PATH[INTER[k].toPoint[levHigh]].y
			    + ((1-GPROX)/2.0)*PATH[INTER[k].fromPoint[levLow]].y );
	      PATH[l].toPoint = INTER[k].toPoint[levLow];
	      PATH[l].fromPoint = INTER[k].fromPoint[levHigh];
	      PATH[l].clist = INTER[k].toCross[levLow];
	      PATH[l].toLev = INTER[k].toLev[levLow];
	      PATH[m].toPoint = INTER[k].toPoint[levHigh];
	      PATH[m].fromPoint = INTER[k].fromPoint[levLow];
	      PATH[m].clist = INTER[k].toCross[levHigh];
	      PATH[m].toLev = INTER[k].toLev[levHigh];
	      forgetCross(INTER,k,levLow,l,m,OLDINTER);
	      forgetCross(INTER,k,levHigh,m,l,OLDINTER);
	      if(PATH[INTER[k].fromPoint[levLow]].clist==k)
		PATH[INTER[k].fromPoint[levLow]].clist = -1;
	      if(PATH[INTER[k].fromPoint[levHigh]].clist==k)
		PATH[INTER[k].fromPoint[levHigh]].clist = -1;
	      PATH[INTER[k].fromPoint[levLow]].toPoint = m;
	      PATH[INTER[k].toPoint[levLow]].fromPoint = l;
	      PATH[INTER[k].fromPoint[levHigh]].toPoint = l;
	      PATH[INTER[k].toPoint[levHigh]].fromPoint = m;
	      /* fix the modified knot into the correct number of components*/
	      INTER[k].touch -= 1;
	      fix_comp(PATH,KNOTL,l,m);
	      /* just in case our redraw caused a crossing */
	      soft_spot.state = invalidSoft;
	      (void)find_cross(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL
			       ,&soft_spot);
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
	    }
	  /*else
	    XFeep(0);*/
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  XSync(X11_display,1);
	  Cmenu();
	  break;
	case 'h':
	  Cprint("reverse knot direction",f1);
	  /*XDefineCursor(theWindow,reverse_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  get_xy(&x,&y,f2);
	  k = node((double)x,(double)y,KNOTL,PATH);  
	  if(k!=-1)
            {
	      name_valid = 0;
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL); 
	      Pfirst(&p,k);
	      m = OLDPATH[k].knot;
	      while(Kvalid(&p))
		{
		  if(p.wstruct==onPath)
		    {
		      PATH[p.node].fromPoint = OLDPATH[p.node].toPoint;
		      PATH[p.node].toPoint = OLDPATH[p.node].fromPoint;
		      p2 = p;
		      Kprev(OLDPATH,OLDINTER,&p2);
		      if(p2.wstruct==onCross)
			{
			  PATH[p.node].clist = p2.node;
			  PATH[p.node].toLev = p2.level;
			}
		      else
			{
			  PATH[p.node].clist = -1;
			  PATH[p.node].toLev = 0;
			}
		    }
		  else
		    {
		      j = p.level;
		      if(OLDPATH[OLDINTER[p.node].toPoint[j]].knot==m)
			{
			  INTER[p.node].fromPoint[j] = OLDINTER[p.node].toPoint[j];
			  INTER[p.node].toPoint[j] = OLDINTER[p.node].fromPoint[j];
			  INTER[p.node].fromCross[j] = OLDINTER[p.node].toCross[j];
			  INTER[p.node].toCross[j] = OLDINTER[p.node].fromCross[j];
			  INTER[p.node].fromLev[j] = OLDINTER[p.node].toLev[j];
			  INTER[p.node].toLev[j] = OLDINTER[p.node].fromLev[j];
			}
		    }
		  Knext(OLDPATH,OLDINTER,&p);
		}
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  /*else
            XFeep(0);*/
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  XSync(X11_display,1);
	  Cmenu();
	  break;
	case 'a':
	  Cprint("toggle arrows",f1);
	  GLOBarrow = !GLOBarrow;
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'l':
	  Cprint("toggle isotopy lock",f1);
	  ISOTOPY_LOCK = !ISOTOPY_LOCK;
	  Cmenu();
	  break;
	case 'b':
	  Cprint("toggle spline",f1);
	  GLOBspline = !GLOBspline;
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'v':
	  Cprint("toggle vertex drawing",f1);
	  GLOBmpoint = !GLOBmpoint;
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case '-':
	  Cprint("toggle sign display",f1);
	  GLOBsign = !GLOBsign;
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'f':
	  Cprint("toggle cross tags",f1);
	  GLOBtag = !GLOBtag;
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'i':
	  Cprint("toggle knot labels",f1);
	  GLOBkl = !GLOBkl;
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'g':
	  Cprint("toggle ghost lines",f1);
	  GLOBghost = !GLOBghost;
	  if(GLOBspline)
            {
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  Cmenu();
	  break;
	case 'p':
	  Cprint("adjust spline (0=meth2)",f1);
	  (void)snprintf(t,strbuflen,"prox = %f",Bparm);
	  Cprint(t,f2);
	  Cread(t,f3);
	  Bparm = atof(t);
	  if(GLOBspline)
            {
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  Cmenu();
	  break;
	case 'j':
	  Cprint("adjust curve draw",f1);
	  (void)snprintf(t,strbuflen,"lines per curve = %d",Bnarcs);
	  Cprint(t,f2);
	  Cread(t,f3);
	  Bnarcs = atoi(t);
	  if(GLOBspline)
            {
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  Cmenu();
	  break;
	case '\'':
	  Cprint("Adjust Line width",f1);
	  (void)snprintf(t,strbuflen,"Width = %d  (0 for fast)?",GLOBfast);
	  Cprint(t,f2);
	  Cread(t,f3);
	  GLOBfast = atoi(t);
	  XSetLineAttributes(X11_display,theGc,GLOBfast
			     ,LineSolid,CapRound,JoinRound);
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'y':
	  Cprint("adjust arrow density",f1);
	  (void)snprintf(t,strbuflen,"curves per arrow = %d",LpA);
	  Cprint(t,f2);
	  Cread(t,f3);
	  LpA = atoi(t);
	  if(GLOBarrow)
            {
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  Cmenu();
	  break;
	case 'w':
	  Cprint("adjust cross width",f1);
	  (void)snprintf(t,strbuflen,"width = %d",GLOBCrad);
	  Cprint(t,f2);
	  Cread(t,f3);
	  GLOBCrad = atoi(t);
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'k':
	  Cprint("adjust hard scale",f1);
	  (void)snprintf(t,strbuflen,"scale = %f",GLOBTScale);
	  Cprint(t,f2);
	  Cread(t,f3);
	  GLOBTScale = atof(t);
	  Cmenu();
	  break;
	case '.':
	  Cprint("Refresh",f1);
	  XSync(X11_display,1);         /* prevent refresh stutter */
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  if(keep_banner)
            make_banner();
	  else
            {
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  Tmenu();
	  break;
	case 'q': 
	  Cprint("Quit",f1);
	  Cprint("Really quit ?(y/n)",f2);
	  Cread(t,f3);
	  if((t[0]=='y')||(t[0]=='Y'))
            {
	      (void)fclose(stdout);
	      done = 1; 
            }
	  else
            Cmenu();
	  break;
	case 'c':
	  Cprint("Clear knot",f1);
	  name_valid = 0;
	  Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	  clearKnot(PATH,INTER,KNOTL);
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
#ifdef tek4014
	case 't':
	  Cprint("Tek plot knot",f1);
	  Cprint("filename",f2);
	  Cread(t,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  if(s_open(t,".tek"))
            {
	      GLOBoutmode = devTEK;
	      (void)fflush(stdout);
	      openpl();
	      erase();
	      plotKnot(PATH,INTER,KNOTL);
	      move(50,50);
	      label(t);
	      closepl();
	      (void)fflush(stdout);
	      GLOBoutmode = devX; 
            }
	  /*else
            XFeep(0);*/
	  Cmenu();
	  /*XDefineCursor(comWindow,com_curs);*/
	  /*XDefineCursor(theWindow,curs);*/
	  XSync(X11_display,1);
	  break;
#endif
#ifdef PIC
	case 't':
	  Cprint("troffable output",f1);
	  Cprint("filename",f2);
	  Cread(t,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  if(s_open(t,".pic"))
            {
	      i = strlen(t) - 1;
	      while((i>0)&&(t[i-1]!='/'))
		--i;
	      GLOBoutmode = devPIC;
	      (void)fflush(stdout);
	      (void)printf("K_%s: [\n",&(t[i]));
	      plotKnot(PATH,INTER,KNOTL);
	      (void)fflush(stdout);
	      GLOBoutmode = devX; 
            }
	  /*else
            XFeep(0);*/
	  Cmenu();
	  /*XDefineCursor(comWindow,com_curs);*/
	  /*XDefineCursor(theWindow,curs);*/
	  XSync(X11_display,1);
	  break;
#endif
	case 's':
	  Cprint("Save knot",f1);
	  Cprint("filename",f2);
	  Cread(t,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  XFlush(X11_display);
	  if((CUR_OUT=g_open(t,".knot"))!=NULL)
            save_knot(CUR_OUT,PATH,INTER,KNOTL);
	  /*else
            XFeep(0);*/
	  Cmenu();
	  /*XDefineCursor(comWindow,com_curs);*/
	  /*XDefineCursor(theWindow,curs);*/
	  XSync(X11_display,1);
	  break;
	case '[':
	  Cprint("write knot string",f1);
	  Cprint("filename",f2);
	  Cread(t,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  XFlush(X11_display);
	  make_string(PATH,INTER,KNOTL,stout,big_stlen);
	  name_valid = 1;
	  if((CUR_OUT=g_open(t,".string"))!=NULL)
            {
	      (void)fprintf(CUR_OUT,"%s",stout);
	      (void)fclose(CUR_OUT);
            }
	  /*else
            XFeep(0);*/
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  x_label("Knot string =");
	  x_label(stout);
	  Cmenu();
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  XSync(X11_display,1);
	  break;
	case 'o':
	  Cprint("write braid word",f1);
	  Cprint("filename",f2);
	  Cread(t,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  XFlush(X11_display);
	  make_string(PATH,INTER,KNOTL,stout,big_stlen);
	  name_valid = 1;
	  make_braid(stout,s,big_stlen);
	  if((CUR_OUT=g_open(t,".braid"))!=NULL)
            {
	      (void)fprintf(CUR_OUT,"%s",s);
	      (void)fclose(CUR_OUT);
            }
	  /*else
            XFeep(0);*/
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  x_label("Braid word = ");
	  x_label(s);
	  Cmenu();
	  /*XDefineCursor(comWindow,com_curs);*/
	  /*XDefineCursor(theWindow,curs);*/
	  XSync(X11_display,1);
	  break;
	case 'e':
	  Cprint("READ BRAID WORD",f1);
	  Cprint("filename",f2);
	  Cread(t,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  XFlush(X11_display);
	  (void)snprintf(s,strbuflen,"%s.braid",t);
	  if((CUR_IN=fopen(s,"r"))!=NULL)
            {
	      name_valid = 0;
	      windowSize(theWindow,&winy,&winx);
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      CreateBraid(PATH,INTER,KNOTL,CUR_IN);
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  Cmenu();
	  /*XDefineCursor(comWindow,com_curs);*/
	  /*XDefineCursor(theWindow,curs);*/
	  XSync(X11_display,1);
	  break;
	case '1' :
	case '2' :
	  Cprint("Add knot",f1);
	  name_valid = 0;
	  /*
	    if(c=='2')
            XDefineCursor(theWindow,add2_curs);
	    else
            XDefineCursor(theWindow,add1_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  get_xy(&x,&y,f2);
	  if(x!=-1)
            {
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      PKnot(PATH,KNOTL,(double)x,(double)y);
	      if(c=='2')
		KNOTL[KNOTL[0].n].l1 = -1;
	      soft_spot.state = lockedSoft;
	      soft_spot.P1 = KNOTL[KNOTL[0].n].n;
	      soft_spot.P2 = KNOTL[KNOTL[0].n].n;
	      soft_spot.level = levHigh;
	      soft_spot.lcross = -1;
	      (void)find_cross(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL
			       ,&soft_spot);
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  /*else
            XFeep(0);*/
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  XSync(X11_display,1);
	  Cmenu();
	  break;
	case ',':
	  Cprint("Move knot",f1);
	  /*XDefineCursor(theWindow,move_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  get_xy(&x,&y,f2);
	  l = node((double)x,(double)y,KNOTL,PATH);
	  if(l!=-1)
            {
	      Pfirst(&p,l);
	      while(Kvalid(&p))
                {
		  x_circle((int)Kx(PATH,INTER,&p),(int)Ky(PATH,INTER,&p),5); 
		  Knext(PATH,INTER,&p);
                }
	      /*XDefineCursor(theWindow,pul_curs);*/
	      XSync(X11_display,1);
	      get_xy(&x2,&y2,f2);
	      if(x2!=-1)
		{
		  Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
		  Pfirst(&p,l);
		  while(Kvalid(&p))
		    {
		      if(p.wstruct==onCross)
			INTER[p.node].tag = 1;
		      Knext(PATH,INTER,&p);
		    }
		  Pfirst(&p,l);
		  soft_spot.state = initialSoft;
		  soft_spot.P1 = p.node;
		  soft_spot.P2 = p.node;
		  soft_spot.lcross = -1;
		  soft_spot.level = levHigh;
		  while(Kvalid(&p))
		    {
		      if(p.wstruct==onPath)
			{
			  PATH[p.node].x += x2-x;
			  PATH[p.node].y += y2-y;
			}
		      else
			{
			  if((INTER[p.node].touch==cross_key(INTER))
			     &&(INTER[p.node].tag==1))
			    {
			      INTER[p.node].tag = 0;
			      if((PATH[INTER[p.node].toPoint[levLow]].knot
				  !=PATH[l].knot)
				 ||(PATH[INTER[p.node].toPoint[levHigh]].knot
				    !=PATH[l].knot)) 
				softDitchCross(PATH,INTER,p.node,&soft_spot);
			      else
				{
				  INTER[p.node].x += x2-x;
				  INTER[p.node].y += y2-y;
				}
			    }
			}
		      Knext(OLDPATH,OLDINTER,&p);
		    }
		  name_valid = 0;
		  (void)find_cross(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL
				   ,&soft_spot);
		}
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  Cmenu();
	  XSync(X11_display,1);
	  break;
	case 'm':
	  Cprint("Move all",f1);
	  /*XDefineCursor(theWindow,move_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  get_xy(&x,&y,f2);
	  if(x!=-1)
            {
	      /*XDefineCursor(theWindow,pul_curs);*/
	      XSync(X11_display,1);
	      get_xy(&x2,&y2,f2);
	      if(x2!=-1)
		{
		  Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
		  for(k=1;k<=KNOTL[0].n;++k)
		    {
		      Pfirst(&p,KNOTL[k].n);
		      while(Kvalid(&p))
			{
			  if(p.wstruct==onPath)
			    {
			      PATH[p.node].x += x2-x;
			      PATH[p.node].y += y2-y;
			    }
			  else
			    {
			      if((INTER[p.node].touch==cross_key(INTER))
				 &&(p.level==levLow))
				{
				  INTER[p.node].x += x2-x;
				  INTER[p.node].y += y2-y;
				}
			    }
			  Knext(OLDPATH,OLDINTER,&p);
			}
		    }
		  XClearWindow(X11_display,theWindow);
		  plotKnot(PATH,INTER,KNOTL);
		  x_move(labx,laby);
		  x_label(Imesg);
		}
            }
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  Cmenu();
	  XSync(X11_display,1);
	  break;
	case '=':
	  Cprint("Push off",f1);
	  /*XDefineCursor(theWindow,push_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  get_xy(&x,&y,f2);
	  l = node((double)x,(double)y,KNOTL,PATH);
	  if(l!=-1)
            {
	      k = ISOTOPY_LOCK;
	      ISOTOPY_LOCK = 0;
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      (void)push_off(PATH,INTER,KNOTL,l);
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
	      name_valid = 0;
	      ISOTOPY_LOCK = k;
            }
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  Cmenu();
	  XSync(X11_display,1);
	  break;
	case 'n':
	  Cprint("Normal view",f1);
	  /*XDefineCursor(theWindow,wait_curs);*/
	  /*XDefineCursor(comWindow,wait_curs);*/
	  XFlush(X11_display);
	  Nveiw(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  Cmenu();
	  XSync(X11_display,1);
	  break;
	case 'x':
	  Cprint("Zoom in",f1);
	  /*XDefineCursor(theWindow,zoom1_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XFlush(X11_display);
	  windowSize(theWindow,&winy,&winx);
	  get_xy(&x,&y,f2);
	  if(x!=-1)
            {
	      minx = (double)x;
	      miny = (double)y;
	      x_line(x,y,x+20,y);
	      x_line(x,y,x,y+20);
	      /*XDefineCursor(theWindow,zoom2_curs);*/
	      get_xy(&x,&y,f2);
	      if((x!= -1)&&(((int)minx)!=x)&&(((int)miny)!=y))
		{
		  Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
		  maxx = (double)x;
		  maxy = (double)y;
		  dx = (maxx+minx)/2;
		  dy = (maxy+miny)/2;
		  scale = scale_border*MIN((double)winx/(maxx-minx)
					   ,(double)(winy-deltcom)/(maxy-miny));
		  Zoom(PATH,INTER,KNOTL,dx,dy,scale);
		}
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  Cmenu();
	  XSync(X11_display,1);
	  break;
	case 'z':
	  Cprint("Zoom out",f1);
	  /*XDefineCursor(theWindow,wait_curs);*/
	  /*XDefineCursor(comWindow,wait_curs);*/
	  XFlush(X11_display);
	  windowSize(theWindow,&winy,&winx);
	  Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	  Zoom(PATH,INTER,KNOTL,(double)winx/2.0,(double)winy/2.0,0.5);
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  Cmenu();
	  XSync(X11_display,1);
	  break;
	case 'd' :
	  Cprint("Delete knot",f1);
	  /*XDefineCursor(theWindow,delek_curs);*/
	  /*XDefineCursor(comWindow,intr_curs);*/
	  XSync(X11_display,1);
	  name_valid = 0;
	  get_xy(&x,&y,f2);
	  l = node((double)x,(double)y,KNOTL,PATH);
	  if(l!=-1)
            { 
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      KNOTL[PATH[l].knot] = KNOTL[KNOTL[0].n];
	      KNOTL[0].n -= 1;
	      k = PATH[l].knot;
	      Pfirst(&p,l);
	      while(Kvalid(&p))
		{
		  if(p.wstruct==onPath)
		    PATH[p.node].touch -= 1;
		  else
		    ditchCross(PATH,INTER,p.node);
		  Knext(OLDPATH,OLDINTER,&p);
		}
	      l = KNOTL[k].n;
	      while(PATH[l].knot!=k)
		{
		  PATH[l].knot = k;
		  l = PATH[l].toPoint;
		}
	      soft_spot.state = invalidSoft;
	      (void)find_cross(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL
			       ,&soft_spot);
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  /*else
            XFeep(0);*/
	  Cmenu();
	  /*XDefineCursor(theWindow,curs);*/
	  /*XDefineCursor(comWindow,com_curs);*/
	  XSync(X11_display,1);
	  break;
	case 'u':
	  Cprint("Undo",f1);
	  /* switch new and old */
	  name_valid = 0;
	  Kbackup(PATH,INTER,KNOTL,UPATH,UINTER,UKNOTL);
	  Kbackup(OLDPATH,OLDINTER,OLDKNOTL,PATH,INTER,KNOTL);
	  Kbackup(UPATH,UINTER,UKNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	  XClearWindow(X11_display,theWindow);
	  plotKnot(PATH,INTER,KNOTL);
	  x_move(labx,laby);
	  x_label(Imesg);
	  Cmenu();
	  break;
	case 'r':
	  Cprint("Read knot from file",f1);
	  Cprint("filename",f2);
	  Cread(s,f3);
	  /*XDefineCursor(comWindow,wait_curs);*/
	  /*XDefineCursor(theWindow,wait_curs);*/
	  XFlush(X11_display);
	  (void)snprintf(t,strbuflen,"%s.knot",s);
	  k = 0;
	  name_valid = 0;
	  if((CUR_IN=fopen(t,"r"))!=NULL)
            {
	      Kbackup(PATH,INTER,KNOTL,OLDPATH,OLDINTER,OLDKNOTL);
	      read_knot(CUR_IN,PATH,INTER,KNOTL);
	      XClearWindow(X11_display,theWindow);
	      plotKnot(PATH,INTER,KNOTL);
	      x_move(labx,laby);
	      x_label(Imesg);
            }
	  /*else
            XFeep(0);*/
	  Cmenu();
	  /*XDefineCursor(comWindow,com_curs);*/
	  /*XDefineCursor(theWindow,curs);*/
	  XSync(X11_display,1);
	  break;
	default:
	  Cmenu();
	  key_was_legal = 0;
	  break;
	}
      if((keep_banner)&&(key_was_legal)&&(c!='.'))
	keep_banner = 0;
      if(last_sel>=0)
	Tline(last_sel,whiteInk);
    }
  return(0);
}

