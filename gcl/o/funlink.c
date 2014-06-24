/*  Copyright William Schelter. All rights reserved.
Fast linking method for kcl by W. Schelter University of Texas
   Note there are also changes to 
 cmpcall.lsp and cmptop.lsp */


#include <stdlib.h>
#include <string.h>
#include "include.h"
#include "sfun_argd.h"
#include "page.h"

static int
clean_link_array(object *,object *);

object sScdefn;
typedef object (*object_func)();

static int     
vpush_extend(void *,object);

object sLAlink_arrayA;
int Rset = 0;

DEFVAR("*LINK-LIST*",sSAlink_listA,SI,0,"");

static inline void
append_link_list(object sym,int n) {

  object x;
  int i;

  if (!Rset || !sSAlink_listA->s.s_dbind) return;
  for (x=sSAlink_listA->s.s_dbind;x!=Cnil && x->c.c_car->c.c_car!=sym;x=x->c.c_cdr);
  if (x==Cnil) 
    sSAlink_listA->s.s_dbind=MMcons((x=list(6,sym,make_fixnum(0),make_fixnum(0),make_fixnum(0),make_fixnum(0),make_fixnum(0))),sSAlink_listA->s.s_dbind);
  else 
    x=x->c.c_car;
  for (x=x->c.c_cdr,i=0;i<n;i++,x=x->c.c_cdr);
  x->c.c_car=one_plus(x->c.c_car);
}


/* cleanup link */
void
call_or_link(object sym, void **link )
{object fun;
 fun = sym->s.s_gfdef;
#if 0
 fprintf ( stderr, "call_or_link: fun %x\n", fun );
#endif 
 if (fun == OBJNULL) {FEinvalid_function(sym); return;}
 if (type_of(fun) == t_cclosure
     && (fun->cc.cc_turbo))
   {if (Rset==0) {MMccall(fun, fun->cc.cc_turbo);}
    else (*(fun)->cf.cf_self)(fun->cc.cc_turbo);
    return;}
 if (Rset==0) funcall(fun);
   else
   if (type_of(fun) == t_cfun)
       { (void) vpush_extend( link,sLAlink_arrayA->s.s_dbind);
	  (void) vpush_extend( *link,sLAlink_arrayA->s.s_dbind);	 
         *link = (void *) (fun->cf.cf_self);
#if 0
         fprintf ( stderr, "call_or_link: cf %x\n", fun->cf );
         fprintf ( stderr, "call_or_link: cf_name %x\n", fun->cf.cf_name );
         fprintf ( stderr, "call_or_link: cf_data %x\n", fun->cf.cf_data );
         fprintf ( stderr, "call_or_link: cf_self %x\n", fun->cf.cf_self );
         fflush ( stderr );
         fprintf ( stderr, "call_or_link: staddr %x\n", fun->cf.cf_name->st.st_self );
         fprintf ( stderr, "call_or_link: ststring %s\n", fun->cf.cf_name->st.st_self );
         fflush ( stderr );
#endif         
	 (*(void (*)())(fun->cf.cf_self))();
       }
   else {
     append_link_list(sym,0);
     funcall(fun);}}

void
call_or_link_closure(object sym, void **link, void **ptr)
{object fun;
 fun = sym->s.s_gfdef;
 if (fun == OBJNULL) {FEinvalid_function(sym); return;}
 if (type_of(fun) == t_cclosure
     && (fun->cc.cc_turbo))
   {if (Rset) {
     (void) vpush_extend( link,sLAlink_arrayA->s.s_dbind);
     (void) vpush_extend( *link,sLAlink_arrayA->s.s_dbind);
     *ptr = (void *)(fun->cc.cc_turbo);
     *link = (void *) (fun->cf.cf_self);
     MMccall(fun, fun->cc.cc_turbo);}
    else
      { 
	append_link_list(sym,1);
	MMccall(fun, fun->cc.cc_turbo);}
    return;}
 if (Rset==0) funcall(fun);
   else
     /* can't do this if invoking foo(a) is illegal when foo is not defined
	to take any arguments.   In the majority of C's this is legal */
     
   if (type_of(fun) == t_cfun)
       { (void) vpush_extend( link,sLAlink_arrayA->s.s_dbind);
	  (void) vpush_extend( *link,sLAlink_arrayA->s.s_dbind);	 
         *link = (void *) (fun->cf.cf_self);
	 (*(void (*)())(fun->cf.cf_self))();
       }
   else {
     	append_link_list(sym,2);
	funcall(fun);}}

