

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
#include <sys/time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "struct.h"
#include "xdriver.h"
#include "misc.h"
#include "main.h"
#include <math.h>


/*     some variables    */
char log_name[strbuflen];
int PID;




/* exit gracefully on an error condition */
void panic(char *t)
{
  FILE *Pfile;
  char s[strbuflen],tp[strbuflen];
  if(PANICING)
    {
      snprintf(s,strbuflen,"%s & multiple panic",t);
      panic_nosave(s);
      exit(1);
    }
  else
    PANICING = 1;
  if(!MEMREADY)
    panic_nosave(t);
  (void)fprintf(stderr,"\nkn Error: %s\n",t);
  (void)getcwd(tp,strbuflen);
  (void)snprintf(s,strbuflen,"%s/panic%d.knot",tp,PID);
  Pfile=fopen(s,"w");
  if(Pfile==NULL)
    {
      (void)snprintf(s,strbuflen,"/tmp/panic%d.knot",PID);
      Pfile=fopen(s,"w");
    }
  if(Pfile!=NULL)
    {
      save_knot(Pfile,OLDPATH,OLDINTER,OLDKNOTL);
      (void)fprintf(stderr,"Saved previous knot in: %s\n",s);
    }
  else
    {
      (void)fprintf(stderr,"Unable to save last knot.\n");
      s[0] = 0;
    }
  exit(1);
}


/* too fucked up to exit gracefully */
void panic_nosave(char *t)
{
  (void)fprintf(stderr,"\nkn Error: %s\n",t);
  exit(1);
}



/*   try to open a file t with extension s */
FILE *g_open(char *t, char *s)
{
  FILE *tmp;
  char t2[strbuflen];
  if(t[0]==0)
    return(NULL);
  (void)snprintf(t2,strbuflen,"%s%s",t,s);
  if((tmp=fopen(t2,"w"))==NULL)
    {
      Cprint("couldn't open out file",f2);
      return(NULL);
    }
  return(tmp);
}


/*   redirect stdout with filename t and extension s */
int s_open(char *t, char *s)
{
  char t2[strbuflen];
  if(t[0]==0)
    return(0);
  (void)snprintf(t2,strbuflen,"%s%s",t,s);
  if(freopen(t2,"w",stdout)==NULL)
    {
      Cprint("couldn't open out file",f2);
      return(0);
    }
  return(1);
}



/*  get a mouse click using line n of menu for prompt  */
void get_xy(int *x, int *y, int n)
{
  XEvent u;

  Cprint("select a point",n);
  XSync(X11_display,1);
  XNextEvent(X11_display,&u);
  while((u.type!=ButtonPress)||(u.xany.window!=theWindow))
    {
      if(u.type==Expose)
	{
	  if(u.xany.window==comWindow)
	    Tmenu();
	  else
	    if(u.xany.window==theWindow)
	      {
		*x = -1;
		*y = -1;
		return;
	      }
	  /* icon refresh could go here */
	}
      else
	{
	  if(u.type!=MotionNotify)
	    {
	      *x = -1;
	      *y = -1;
	      return;
	    }
	}
      XNextEvent(X11_display,&u);
    }
  *x = u.xmotion.x;
  *y = u.xmotion.y;
}


/* no default as some point is always present */
/* find the shortest distance from point p to any other */
/* point or line segment */
double min_dist(struct point *PATH, 
		struct cross *INTER,
		struct ktag *KNOTL,
		int p)
{
  int k,t,n;
  double d,x,y,td;
  x = PATH[p].x;
  y = PATH[p].y;
  d = MIN(dist2(x,y,PATH[PATH[p].toPoint].x,PATH[PATH[p].toPoint].y)
	  ,dist2(x,y,PATH[PATH[p].fromPoint].x,PATH[PATH[p].fromPoint].y));
  for(k=1;k<=KNOTL[0].n;++k)
    {
      t = KNOTL[k].n;
      do {
	n = PATH[t].toPoint;
	if((t!=p)&&(PATH[t].toPoint!=p))
	  td = pdist(x,y,PATH[t].x,PATH[t].y,PATH[n].x,PATH[n].y);
	if(td<d)
	  d = td;
	t = n;
      }while(t!=KNOTL[k].n);
    }
  return(d);
}


/* get the channel width for a push off */
double chan_width(struct point *PATH,
		  struct cross *INTER,
		  struct ktag *KNOTL,
		  int k)
{
  struct kpt p;
  double d;
  d = huge;
  Pfirst(&p,k);
  while(Kvalid(&p))
    {
      if(p.wstruct==onPath)
	d = MIN(d,min_dist(PATH,INTER,KNOTL,p.node));
      Knext(PATH,INTER,&p);
    }
  return(d); 
}



/*  calculate a unit vector pointing off a point in the direction of the */
/* angle bisector (pointing left as running along the knot) */
void pdelta(struct point *PATH, struct cross *INTER,
	    int p, double *x, double *y)
{
  struct kpt b,a;
  double x1,y1,x2,y2,d;
  Pfirst(&b,p);
  Kprev(PATH,INTER,&b);
  Pfirst(&a,p);
  Knext(PATH,INTER,&a);
  x1 = PATH[p].x - Kx(PATH,INTER,&b);
  y1 = PATH[p].y - Ky(PATH,INTER,&b);
  d = dist2(0.0,0.0,x1,y1);
  x1 = x1/d;
  y1 = y1/d;
  x2 = Kx(PATH,INTER,&a) - PATH[p].x; 
  y2 = Ky(PATH,INTER,&a) - PATH[p].y;
  d = dist2(0.0,0.0,x2,y2);
  x2 = x2/d;
  y2 = y2/d;
  *x = (y1+y2)/2.0;
  *y = -(x1+x2)/2.0;
  d = dist2(0.0,0.0,*x,*y);
  *x = *x/d;
  *y = *y/d;
}


