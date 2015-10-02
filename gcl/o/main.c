/*
 Copyright (C) 1994 M. Hagiya, W. Schelter, T. Yuasa

This file is part of GNU Common Lisp, herein referred to as GCL

GCL is free software; you can redistribute it and/or modify it under
the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCL is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
License for more details.

You should have received a copy of the GNU Library General Public License 
along with GCL; see the file COPYING.  If not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/*
	main.c
	IMPLEMENTATION-DEPENDENT
*/

#include <unistd.h>

static void init_main(void);
static void initlisp(void);
static int  multiply_stacks(int);
void install_segmentation_catcher(void);

#define IN_MAIN

#ifdef KCLOVM
#include <ovm/ovm.h>
void change_contexts();
int ovm_process_created; 
void initialize_process();
#endif

#define INLINE

#include "include.h"
#include <signal.h>
#include "page.h"

bool saving_system ;

#ifdef BSD
#include <sys/time.h>
#ifndef SGI
#include <sys/resource.h>
#endif
#endif

#ifdef AOSVS

#endif

#ifdef _WIN32
#include <fcntl.h>
#endif

#define LISP_IMPLEMENTATION_VERSION "April 1994"

char *system_directory;

#define EXTRA_BUFSIZE 8
char stdin_buf[BUFSIZ + EXTRA_BUFSIZE];
char stdout_buf[BUFSIZ + EXTRA_BUFSIZE];

#include "stacks.h"

int debug;			/* debug switch */
int raw_image = TRUE;		/* raw or saved image -- CYGWIN will only place this in .data and not in .bss if initialized to non-zero */
bool GBC_enable=FALSE;

long real_maxpage;
object sSAlisp_maxpagesA;

object siClisp_pagesize;

object sStop_level;


object sSAmultiply_stacksA;
int stack_multiple=1;
static object stack_space;

#ifdef _WIN32
unsigned int _dbegin = 0x10100000;
#endif
#ifdef __CYGWIN__
unsigned long _dbegin = 0;
#endif

#ifdef SGC
int sgc_enabled;
#endif
void install_segmentation_catcher(void);

int cstack_dir=0;

int
get_cstack_dir(VOL fixnum j) {
  static fixnum n;
  fixnum q=n;
  n=1-n;
  return q ? ((fixnum)&j<j ? -1 : 1) : get_cstack_dir((fixnum)&j);
}

void
wipe_stack(VOL void *l) {

  if (cstack_dir==-1) {
    if (l>(void *)&l) bzero((void *)&l,l-(void *)&l);
  } else {
     l+=sizeof(l);
     if ((void *)&l>l) bzero((void *)l,(void *)&l-l);
   }
}

void
clear_c_stack(VOL unsigned n) {

  void *v=OBJNULL;
  alloca(n);
  wipe_stack(&v);

}

fixnum log_maxpage_bound=sizeof(fixnum)*8-1;

static inline int
mbrk(void *v) {
  ufixnum uv=(ufixnum)v,uc=(ufixnum)sbrk(0),ux,um;
  fixnum m=((1UL<<(sizeof(fixnum)*8-1))-1);

#ifdef MAX_BRK /*GNU Hurd fragmentation bug*/
  if ((ufixnum)v>MAX_BRK) return -1;
#endif

  if (uv<uc) {
    um=uv;
    ux=uc;
  } else {
    um=uc;
    ux=uv;
  }
  if (((fixnum)(ux-um))<0)
    return mbrk((void *)uc+(uv<uc ? -m : m)) || mbrk(v);
  return uc==(ufixnum)sbrk(uv-uc) ? 0 : -1;
}
    
int
update_real_maxpage(void) {

  ufixnum i,j,k;
  void *end,*cur;
#ifdef __MINGW32__
  static fixnum n;

  if (!n) {
    init_shared_memory();
    n=1;
  }
#endif

  massert(cur=sbrk(0));
  for (i=0,j=(1L<<log_maxpage_bound);j>PAGESIZE;j>>=1)
    if ((end=data_start+i+j-PAGESIZE)>cur)
      if (!mbrk(end)) {
	real_maxpage=page(end);
	i+=j;
      }
  massert(!mbrk(cur));

#ifdef HAVE_SYSCONF_PHYS_PAGES
  phys_pages=sysconf(_SC_PHYS_PAGES);
#ifdef BRK_DOES_NOT_GUARANTEE_ALLOCATION
  if (phys_pages>0 && real_maxpage>phys_pages+first_data_page) real_maxpage=phys_pages+first_data_page;
#endif
#endif

  available_pages=real_maxpage-first_data_page;
  for (i=t_start,j=0;i<t_other;i++) {
    k=tm_table[i].tm_maxpage;
    if (tm_table[i].tm_type==t_relocatable)
      k*=2;
    else
      j+=k;
    available_pages-=k;
  }
  resv_pages=40<available_pages ? 40 : available_pages;
  available_pages-=resv_pages;

  new_holepage=available_pages/starting_hole_div;
  k=available_pages/20;
  j*=starting_relb_heap_mult;
  j=j<k ? j : k;
  if (maxrbpage<j)
    set_tm_maxpage(tm_table+t_relocatable,j);

  return 0;

}