/* for pushing item into an array, where item is an address if array-type = t
or a fixnum if array-type = fixnum */

#define SET_ITEM(ar,ind,val) (*((object *)(&((ar)->ust.ust_self[ind]))))= val
static int     
vpush_extend(void *item, object ar)
{ register int ind = ar->ust.ust_fillp;
 AGAIN:
  if (ind < ar->ust.ust_dim)
   {SET_ITEM(ar,ind,item);
    ind += sizeof(void *); 
    return(ar->v.v_fillp = ind);}
       else
    { 
      int newdim= ROUND_UP_PTR((2 + (int) (1.3 * ind)));
      unsigned char *newself;
      newself = (void *)alloc_relblock(newdim);
      bcopy(ar->ust.ust_self,newself,ind);
      ar->ust.ust_dim=newdim;
      ar->ust.ust_self=newself;
      goto AGAIN;
    }}


/* if we unlink a bunch of functions, this will mean there are some
   holes in the link array, and we should probably go through it and
   push them back  */
static int number_unlinked=0;

static void
delete_link(void *address, object link_ar)
{object *ar,*ar_end,*p;
 p=0;
 ar = link_ar->v.v_self;
 ar_end = (object *)&(link_ar->ust.ust_self[link_ar->v.v_fillp]);
 while (ar < ar_end)
   { if (*ar && *((void **)*ar)==address)
       { p = (object *) *ar;
	 *ar=0;
	 *p = *(ar+1);
	 number_unlinked++;}
     ar=ar+2;}
 if (number_unlinked > 40)
   link_ar->v.v_fillp=
     clean_link_array(link_ar->v.v_self,ar_end); }


DEFUN_NEW("USE-FAST-LINKS",object,fSuse_fast_links,SI,1,2,NONE,OO,OO,OO,OO,(object flag,...),
      "Usage: (use-fast-links {nil,t} &optional fun) turns on or off \
the fast linking depending on FLAG, so that things will either go \
faster, or turns it off so that stack information is kept.  If SYMBOL \
is supplied and FLAG is nil, then this function is deleted from the fast links")
{int n = VFUN_NARGS;
 object sym;
 va_list ap;
 object *p,*ar,*ar_end;
 object link_ar;
 object fun=Cnil;

{ va_start(ap,flag);
 if (n>=2) sym=va_arg(ap,object);else goto LDEFAULT2;
 goto LEND_VARARG;
 LDEFAULT2: sym = Cnil ;
 LEND_VARARG: va_end(ap);}

  if (sLAlink_arrayA ==0)    RETURN1(Cnil);
  link_ar = sLAlink_arrayA->s.s_dbind;
  if (link_ar==Cnil && flag==Cnil) RETURN1(Cnil);
  check_type_array(&link_ar);
  if (type_of(link_ar) != t_string)
  { FEerror("*LINK-ARRAY* must be a string",0);}
  ar = link_ar->v.v_self;
  ar_end = (object *)&(link_ar->ust.ust_self[link_ar->v.v_fillp]);
 switch (n)
      {
  case 1:
   if (flag==Cnil)
    { Rset=0;
     while ( ar < ar_end)
      /* set the link variables back to initial state */
	 { 
	    p = (object *) *ar;
	    if (p) *p = (ar++, *ar); else ar++;
	   ar++;
	 }
    link_ar->v.v_fillp = 0;
    }
  else
    { Rset=1;}
    break;
  case 2:

   if ((type_of(sym)==t_symbol))
     fun = sym->s.s_gfdef;
   else
     if (type_of(sym)==t_cclosure)
       fun = sym;
   else {FEerror("Second arg: ~a must be symbol or closure",0,sym);
       }
   if(Rset)
     {
      if(!fun) RETURN1(Cnil);
      switch(type_of(fun)){
      case t_cfun:
      case t_sfun:
      case t_vfun:	
      case t_gfun:
      case t_cclosure:
      case t_closure:
      case t_afun:
	delete_link(fun->cf.cf_self,link_ar);
	/* becoming obsolete 
	 y=getf(sym->s.s_plist,sScdefn,Cnil);
	 if (y!=Cnil)
	   delete_link(fix(y),link_ar);
	   */

      break;
       default: 
        /* no link for uncompiled functions*/
        break;	
    }
  }
    break;
  default:
    FEerror("Usage: (use-fast-links {nil,t} &optional fun)",0);
}
  RETURN1(Cnil);
}
object
fSuse_fast_links_2(object flag,object res) {
  VFUN_NARGS=2;
  return FFN(fSuse_fast_links)(flag,res);
}