/* build a push-off */
/* which is in itself useless but is a first step for a channel sum or */
/* doubling of a link */
int push_off(struct point *PATH,
	     struct cross *INTER,
	     struct ktag *KNOTL,
	     int p)
{
  double w,dx,dy;
  struct kpt o;
  int n,first,last,p1,p3;
  w = chan_width(PATH,INTER,KNOTL,p)/2.0;
  first = -1;
  last = -1;
  KNOTL[0].n += 1;
  KNOTL[KNOTL[0].n] = K_0;             /* put in points */
  first = -1;
  last = -1;
  Pfirst(&o,p);
  while(Kvalid(&o))
    {
      if(o.wstruct==onPath)
	{
	  n = newpt(PATH);
	  pdelta(PATH,INTER,o.node,&dx,&dy);
	  dx = w*dx;
	  dy = w*dy;
	  PATH[n].x = PATH[o.node].x + dx; 
	  PATH[n].y = PATH[o.node].y + dy;
	  PATH[n].knot = KNOTL[0].n;
	  if(last==-1)
	    {
	      KNOTL[KNOTL[0].n].n = n; 
	      first = n;
	    }
	  else
	    {
	      PATH[n].fromPoint = last;
	      PATH[last].toPoint = n;
	    }
	  last = n;
	}
      Knext(PATH,INTER,&o);
    }
  PATH[last].toPoint = first;
  PATH[first].fromPoint = last;
  Kbackup(PATH,INTER,KNOTL,UPATH,UINTER,UKNOTL);  /* brute force in crossings */
  soft_spot.state = invalidSoft;
  (void)find_cross(PATH,INTER,KNOTL,UPATH,UINTER,UKNOTL,&soft_spot);
  Kbackup(PATH,INTER,KNOTL,UPATH,UINTER,UKNOTL);  
  /* build map from new knot to old */
  p1 = p;
  p3 = first;
  do {
    PATH[p3].tag = p1;
    PATH[p1].tag = p3;
    p1 = PATH[p1].toPoint;
    p3 = PATH[p3].toPoint;
  }while(p1!=p);
  /* fix up the crossings to match the older knot */
  Pfirst(&o,first);
  while(Kvalid(&o))
    {
      if(o.wstruct==onCross)
	{
	  if((PATH[INTER[o.node].toPoint[levLow]].knot==PATH[p].knot)
	     ||(PATH[INTER[o.node].toPoint[levHigh]].knot==PATH[p].knot))
	    {
	      if(PATH[INTER[o.node].toPoint[levHigh]].knot==PATH[p].knot)
		flipCross(PATH,INTER,o.node);
	    }
	  else
	    {
	      /* find the matching crossing */
	      p1 = INTER[o.node].fromPoint[levLow];
	      if(PATH[p1].knot==KNOTL[0].n)
		p1 = PATH[p1].tag;
	      p3 = INTER[o.node].fromPoint[levHigh];
	      if(PATH[p3].knot==KNOTL[0].n)
		p3 = PATH[p3].tag;
	      if((n = Gcross(p1,p3,UPATH,UINTER))==-1)
		panic("push off confused");
	      else
		{
		  if(Csign(PATH,INTER,n)!=Csign(PATH,INTER,o.node))
		    flipCross(PATH,INTER,o.node);
		}
	    }
	}
      Knext(UPATH,UINTER,&o);
    }
  /* add in the twist here before returning to another routine  */
  return(KNOTL[0].n);
}


/* poject an endpoint of a line segment into a window return 1 if okay */
/* 0 if line segment misses window */
int Wcorrect(double *x, double *y, double x2, double y2)
{
  int cn,i;
  double cx[2],cy[2],wx[4],wy[4];
  /* check if point is allready inbounds */
  if((*x>=0)&&(((int)*x)<=winx)&&(*y>=0)&&(((int)*y)<=winy))
    return(1);
  /* else fix it */
  cn = 0;
  /* set up window boundaries */
  wx[0] = (double)0;
  wy[0] = (double)0;
  wx[1] = (double)winx;
  wy[1] = (double)0;
  wx[2] = (double)winx;
  wy[2] = (double)winy;
  wx[3] = (double)0;
  wy[3] = (double)winy;
  /* find intersections (there are at most 2) */
  for(i=0;(i<4)&&(cn<2);++i)
    if(!ldisj(*x,*y,x2,y2
	      ,wx[i],wy[i],wx[(i+1)%4],wy[(i+1)%4],&(cx[cn]),&(cy[cn])))
      ++cn;
  if(cn==0)
    return(0);   /* line segment does not touch window */
  /* else */
  if((cn>1)&&(dist2(*x,*y,cx[0],cy[0])>dist2(*x,*y,cx[1],cy[1])))
    {
      *x = cx[1];
      *y = cy[1];
      return(1);
    }
  /* else */
  *x = cx[0];
  *y = cy[0];
  return(1);
}

/* put a line to the X display or the tektronics emulator depending */
/* on current output settings */
void Vline(double x1, double y1, double x2, double y2)
{
  if(!Wcorrect(&x1,&y1,x2,y2))
    return;
  if(!Wcorrect(&x2,&y2,x1,y1))
    return;
  switch(GLOBoutmode)
    {
    case devX:
      x_line((int)x1,(int)y1,(int)x2,(int)y2);
      break;
#ifdef tek4014
    case devTEK:
      line((int)(GLOBTScale*(x1)),(int)(GLOBTScale*(winy-y1))
	   ,(int)(GLOBTScale*(x2)),(int)(GLOBTScale*(winy-y2)));
      break;
#endif
#ifdef PIC
    case devPIC:
      (void)printf("line from (%.3fi,%.3fi) to (%.3fi,%.3fi)\n"
		   ,GLOBTScale*x1,GLOBTScale*(winy-y1)
		   ,GLOBTScale*x2,GLOBTScale*(winy-y2));
      break;
#endif
    default:
      panic("bad output mode");
    }
}



/* put text to X or TEK depending on output mode settings */
void Vlabel(double x, double y, char *s)
{
  if((x<0)||(((int)x)>winx)||(y<0)||(((int)y)>winx))
    return;
  switch(GLOBoutmode)
    {
    case devX:
      x_move((int)x,(int)y);
      x_label(s);
      break;
#ifdef tek4014
    case devTEK:
      move((int)(GLOBTScale*(x)),(int)(GLOBTScale*(winy-y)));
      label(s);
      break;
#endif
#ifdef PIC
    case devPIC:
      printf("\"%s\" above at (%.3fi,%.3fi)\n",s
	     ,GLOBTScale*x,GLOBTScale*(winy-y));
      break;
#endif
    default:
      panic("bad output mode");
    }
}



/* set up one line of the user menu */
/* used only once, during initialization to fix up the user menu */
void Senter(char *s, int *b)
{
  int l;
  (void)snprintf(Xmes[Ncom],strbuflen,"%s",s);
  Xbool[Ncom] = b;
  l = XTextWidth(com_font,s,strlen(s));
  l = (int)(1.5*l);  /* make window more generous */
  if(l>comx)
    comx = l;
  com_list[s[1]] = Ncom;
  Ncom += 1;
  if(Ncom>Ncoms)
    panic_nosave("Too many commands");
}



/* find closest point on PATH to x,y */
int node(double x, double y, struct ktag *KL, struct point *PATH)
{
  double d,t;
  int n,a,p,i;
  if((x<0)||(y<0)||(KL[0].n<=0))
    return(-1);
  d = -1;
  for(i=1;i<=KL[0].n;++i)
    {
      p = KL[i].n;
      a = p;
      do {
	if(((t=dist2(x,y,PATH[p].x,PATH[p].y))<d)||(d==-1))
	  {
	    n = p;
	    d = t;
	  }
	p = PATH[p].toPoint;
      } while(p!=a);
    }
  return(n);
}



/* knot traversal aids */


/* mark current location as starting point */
void Rfirst(struct kpt *p)
{
  p->start = 1;
  p->swstruct = p->wstruct;
  p->slevel = p->level;
  p->snode = p->node;
}


/* mark passed in point a as starting point */
void Pfirst(struct kpt *p, int a)
{
  p->wstruct = onPath;
  p->level = levLow;
  p->node = a;
  Rfirst(p);
}