static int
minimize_image(void) {

#ifdef SGC
  int in_sgc=sgc_enabled;
#else
  int in_sgc=0;
#endif
  extern long new_holepage;
  fixnum old_holepage=new_holepage,i;
  void *new;
  
  if (in_sgc) sgc_quit();
  holepage=new_holepage=1;
  GBC(t_relocatable);
  if (in_sgc) sgc_start();
  new = (void *)(((((ufixnum)rb_pointer)+ PAGESIZE-1)/PAGESIZE)*PAGESIZE);
  core_end = new;
  rb_end=rb_limit=new;
  set_tm_maxpage(tm_table+t_relocatable,(nrbpage=((char *)new-REAL_RB_START)/PAGESIZE));
  new_holepage=old_holepage;
  
#ifdef GCL_GPROF
  gprof_cleanup();
#endif
  
#if defined(BSD) || defined(ATT)  
  mbrk(core_end);
#endif
  
  cbgbccount = tm_table[t_contiguous].tm_adjgbccnt = tm_table[t_contiguous].tm_opt_maxpage = 0;
  rbgbccount = tm_table[t_relocatable].tm_adjgbccnt = tm_table[t_relocatable].tm_opt_maxpage = 0;
  for (i = 0;  i < (int)t_end;  i++)
    tm_table[i].tm_gbccount = tm_table[i].tm_adjgbccnt = tm_table[i].tm_opt_maxpage = 0;
  
  return 0;
  
}

#include <dlfcn.h>

void
init_boot(void) {

  void *v,*q;
  char *z,*s="libboot.so";
  size_t m=sSAsystem_directoryA->s.s_dbind->st.st_fillp,n=m+strlen(s)+1;
  z=alloca(n);
  snprintf(z,n,"%-*.*s%s",(int)m,(int)m,sSAsystem_directoryA->s.s_dbind->st.st_self,s);
  if (!(v=dlopen(z,RTLD_LAZY|RTLD_GLOBAL)))
    printf("%s\n",dlerror());
  if (!(q=dlsym(v,"gcl_init_boot")))
    printf("%s\n",dlerror());
  ((void (*)())q)();

}

DEFUN("SET-LOG-MAXPAGE-BOUND",fixnum,fSset_log_maxpage_bound,SI,1,1,NONE,II,OO,OO,OO,(fixnum l),"") {

  void *end,*dend;
  fixnum def=sizeof(fixnum)*8-1;

  l=l<def ? l : def;
  end=data_start+(1L<<l)-PAGESIZE;
  GBC(t_relocatable);
  dend=heap_end+PAGESIZE+(((rb_pointer-REAL_RB_START)+PAGESIZE-1)&(-PAGESIZE));
  if (end >= dend) {
    minimize_image();
    log_maxpage_bound=l;
    update_real_maxpage();
  }

  return log_maxpage_bound;

}

int pre_gcl=0;
object def_env1[2]={(object)1,Cnil},*def_env=def_env1+1;
object src_env1[2]={(object)1,Cnil},*src_env=src_env1+1;

int
main(int argc, char **argv, char **envp) {

  gcl_init_alloc(&argc);

#ifdef CAN_UNRANDOMIZE_SBRK
#include <stdio.h>
#include "unrandomize.h"
#endif
  
#ifdef LD_BIND_NOW
#include <stdio.h>
#include "ld_bind_now.h"
#endif
  
  
  setbuf(stdin, stdin_buf); 
  setbuf(stdout, stdout_buf);
#ifdef _WIN32
  _fmode = _O_BINARY;
  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );
  _setmode( _fileno( stderr ), _O_BINARY );
#endif
  ARGC = argc;
  ARGV = argv;
  ENVP = envp;
  
#ifdef GET_FULL_PATH_SELF
  GET_FULL_PATH_SELF(kcl_self);
#else
  kcl_self = argv[0];
#endif
#ifdef __MINGW32__
  {
    char *s=kcl_self;
    for (;*s;s++) if (*s=='\\') *s='/';
  }
#endif	
  *argv=kcl_self;
  
  vs_top = vs_base = vs_org;
  ihs_top = ihs_org-1;
  bds_top = bds_org-1;
  frs_top = frs_org-1;

  if (raw_image) {

    printf("GCL (GNU Common Lisp)  %s  %ld pages\n",LISP_IMPLEMENTATION_VERSION,real_maxpage);
    fflush(stdout);
    
    if (argc>1) {
      massert(argv[1][strlen(argv[1])-1]=='/');
      system_directory=argv[1];
    }

    initlisp();
    ihs_top++;lex_new();/*FIXME*/
    
    GBC_enable = TRUE;
    
    gcl_init_init();
  
    sLApackageA->s.s_dbind = user_package;
    
  } else {

    saving_system = FALSE;
    terminal_io->sm.sm_object0->sm.sm_fp = stdin;
    terminal_io->sm.sm_object1->sm.sm_fp = stdout;
#ifdef LD_BIND_NOW /*FIXME currently mips only, verify that these two requirements are the same*/
    reinit_gmp();
#endif
    gcl_init_big1();
#ifdef HAVE_READLINE
    gcl_init_readline_function();
#endif

    if (pre_gcl) init_boot();

  }

