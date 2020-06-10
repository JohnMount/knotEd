

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

#ifndef main_h
#define main_h



extern int com_list[];
extern char Xmes[][strbuflen];
extern int *Xbool[];
extern struct point *PATH,*OLDPATH,*UPATH,*TPATH;
extern struct cross *INTER,*OLDINTER,*UINTER,*TCROS;
extern struct ktag *KNOTL,*OLDKNOTL,*UKNOTL,*TKNTL;
extern int GLOBspline,GLOBarrow,GLOBoutmode,name_valid,GLOBfast;
extern int GLOBCrad,Bnarcs,LpA,WRITTEN,ISOTOPY_LOCK;
extern int GLOBmpoint,GLOBsign,GLOBtag,GLOBghost,GLOBkl;
extern int Ncom,comx,comy,last_sel,f1,f2,f3,*Atag,*Btag;
extern struct spst soft_spot;
extern double Bparm,GLOBTScale;
extern struct ktag K_0;


#define minPoints 4
#define sq(x) ((x)*(x))
#define mes1 "Pull Knot"
#define mes2 "Rip Knot"
#define mes3 "Flip string"
#define huge 5000000.0


#define Winname "knot"
#define Comname "com"
#define Grpname "  "
#define NrmInterest ButtonPressed | ExposeRegion | KeyPressed
#define ComInterest NrmInterest | MouseMoved 
#define GrpInterest NrmInterest
#define lat_gran 10
#define Xprint(s) x_label(s)
#define Ncoms 40
#define scale_border 0.85

extern void save_knot(FILE *CUR_OUT, struct point *PATH, struct cross *INTER, struct ktag *KNOTL);

		      

#endif

