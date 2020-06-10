
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

#ifndef struct_h
#define struct_h

#define strbuflen 1024


struct cross   /* redundant as hell but allows some computational savings */
   {
   double x,y;
   int touch,toPoint[2],fromPoint[2],toCross[2],fromCross[2];
   int toLev[2],fromLev[2],tag;
   };
#define levLow 0
#define levHigh 1

struct point 
   {
   double x,y;
   int touch,toPoint,fromPoint,clist,toLev,tag,knot;
   };

struct ktag
   {
   int n;
   int l1,l2;    /* label variables for later use */
   };

struct kpt   /* knot traversal data structure */
   {
   int wstruct,node,level,snode,swstruct,slevel,start;
   };
#define onPath 0
#define onCross 1

extern int GLOBoutmode,MEMREADY;
extern int maxlink,maxcross,maxpoint;
#define devX 0
#define devTEK 1
#define devPIC 2

/* sprintf def to shut lint up */
#if 0
extern char *sprintf(/* stuff */);
#endif
extern char version_stamp[];
extern FILE *CUR_IN;

/* soft ditch states */
#define initialSoft 0
#define tenativeSoft 1
#define lockedSoft 2
#define invalidSoft 3

struct spst /* softness state */
   {
   int state,level,P1,P2,lcross;
   };

extern int PANICING,deltay,deltcom;

#define stlen strbuflen
#define big_stlen strbuflen

#endif