#ifdef _WIN32
  detect_wine();
#endif

  sSAlisp_maxpagesA->s.s_dbind = make_fixnum(real_maxpage);

  ihs_push(Cnil);
  lex_new();
  vs_base = vs_top;
  
  interrupt_enable = TRUE;
  install_default_signals();
    
  do 
    super_funcall(sStop_level);
  while (type_of(sSAmultiply_stacksA->s.s_dbind)==t_fixnum && multiply_stacks(fix(sSAmultiply_stacksA->s.s_dbind)));
    
  return 0;

}

/* catch certain signals */
void install_segmentation_catcher(void)
{
  (void) gcl_signal(SIGSEGV,segmentation_catcher);
  (void) gcl_signal(SIGBUS,segmentation_catcher);
}

int catch_fatal=1;
void
error(char *s)
{
        if (catch_fatal>0 && interrupt_enable )
            {catch_fatal = -1;
#ifdef SGC
    if (sgc_enabled)
      sgc_quit();
    if (sgc_enabled==0)
#endif
      install_segmentation_catcher();
    {
      union lispunion st;
      set_type_of(&st,t_string);
      st.st.st_dim=st.st.st_fillp=s ? strlen(s) : 0;
      st.st.st_self=s;
      FEerror("Caught fatal error [memory may be damaged]: ~a",1,&st);
    }
  }
  printf("\nUnrecoverable error: %s.\n", s);
  fflush(stdout);
#ifdef UNIX
  gcl_abort();
#endif
}