/* mark the passed in point a and level i as the starting point */
void Cfirst(struct kpt *p,int a,int i)
{
  p->wstruct = onCross;
  p->level = i;
  p->node = a;
  Rfirst(p);
}


/* move to next point or crossing */
void Knext(struct point *PATH, struct cross *INTER, struct kpt *p)
{
  int l;
  p->start = 0;
  if(p->node==0)
    panic("bad call to Knext");
  if(p->wstruct==onPath)
    {
      if(PATH[p->node].clist==-1)
	p->node = PATH[p->node].toPoint;
      else
	{
	  p->wstruct = onCross;
	  p->level = PATH[p->node].toLev;
	  p->node = PATH[p->node].clist;
	}
    }
  else
    {
      if(INTER[p->node].toCross[p->level]==-1)
	{
	  p->wstruct = onPath;
	  p->node = INTER[p->node].toPoint[p->level];
	  p->level = levLow;
	}
      else
	{
	  l = p->level;
	  p->level = INTER[p->node].toLev[l];
	  p->node = INTER[p->node].toCross[l];
	}
    }
}


/* move to previous point or crossing.  Here is where we pay for the mistake */
/* I made in not having points have backward pointers to crossings */
void Kprev(struct point *PATH, struct cross *INTER, struct kpt *p)
{
  int l,n,t;
  p->start = 0;
  if(p->wstruct==onPath)
    {
      n = PATH[p->node].fromPoint;
      if(PATH[n].clist==-1)
	p->node = n;
      else
	{
	  t = PATH[n].toLev;
	  n = PATH[n].clist;   /* find previous crossing */
	  l = t;
	  while(INTER[n].toCross[l]!=-1)
	    {
	      t = INTER[n].toLev[l];
	      n = INTER[n].toCross[l];
	      l = t;
	    }
	  p->wstruct = onCross;
	  p->level = l;
	  p->node = n;
	}
    }
  else
    {
      if(INTER[p->node].fromCross[p->level]==-1)
	{
	  p->wstruct = onPath;
	  p->node = INTER[p->node].fromPoint[p->level];
	  p->level = levLow;
	}
      else
	{
	  l = p->level;
	  p->level = INTER[p->node].fromLev[l];
	  p->node = INTER[p->node].fromCross[l];
	}
    }
}


/* check that we have not returned to the starting point */
int Kvalid(struct kpt *p)
{
  if(p->start)
    return(1);
  /* else */
  if((p->swstruct!=p->wstruct)||(p->node!=p->snode))
    return(1);
  /* else */
  if((p->wstruct==onCross)&&(p->slevel!=p->level))
    return(1);
  /* else */
  return(0);
}


/* return the x coordinate of curint point or crossing */
double Kx(struct point *PATH, struct cross *INTER, struct kpt *p)
{
  if(p->wstruct==onPath)
    return(PATH[p->node].x);
  /* else */
  return(INTER[p->node].x);
}


/* return the y coordinate of curint point or crossing */
double Ky(struct point *PATH, struct cross *INTER, struct kpt *p)
{
  if(p->wstruct==onPath)
    return(PATH[p->node].y);
  /* else */
  return(INTER[p->node].y);
}


/* tag a knot list */
/* set all the tags to t */
void Ltag(struct point *PATH, struct cross *INTER, int t)
{
  int i;
  for(i=1;i<maxcross;++i)
    INTER[i].tag = t;
  for(i=1;i<maxpoint;++i)
    PATH[i].tag = t;
}


/* copy the PATH INTER and KNOTL into the bpath binter and bknot */
void Kbackup(struct point *PATH, struct cross *INTER, struct ktag *KNOT,
	     struct point *bpath, struct cross *binter, struct ktag *bknot)
{
  int i;
  struct kpt p;
  bpath[0] = PATH[0];
  binter[0] = INTER[0];
  bknot[0] = KNOT[0];
  for(i=1;i<=KNOT[0].n;++i)
    {
      Pfirst(&p,KNOT[i].n);
      while(Kvalid(&p))
	{
	  if(p.wstruct==onPath)
	    bpath[p.node] = PATH[p.node];
	  else
	    binter[p.node] = INTER[p.node];
	  Knext(PATH,INTER,&p);
	}
      bknot[i] = KNOT[i];
    }
}


/* find closest crossing  to x,y */
int Kcross(int x,int y,
	   struct ktag *k, struct point *PATH, struct cross *INTER)
{
  int i,r;
  double t,d;
  struct kpt p;
  d = -1;
  r = -1;
  if((x<0)||(y<0))
    return(-1);
  for(i=1;i<=k[0].n;++i)
    {
      Pfirst(&p,k[i].n);
      while(Kvalid(&p))
	{
	  if(p.wstruct==onCross)
	    {
	      t = dist2((double)x,(double)y,Kx(PATH,INTER,&p),Ky(PATH,INTER,&p));
	      if((r==-1)||(t<d))
		{
		  d = t;
		  r = p.node; 
		}
	    }
	  Knext(PATH,INTER,&p);
	}
    }
  return(r);
}



/* flip a crossing over */
void flipCross(struct point *PATH, struct cross *INTER, int c)
{
  int i;
  struct cross t;
  t = INTER[c];
  for(i=0;i<2;++i)
    {
      INTER[c].toPoint[i] = t.toPoint[1-i];
      INTER[c].fromPoint[i] = t.fromPoint[1-i];
      INTER[c].toCross[i] = t.toCross[1-i];
      INTER[c].fromCross[i] = t.fromCross[1-i];
      INTER[c].toLev[i] = t.toLev[1-i];
      INTER[c].fromLev[i] = t.fromLev[1-i];
      if(t.fromCross[i]!=-1)
	INTER[t.fromCross[i]].toLev[t.fromLev[i]] 
	  = 1 - INTER[t.fromCross[i]].toLev[t.fromLev[i]];
      else
	PATH[t.fromPoint[i]].toLev = 1 - PATH[t.fromPoint[i]].toLev;
      if(t.toCross[i]!=-1)
	INTER[t.toCross[i]].fromLev[t.toLev[i]] 
	  = 1 - INTER[t.toCross[i]].fromLev[t.toLev[i]];
    }
}



double MIN(double x, double y)
{
  if(x>y)
    return(y);
  /* else */
  return(x);
}


double MAX(double x,double y)
{
  if(x>y)
    return(x);
  /* else */
  return(y);
}


/* get distance */
double dist2(double x0, double y0, double x1, double y1)
{
  return(sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)));
}



/* project  point (x,y) onto a line */
/* return 1 if it is on the specified segment */
int project(double x, double y, double x0, double y0, double x1, double y1,
	    double *xr, double *yr)
{
  double t;
  if(dist2(x0,y0,x1,y1)==0)
    {
      *xr = x0;
      *yr = y0;
      return(1);
    }
  /* else */
  t = (y1*y1-y0*y1-y*y1+y*y0+x1*x1-x0*x1-x*x1+x*x0)
    /((y1-y0)*(y1-y0)+(x1-x0)*(x1-x0));
  *xr = t*x0 + (1-t)*x1; 
  *yr = t*y0 + (1-t)*y1;
  if((t>=0)&&(t<=1))
    return(1);
  /* else */
  return(0);
}




