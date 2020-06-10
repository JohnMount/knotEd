
/*           
 * Braider code: Author unknown
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXWSIZE 500
#define MAXCROSS 50

#define BADD(a,b) (a > 0 ? a+b : a-b)
#define ABS(a) (a > 0 ? a : -a)
#define SIGN(a) (a > 0 ? 1 : -1)
#define BETWEEN(a,b,c) (a<c && a<=b && b<=c || c<a && (a<=b || b<=c))


/* Sbknot = 'Seifert braided knot', a circular list of Seifert braids */
typedef struct sbknot {
  int nsbraids;          /* number of Seifert braids */
  struct sbraid *base;   /* pointer to the base Seifert braid of the list */
} Sbknot;

/* Sbraid = 'Seifert braid', a circular list of rlbraids */
typedef struct sbraid {
  int nstrands;           /* number of strands */
  struct rlbraid *base;   /* pointer to base rlbraid */
  struct sbraid *prev;
  struct sbraid *next;
} Sbraid;

/* Rlbraid = 'right/left braid', a braiding of two Seifert braids.
 *  Note that each such braid appears twice, once from the right
 *   and once from the left. */
typedef struct rlbraid {
  char rl;                   /* rl = 'r' if the other sbraid is on
				the right, ='l' otherwise */
  struct braid *braid;       /* a pointer to the braid. Note the the twin
				rlbraid points to the SAME braid */
  struct sbraid *osbraid;    /* sbraid containing the other/twin rlbraid */
  struct rlbraid *orlbraid;  /* twin ('other') rlbraid */
  struct sbraid *sbraid;     /* sbraid containing this rlbraid */
  struct rlbraid *prev;
  struct rlbraid *next;
} Rlbraid;

typedef struct braid {
  int nstrands;        /* number of strands (= sum of the numbers of 
			  strands of the two sbraids in intertwines */
  int word[MAXWSIZE];  /* +n means \sigma_n, -n means \sigma_n^{-1},
			  and the word is terminated by a 0 */
} Braid;

/* Aknot : a rendundant but convenient way of encoding a knot projection */
typedef struct aknot {
  int ncross;              /* number of crossings */
  struct uopass *uopass;   /* an array of 2*n under/over passes */
} Aknot;

/* Uopass = 'under/over pass' */
typedef struct uopass {
  int cross;   /* a label for the crossing (each label appears twice in
		  the array */
  int sign;    /* the sign of the crossing, = +1 or -1 */
  char uo;     /* = 'u' for an underpass, = 'o' for an overpass */
} Uopass;

/* Bknot: a more compact way of encoding a knot projection */
typedef struct bknot {
  int ncross;                 /* number of crossings */
  int semicross[2*MAXCROSS];  /* = +n or -n, where n labels the crossing,
				 the sign of the first occurance of +n/-n
				 is positive if we're going over, negative
				 if we're going under, and the sign of the
				 second occurance of +n/-n gives the sign of
				 the crossing.  For example, +1 -2 +3 +1 +2 +3
				 is a right-handed trefoil. */
} Bknot;

/* Tknot = 'Thistlethwaite knot'; see T. and Dowker's paper for an
   explanation */
typedef struct tknot {
  int ncross;
  int list[MAXCROSS];
} Tknot;


/*----------------------------------------------*/

Bknot bk_store;
Tknot tk_store;


// forward refs:
extern void make_braid(char *inst, char *outst,int outlen);
extern Braid *sbktob(Sbknot *sbk);
extern void glob(Sbknot *sbk, Rlbraid *rlb1, Rlbraid *rlb2);
extern void rextend(Braid *b, int ns);
extern void lextend(Braid *b, int ns);
extern void rexconj(Braid *b, int ns1, int ns2, char uo);
extern void lexconj(Braid *b, int ns1, int ns2, char uo);
extern Sbknot *aktosbk(Aknot *ak);
extern Aknot *bktoak(Bknot *bk);
extern void brsimp(Braid *br);