static void
initlisp(void) {

        void *v=&v;
	object a;

	a=Cnil;
	if (NOT_OBJECT_ALIGNED(a))
	  error("Cnil is not properly aligned");

	a=Ct;
	if (NOT_OBJECT_ALIGNED(a))
	  error("Ct is not properly aligned");

	if (NULL_OR_ON_C_STACK(v) == 0
#if defined(IM_FIX_BASE)
             || NULL_OR_ON_C_STACK(IM_FIX_BASE) == 0
             || NULL_OR_ON_C_STACK((IM_FIX_BASE|IM_FIX_LIM)) == 0
#endif
	    /* || NULL_OR_ON_C_STACK(vv) */
	    || NULL_OR_ON_C_STACK(pagetoinfo(first_data_page))
	    || NULL_OR_ON_C_STACK(core_end-1)) {
	  /* check person has correct definition of above */
	  fprintf(stderr,"%p %d "
#if defined(IM_FIX_BASE)
		  "%p %d %p %d "
#endif
		  "%p %d %p %d\n",
		  v,NULL_OR_ON_C_STACK(v),
#if defined(IM_FIX_BASE)
		  (void *)IM_FIX_BASE,NULL_OR_ON_C_STACK(IM_FIX_BASE),
		  (void *)(IM_FIX_BASE|IM_FIX_LIM),NULL_OR_ON_C_STACK(IM_FIX_BASE|IM_FIX_LIM),
#endif
		  pagetoinfo(first_data_page),NULL_OR_ON_C_STACK(pagetoinfo(first_data_page)),
		  core_end-1,NULL_OR_ON_C_STACK(core_end-1));
	  error("NULL_OR_ON_C_STACK macro invalid");
	}

 	Cnil->c.c_cdr=Cnil;
 	Cnil->s.s_dbind = Cnil;
 	Cnil->s.s_sfdef = NOT_SPECIAL;
 	Cnil->s.s_fillp = 3;
 	Cnil->s.s_self = "NIL";
 	Cnil->s.s_gfdef = OBJNULL;
 	Cnil->s.s_plist = Cnil;
 	Cnil->s.s_hpack = Cnil;
 	Cnil->s.s_stype = (short)stp_constant;
 	Cnil->s.s_mflag = FALSE;
	Cnil->s.s_hash = ihash_equal1(Cnil,0);
	
	Ct->fw=0;
	set_type_of(Ct,t_symbol);
 	Ct->s.s_dbind = Ct;
	Ct->s.tt=1;
 	Ct->s.s_sfdef = NOT_SPECIAL;
 	Ct->s.s_fillp = 1;
 	Ct->s.s_self = "T";
 	Ct->s.s_gfdef = OBJNULL;
 	Ct->s.s_plist = Cnil;
 	Ct->s.s_hpack = Cnil;
 	Ct->s.s_stype = (short)stp_constant;
 	Ct->s.s_mflag = FALSE;
	Ct->s.s_hash = ihash_equal1(Ct,0);
	
	gcl_init_symbol();

	gcl_init_package();

	Cnil->s.s_hpack = lisp_package;
	import(Cnil, lisp_package);
	export(Cnil, lisp_package);

	Ct->s.s_hpack = lisp_package;
	import(Ct, lisp_package);
	export(Ct, lisp_package);

/* #ifdef ANSI_COMMON_LISP */
/* 	import(Cnil, common_lisp_package); */
/* 	export(Cnil, common_lisp_package); */

/* 	import(Ct, common_lisp_package); */
/* 	export(Ct, common_lisp_package); */
/* #endif */

	sLlambda = make_ordinary("LAMBDA");
	sLlambda_block = make_si_ordinary("LAMBDA-BLOCK");
	sLlambda_closure = make_si_ordinary("LAMBDA-CLOSURE");
	sLlambda_block_closure = make_si_ordinary("LAMBDA-BLOCK-CLOSURE");
	sLspecial = make_ordinary("SPECIAL");

	NewInit();
	init_boot();

	dlopen("libboot",RTLD_LAZY|RTLD_GLOBAL);
	gcl_init_typespec();
	gcl_init_number();
	gcl_init_character();

	gcl_init_read();
	gcl_init_bind();
	gcl_init_pathname();
	gcl_init_print();
	gcl_init_GBC();

	gcl_init_unixfasl();
	gcl_init_unixsys();
	gcl_init_unixsave();

	gcl_init_alloc_function();
	gcl_init_array_function();
	gcl_init_character_function();
	gcl_init_file_function();
	gcl_init_list_function();
	gcl_init_package_function();
	gcl_init_pathname_function();
	gcl_init_predicate_function();
	gcl_init_print_function();
	gcl_init_read_function();
	gcl_init_sequence_function();
#if  defined(KCLOVM) || defined(RUN_PROCESS)
	gcl_init_socket_function();
#endif	
	gcl_init_structure_function();
	gcl_init_string_function();
	gcl_init_symbol_function();
	gcl_init_typespec_function();
	gcl_init_hash();
	gcl_init_cfun();

	gcl_init_unixfsys();
	gcl_init_unixtime();
	gcl_init_eval();
	gcl_init_lex();
	gcl_init_prog();
	gcl_init_catch();
	gcl_init_block();
        gcl_init_macros();
	gcl_init_conditional();
	gcl_init_reference();
	gcl_init_assignment();
	gcl_init_multival();
	gcl_init_error();
	gcl_init_let();
	gcl_init_mapfun();
	gcl_init_iteration();
	gcl_init_toplevel();

	gcl_init_cmpaux();

	init_main();

	gcl_init_format();
	gcl_init_links();

	gcl_init_fat_string();
	gcl_init_sfasl();
#ifdef CMAC
	gcl_init_cmac();
#endif	
#ifdef HAVE_READLINE
	gcl_init_readline();
#endif

}
object
vs_overflow(void)
{
	if (vs_limit > vs_org + stack_multiple *  VSSIZE)
		error("value stack overflow");
	vs_limit += STACK_OVER*VSGETA;
	FEerror("Value stack overflow.", 0);
	return Cnil;
}

void
bds_overflow(void) {
	--bds_top;
	if (bds_limit > bds_org + stack_multiple *  BDSSIZE) {
            error("bind stack overflow");
        }
	bds_limit += STACK_OVER  *BDSGETA;
	FEerror("Bind stack overflow.", 0);
}

void
frs_overflow(void) {
	--frs_top;
	if (frs_limit > frs_org + stack_multiple *  FRSSIZE)
		error("frame stack overflow");
	frs_limit += STACK_OVER* FRSGETA;
	FEerror("Frame stack overflow.", 0);
}

void
ihs_overflow(void) {
	--ihs_top;
	if (ihs_limit > ihs_org + stack_multiple *  IHSSIZE)
		error("invocation history stack overflow");
	ihs_limit += STACK_OVER*IHSGETA;
	FEerror("Invocation history stack overflow.", 0);
}

void
segmentation_catcher(int i, long code, void *scp, char *addr) {
#ifndef _WIN32
  void *faddr;
  faddr=GET_FAULT_ADDR(sig,code,scp,addr); 

  if ((cstack_dir==-1 && faddr < (void *)cs_limit && (void *)cs_limit-faddr <= PAGESIZE) ||
      (cstack_dir==1 && faddr > (void *)cs_limit && faddr-(void *)cs_limit <= PAGESIZE))
    FEerror("Control stack overflow.",0); /*FIXME -- provide getrlimit here.*/
  else 
    printf("Segmentation violation: c stack ok:signalling error");
#endif
  error("Segmentation violation.");
}