/* get minimum distance between a vertex and a line  */
double Ldist(double x, double y, double x0, double y0, double x1, double y1)
{
  double xt,yt;
  if((x0==x1)&&(y0==y1))
    return(0.0);
  project(x,y,x0,y0,x1,y1,&xt,&yt);
  return(dist2(x,y,xt,yt));
}



/* get minimum distance between a vertex and a line segment */
double pdist(double x, double y, double x0, double y0, double x1, double y1)
{
  double xt,yt,d;
  if(project(x,y,x0,y0,x1,y1,&xt,&yt))
    d =dist2(x,y,xt,yt);
  else
    d = MIN(dist2(x,y,x0,y0),dist2(x,y,x1,y1));
  return(d);
}



#define Npref 12
/* insert a node on the nearest string */
int Inode(double x, double y,
	  struct ktag *KL, struct point *PATH, struct cross *INTER)
{
  int cnode,seg,p,n,i;
  double tmp,t2,xt,yt,xp,yp,nd;
  cnode = node(x,y,KL,PATH);  /* see if a node is close */
  if(cnode==-1)
    return(-1);   /* pass up bad state */
  nd = dist2(x,y,PATH[cnode].x,PATH[cnode].y);
  tmp = nd;
  seg = cnode;
  xp = PATH[cnode].x;
  yp = PATH[cnode].y;
  for(i=1;i<=KL[0].n;++i)  /* find closest string segment */
    {
      p = KL[i].n;
      do {
	n = PATH[p].toPoint;
	if(project(x,y,PATH[p].x,PATH[p].y,PATH[n].x,PATH[n].y,&xt,&yt))
	  {
	    t2 = dist2(x,y,xt,yt);
	    if(t2<tmp)
	      {
		tmp = t2;
		seg = p;
		xp = xt;
		yp = yt;
	      }
	  }
	p = n;
      }while(p!=KL[i].n);
    }
  if(nd-Npref>tmp)
    cnode = insControl(PATH,INTER,seg,xp,yp);
  return(cnode);
}



/* add an extra control point after control point seg at coordinats (xp,yp) */
/* and fix up all of the stinking pointers */
int insControl(struct point *PATH, struct cross *INTER,
	       int seg, double xp, double yp)
{
  int nc,k,l,t,lvalid,lastk,lastl;
  nc = newpt(PATH);          /* insert a control point into crossing list*/
  PATH[nc].knot = PATH[seg].knot;
  PATH[nc].x = xp;
  PATH[nc].y = yp;
  PATH[nc].toPoint = PATH[seg].toPoint;
  PATH[nc].fromPoint = seg;
  PATH[PATH[seg].toPoint].fromPoint = nc;
  PATH[seg].toPoint = nc;
  if(PATH[seg].clist!=-1) 
    {
      k = PATH[seg].clist;
      l = PATH[seg].toLev;
      lastk = k;
      lastl = l;
      lvalid = 0;
      while((k!=-1)
	    &&(inbetween(INTER[k].x,INTER[k].y,PATH[seg].x,PATH[seg].y,xp,yp)))
	{
	  lvalid = 1;
	  INTER[k].toPoint[l] = nc;
	  lastk = k;
	  lastl = l;
	  t = INTER[k].toLev[l];
	  k = INTER[k].toCross[l];
	  l = t;
	}
      if(!lvalid)
	PATH[seg].clist = -1;
      PATH[nc].clist = k;
      PATH[nc].toLev = l;
      if(lastk!=-1)
	{
	  INTER[lastk].toCross[lastl] = -1;
	}
      if(k!=-1)
	{
	  INTER[k].fromCross[l] = -1;
	  while(k!=-1)
	    {
	      INTER[k].fromPoint[l] = nc;
	      t = INTER[k].toLev[l];
	      k = INTER[k].toCross[l];
	      l = t;
	    }
	}
    }
  return(nc);
}



/* check if 1 d line segments could intersect */
/* return 1 if they could 0 if not */
/* just a simple box check to do before the other routines which I don't */
/* always like to trust */
int intr1(double x0, double x1, double x2, double x3)
{
  if( (MIN(x0,x1)>MAX(x2,x3)) || (MIN(x2,x3)>MAX(x0,x1)) )
    return(0);
  /* else */
  return(1);
}



/* check two line segments for intersection */
/* make sure to check out vertext situation first */
/* return 0 if they cross, 1 other wise */
/* return where the cross in x,y */
int ldisj(double x0, double y0, double x1, double y1,
	  double x2, double y2, double x3, double y3,
	  double *x, double *y)
{
  double t1,t2;
  if((!intr1(x0,x1,x2,x3))||(!intr1(x0,x1,x2,x3)))
    return(1);
  t2 = (x1-x0)*y3+(x0-x1)*y2+(x2-x3)*y1+(x3-x2)*y0;
  if(t2==0)
    return(1);  /* parallel lines */
  t1 = -((x2-x1)*y3+(x1-x3)*y2+(x3-x2)*y1)/t2;
  t2 = ((x1-x0)*y3+(x0-x3)*y1+(x3-x1)*y0)/t2;
  if((t1<0)||(t1>1)||(t2<0)||(t2>1))
    return(1);
  /*else*/
  *x = t1*x0+(1-t1)*x1;
  *y = t1*y0+(1-t1)*y1;
  return(0);
}



/* return a new crossing data structure */
int ncross(struct cross *INTER)
{
  int i,first,start;
  first = 1;
  start = next_cross(INTER);
  while(INTER[next_cross(INTER)].touch==cross_key(INTER))
    {
      next_cross(INTER) = (next_cross(INTER)+1)%maxcross;
      if((!first)&&(next_cross(INTER)==start))
	panic("out of cross memory");
      first = 0;
    }
  INTER[next_cross(INTER)].touch = cross_key(INTER);
  INTER[next_cross(INTER)].tag = 0;
  INTER[next_cross(INTER)].x = 0;
  INTER[next_cross(INTER)].y = 0;
  for(i=0;i<2;++i)
    {
      INTER[next_cross(INTER)].toPoint[i] = -1;
      INTER[next_cross(INTER)].fromPoint[i] = -1;
      INTER[next_cross(INTER)].toCross[i] = -1;
      INTER[next_cross(INTER)].fromCross[i] = -1;
    }
  return(next_cross(INTER));
}



/* check that there are at least minPoints on a  link */
/* lots of my routines are counting on at least 4 control points */
int Npath(struct point *PATH, int A)
{
  int n,P;
  n = 0;
  P = A;
  do {
    A = PATH[A].toPoint;
    ++n;
  } while((P!=A)&&(n<minPoints)&&(A!=-1));
  if(n<minPoints)
    return(1);
  /* else */
  return(0);
}


