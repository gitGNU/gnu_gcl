#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <stdio.h>
#include <math.h>

#include "include.h"


typedef struct {
  const char *n;
  unsigned long ad;
} Plt;

#ifdef LEADING_UNDERSCORE
#define stn(a_) (*(a_)=='_' ? (a_)+1 : (a_))
#else
#define stn(a_) a_
#endif

static int
pltcomp(const void *v1,const void *v2) {
  const Plt *p1=v1,*p2=v2;

  return strcmp(p1->n,p2->n);

}

/* extern int _mcount(); */
extern int mcount();
extern void sincos(double,double *,double *);
extern int __divdi3();
extern int __moddi3();
extern int __udivdi3();
extern int __umoddi3();
extern int __divsi3();
extern int __modsi3();
extern int __udivsi3();
extern int __umodsi3();
extern int $$divI();
extern int $$divU();
extern int $$remI();
extern int $$remU();
extern int __divq();
extern int __divqu();
extern int __remq();
extern int __remqu();

#ifndef _WIN32
#  include "pltd.h"
#endif

#define MY_PLT(a_) {#a_,(unsigned long)(void *)a_}

/* static double cos1(double x) { */
/*   return cos(x); */
/* } */
/* #undef cos */
/* #define cos cos1 */

static Plt mplt[]={
	/* This is an attempt to at least capture the addresses to
	   which the compiler directly refers in C code. (Some symbols
	   are not explicitly mentioned in the C source but are
	   generated by gcc, usually in a platform specific way). At
	   the time of this writing, these symbols alone are
	   sufficient for compiling maxima,acl2,and axiom on x86.
	   This table is not (currently at least) consulted in
	   actuality -- the mere mention of the symbols here (at
	   present) ensures that the symbols are assigned values by
	   the linker, which are used preferentially to these values
	   in sfasli.c.  FIXME -- this should be made synchronous with
	   compiler changes; sort the list automatically.  SORT THIS
	   LIST BY HAND FOR THE TIME BEING. */
#ifndef _WIN32
#  include "plt.h"
#endif    
};

#undef MY_PLT
#define MY_PLT(a_) ({if (!strcmp(p->n,(#a_))) {p->ad=(unsigned long)(void *)a_;continue;}})


int
reset_plt() {

  Plt *p=mplt,*pe=p+sizeof(mplt)/sizeof(*mplt);

  for (;p<pe;p++) {
#ifndef _WIN32
#  include "plt.h"
#endif    
    ;
  }    

  return 0;

}


object sSAplt_tableA;
DEFVAR("*PLT-TABLE*",sSAplt_tableA,SI,Cnil,"");

static int
arsort(const void *v1,const void *v2) {
  const object *op1=v1,*op2=v2;
  object o1=*op1,o2=*op2;
  int j;

  o1=o1->c.c_car;
  o2=o2->c.c_car;
  if ((j=strncmp(o1->st.st_self,
		 o2->st.st_self,
		 o1->st.st_dim<o2->st.st_dim ? 
		 o1->st.st_dim : o2->st.st_dim)))
    return j;
  j=o1->st.st_dim-o2->st.st_dim;
  return j>0 ? 1 : (!j ? 0 : -1);

}

static int
arsearch(const void *v1,const void *v2) {
  const char *s=v1;
  const object *op=v2;

  int j;
  if ((j=strncmp(s,(*op)->c.c_car->st.st_self,(*op)->c.c_car->st.st_dim))) 
    return j;
  j=strlen(s)-(*op)->c.c_car->st.st_dim;
  return j>0 ? 1 : (!j ? 0 : -1);

}

int
parse_plt() {

  FILE *f;
  char b[1024],b1[1024];
  unsigned i,n,j;
  unsigned long u;
#ifdef _WIN32
  char *exe_start = NULL;           /* point to start of .exe */
#endif  
  char *c,*d;
  object st,fi,li,ar,*op;
  Plt *p=mplt,*pe=p+sizeof(mplt)/sizeof(*mplt);
  struct stat ss;

  if (snprintf(b,sizeof(b),"%s",kcl_self)<=0)
    FEerror("Cannot write map filename",0);
#ifdef _WIN32
  exe_start = strstr ( b, ".exe" );
  if ( NULL != exe_start ) *exe_start = '\0';
#endif  
  c=b+strlen(b);
  if (sizeof(b)-(c-b)<5)
    FEerror("Cannot write map filename",0);
  strcpy(c,"_map");
  strcpy(b1,b);
  if (stat(b1,&ss))
    return 0;
  if (!(f=fopen(b1,"r")))
    FEerror("Cannot open map file", 0);
  for (i=j=0,li=Cnil;fgets(b,sizeof(b),f);) {
    if (!memchr(b,10,sizeof(b)-1))
      FEerror("plt buffer too small", 0);
    if (!memcmp(b," .plt",4)) {
      i=1;
      continue;
    }
    if (*b!=' ' || b[1]!=' ' || !i) {
      i=0;
      continue;
    }
    if (sscanf(b,"%lx%n",&u,&n)!=1)
      FEerror("Cannot read address", 0);
    for (c=b+n;*c==32;c++);
    for (d=c;*d!='@' && *d!='\r' && *d!='\n';d++);
    *d=0;
    st=make_simple_string(c);
    fi=make_fixnum(u);
    li=make_cons(make_cons(st,fi),li);
    j++;
  }
  fclose(f);
  unlink(b1);
  ar=fSmake_vector1_1(j,aet_object,Cnil);
  for (;j && !endp(li);li=li->c.c_cdr) 
    ar->v.v_self[--j]=li->c.c_car;

  if (j || !endp(li))
    FEerror("plt list mismatch", 0);
  qsort(ar->v.v_self,ar->v.v_dim,sizeof(*ar->v.v_self),arsort);

  for (;p<pe;p++)
    if ((op=bsearch(p->n,ar->v.v_self,ar->v.v_dim,sizeof(*ar->v.v_self),arsearch)) &&
	fix((*op)->c.c_cdr) != p->ad)
      FEerror("plt/ld address mismatch",0);

  sSAplt_tableA->s.s_dbind=ar;

  return 0;

}
	      


int
my_plt(const char *s,unsigned long *v) {

  Plt *p=mplt,*pe=p+sizeof(mplt)/sizeof(*mplt),tp;
  object *op;

  if (sSAplt_tableA->s.s_dbind && 
      (op=bsearch(s,sSAplt_tableA->s.s_dbind->v.v_self,
		 sSAplt_tableA->s.s_dbind->v.v_dim,
		 sizeof(*sSAplt_tableA->s.s_dbind->v.v_self),
		 arsearch))) {
    *v=fix((*op)->c.c_cdr);
    return 0;
  }
    
  tp.n=stn(s);
  if ((p=bsearch(&tp,p,pe-p,sizeof(*p),pltcomp))) {
    *v=p->ad;
    return 0;
  }

  return -1;

}


int
my_pltp(const char *s,unsigned long *v) {

  Plt *p=mplt,*pe=p+sizeof(mplt)/sizeof(*mplt),tp;

  tp.n=stn(s);
  if ((p=bsearch(&tp,p,pe-p,sizeof(*p),pltcomp))) {
    *v=(unsigned long)&p->ad;
    return 0;
  }

  return -1;

}