DEFUN("BYE",object,fSbye,SI,0,1,NONE,OI,OO,OO,OO,(ufixnum exit_code,...),"") {

  fixnum n=INIT_NARGS(0);
  object l=Cnil,f=(object)(exit_code+1);
  va_list ap;

  va_start(ap,exit_code);
  exit_code=((fixnum)NEXT_ARG(n,ap,l,f,(object)1))-1;
  va_end(ap);

#ifdef UNIX
  exit(exit_code);
#else
  RETURN(1,int,exit_code, 0); 
#endif

}

DEFUN("QUIT",object,fSquit,SI,0,1,NONE,OI,OO,OO,OO,(fixnum exitc),"") {
  return FFN(fSbye)(exitc); 
}
 

static void
FFN(siLargc)(void) {
  check_arg(0);
  vs_push(make_fixnum(ARGC));
}

static void
FFN(siLargv)(void) {
  int i=0;
  
  check_arg(1);
  if (type_of(vs_base[0]) != t_fixnum ||
      (i = fix(vs_base[0])) < 0 ||
      i >= ARGC)
    FEerror("Illegal argument index: ~S.", 1, vs_base[0]);
  vs_base[0] = make_simple_string(ARGV[i]);
}

static void
FFN(siLgetenv)(void) {

  char name[256];
  int i;
  char *value;
  extern char *getenv(const char *);
  
  check_arg(1);
  check_type_string(&vs_base[0]);
  if (vs_base[0]->st.st_fillp >= 256)
    FEerror("Too long name: ~S.", 1, vs_base[0]);
  for (i = 0;  i < vs_base[0]->st.st_fillp;  i++)
    name[i] = vs_base[0]->st.st_self[i];
  name[i] = '\0';
  if ((value = getenv(name)) != NULL) {
#ifdef _WIN32
    fix_filename(NULL, value);
#endif    
    vs_base[0] = make_simple_string(value);
#ifdef FREE_GETENV_RESULT
    free(value);
    
#endif		
    }
  else
    vs_base[0] = Cnil;

}

object *vs_marker;

static void
FFN(siLmark_vs)(void) {
  check_arg(0);
  vs_marker = vs_base;
  vs_base[0] = Cnil;
}

static void
FFN(siLcheck_vs)(void) {
  check_arg(0);
  if (vs_base != vs_marker)
    FEerror("Value stack is flawed.", 0);
  vs_base[0] = Cnil;
}

DEFUN("CATCH-FATAL",object,fScatch_fatal,SI,1,1,NONE,OI,OO,OO,OO,(fixnum i),"") {
  catch_fatal=i;
  return Cnil;
}

LFD(siLreset_stack_limits)(void)
{
  int i=0;

  check_arg(0);
  if(catch_fatal <0) catch_fatal=1;
#ifdef SGC	
  {extern int fault_count ; fault_count = 0;}
#endif 
  if (vs_top < vs_org + stack_multiple *  VSSIZE)
    vs_limit = vs_org + stack_multiple *  VSSIZE;
  else
    error("can't reset vs_limit");
  if (bds_top < bds_org + stack_multiple *  BDSSIZE)
    bds_limit = bds_org + stack_multiple *  BDSSIZE;
  else
    error("can't reset bds_limit");
  if (frs_top < frs_org + stack_multiple *  FRSSIZE)
    frs_limit = frs_org + stack_multiple *  FRSSIZE;
  else
    error("can't reset frs_limit");
  if (ihs_top < ihs_org + stack_multiple *  IHSSIZE)
    ihs_limit = ihs_org + stack_multiple *  IHSSIZE;
  else
    error("can't reset ihs_limit");
  if (cs_base==cs_org)
    cs_org=(void *)&i;
#ifdef __ia64__
 {
   extern void * GC_save_regs_in_stack();
   if (cs_base2==cs_org2)
     cs_org2=GC_save_regs_in_stack();
 }
#endif
  /* reset_cstack_limit(i); */
  vs_base[0] = Cnil;
}

#define COPYSTACK(org,p,typ,lim,top,geta,size) \
  {unsigned long topl=top-org;\
   bcopy(org,p,(lim-org)*sizeof(typ));\
   org=p;\
   top=org+topl;\
   lim=org+stack_multiple*size;\
   p=lim+(STACK_OVER+1)*geta;\
   }