/* return a new point data structure */
int newpt(struct point *PATH)
{
  int first,start;
  first = 1;
  start = next_point(PATH);
  while(PATH[next_point(PATH)].touch==point_key(PATH))
    {
      next_point(PATH) = (next_point(PATH)+1)%maxpoint;
      if((!first)&&(next_point(PATH)==start))
	panic("out of point memory");
      first = 0;
    }
  PATH[next_point(PATH)].touch = point_key(PATH);
  PATH[next_point(PATH)].x = 0;
  PATH[next_point(PATH)].y = 0;
  PATH[next_point(PATH)].toPoint = -1;
  PATH[next_point(PATH)].fromPoint = -1;
  PATH[next_point(PATH)].clist = -1;
  PATH[next_point(PATH)].toLev = -1;
  PATH[next_point(PATH)].tag = 0;
  PATH[next_point(PATH)].knot = 0;
  return(next_point(PATH));
}



/* check of x,y is inbetween the other two points (they are all assumed) */
/* to be known to be colinear */
int inbetween(double x, double y, 
	      double x0, double y0,
	      double x1, double y1)
{
  if( (((x0<=x)&&(x<=x1))||((x1<=x)&&(x<=x0))) 
      && (((y0<=y)&&(y<=y1))||((y1<=y)&&(y<=y0))) )
    return(1);
  else
    return(0);
}



/* insert crossing c inbetween i and next i at level pev */
/* in the correct position */
/* only does half of the work (must be called twice) */
void insert_cross(struct point *PATH, struct cross *INTER,
		  int i,int c,int pev)
{
  int k,l,kp,lp;
  INTER[c].toPoint[pev] = PATH[i].toPoint;
  INTER[c].fromPoint[pev] = i;
  kp = -1;
  lp = -1;
  k = PATH[i].clist;
  l = PATH[i].toLev;
  while((k!=-1)&&(!inbetween(INTER[c].x,INTER[c].y,INTER[k].x,INTER[k].y
			     ,PATH[i].x,PATH[i].y)))
    {
      kp = k;
      lp = l;
      k = INTER[kp].toCross[lp];
      l = INTER[kp].toLev[lp];
    }
  INTER[c].toCross[pev] = k;
  INTER[c].toLev[pev] = l;
  INTER[c].fromCross[pev] = kp;
  INTER[c].fromLev[pev] = lp;
  if(kp==-1)
    {
      PATH[i].clist = c;
      PATH[i].toLev = pev;
    }
  else
    {
      INTER[kp].toLev[lp] = pev;
      INTER[kp].toCross[lp] = c;
    }
  if(k!=-1)
    {
      INTER[k].fromLev[l] = pev;
      INTER[k].fromCross[l] = c;
    }
}



/* take a crossing out and fix all the pointers */
void ditchCross(struct point *PATH, struct cross *INTER, int n)
{
  int i;
  for(i=0;i<2;++i)
    {
      if(INTER[n].fromCross[i]!=-1)
	{
	  INTER[INTER[n].fromCross[i]].toCross[INTER[n].fromLev[i]]
	    = INTER[n].toCross[i];
	  INTER[INTER[n].fromCross[i]].toLev[INTER[n].fromLev[i]]
	    = INTER[n].toLev[i];
	}
      else   
	{
	  PATH[INTER[n].fromPoint[i]].clist = INTER[n].toCross[i];
	  PATH[INTER[n].fromPoint[i]].toLev = INTER[n].toLev[i];
	}
      if(INTER[n].toCross[i]!=-1)
	{
	  INTER[INTER[n].toCross[i]].fromCross[INTER[n].toLev[i]]
	    = INTER[n].fromCross[i];
	  INTER[INTER[n].toCross[i]].fromLev[INTER[n].toLev[i]]
	    = INTER[n].fromLev[i];
	}
    }
  INTER[n].touch = cross_key(INTER) - 1;
}


/* take a crossing out and update pointers and the over/under sensing state */
/* machine, this state machine allows over/under to be violated if the */
/* violation is the trivail (Reidemeister 1) type */
void softDitchCross(struct point *PATH, struct cross *INTER,
		    int n, struct spst *soft_spot)
{
  struct kpt p;
  Pfirst(&p,soft_spot->P1);
  while(Kvalid(&p)&&((p.wstruct!=onCross)||(p.node!=n)))
    Knext(PATH,INTER,&p);
  if((p.wstruct!=onCross)||(p.node!=n))
    panic("bad soft ditch");
  /* else */
  switch(soft_spot->state)    /* doesn't catch nested loops (just free ones) */
    {
    case initialSoft:
      soft_spot->level = p.level;
      soft_spot->state = tenativeSoft;
      soft_spot->lcross = p.node;
      break;
    case tenativeSoft:
      if(soft_spot->lcross==p.node)
	{
	  soft_spot->state = initialSoft;
	  soft_spot->level = levHigh;
	}
      else
	{
	  if((soft_spot->level!=p.level)&&(soft_spot->lcross!=p.node))
            {
	      soft_spot->state = invalidSoft;
	      soft_spot->level = levHigh;
            }
	  else
            {
	      if(soft_spot->lcross==p.node)
		{
		  soft_spot->state = initialSoft;
		  soft_spot->level = levHigh;
		}
	      else
		soft_spot->lcross = p.node;
            }
	}
      break;
    case lockedSoft:
      if((soft_spot->level!=p.level)&&(soft_spot->lcross!=p.node))
	{
	  soft_spot->state = invalidSoft;
	  soft_spot->level = levHigh;
	}
      else
	soft_spot->lcross = p.node;
      break;
    case invalidSoft:
      soft_spot->level = levHigh;
      break;
    default:
      panic("bad soft state");
      break;
    }
  ditchCross(PATH,INTER,n);
}



/* chuck all of the crossings and forget we ever had them */
void ditchCrossings(struct point *PATH, struct cross *INTER, struct ktag *a)
{
  int i,j;
  dispose_INTER(INTER);   /* get rid of record of crossings */
  for(j=1;j<=a[0].n;++j)
    {
      i = a[j].n;
      do {
	PATH[i].clist = -1;
	PATH[i].toLev = -1; 
	i = PATH[i].toPoint;
      } while(i!=a[j].n);
    }
}


/* check of there exists a crossing involving the line segments drawn from */
/* p1 and p3 on PATH */
int Gcross(int p1, int p3, struct point *PATH, struct cross *INTER)
{
  struct kpt p;
  int c,l,t,p4;
  p4 = PATH[p3].toPoint;
  Pfirst(&p,p1);
  if(PATH[p1].clist==-1)
    return(-1);
  c = PATH[p1].clist;
  l = PATH[p1].toLev;
  while((c!=-1)&&((INTER[c].toPoint[1-l]!=p4)||(INTER[c].fromPoint[1-l]!=p3)))
    {
      t = INTER[c].toLev[l];
      c = INTER[c].toCross[l];
      l = t;
    }
  return(c);
}