object
clear_compiler_properties(object sym, object code) { 
  object tem;
  extern object sSclear_compiler_properties;  
  
  if (sSclear_compiler_properties && sSclear_compiler_properties->s.s_gfdef!=OBJNULL)
    if ((sSAinhibit_macro_specialA && sSAinhibit_macro_specialA->s.s_dbind != Cnil) ||
	sym->s.s_sfdef == NOT_SPECIAL)
      (void)ifuncall2(sSclear_compiler_properties,sym,code);
  tem = getf(sym->s.s_plist,sStraced,Cnil);

  VFUN_NARGS=2;
  FFN(fSuse_fast_links)(Cnil,sym);
  return tem!=Cnil ? tem : sym;
  
}

static int
clean_link_array(object *ar, object *ar_end)
{int i=0;
 object *orig;
 orig=ar;
 number_unlinked=0;
  while( ar<ar_end)
   {if(*ar)
      {orig[i++]= *ar++ ;
	 orig[i++]= *ar++;
       }
   else ar=ar+2;       
    }
 return(i*sizeof(object *));
 }

#include "apply_n.h"
  
/* Used for calling cfunctions which take object args, and return object 
value.  This function is called by the static lnk function in the reference
file */


static object
call_proc(object sym, void **link, int argd, va_list ll)
{object fun;
 int nargs;
 check_type_symbol(&sym);
 fun=sym->s.s_gfdef;
 if (fun && (type_of(fun)==t_sfun
	     || type_of(fun)==t_gfun
	     || type_of(fun)==t_afun
	     || type_of(fun)== t_vfun)
	     && Rset) /* the && Rset is to allow tracing */
   {object (*fn)();
    fn = fun->sfn.sfn_self;
    if (type_of(fun)==t_vfun)
      { /* argd=VFUN_NARGS; */ /*remove this! */
	nargs=SFUN_NARGS(argd);
	if (nargs < fun->vfn.vfn_minargs || nargs > fun->vfn.vfn_maxargs
	    || (argd & (SFUN_ARG_TYPE_MASK | SFUN_RETURN_MASK)))
	 goto WRONG_ARGS;
	if ((VFUN_NARG_BIT & argd) == 0)
	 /* don't link */
	 { 
	   VFUN_NARGS = nargs;
	   goto   AFTER_LINK;
	 }
      }
    else if (type_of(fun)==t_afun) {
      ufixnum at=F_TYPES(fun->sfn.sfn_argd)>>F_TYPE_WIDTH;
      ufixnum ma=F_MIN_ARGS(fun->sfn.sfn_argd);
      ufixnum xa=F_MAX_ARGS(fun->sfn.sfn_argd);
      ufixnum rt=F_RESULT_TYPE(fun->sfn.sfn_argd);

      nargs=SFUN_NARGS(argd);
      if (nargs<ma || nargs > xa)
	goto WRONG_ARGS;
      if (((argd>>8)&0x3)!=rt)
	  FEerror("Return type mismatch in call to  ~s",1,sym);
      if ((argd>>12)!=at)
	  FEerror("Arg type mismatch in call to  ~s",1,sym);
    }
    else /* t_gfun,t_sfun */
      { nargs= SFUN_NARGS(argd);
	if ((argd & (~VFUN_NARG_BIT)) != fun->sfn.sfn_argd) 
	WRONG_ARGS:    
	  FEerror("Arg or result mismatch in call to  ~s",1,sym);
      }
   
    (void) vpush_extend(link,sLAlink_arrayA->s.s_dbind);
    (void) vpush_extend(*link,sLAlink_arrayA->s.s_dbind);	 
    *link = (void *)fn;
  AFTER_LINK:	
 
    {
      object *new;
      COERCE_VA_LIST(new,ll,nargs);
      return(c_apply_n_fun(fun,nargs,new));
    }
   }
 else				/* there is no cdefn property */
/* regular_call: */
   { 
     object fun;
     register object *base;
     enum ftype result_type;
     /* we check they are valid functions before calling this */

     append_link_list(sym,3);

     if(type_of(sym)==t_symbol) fun = symbol_function(sym);
     else fun = sym;
     vs_base= (base =   vs_top);
     if (fun == OBJNULL) FEinvalid_function(sym);
     /* push the args */
/*     if (type_of(fun)==t_vfun) argd=fcall.argd; */ /*remove this! */
     nargs=SFUN_NARGS(argd);
     result_type=SFUN_RETURN_TYPE(argd);
     SFUN_START_ARG_TYPES(argd);
     {int i=0;
      if (argd==0)
	{while(i < nargs)
	    {vs_push(va_arg(ll,object));
	     i++;}}
      else
	{while(i < nargs)
	    {enum ftype typ=SFUN_NEXT_TYPE(argd);
	      vs_push((typ==f_object? va_arg(ll,object):
		       make_fixnum(va_arg(ll,fixnum))));
	     i++;}}
    }

     vs_check;
     
     funcall(fun);
      vs_top=base;
	/* vs_base=oldbase;
      The caller won't expect us to restore these.  */
     return((result_type==f_object? vs_base[0] : (object)fix(vs_base[0])));
   }
}