static int
multiply_stacks(int m) {  
  void *p;
  int vs,bd,frs,ihs;
  stack_multiple=stack_multiple*m;
#define ELTSIZE(x) (((char *)((x)+1)) - ((char *) x))
  vs  = (stack_multiple*VSSIZE  + (STACK_OVER+1)*VSGETA)* ELTSIZE(vs_org);
  bd  = (stack_multiple*BDSSIZE + (STACK_OVER+1)*BDSGETA)*ELTSIZE(bds_org);
  frs = (stack_multiple*FRSSIZE + (STACK_OVER+1)*FRSGETA)*ELTSIZE(frs_org);
  ihs = (stack_multiple*IHSSIZE + (STACK_OVER+1)*IHSGETA)*ELTSIZE(ihs_org);
  if (stack_space==0) {enter_mark_origin(&stack_space);}
  stack_space = alloc_simple_string(vs+bd+frs+ihs);
  array_allocself(stack_space,1,code_char(0));
  p=stack_space->st.st_self;
  COPYSTACK(vs_org,p,object,vs_limit,vs_top,VSGETA,VSSIZE);
  COPYSTACK(bds_org,p,struct bds_bd,bds_limit,bds_top,BDSGETA,BDSSIZE);
  COPYSTACK(frs_org,p,struct frame,frs_limit,frs_top,FRSGETA,FRSSIZE);
  COPYSTACK(ihs_org,p,struct invocation_history,ihs_limit,ihs_top,
	    IHSGETA,IHSSIZE);
  vs_base=vs_top;
  return stack_multiple;
}

DEFVAR("*NO-INIT*",sSAno_initA,SI,Cnil,"");

LFD(siLinit_system)(void) {
  check_arg(0);
  gcl_init_system(sSAno_initA);
  vs_base[0] = Cnil;
  vs_top=vs_base+1;
}

static void
FFN(siLuser_init)(void) {
  check_arg(0);
  sLApackageA->s.s_dbind = user_package;
  user_init();
  vs_base[0] = Cnil;
}

static void
FFN(siLaddress)(void) {
  check_arg(1);
  vs_base[0] = make_fixnum((long)vs_base[0]);
}

static void
FFN(siLnani)(void) {
  check_arg(1);
  switch(type_of(vs_base[0])) {
  case t_fixnum:
    vs_base[0] = (object)fixint(vs_base[0]);
    break;
  case t_bignum:
    if (mpz_fits_slong_p(MP(vs_base[0]))) {
      MP_INT *u = MP(vs_base[0]);
      vs_base[0]=(object)mpz_get_si(u);
      break;
    }
  default:
    FEerror("Cannot coerce ~s to an address",1,vs_base[0]);
  }

}

static void
FFN(siLinitialization_failure)(void) {
  check_arg(0);
  printf("lisp initialization failed\n");
  exit(0);
}

DEFUN("IDENTITY",object,fLidentity,LISP,1,1,NONE,OO,OO,OO,OO,(object x0),"") {
  RETURN1 (x0);
}

DEFUN("GCL-COMPILE-TIME",object,fSgcl_compile_time,SI,0,0,NONE,OO,OO,OO,OO,(void),"") {
  RETURN1 (make_simple_string(__DATE__ " " __TIME__));
}

DEFUN("LISP-IMPLEMENTATION-VERSION",object,fLlisp_implementation_version,LISP,0,0,NONE,OO,OO,OO,OO,(void),"") {
  RETURN1((make_simple_string(LISP_IMPLEMENTATION_VERSION)));
}


static void
FFN(siLsave_system)(void) {
  
#ifdef HAVE_YP_UNBIND
  extern object truename(),namestring();
  check_arg(1);
  /* prevent subsequent consultation of yp by getting
     truename now*/
  vs_base[0]=namestring(truename(vs_base[0]));
  {char name[200];
  char *dom = name;
  if (0== getdomainname(dom,sizeof(name)))
    yp_unbind(dom);}
#endif
  
#ifdef DO_BEFORE_SAVE
  DO_BEFORE_SAVE
#endif	
    
  saving_system = TRUE;

  minimize_image();

  Lsave();
  saving_system = FALSE;
  alloc_page(-(holepage+nrbpage));

}

DEFVAR("*LISP-MAXPAGES*",sSAlisp_maxpagesA,SI,make_fixnum(real_maxpage),"");
DEFVAR("*SYSTEM-DIRECTORY*",sSAsystem_directoryA,SI,make_simple_string(system_directory),"");
DEFVAR("*MULTIPLY-STACKS*",sSAmultiply_stacksA,SI,Cnil,"");
DEF_ORDINARY("TOP-LEVEL",sStop_level,SI,"");
DEFVAR("*COMMAND-ARGS*",sSAcommand_argsA,SI,sLnil,"");