/* lock a loop of crossings  so find_cross will not flip them */
void lock_loop(struct point *PATH, struct cross *INTER, int i, int knot)
{
  struct kpt p1;
  int l;
  if(PATH[INTER[i].toPoint[levLow]].knot==knot)
    l = levLow;
  else
    if(PATH[INTER[i].toPoint[levHigh]].knot==knot)
      l = levHigh;
    else
      panic("bad call to lock loop");
  Cfirst(&p1,i,l);
  while(Kvalid(&p1))
    {
      if(p1.wstruct==onCross)
	{
	  if(INTER[p1.node].tag!=0)
	    INTER[p1.node].tag = -1; 
	}
      Knext(PATH,INTER,&p1);
    }
}


/* try to flip all of the crossings to match given the assumption that */
/* crossing j on PATH is the same crossing as guess on OLDPATH */
void try_fix(struct point *PATH, struct cross *INTER,
	     struct point *OLDPATH, struct cross *OLDINTER,
	     int j,int guess)
{
  struct kpt p1,p2;
  int i,n;
  for(i=0;i<maxcross;++i)
    {
      Atag[i] = 0;
      Btag[i] = 0;
    }
  i = guess;
  Cfirst(&p1,j,levLow);
  p2.wstruct = onCross;
  n = 1;
  while(Kvalid(&p1)&&(p2.wstruct==onCross))
    {
      if(p1.wstruct==onCross)
	{
	  if(INTER[i].tag>0)   /* only flip certain points */
	    {
	      if(Csign(OLDPATH,OLDINTER,p1.node)!=Csign(PATH,INTER,i))
		flipCross(PATH,INTER,i);
	    }
	  Btag[p1.node] = n;
	  Atag[i] = n;
	  ++n;
	  Cfirst(&p2,i,p1.level);
	  do {
	    Knext(PATH,INTER,&p2);
	  }while(Kvalid(&p2)&&(p2.wstruct!=onCross));
	  i = p2.node;
	}
      Knext(OLDPATH,OLDINTER,&p1);
    }
}


/* try to match knot strings, if no match return minus 1, else return */
/* sum of distances from nodes and their images */
/* try fix must be called first */
double rate_match(struct point *PATH, struct cross *INTER,
		  struct point *OLDPATH, struct cross *OLDINTER,
		  int j,int guess)
{
  struct kpt p1,p2;
  double d;
  d = 0;
  Cfirst(&p1,j,levLow);
  Cfirst(&p2,guess,levLow);
  while(Kvalid(&p1)&&Kvalid(&p2))
    {
      while(Kvalid(&p1)&&(p1.wstruct!=onCross))
	Knext(OLDPATH,OLDINTER,&p1);
      while(Kvalid(&p1)&&Kvalid(&p2)&&(p2.wstruct!=onCross))
	Knext(PATH,INTER,&p2);
      if(Kvalid(&p1)&&Kvalid(&p2))
	{
	  if((Csign(OLDPATH,OLDINTER,p1.node)!=Csign(PATH,INTER,p2.node))
	     ||(Btag[p1.node]!=Atag[p2.node])||(Btag[p1.node]==0)
	     ||(Atag[p2.node]==0))
	    return(-1.0);
	  /* else */
	  d += dist2(Kx(OLDPATH,OLDINTER,&p1),Ky(OLDPATH,OLDINTER,&p1)
		     ,Kx(PATH,INTER,&p2),Ky(PATH,INTER,&p2));
	  Knext(OLDPATH,OLDINTER,&p1);
	  Knext(PATH,INTER,&p2);
	}
    }
  if(Kvalid(&p1))
    Knext(OLDPATH,OLDINTER,&p1);
  while(Kvalid(&p1))
    {
      if(p1.wstruct==onCross)
	return(-1.0);
      Knext(OLDPATH,OLDINTER,&p1);
    }
  if(Kvalid(&p2))
    Knext(PATH,INTER,&p2);
  while(Kvalid(&p2))
    {
      if(p2.wstruct==onCross)
	return(-1.0);
      Knext(PATH,INTER,&p2);
    }
  return(d);
}