/* static object call_vproc(object sym, void *link, va_list ll) */
/* {return call_proc(sym,link,VFUN_NARGS | VFUN_NARG_BIT,ll);} */

/* For ANSI C stdarg */

object
call_proc_new(object sym, void **link, int argd, object first, va_list ll)
{object fun;
 int nargs;
 check_type_symbol(&sym);
 fun=sym->s.s_gfdef;
 if (fun && (type_of(fun)==t_sfun
	     || type_of(fun)==t_gfun
	     || type_of(fun)==t_afun
	     || type_of(fun)== t_vfun)
     && Rset) /* the && Rset is to allow tracing */
   {object (*fn)();
   fn = fun->sfn.sfn_self;
   if (type_of(fun)==t_vfun)
     { /* argd=VFUN_NARGS; */ /*remove this! */
       nargs=SFUN_NARGS(argd);
       if (nargs < fun->vfn.vfn_minargs || nargs > fun->vfn.vfn_maxargs
	   || (argd & (SFUN_ARG_TYPE_MASK | SFUN_RETURN_MASK)))
	 goto WRONG_ARGS;
       if ((VFUN_NARG_BIT & argd) == 0)
	 /* don't link */
	 { 
	   VFUN_NARGS = nargs;
	   goto   AFTER_LINK;
	 }
     }
    else if (type_of(fun)==t_afun) {
      ufixnum at=F_TYPES(fun->sfn.sfn_argd)>>F_TYPE_WIDTH;
      ufixnum ma=F_MIN_ARGS(fun->sfn.sfn_argd);
      ufixnum xa=F_MAX_ARGS(fun->sfn.sfn_argd);
      ufixnum rt=F_RESULT_TYPE(fun->sfn.sfn_argd);

      nargs=SFUN_NARGS(argd);
      if (nargs<ma || nargs > xa)
	goto WRONG_ARGS;
      if (((argd>>8)&0x3)!=rt)
	  FEerror("Return type mismatch in call to  ~s",1,sym);
      if ((argd>>12)!=at)
	  FEerror("Arg type mismatch in call to  ~s",1,sym);
    }
   else /* t_gfun,t_sfun */
     { nargs= SFUN_NARGS(argd);
     if ((argd & (~VFUN_NARG_BIT)) != fun->sfn.sfn_argd) 
       WRONG_ARGS:    
     FEerror("Arg or result mismatch in call to  ~s",1,sym);
     }
   
   (void) vpush_extend(link,sLAlink_arrayA->s.s_dbind);
   (void) vpush_extend(*link,sLAlink_arrayA->s.s_dbind);	 
   *link = (void *)fn;
   AFTER_LINK:	
   
   {
     object *new;
     COERCE_VA_LIST_NEW(new,first,ll,nargs);
     return(c_apply_n_fun(fun,nargs,new));
   }
   
   }
 else				/* there is no cdefn property */
   /* regular_call: */
   { 
     object fun;
     register object *base;
     enum ftype result_type;

     append_link_list(sym,4);

     /* we check they are valid functions before calling this */
     if(type_of(sym)==t_symbol) fun = symbol_function(sym);
     else fun = sym;
     vs_base= (base =   vs_top);
     if (fun == OBJNULL) FEinvalid_function(sym);
     /* push the args */
/*     if (type_of(fun)==t_vfun) argd=fcall.argd; */ /*remove this! */
     nargs=SFUN_NARGS(argd);
     result_type=SFUN_RETURN_TYPE(argd);
     SFUN_START_ARG_TYPES(argd);
     {int i=0;
      if (argd==0)
	{while(i < nargs)
	    {vs_push(i ? va_arg(ll,object) : first);
	     i++;}}
      else
	{
	  while(i < nargs) {
	    enum ftype typ=SFUN_NEXT_TYPE(argd);
	    object _xx;
	    if (typ==f_object)
	      _xx=i ? va_arg(ll,object) : first;
	    else {
	      long _yy;
	      _yy=i ? va_arg(ll,fixnum) : (fixnum)first;
	      _xx=make_fixnum(_yy);
	    }
	    vs_push(_xx);
	    i++;
	  }
	}
    }

     vs_check;
     
     funcall(fun);
     vs_top=base;
     /* vs_base=oldbase;
	The caller won't expect us to restore these.  */
     return((result_type==f_object? vs_base[0] : (object)fix(vs_base[0])));
   }
}