static void
init_main(void) {

  /* make_function("BY", Lby); */
  make_si_function("ARGC", siLargc);
  make_si_function("ARGV", siLargv);
  
  make_si_function("GETENV", siLgetenv);
  
  make_si_function("MARK-VS", siLmark_vs);
  make_si_function("CHECK-VS", siLcheck_vs);
  make_si_function("RESET-STACK-LIMITS", siLreset_stack_limits);
  make_si_function("INIT-SYSTEM", siLinit_system);
  make_si_function("USER-INIT", siLuser_init);
  make_si_function("ADDRESS", siLaddress);
  make_si_function("NANI", siLnani);
  make_si_function("INITIALIZATION-FAILURE",
		   siLinitialization_failure);
  
  siClisp_pagesize =
    make_si_constant("LISP-PAGESIZE", make_fixnum(PAGESIZE));
  
  
  {object features;
  
#define ADD_FEATURE(name) \
	 features=  make_cons(make_keyword(name),features)
  
  features=    make_cons(make_keyword("COMMON"),
			 make_cons(make_keyword("KCL"), Cnil));
  ADD_FEATURE("AKCL");
  ADD_FEATURE("GCL");
#ifdef BROKEN_O4_OPT
  ADD_FEATURE("BROKEN_O4_OPT");
#endif
#ifdef GMP
  ADD_FEATURE("GMP");
#endif	 
#ifdef GCL_GPROF
  ADD_FEATURE("GPROF");
#endif	 
  
#if defined ( UNIX ) && !defined ( _WIN32 )
  ADD_FEATURE("UNIX");
#endif

#if defined ( _WIN32 ) 
  ADD_FEATURE("WINNT");
  ADD_FEATURE("WIN32");
#endif

#ifdef IEEEFLOAT
  ADD_FEATURE("IEEE-FLOATING-POINT");
#endif
#ifdef SGC
  ADD_FEATURE("SGC");
#endif	 

  ADD_FEATURE(HOST_CPU);
  ADD_FEATURE(HOST_KERNEL);
#ifdef HOST_SYSTEM
  ADD_FEATURE(HOST_SYSTEM);
#endif
#ifdef  BSD
  ADD_FEATURE("BSD");
#endif
  
#if !defined(DOUBLE_BIGENDIAN)
  ADD_FEATURE("CLX-LITTLE-ENDIAN");
#endif
  
#ifndef PECULIAR_MACHINE
#define BIGM    (int)((((unsigned int)(-1))/2))	 
  { 
/*      int ONEM = -1; */
    int Bigm  = BIGM;
    int Smallm = -BIGM-1;
    int Seven = 7;
    int Three = 3;
    if ( (Smallm / Seven)  < 0
	 && (Smallm / (-Seven))  > 0
	 && (Bigm / (-Seven)) < 0 
	 && ((-Seven) / Three) == -2
	 && (Seven / (-Three)) == -2
	 && ((-Seven)/ (-Three)) == 2)
      { ADD_FEATURE("TRUNCATE_USE_C");
      }  }
#endif /* PECULIAR_MACHINE */
  
#ifdef HAVE_READLINE
  ADD_FEATURE("READLINE");
#endif
#if !defined(USE_DLOPEN)
  ADD_FEATURE("NATIVE-RELOC");
#if defined(HAVE_LIBBFD) 
  ADD_FEATURE("BFD");
#endif
#endif
  ADD_FEATURE("UNEXEC");
#ifdef HAVE_XGCL
  ADD_FEATURE("XGCL");
#endif

#ifdef HAVE_GNU_LD
  ADD_FEATURE("GNU-LD");
#endif
  
#ifndef NO_C99
  ADD_FEATURE("C99");
#endif	 

#ifdef STATIC_LINKING
  ADD_FEATURE("STATIC");
#endif	 

#if SIZEOF_LONG==8
  ADD_FEATURE("64BIT");
#endif	 
  ADD_FEATURE("INTDIV");
  ADD_FEATURE("DYNAMIC-EXTENT");

  make_special("*FEATURES*",features);}
  
  make_si_function("SAVE-SYSTEM", siLsave_system);
/*   make_si_sfun("CATCH-FATAL",siLcatch_fatal,ARGTYPE1(f_fixnum)); */
  /* make_si_function("WARN-VERSION",Lidentity); */
  
}

#ifdef SGC
#include "writable.h"
#endif

/* #include \"page.h\" */

typedef struct {
  enum type tt;
  struct typemanager *tp;
} Tbl;

#define Tblof(a_)       {(a_),tm_of(a_)}
#define tblookup(a_,b_) ({Tbl *tb=tb1;(b_)=(a_);for (;tb->tt && tb->b_!=(b_);tb++);tb->tt;})
#define mtm_of(a_)      (a_)>=t_other ? NULL : tm_of(a_)

DEFUN("FUNCTION-BY-ADDRESS",object,fSfunction_by_address,SI,1,1,NONE,OI,OO,OO,OO,(fixnum ad),"") {

  ufixnum m=-1,mm,j;
  void *o;
  object x,xx=Cnil;
  Tbl tb1[]={Tblof(t_function),{0}};
  struct typemanager *tp;
  enum type tt;
  struct pageinfo *v;

  if (VALID_DATA_ADDRESS_P(ad))
    for (v=cell_list_head;v;v=v->next)
      if (tblookup(mtm_of(v->type),tp))
	for (o=pagetochar(page(v)),j=tp->tm_nppage;j--;o+=tp->tm_size)
	  if (tblookup(type_of((x=o)),tt))
	    if (!is_free(x) && (mm=ad-(ufixnum)x->fun.fun_self)<m) {
	      m=mm;
	      xx=x;
	    }
  
  return xx;

}


#ifdef PRINT_INSN