/* return 0 if screwed up */
/* find crosses on graph */
/* assumes a loop of at least 4 vertices */
/* also uses the fact that if a point or crossing is undesturbed between */
/* old and new knots that it WILL have the same record number, this is a */
/* perversion I instituted for this purpose */
/* restores the old knot if the match fails and isotopy lock is on */
/* note: if one crossing fails to match the others will probably be */
/* messed up */
/* a BIG (time compexity reduction) performance improvement could */
/* be had by keeping track of old line segments as we track old crossings */
/* and only looking for intersections with new line segments */
int find_cross(struct point *PATH, struct cross *INTER, struct ktag *a,
	       struct point *OLDPATH, struct cross *OLDINTER, struct ktag *OLDKNOTL,
	       struct spst *soft_point)
{
  double x0,y0,x1,y1,x2,y2,x3,y3,x,y,rt,ra;
  int i,j,c,knot,knot2,start,s2,isOkay;
  struct kpt p1;
  /* build a new crossing set */
  /*XDefineCursor(theWindow,wait_curs);*/
  /*XDefineCursor(comWindow,wait_curs);*/
  XFlush(X11_display);
  Ltag(PATH,INTER,0);
  isOkay = 1;
  for(knot=1;knot<=a[0].n;++knot)
    {
      i = a[knot].n;
      s2 = 1;
      while((i!=a[knot].n)||(s2==1))
	{
	  s2 = 0;
	  x0 = PATH[i].x;
	  y0 = PATH[i].y;
	  x1 = PATH[PATH[i].toPoint].x;
	  y1 = PATH[PATH[i].toPoint].y;
	  for(knot2=knot;knot2<=a[0].n;++knot2) 
	    {
	      if(knot2==knot)
		{
		  if((PATH[i].toPoint==a[knot].n)
		     ||(PATH[PATH[i].toPoint].toPoint==a[knot].n))
		    j = -1;  /* force abort */
		  else
		    j = PATH[PATH[i].toPoint].toPoint;
		}
	      else
		j = a[knot2].n;
	      start = 1;
	      while( (j!=-1) && (PATH[j].toPoint!=i) && ((j!=a[knot2].n)||(start)) )
		{
		  start = 0;
		  x2 = PATH[j].x;
		  y2 = PATH[j].y;
		  x3 = PATH[PATH[j].toPoint].x;
		  y3 = PATH[PATH[j].toPoint].y;
		  if(pdist(x0,y0,x2,y2,x3,y3)==0)
		    {
		      x_label("points too close");
		      /*XFeep(0);*/
		      Kbackup(OLDPATH,OLDINTER,OLDKNOTL,PATH,INTER,KNOTL);
		      return(0);
		    }
		  if(!ldisj(x0,y0,x1,y1,x2,y2,x3,y3,&x,&y))
		    {
		      if((c=Gcross(i,j,PATH,INTER))==-1)
			{
			  c = ncross(INTER);
			  INTER[c].x = x;
			  INTER[c].y = y;
			  insert_cross(PATH,INTER,i,c,levLow);
			  insert_cross(PATH,INTER,j,c,levHigh);
			  INTER[c].tag = 1;
			}
		      else
			{
			  INTER[c].x = x;
			  INTER[c].y = y;
			} 
		    }
		  j = PATH[j].toPoint;
		}
	    }
	  i = PATH[i].toPoint;
	}
    }
  /* fix up any and all flubs */
  for(knot=1;knot<=a[0].n;++knot)
    {
      Pfirst(&p1,a[knot].n);
      while(Kvalid(&p1))
	{
	  if(p1.wstruct==onPath)
	    PATH[p1.node].knot = knot;  /* sometimes wrong FOCRE it */
	  else
	    {
	      if(
		 (PATH[INTER[p1.node].fromPoint[levLow]].toPoint
		  !=INTER[p1.node].toPoint[levLow])
		 ||(PATH[INTER[p1.node].fromPoint[levHigh]].toPoint
		    !=INTER[p1.node].toPoint[levHigh])
		 ||(Gcross(INTER[p1.node].fromPoint[levLow]
			   ,INTER[p1.node].fromPoint[levHigh],PATH,INTER)!=p1.node)
		 ||(ldisj(PATH[INTER[p1.node].fromPoint[levLow]].x
			  ,PATH[INTER[p1.node].fromPoint[levLow]].y
			  ,PATH[INTER[p1.node].toPoint[levLow]].x
			  ,PATH[INTER[p1.node].toPoint[levLow]].y
			  ,PATH[INTER[p1.node].fromPoint[levHigh]].x
			  ,PATH[INTER[p1.node].fromPoint[levHigh]].y
			  ,PATH[INTER[p1.node].toPoint[levHigh]].x
			  ,PATH[INTER[p1.node].toPoint[levHigh]].y,&x,&y))
		 )
		{
		  ditchCross(PATH,INTER,p1.node); /*still knext from a ditched */
		} 
	      else
		{
		  INTER[p1.node].x = x;
		  INTER[p1.node].y = y;
		}
	    }
	  Knext(PATH,INTER,&p1);
	}
    }
  if(soft_point->state==invalidSoft)
    {
      for(knot=1;(knot<=a[0].n)&&(isOkay);++knot)
	{
	  Pfirst(&p1,a[knot].n);
	  /* does this component have any crossings that need fixing, and can */
	  /* we find a stablized crossing to do the job? */ 
	  i = -1;
	  j = -1;
	  while(Kvalid(&p1)&&(i==-1))
	    {
	      if(p1.wstruct==onCross)
		{
		  if(INTER[p1.node].tag==0)
		    i = p1.node;    /* old crossing*/
		  j = p1.node;      /* have some crossing */
		}
	      Knext(PATH,INTER,&p1);
	    }
	  if(j!=-1)
	    {
	      if(i!=-1)   /* use hint how to fix this */
		{
		  /* left over node has identical number for both knots */
		  try_fix(PATH,INTER,OLDPATH,OLDINTER,i,i);
		  ra = rate_match(PATH,INTER,OLDPATH,OLDINTER,i,i);
		  if(ra<0)
		    isOkay = 0;
		  else
		    lock_loop(PATH,INTER,i,knot);  /* stabalize crossing signs */
		}
	      else
		{      /* try to fix it, no gaurantees */
		  i = -1;
		  ra = -1;
		  Pfirst(&p1,OLDKNOTL[knot].n);
		  while(Kvalid(&p1))
		    {
		      if(p1.wstruct==onCross)
			{
			  try_fix(PATH,INTER,OLDPATH,OLDINTER,p1.node,j);
			  rt = rate_match(PATH,INTER,OLDPATH,OLDINTER,p1.node,j);
			  if((rt!=-1)&&((rt<ra)||(ra==-1)))
			    {
			      ra = rt;
			      i = p1.node;
			    }
			}
		      Knext(OLDPATH,OLDINTER,&p1);
		    }
		  if(ra>=0)
		    {
		      try_fix(PATH,INTER,OLDPATH,OLDINTER,i,j);
		      lock_loop(PATH,INTER,j,knot);  /* stabalize crossing signs */
		    }
		  else 
		    isOkay = 0;
		}
	    }
	  else
	    {
	      Pfirst(&p1,OLDKNOTL[knot].n);
	      while(Kvalid(&p1)&&(isOkay))
		{
		  if(p1.wstruct==onCross)
		    isOkay = 0;
		  Knext(OLDPATH,OLDINTER,&p1);
		}
	    }
	}
    }
  else
    {    /* use the soft hints to recompute crossings  */
      /* note: P1 is a stable point!!! */
      Kbackup(PATH,INTER,KNOTL,TPATH,TCROS,TKNTL);
      Pfirst(&p1,soft_point->P1); 
      isOkay = 1;
      while(Kvalid(&p1))
	{
	  if(p1.wstruct==onCross)  /* not worring about loops yet */ 
	    {
	      if((INTER[p1.node].tag!=0)&&(p1.level!=soft_point->level))
		flipCross(PATH,INTER,p1.node);
	      INTER[p1.node].tag = 0;
	    }
	  Knext(TPATH,TCROS,&p1);
	}
    }
  if((ISOTOPY_LOCK)&&(!isOkay))
    {
      x_label("Isotopy violated, move rejected");
      /*XFeep(0);*/
      Kbackup(OLDPATH,OLDINTER,OLDKNOTL,PATH,INTER,KNOTL);
      /* assume no cross detection mode change */
    }
  /*XDefineCursor(theWindow,curs);*/
  /*XDefineCursor(comWindow,com_curs);*/
  XSync(X11_display,11);
  return((!ISOTOPY_LOCK)||isOkay);
}





/* change the x,y coordinates to draw the bridge of a crossing */
void assignXY(struct point *PATH, struct cross *INTER,
	      struct kpt *p1, struct kpt *p2,
	      double *x, double *y)
{
  double dist;
  double adjx,adjy;
  *x = Kx(PATH,INTER,p1);
  *y = Ky(PATH,INTER,p1);
  if((p1->level==levLow)&&(p1->wstruct==onCross))
    {
      dist = dist2(*x,*y,Kx(PATH,INTER,p2),Ky(PATH,INTER,p2));
      if(dist>0)
	{
	  adjx = MIN((double)GLOBCrad,(dist/3))*(*x-Kx(PATH,INTER,p2))/dist;
	  adjy = MIN((double)GLOBCrad,(dist/3))*(*y-Ky(PATH,INTER,p2))/dist;
	}
      else
	{
	  adjx = 0;
	  adjy = 0;
	}
      *x -= adjx;
      *y -= adjy;
    }
}



/* do a rotation given sin(theta) = s and cos(theta) = c */
void rotate(double *x, double *y, double s, double c)
{
  double t;
  t = (*x)*c - (*y)*s;
  *y = (*y)*c + (*x)*s;
  *x = t;
}


/* get the slope for the spliner to use for its derviative */
double dslope(double x0, double y0,
	      double xz, double yz,
	      double x1, double y1)
{  /* return 0's are all bad slopes! */
  double t,m,a,b;
  /* angle bisector method */
  t = dist2(x0,y0,xz,yz);
  if(t==0.0)
    return(0);
  a = (x0-xz)/t;
  b = (y0-yz)/t;
  t = dist2(x1,y1,xz,yz);
  if(t==0.0)
    return(0);
  a = a - (x1-xz)/t;
  b = b - (y1-yz)/t;
  if(a==0.0)
    return(0);
  m = b/a;
  return(m);
}