object call_vproc_new(object sym, void *link, object first,va_list ll)
{return call_proc_new(sym,link,VFUN_NARGS | VFUN_NARG_BIT,first,ll);}

static object
mcall_proc0(object sym,void *link,int argd,...) 
{
  object res;
  va_list ap;

  va_start(ap,argd);
  res=call_proc(sym,link,argd,ap);
  va_end(ap);

  return res;

}

object
call_proc0(object sym, void *link)
{return mcall_proc0(sym,link,0);}

#if 0
object
call_proc1(object sym,void *link,...)
{  va_list ll;
   va_start(ll,link);
return (call_proc(sym,link,1,ll));
    va_end(ll);
}

object
call_proc2(object sym,object link,...)
{ va_list ll;
   va_start(ll,link);
   return (call_proc(sym,link,2,ll));
    va_end(ll);
}
  
#endif

   

object
ifuncall(object sym,int n,...)
{ va_list ap;
  int i;
  object *old_vs_base;
  object *old_vs_top;
  object x;
  old_vs_base = vs_base;
  old_vs_top = vs_top;
  vs_base = old_vs_top;
  vs_top=old_vs_top+n;
  vs_check;
  va_start(ap,n);
  for(i=0;i<n;i++)
    old_vs_top[i]= va_arg(ap,object);
  va_end(ap);
  if (type_of(sym->s.s_gfdef)==t_cfun)
    (*(sym->s.s_gfdef)->cf.cf_self)();
  else  super_funcall(sym);
  x = vs_base[0];
  vs_top = old_vs_top;
  vs_base = old_vs_base;
  return(x);
}


/* static object */
/* imfuncall(object sym,int n,...) */
/* { va_list ap; */
/*   int i; */
/*   object *old_vs_top; */
/*   old_vs_top = vs_top; */
/*   vs_base = old_vs_top; */
/*   vs_top=old_vs_top+n; */
/*   vs_check; */
/*   va_start(ap,n); */
/*   for(i=0;i<n;i++) */
/*     old_vs_top[i]= va_arg(ap,object); */
/*   va_end(ap); */
/*   if (type_of(sym->s.s_gfdef)==t_cfun) */
/*     (*(sym->s.s_gfdef)->cf.cf_self)(); */
/*   else  super_funcall(sym); */
/*   return(vs_base[0]); */
/* } */

/* go from beg+1 below limit setting entries equal to 0 until you
   come to FRESH 0's . */

#define FRESH 40

int
clear_stack(object *beg, object *limit)
{int i=0;
 while (++beg < limit)
  {if (*beg==0) i++;
   if (i > FRESH) return 0;
   ;*beg=0;} return 0;}

static object
FFN(set_mv)(int i, object val)
{ if (i >= (sizeof(MVloc)/sizeof(object)))
     FEerror("Bad mv index",0);
  return(MVloc[i]=val);
}


static object
FFN(mv_ref)(unsigned int i)
{ object x;
  if (i >= (sizeof(MVloc)/sizeof(object)))
     FEerror("Bad mv index",0);
  x = MVloc[i];
  return x;
}


#include "xdrfuns.c"

DEF_ORDINARY("CDEFN",sScdefn,SI,"");
DEFVAR("*LINK-ARRAY*",sLAlink_arrayA,LISP,Cnil,"");

void
gcl_init_links(void)
{	

	make_si_sfun("SET-MV",set_mv, ARGTYPE2(f_fixnum,f_object) |
		     RESTYPE(f_object));
	make_si_sfun("MV-REF",mv_ref, ARGTYPE1(f_fixnum) | RESTYPE(f_object));
	gcl_init_xdrfuns();
	      }