void make_braid(char *inst, char *outst,int outlen) 
{
  Aknot *ak;
  Bknot *bk;
  Tknot *tk;
  Sbknot *sbk;
  Braid *br;
  int i,p,n,sign;
  char s[256];
  
  outst[0] = 0;
  bk = &bk_store;
  tk = &tk_store;

    /* get a Bknot */
    p = 0;
    while(inst[p]!='[')   /* get first brace */
       ++p;
    ++p;
    while(inst[p]!='[')   /* get second brace */
       {
       if(inst[p]==']')
          {
          (void)fprintf(stderr,"null knot to braider\n");
          return;
          }
       ++p;
       }
    ++p;
    i=0;
    while(inst[p]!=']')
       {
       n = 0;
       sign = 1;
       while(inst[p]==' ')
          ++p;
       if(inst[p]!=']')
          {
          if(inst[p]=='-')
             {
             sign = -1;
             ++p;
             }
          if(inst[p]=='+')
             ++p;
          while((inst[p]>='0')&&(inst[p]<='9'))
             {
             n = 10*n + inst[p] - '0';
             ++p;
             }
          bk->semicross[i] = n*sign;
          ++i;
          }
       }
    /* make sure we had only one component */
    ++p;
    while(inst[p]!=']')
       {
       if(inst[p]=='[')
          {
          fprintf(stderr,"multiple components to braider\n");
          return;
          }
       ++p;
       }
    bk->ncross = i/2;
    /* convert it to an Aknot */
    ak = bktoak(bk);

  if (ak == NULL) {
    fprintf(stderr,"braid error\n");
    return;
  }
  
/* convert to a sbknot */
  sbk = aktosbk(ak);
  
/* convert to a braid */
if(sbk->nsbraids==0)
   return;
  br = sbktob(sbk);

/* simplify braid */
  brsimp(br);

/* print braid */
  i = 0;
  while ((br->word[i] != 0)&&(outlen>0)) {
    snprintf(s,256,"%d ", br->word[i++]);
    strncat(outst,s,outlen);
    outlen -= strlen(s);
    if(outlen>0) {
      if (i%20 == 0) {
	strcat(outst,"\n");
	outlen--;
      }
    }
  }
  if(outlen>0) {
    strcat(outst,"\n");
    outlen--;
  }
}

/*----------------------------------------------*/
/* This guy takes an Sbknot and returns a Braid */


Braid *sbktob(Sbknot *sbk)
{
  Sbraid *sb, *sbp[20], *lcsb;
  Rlbraid *rlb, *lastlb, *lastrb, *lastrlb, *nextrlb;
  Braid *br;
  int i, j, n;

  br = (Braid *) malloc(sizeof(Braid));
  
/* loop through sbraids */
  sb = sbk->base;
  lcsb = sbk->base;   /* lcsb = 'last changed sbraid' */
  do {

/* loop through rlbraids */
    rlb = sb->base;
    lastlb = NULL;    /* last left rlbraid encountered */
    lastrb = NULL;    /* last right rlbraid encountered */
    while (rlb->next != sb->base) {
      if (rlb->rl == 'l') {
	lastlb = rlb;
      } else {
	lastrb = rlb;
      }
      rlb = rlb->next;
      if (rlb->rl == 'l') {
	if (lastlb != NULL) {
/* if they connect to different sbraids, glob the sbraids together */
	  if (rlb->osbraid != lastlb->osbraid) {
            glob(sbk, lastlb->orlbraid, rlb->orlbraid);
	    lcsb = sb;
	  }
	}
      } else {
	if (lastrb != NULL) {
/* if they connect to different sbraids, glob the sbraids together */
	  if (rlb->osbraid != lastrb->osbraid) {
	    glob(sbk, lastrb->orlbraid, rlb->orlbraid);
	    lcsb = sb;
	  }
	}
      }

    }
    sb = sb->next;

  } while (sb != lcsb);

      
/* find leftmost sbraid */
  sbp[0] = sbk->base;
 label1:
  rlb = sbp[0]->base;
  do {
    if (rlb->rl == 'l') {
      sbp[0] = rlb->osbraid;
      goto label1;
    }
    rlb = rlb->next;
  } while (rlb != sbp[0]->base);
/* put sbraids in order (left to right) */
  i = 0;
 label2:
  rlb = sbp[i]->base;
  do {
    if (rlb->rl == 'r') {
      sbp[++i] = rlb->osbraid;
      goto label2;
    }
    rlb = rlb->next;
  } while (rlb != sbp[i]->base);


/* combine into one braid */
  n = 0;
  /* loop through sbraids (except for leftmost and rightmost) */
  for (i=1; i <= sbk->nsbraids-2; i++) {
    n += sbp[i-1]->nstrands;
    /* move base to a left rlbraid */
    while (sbp[i]->base->rl != 'l') {
      sbp[i]->base = sbp[i]->base->next;
    }
    /* insert the right rlbraids into the list */
    rlb = sbp[i]->base;
    do {
      nextrlb = rlb->next;
      if (rlb->rl == 'l') {
	lastrlb = rlb->orlbraid;
      } else {
	lextend(rlb->braid, n);
	lastrlb->next->prev = rlb;
	rlb->next = lastrlb->next;
	lastrlb->next = rlb;
	rlb->prev = lastrlb;
	lastrlb = rlb;
      }
      rlb = nextrlb;
    } while (rlb != sbp[i]->base);
  }
  /* add them all up */
  br->nstrands = n + sbp[sbk->nsbraids-1]->nstrands
                    +  sbp[sbk->nsbraids-2]->nstrands;
  i = 0;
  rlb = sbp[0]->base;
  do {
    j = 0;
    while (rlb->braid->word[j] != 0) {
      br->word[i++] = rlb->braid->word[j++];
    }
    rlb = rlb->next;
  } while (rlb != sbp[0]->base);
  br->word[i++] = 0;

  return br;
  

}