/* build a little knot centered at (cx,cy) of 4 points */
#define ninitv 4
void PKnot(struct point *PATH, struct ktag *KNOT,
	   double cx, double cy)
{
  int k,pt;
  KNOT[0].n += 1;
  KNOT[KNOT[0].n] = K_0;
  KNOT[KNOT[0].n].n = newpt(PATH);   /* build a trivial knot */
  pt = KNOT[KNOT[0].n].n;
  for(k=1;k<=ninitv;++k)
    {
      if(k!=ninitv)
	PATH[pt].toPoint = newpt(PATH);
      else
	PATH[pt].toPoint = KNOT[KNOT[0].n].n;
      PATH[PATH[pt].toPoint].fromPoint = pt;
      PATH[pt].knot = KNOT[0].n;
      PATH[pt].x = (20*cos(2*k*M_PI/ninitv)) + cx; 
      PATH[pt].y = (20*sin(2*k*M_PI/ninitv)) + cy; 
      pt = PATH[pt].toPoint;
    }
}


/* clear out the knot data structures */
void clearKnot(struct point *PATH, struct cross *INTER, struct ktag *KNOT)
{
  dispose_PATH(PATH);
  dispose_INTER(INTER);
  KNOT[0] = K_0;
  KNOT[0].n = 0;
}


/* return +- 1 sign of crossing */
int Csign(struct point *PATH, struct cross *INTER, int n)
{
  double dx1,dy1,dx2,dy2,r;
  dx1 = PATH[INTER[n].toPoint[levLow]].x-PATH[INTER[n].fromPoint[levLow]].x;
  dy1 = -(PATH[INTER[n].toPoint[levLow]].y-PATH[INTER[n].fromPoint[levLow]].y);
  dx2 = PATH[INTER[n].toPoint[levHigh]].x-PATH[INTER[n].fromPoint[levHigh]].x;
  dy2 = -(PATH[INTER[n].toPoint[levHigh]].y-PATH[INTER[n].fromPoint[levHigh]].y);
  r = dist2(0.0,0.0,dx1,dy1);
  dx1 = dx1/r;
  dy1 = dy1/r;
  r = dist2(0.0,0.0,dx2,dy2);
  dx2 = dx2/r;
  dy2 = dy2/r;
  /* rotate the bottom line to the horizontal */
  dy2 = dy2*dx1-dx2*dy1;
  /* check the damn thing out */
  if(dy2>0)
    return(-1);
  /* else */
  return(1);
}



/* fix a knot's knot number */
void Kmark(struct point *PATH, struct ktag *KNOTL, int k)
{
  int n;
  n = KNOTL[k].n;
  do {
    PATH[n].knot = k;
    n = PATH[n].toPoint;
  }while(n!=KNOTL[k].n);
}


/* do all of the pointer fixing after a channel cut */
void fix_comp(struct point *PATH, struct ktag *KNOTL,
	      int l, int m)
{
  int n,d1,d2;
  /* fix knot labelings */
  d1 = 0;
  d2 = 0;
  n = l;
  do {
    if(PATH[n].knot!=PATH[l].knot)
      {
	if(d1==0)
	  d1 = PATH[n].knot; 
	PATH[n].knot = PATH[l].knot;
      }
    n = PATH[n].toPoint;
  }while(n!=l);
  n = m;
  do {
    if(PATH[n].knot!=PATH[m].knot)
      {
	if(d2==0)
	  d2 = PATH[n].knot; 
	PATH[n].knot = PATH[m].knot;
      }
    n = PATH[n].toPoint;
  }while(n!=m);
  /* delete components that got merged */
  if((d1!=0)||(d2!=0))
    {
      if((d1!=0)&&(d1!=PATH[m].knot))
	{
	  KNOTL[d1] = KNOTL[KNOTL[0].n];
	  Kmark(PATH,KNOTL,d1);
	  if(d2==d1)
	    d2 = 0;
	  if(d2==KNOTL[0].n)
	    d2 = d1;
	  KNOTL[0].n -= 1; 
	}
      if((d2!=0)&&(d2!=PATH[l].knot))   /* should never reach here anyway */
	{
	  KNOTL[d2] = KNOTL[KNOTL[0].n];
	  Kmark(PATH,KNOTL,d2);
	  KNOTL[0].n -= 1;
	}
    }
  /* check for a disconnection */
  if((PATH[l].knot==PATH[m].knot)&&(l!=m))
    {
      n = l;
      do {
	n = PATH[n].toPoint;
      }while((n!=l)&&(n!=m));
      if(n!=m)
	{
	  KNOTL[PATH[l].knot].n = l;
	  KNOTL[0].n += 1;
	  KNOTL[KNOTL[0].n] = K_0;
	  KNOTL[KNOTL[0].n].n = m;
	  n = m;
	  do {
	    PATH[n].knot = KNOTL[0].n;
	    n = PATH[n].toPoint;
	  }while(n!=m);
	}
    }
  MinPoints(PATH,l);
  MinPoints(PATH,m);
}



/* insert colinear control points if we have too few points on link */
void MinPoints(struct point *PATH, int p)
{
  int i;
  /* insure 4 points */
  while(Npath(PATH,p))
    {
      i = newpt(PATH);
      PATH[i].knot = PATH[p].knot;
      PATH[i].x = (PATH[p].x+PATH[PATH[p].toPoint].x)/2;
      PATH[i].y = (PATH[p].y+PATH[PATH[p].toPoint].y)/2;
      PATH[i].fromPoint = p;
      PATH[i].toPoint = PATH[p].toPoint;
      PATH[PATH[p].toPoint].fromPoint = i;
      PATH[p].toPoint = i;
      p = i;
    }
}






/* find out who we are and get the user id set properly for user file creation*/
void userId()
{
  char h[strbuflen];
  PID = getpid();
  (void)gethostname(h,strbuflen);
  (void)snprintf(log_name,strbuflen,"%s at %s (PID=%d)",getlogin(),h,PID);
}




/* print a record */
int pcross(struct cross c)
{
  int i;
  printf("(x=%f,y=%f) touch=%d tag=%d\n",c.x,c.y,c.touch,c.tag);
  printf("\tpoint\t\tcross\n");
  printf("\tfrom\tto\t\tfrom\tto\n");
  for(i=0;i<2;++i)
    printf("\t%d\t%d\t\t%d\t%d\n",c.fromPoint[i],c.toPoint[i]
	   ,c.fromCross[i],c.toCross[i]);
  return(0);
}

/* print a point */
int ppoint(struct point p)
{
  printf("(x=%f,y=%f) touch=%d tag=%d knot=%d\n",p.x,p.y,p.touch,p.tag,p.knot);
  printf("fromPoint=%d toPoint=%d clist=%d\n",p.toPoint,p.fromPoint,p.clist);
  return(0);
}


