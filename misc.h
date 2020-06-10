
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

#ifndef misc_h
#define misc_h


extern char user_name[];
extern char log_name[];

/* tek plotter settings */
#define hard_big_radius 1400.0
#define hard_little_radius 100
#define conversion hard_big_radius/bigradius
#define cross_key(INTER) (INTER[0].touch)
#define next_cross(INTER) (INTER[0].toPoint[0])
#define dispose_INTER(INTER) (++cross_key(INTER),next_cross(INTER)=1)
#define point_key(PATH) (PATH[0].touch)
#define next_point(PATH) (PATH[0].toPoint)
#define dispose_PATH(PATH) (++point_key(PATH),next_point(PATH)=1)
#define absolute 0
#define delta 1


/* service routines */
extern void panic(char *t);
extern void panic_nosave(char *t);
extern FILE *g_open(char *t, char *s);
extern int s_open(char *t, char *s);
extern void get_xy(int *x, int *y, int n);
extern double min_dist(struct point *PATH, 
		struct cross *INTER,
		struct ktag *KNOTL,
		       int p);
extern double chan_width(struct point *PATH,
		  struct cross *INTER,
		  struct ktag *KNOTL,
			 int k);
extern void pdelta(struct point *PATH, struct cross *INTER,
		   int p, double *x, double *y);
extern int push_off(struct point *PATH,
	     struct cross *INTER,
	     struct ktag *KNOTL,
		    int p);
extern int Wcorrect(double *x, double *y, double x2, double y2);
extern void Vline(double x1, double y1, double x2, double y2);
extern void Vlabel(double x, double y, char *s);
extern void Senter(char *s, int *b);
extern int node(double x, double y, struct ktag *KL, struct point *PATH);
extern void Rfirst(struct kpt *p);
extern void Pfirst(struct kpt *p, int a);
extern void Cfirst(struct kpt *p,int a,int i);
extern void Knext(struct point *PATH, struct cross *INTER, struct kpt *p);
extern void Kprev(struct point *PATH, struct cross *INTER, struct kpt *p);
extern int Kvalid(struct kpt *p);
extern double Kx(struct point *PATH, struct cross *INTER, struct kpt *p);
extern double Ky(struct point *PATH, struct cross *INTER, struct kpt *p);
extern void Ltag(struct point *PATH, struct cross *INTER, int t);
extern void Kbackup(struct point *PATH, struct cross *INTER, struct ktag *KNOT,
		    struct point *bpath, struct cross *binter, struct ktag *bknot);
extern int Kcross(int x,int y,
		  struct ktag *k, struct point *PATH, struct cross *INTER);
extern void flipCross(struct point *PATH, struct cross *INTER, int c);
extern double MIN(double x, double y);
extern double MAX(double x,double y);
extern double dist2(double x0, double y0, double x1, double y1);
extern int project(double x, double y, double x0, double y0, double x1, double y1,
		   double *xr, double *yr);
extern double Ldist(double x, double y, double x0, double y0, double x1, double y1);
extern double pdist(double x, double y, double x0, double y0, double x1, double y1);
extern int Inode(double x, double y,
		 struct ktag *KL, struct point *PATH, struct cross *INTER);
extern int insControl(struct point *PATH, struct cross *INTER,
		      int seg, double xp, double yp);
extern int intr1(double x0, double x1, double x2, double x3);
extern int ldisj(double x0, double y0, double x1, double y1,
	  double x2, double y2, double x3, double y3,
		 double *x, double *y);
extern int ncross(struct cross *INTER);
extern int Npath(struct point *PATH, int A);
extern int newpt(struct point *PATH);
extern int inbetween(double x, double y, 
	      double x0, double y0,
		     double x1, double y1);
extern void insert_cross(struct point *PATH, struct cross *INTER,
			 int i,int c,int pev);
extern void ditchCross(struct point *PATH, struct cross *INTER, int n);
extern void softDitchCross(struct point *PATH, struct cross *INTER,
			   int n, struct spst *soft_spot);
extern void ditchCrossings(struct point *PATH, struct cross *INTER, struct ktag *a);
extern int Gcross(int p1, int p3, struct point *PATH, struct cross *INTER);
extern void lock_loop(struct point *PATH, struct cross *INTER, int i, int knot);
extern void try_fix(struct point *PATH, struct cross *INTER,
	     struct point *OLDPATH, struct cross *OLDINTER,
		    int j,int guess);
extern double rate_match(struct point *PATH, struct cross *INTER,
		  struct point *OLDPATH, struct cross *OLDINTER,
			 int j,int guess);
extern int find_cross(struct point *PATH, struct cross *INTER, struct ktag *a,
	       struct point *OLDPATH, struct cross *OLDINTER, struct ktag *OLDKNOTL,
		      struct spst *soft_point);
extern void assignXY(struct point *PATH, struct cross *INTER,
	      struct kpt *p1, struct kpt *p2,
		     double *x, double *y);
extern void rotate(double *x, double *y, double s, double c);
extern double dslope(double x0, double y0,
	      double xz, double yz,
		     double x1, double y1);
extern void PKnot(struct point *PATH, struct ktag *KNOT,
		  double cx, double cy);
extern void clearKnot(struct point *PATH, struct cross *INTER, struct ktag *KNOT);
extern int Csign(struct point *PATH, struct cross *INTER, int n);
extern void Kmark(struct point *PATH, struct ktag *KNOTL, int k);
extern void fix_comp(struct point *PATH, struct ktag *KNOTL,
		     int l, int m);
extern MinPoints(struct point *PATH, int p);
extern void userId();
extern int pcross(struct cross c);
extern int ppoint(struct point p);


#endif