/*---------------------------------------------*/
/* This guy performs the crucial move of Yamada's algorthm */

void glob(Sbknot *sbk, Rlbraid *rlb1, Rlbraid *rlb2)
{
  Sbraid *sb1, *sb2;
  Rlbraid *rlb;

  sb1 = rlb1->sbraid;
  sb2 = rlb2->sbraid;

/* loop over rlbraids in sb1 */
  rlb = rlb1;
  do {
/* change braid */
    if (rlb->rl == 'l') {
      rextend(rlb->braid, sb2->nstrands);
    } else {
      if (sb1->nstrands <= rlb->osbraid->nstrands) {
	lexconj(rlb->braid, sb1->nstrands, sb2->nstrands, 'o');
      } else {
	rexconj(rlb->braid, sb2->nstrands, rlb->osbraid->nstrands, 'o');
      }
    }
    rlb = rlb->next;
  } while (rlb != rlb1);
  
/* loop over rlbraids in sb2 */
  rlb = rlb2;
  do {

/* change braid */
    if (rlb->rl == 'r') {
      lextend(rlb->braid, sb1->nstrands);
    } else {
      if (sb2->nstrands <= rlb->osbraid->nstrands) {
	rexconj(rlb->braid, sb1->nstrands, sb2->nstrands, 'u');
      } else {
	lexconj(rlb->braid, rlb->osbraid->nstrands, sb1->nstrands, 'u');
      }
    }

/* change pointers */
    rlb->sbraid = sb1;
    rlb->orlbraid->osbraid = sb1;

    rlb = rlb->next;
  } while (rlb != rlb2);

/* splice sb1 and sb2 */
  rlb1->next->prev = rlb2->prev;
  rlb2->prev->next = rlb1->next;
  rlb1->next = rlb2;
  rlb2->prev = rlb1;
  sb1->nstrands += sb2->nstrands;
/* remove sb2 */
  sb2->prev->next = sb2->next;
  sb2->next->prev = sb2->prev;
  if (sbk->base == sb2) {
    sbk->base = sb2->next;
  }
  sbk->nsbraids -= 1;

}


/*--------------------------------------------*/

void rextend(Braid *b, int ns)
{
  b->nstrands += ns;

}


/*---------------------------------------------*/

void lextend(Braid *b, int ns)
{
  int i;

  b->nstrands += ns;
  for (i=0; b->word[i] != 0; i++) {
    b->word[i]  = BADD(b->word[i], ns);
  }
  
}


/*---------------------------------------------*/

void rexconj(Braid *b, int ns1, int ns2, char uo)
{
  int tw[MAXWSIZE];
  int i, j, k, s, ns3;

  s = (uo == 'u' ? 1 : -1);
  ns3 = b->nstrands - ns2;
  
  b->nstrands += ns1;

  i = 0;
  for (j=ns1+ns3; j >= 1+ns3; j--) {
    for (k=j; k <= j+ns2-1; k++) {
      tw[i++] = s*k;
    }
  }
  j = 0;
  while (b->word[j] != 0) {
    tw[i++] = b->word[j++];
  }
  for (j=ns1*ns2-1; j >= 0; j--) {
    tw[i++] = -tw[j];
  }
  tw[i++] = 0;
  i = 0;
  do {
    b->word[i] = tw[i];
  } while (tw[i++] != 0);

}


/*---------------------------------------------*/

void lexconj(Braid *b, int ns1, int ns2, char uo)
{
  int tw[MAXWSIZE];
  int i, j, k, s;

  s = (uo == 'o' ? 1 : -1);
  
  b->nstrands += ns2;

  i = 0;
  for (j=ns1; j >= 1; j--) {
    for (k=j; k <= j+ns2-1; k++) {
      tw[i++] = s*k;
    }
  }
  j = 0;
  while (b->word[j] != 0) {
    tw[i++] = BADD(b->word[j], ns2);
    j++;
  }
  for (j=ns1*ns2-1; j >= 0; j--) {
    tw[i++] = -tw[j];
  }
  tw[i++] = 0;
  i = 0;
  do {
    b->word[i] = tw[i];
  } while (tw[i++] != 0);

}


/*--------------------------------------------*/
/* This guy converts an Aknot into an Sbknot.
 * Basically, it returns the Seifert circles and their interconnections */