#define MC(b_) v.uc_mcontext.b_
#define REG_LIST(a_,b_) list(3,make_fixnum((void *)&(a_)-(void *)(b_)),make_fixnum(sizeof(a_)),make_fixnum(sizeof(*a_)))
#define MCF(b_) (((struct _fpstate *)MC(fpregs))->b_)

DEFCONST("+MC-CONTEXT-OFFSETS+",sSPmc_context_offsetsP,SI,
	 ({ucontext_t v;list(4,REG_LIST(MC(gregs),&v),REG_LIST(MC(fpregs),&v),
			     REG_LIST(MCF(_st),MC(fpregs)),REG_LIST(MCF(_xmm),MC(fpregs)));}),"");

#define ASM __asm__ __volatile__

DEFUN("FNSTSW",fixnum,fSfnstsw,SI,0,0,NONE,IO,OO,OO,OO,(void),"") {
  unsigned short t;
  ASM ("fnstsw %0" : "=m" (t));
  RETURN1(t);
}

DEFUN("FNSTCW",fixnum,fSfnstcw,SI,0,0,NONE,IO,OO,OO,OO,(void),"") {
  unsigned short t;
  ASM ("fnstcw %0" : "=m" (t));
  RETURN1(t);
}
DEFUN("STMXCSR",fixnum,fSstmxcsr,SI,0,0,NONE,IO,OO,OO,OO,(void),"") {
  unsigned int t;
  ASM ("stmxcsr %0" : "=m" (t));
  RETURN1(t);
}
DEFUN("CPUID",fixnum,fScpuid,SI,0,0,NONE,IO,OO,OO,OO,(void),"") {
  unsigned int t;
  ASM ("movl $1,%%eax;cpuid;movl %%edx,%0":"=m"(t)::"eax","ebx","ecx","edx");
  RETURN1(t);
}
DEFUN("FNCLEX",object,fSfnclex,SI,0,0,NONE,OO,OO,OO,OO,(void),"") {
  ASM ("fnclex");
  RETURN1(Cnil);
}
DEFUN("LDMXCSR",fixnum,fSldmxcsr,SI,1,1,NONE,II,OO,OO,OO,(fixnum val),"") {
  ASM ("ldmxcsr %0" :: "m" (val));
  RETURN1(val);
}
DEFUN("FLDCW",fixnum,fSfldcw,SI,1,1,NONE,II,OO,OO,OO,(fixnum val),"") {
  ASM ("fldcw %0" :: "m" (val));
  RETURN1(val);
}
DEFUN("FLD",object,fSfld,SI,1,1,NONE,OI,OO,OO,OO,(fixnum val),"") {
  double d;
  ASM ("fldt %1;fstpl %0" : "=m" (d): "m" (*(char *)val));
  RETURN1(make_longfloat(d));
}

DEFCONST("+FE-LIST+",sSPfe_listP,SI,list(5,
					 list(3,sLdivision_by_zero,make_fixnum(FPE_FLTDIV),make_fixnum(FE_DIVBYZERO)),
					 list(3,sLfloating_point_overflow,make_fixnum(FPE_FLTOVF),make_fixnum(FE_OVERFLOW)),
					 list(3,sLfloating_point_underflow,make_fixnum(FPE_FLTUND),make_fixnum(FE_UNDERFLOW)),
					 list(3,sLfloating_point_inexact,make_fixnum(FPE_FLTRES),make_fixnum(FE_INEXACT)),
					 list(3,sLfloating_point_invalid_operation,make_fixnum(FPE_FLTINV),make_fixnum(FE_INVALID))),"");

static object disassemble_instruction(fixnum);

DEFUN("DISASSEMBLE-INSTRUCTION",object,fSdisassemble_instruction,SI,1,1,NONE,OI,OO,OO,OO,(fixnum addr),"") {
  return disassemble_instruction(addr);
}

#include "dis-asm.h"

static char b[4096],*bp;

static int
my_fprintf(void *v,const char *f,...) {
  va_list va;
  int r;
  va_start(va,f);
  bp+=(r=vsnprintf(bp,sizeof(b)-(bp-b),f,va));
  va_end(va);
  return r;
}

static int
my_read(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *dinfo) {
  memcpy(myaddr,(void *)memaddr,length);
  return 0;
}

static void
my_pa(bfd_vma addr,struct disassemble_info *dinfo) {
  dinfo->fprintf_func(dinfo->stream,"%p",(void *)addr);
}

static object
disassemble_instruction(fixnum addr) {

  static disassemble_info i;
  /* static int k; */
  int j;

  /* if (!k) {init_disassemble_info(&i,NULL,my_fprintf);k=1;} */
  memset(&i,0,sizeof(i));
  i.fprintf_func=my_fprintf;
  i.read_memory_func=my_read;
  i.print_address_func=my_pa;
  bp=b;
  
  j=PRINT_INSN(addr,&i);
  my_fprintf(NULL," ;");
  return MMcons(make_simple_string(b),make_fixnum(j));
}


#endif