Sbknot *aktosbk(Aknot *ak)
{
  Sbknot *sbk;
  Sbraid *sb;
  Rlbraid *rlb, *rlbx;
  Braid *br;
  int ip, ic, nc, jp, isb, nsb;

  nc = ak->ncross;
  sbk = (Sbknot *) malloc(sizeof(Sbknot));
  sb = (Sbraid *) calloc(nc+1, sizeof(Sbraid));
  rlb = (Rlbraid *) calloc(2*nc, sizeof(Rlbraid));
  br = (Braid *) calloc(nc+1, sizeof(Braid));  /* '+1' because labels
						  might start at 1 */
  /* do braids and rlbraids */
  for (ip=0; ip < 2*nc; ip++) {
    ic = ak->uopass[ip].cross;
    br[ic].nstrands = 2;
    br[ic].word[0] = ak->uopass[ip].sign;
    br[ic].word[1] = 0;
    rlb[ip].braid = br+ic;
    if (ak->uopass[ip].sign == 1 && ak->uopass[ip].uo == 'o' ||
	ak->uopass[ip].sign == -1 && ak->uopass[ip].uo == 'u') {
      rlb[ip].rl = 'l';
    } else {
      rlb[ip].rl = 'r';
    }
    /* find other uopass for this crossing */
    for (jp = (ip+1)%(2*nc); jp != ip; jp = (jp+1)%(2*nc)) {
      if (ak->uopass[jp].cross == ic) break;
    }
    rlb[ip].orlbraid = rlb + jp;
    rlb[ip].next = rlb + (jp+1)%(2*nc);
    rlb[(jp+1)%(2*nc)].prev = rlb + ip;
    rlb[ip].sbraid = NULL;
  }
  /* do sbraids */
  isb = -1;
  for (ip=0; ip < 2*nc; ip++) {
    if (rlb[ip].sbraid == NULL) {
      sb[++isb].nstrands = 1;
      sb[isb].base = rlb+ip;
    }
    for (rlbx = rlb+ip; rlbx->sbraid == NULL; rlbx = rlbx->next) {
      rlbx->sbraid = sb+isb;
      rlbx->orlbraid->osbraid = sb+isb;
    }
  }
  nsb = isb+1;
  for (isb=0; isb < nsb; isb++) {
    sb[isb].prev = sb + (isb+nsb-1)%nsb;
    sb[isb].next = sb + (isb+1)%nsb;
  }
  /* do sbk */
  sbk->nsbraids = nsb;
  sbk->base = sb;

  return sbk;


}


/*--------------------------------------------*/

Aknot *bktoak(Bknot *bk)
{
  Aknot *ak;
  Uopass *uop;
  int i, j;
  
  
  ak = (Aknot *) malloc(sizeof(Aknot));
  uop = (Uopass *) calloc(2*bk->ncross, sizeof(Uopass));

  ak->ncross = bk->ncross;
  ak->uopass = uop;

  for (i=0; i < 2*bk->ncross; i++) {
    uop[i].cross = ABS(bk->semicross[i]);
    /* see if we've been here before */
    for (j=i-1; j >= 0; j--) {
      if (uop[j].cross == uop[i].cross) {
	uop[i].sign = SIGN(bk->semicross[i]);
	uop[j].sign = SIGN(bk->semicross[i]);
	if (SIGN(bk->semicross[j]) == 1) {
	  uop[j].uo = 'o';
	  uop[i].uo = 'u';
	} else {
	  uop[j].uo = 'u';
	  uop[i].uo = 'o';
	}
      }
    }
  }

  return ak;

}

   

/*----------------------------------------*/
/* This guy simplifies a braid word using 
     1) conjugation
     2) obvious cancellation
     3) the relation i j  = j i (where abs(abs(i)-abs(j)) > 1). */

void brsimp(Braid *br)
{

  int i, j, lw, lci;

  /* find length of word */
  lw = 0;
  while (br->word[lw++] != 0);
  lw -= 1;

  /* simplify word */
  lci = 0;
  i = 0;
  do {
    if (br->word[i] != 0) {
      for (j = (i+1)%lw; j != i; j = (j+1)%lw) {
	if (br->word[j] == 0) continue;
	if (ABS(br->word[j]) == ABS(br->word[i])+1 ||
	    ABS(br->word[j]) == ABS(br->word[i])-1) break;
	if (br->word[j] == -br->word[i]) {
	  br->word[j] = 0;
	  br->word[i] = 0;
	  lci = i;
	  break;
	}
      }
    }
    i = (i+1)%lw;
  } while (i != lci);

  /* compress simplified word */
  i = 0;
  for (j=0; j < lw; j++) {
    if (br->word[j] != 0) br->word[i++] = br->word[j];
  }
  br->word[i] = 0;
}
