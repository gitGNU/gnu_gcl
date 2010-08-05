/*
 Copyright (C) 1994 W. Schelter

This file is part of GNU Common Lisp, herein referred to as GCL

GCL is free software; you can redistribute it and/or modify it under
the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCL is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
License for more details.

*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <elf.h>

#include "gclincl.h"


#if SIZEOF_LONG == 4
#define Elfw 32
#else
#define Elfw 64
#endif

#define Elf  Mjoin(Elf,Elfw)
#define ELF  Mjoin(ELF,Elfw)
#define Ehdr Mjoin(Elf,_Ehdr)
#define Shdr Mjoin(Elf,_Shdr)
#define Sym  Mjoin(Elf,_Sym)
#define Rel  Mjoin(Elf,_Rel)
#define Rela Mjoin(Elf,_Rela)
#define Word Elf32_Word

#define   ELF_R_SYM(a)  Mjoin(ELF,_R_SYM)(a)
#define  ELF_R_TYPE(a)  Mjoin(ELF,_R_TYPE)(a)
#define ELF_ST_BIND(a)  Mjoin(ELF,_ST_BIND)(a)


#define ulmax(a_,b_) ({ul _a=a_,_b=b_;_a<_b ? _b : _a;})
#define ALLOC_SEC(sec) (sec->sh_flags&SHF_ALLOC && (sec->sh_type==SHT_PROGBITS || sec->sh_type==SHT_NOBITS))
#define  LOAD_SEC(sec) (sec->sh_flags&SHF_ALLOC &&  sec->sh_type==SHT_PROGBITS)
#define  LOAD_SYM(sym) ({ul _b=ELF_ST_BIND(sym->st_info);\
      sym->st_value && (_b==STB_GLOBAL || _b==STB_WEAK);})

#ifndef GOT_RELOC
#define GOT_RELOC(a) 0
#endif

#define MASK(n) (~(~0L << (n)))



typedef unsigned long ul;



#ifdef STATIC_RELOC_VARS
STATIC_RELOC_VARS
#endif



static int
ovchk(ul v,ul m) {

  m|=m>>1;
  v&=m;

  return (!v || v==m);

}

static int
store_val(ul *w,ul m,ul v) {

  massert(ovchk(v,~m));
  *w=(v&m)|(*w&~m);

  return 0;

}

static int
add_val(ul *w,ul m,ul v) {

  return store_val(w,m,v+(*w&m));

}

static int
add_vals(ul *w,ul m,ul v) {

  ul l=*w&m,mm;
  
  mm=~m;
  mm|=mm>>1;
  if (l&mm) l|=mm;

  return store_val(w,m,v+l);

}

static int 
or_val(ul *w,ul m,ul v) { 
  
  return store_val(w,m,v|(*w&m)); 
  
} 


static void
relocate(Sym *sym1,void *v,ul a,ul start,ul *got,ul *gote) {
  
  Rel *r=v;
  ul *where,p,s,tp;
  
  where=(void *)start+r->r_offset;
  p=(ul)where;

  s=sym1[ELF_R_SYM(r->r_info)].st_value;
  
  switch((tp=ELF_R_TYPE(r->r_info))) {

#include RELOC_H

  default:
    massert(tp&~tp);

  }

}

static Shdr *
get_section(char *s,Shdr *sec,Shdr *sece,const char *sn) {

  for (;sec<sece && strcmp(sn+sec->sh_name,s);sec++);
  return sec<sece ? sec : 0;

}

static int
find_init_address(Sym *sym,Sym *syme,Shdr *sec1,Shdr *sece,
		  const char *sn,const char *st1,ul *init) {

  Shdr *sec;

  for (;sym<syme;sym++) {
    
    sec=sec1+sym->st_shndx;

    if (sec<sec1 || sec>=sece)
      continue;

    if (strcmp(sn+sec->sh_name,".text"))
      continue;

    if (memcmp("init_",st1+sym->st_name,4))
      continue;

    *init=sym->st_value;

    return 0;

  }

  return -1;

}

static int
relocate_symbols(Sym *sym,Sym *syme,Shdr *sec1,Shdr *sece,const char *st1) {

  Shdr *sec;
  struct node *a;

  for (;sym<syme;sym++) {
    
    sec=sec1+sym->st_shndx;

    if (sec<sece && ALLOC_SEC(sec))
      sym->st_value+=sec->sh_addr; 

    else if ((a=find_sym_ptable(st1+sym->st_name)))
      sym->st_value=a->address;

  }

  return 0;
  
}

static object
load_memory(Shdr *sec1,Shdr *sece,void *v1,ul **got,ul **gote) {

  object memory;
  Shdr *sec;
  ul gsz,sz,a,ma;

  BEGIN_NO_INTERRUPT;

  for (sec=sec1,ma=sz=0;sec<sece;sec++)
    if (ALLOC_SEC(sec)) {

      a=sec->sh_addralign;
      ma=ma ? ma : a;
      sz=(sz+a-1)&~(a-1);
      sec->sh_addr=sz;
      sz+=sec->sh_size;

    }

  ma=ma>sizeof(struct contblock) ?  ma-1 : 0;
  sz+=ma;

  gsz=0;
  if (**got) {
    gsz=(**got+1)*sizeof(**got)-1;
    sz+=gsz;
  }

  memory=alloc_object(t_cfdata);
  memory->cfd.cfd_size=sz;
  memory->cfd.cfd_self=0;
  memory->cfd.cfd_start=0;
  memory->cfd.cfd_start=alloc_contblock(sz);

  a=(ul)memory->cfd.cfd_start;
  a=(a+ma)&~ma;
  for (sec=sec1;sec<sece;sec++) 
    if (ALLOC_SEC(sec)) {
      sec->sh_addr+=a;
      if (LOAD_SEC(sec))
	memcpy((void *)sec->sh_addr,v1+sec->sh_offset,sec->sh_size);
    }
  
  if (**got) {
    sz=**got;
    *got=(void *)memory->cfd.cfd_start+memory->cfd.cfd_size-gsz;
    gsz=sizeof(**got)-1;
    *got=(void *)(((ul)*got+gsz)&~gsz);
    *gote=*got+sz;
  }

  END_NO_INTERRUPT;

  return memory;

}

 
static int
relocate_code(void *v1,Shdr *sec1,Shdr *sece,Sym *sym1,ul *got,ul *gote) {

  Shdr *jsec,*sec;
  void *v,*ve;
  Rela *ra;

  for (sec=sec1;sec<sece;sec++) {

    jsec=sec1+sec->sh_info;
    
    if (jsec<sec1 || jsec>=sece) 
      continue;
    if (!ALLOC_SEC(jsec)) 
      continue;
    
    if (sec->sh_type!=SHT_REL && sec->sh_type!=SHT_RELA)
      continue;
    
    for (v=v1+sec->sh_offset,ve=v+sec->sh_size,ra=v;v<ve;v+=sec->sh_entsize,ra=v)
      relocate(sym1,ra,sec->sh_type==SHT_RELA ? ra->r_addend : 0,jsec->sh_addr,got,gote);

  }
  
  return 0;

}

static int
label_got_symbols(void *v1,Shdr *sec1,Shdr *sece,Sym *sym1,Sym *syme,ul *gs) {

  Rel *r;
  Sym *sym;
  Shdr *sec;
  void *v,*ve;

  for (sym=sym1;sym<syme;sym++)
    sym->st_size=0;

  for (*gs=0,sec=sec1;sec<sece;sec++)
    if (sec->sh_type==SHT_REL || sec->sh_type==SHT_RELA)
      for (v=v1+sec->sh_offset,ve=v+sec->sh_size,r=v;v<ve;v+=sec->sh_entsize,r=v)
	if (GOT_RELOC(r)) {
	  sym=sym1+ELF_R_SYM(r->r_info);
	  if (!sym->st_size)
	    sym->st_size=++*gs;
	}
  
  return 0;
  
}

static int
parse_map(void *v1,Shdr **sec1,Shdr **sece,
	  char **sn,Sym **sym1,Sym **syme,char **st1,ul *end,
	  Sym **dsym1,Sym **dsyme,char **dst1) {

  Ehdr *fhp;
  Shdr *sec;

  fhp=v1;
  *sec1=v1+fhp->e_shoff;
  *sece=*sec1+fhp->e_shnum;

  *sn=v1+(*sec1)[fhp->e_shstrndx].sh_offset;

  massert(sec=get_section(".symtab",*sec1,*sece,*sn));
  *sym1=v1+sec->sh_offset;
  *syme=*sym1+sec->sh_size/sec->sh_entsize;

  massert(sec=get_section(".strtab",*sec1,*sece,*sn));
  *st1=v1+sec->sh_offset;

  *dsym1=*dsyme=NULL;
  *dst1=NULL;
  if ((sec=get_section(".dynsym",*sec1,*sece,*sn))) {
    *dsym1=v1+sec->sh_offset;
    *dsyme=*dsym1+sec->sh_size/sec->sh_entsize;
    massert(sec=get_section(".dynstr",*sec1,*sece,*sn));
    *dst1=v1+sec->sh_offset;
  }

  for (*end=0,sec=*sec1;sec<*sece;sec++) 
    *end=ulmax(*end,sec->sh_offset+sec->sh_size);

  return 0;

}

static int
set_symbol_stubs(void *v,Shdr *sec1,Shdr *sece,const char *sn,Sym *ds1,const char *st1) {

  Shdr *sec,*psec;
  Rel *r;
  ul np,ps,p;
  void *ve;

  massert(psec=get_section(".plt",sec1,sece,sn));

  massert((sec=get_section( ".rel.plt",sec1,sece,sn)) ||
	  (sec=get_section(".rela.plt",sec1,sece,sn)));

  np=sec->sh_size/sec->sh_entsize;
  ps=psec->sh_size/np;

  v+=sec->sh_offset;
  ve=v+np*sec->sh_entsize;

  p=psec->sh_addr+psec->sh_size%np;

  for (r=v;v<ve;v+=sec->sh_entsize,p+=ps,r=v) 
    if (!ds1[ELF_R_SYM(r->r_info)].st_value)
      ds1[ELF_R_SYM(r->r_info)].st_value=p;

  return 0;

}

static int
calc_space(ul *ns,ul *sl,Sym *sym1,Sym *syme,const char *st1) {

  Sym *sym;

  for (sym=sym1;sym<syme;sym++) {
    
    if (!LOAD_SYM(sym))
      continue;

    (*ns)++;
    (*sl)+=strlen(st1+sym->st_name)+1;

  }

  return 0;

}

static int
load_ptable(struct node **a,char **s,Sym *sym1,Sym *syme,const char *st1) {

  Sym *sym;

  for (sym=sym1;sym<syme;sym++) {

    if (!LOAD_SYM(sym))
      continue;

    (*a)->address=sym->st_value;
    (*a)->string=(*s);
    strcpy((*s),st1+sym->st_name);

    (*a)++;
    (*s)+=strlen(*s)+1;

  }

  return 0;

}

static int 
load_self_symbols() {

  FILE *f;
  char *sn,*st1,*s,*dst1;
  Shdr *sec1,*sece;
  Sym *sym1,*syme,*dsym1,*dsyme;
  void *v1,*ve;
  ul ns,sl,end;
  struct node *a;

  massert(f=fopen(kcl_self,"r"));
  massert(v1=get_mmap(f,&ve));

  massert(!parse_map(v1,&sec1,&sece,&sn,&sym1,&syme,&st1,&end,&dsym1,&dsyme,&dst1));

#ifndef STATIC_LINKING
  massert(!set_symbol_stubs(v1,sec1,sece,sn,dsym1,dst1));
#endif

  ns=sl=0;
  massert(!calc_space(&ns,&sl,sym1,syme,st1));
  massert(!calc_space(&ns,&sl,dsym1,dsyme,dst1));

  c_table.alloc_length=c_table.length=ns;
  massert(c_table.ptable=malloc(sizeof(*c_table.ptable)*c_table.alloc_length));
  massert(s=malloc(sl));

  a=c_table.ptable;
  massert(!load_ptable(&a,&s,sym1,syme,st1));
  massert(!load_ptable(&a,&s,dsym1,dsyme,dst1));
  
  qsort(c_table.ptable,c_table.length,sizeof(*c_table.ptable),node_compare);

  massert(!un_mmap(v1,ve));
  massert(!fclose(f));

  return 0;

}

int
seek_to_end_ofile(FILE *fp) {

  void *v1,*ve;
  Shdr *sec1,*sece;
  Sym  *sym1,*syme,*dsym1,*dsyme;
  char *sn,*st1,*dst1;
  ul end;
  
  massert(v1=get_mmap(fp,&ve));

  massert(!parse_map(v1,&sec1,&sece,&sn,&sym1,&syme,&st1,&end,&dsym1,&dsyme,&dst1));

  massert(!fseek(fp,end,SEEK_SET));

  massert(!un_mmap(v1,ve));

  return 0;

}

int
fasload(object faslfile) {

  FILE *fp;
  char filename[256],*sn,*st1,*dst1;
  ul init_address=0,end,gs,*got=&gs,*gote;
  object memory,data;
  Shdr *sec1,*sece;
  Sym *sym1,*syme,*dsym1,*dsyme;
  void *v1,*ve;

  coerce_to_filename(faslfile, filename);
  faslfile = open_stream(faslfile, smm_input, Cnil, sKerror);
  fp = faslfile->sm.sm_fp;
  
  massert(v1=get_mmap(fp,&ve));

  massert(!parse_map(v1,&sec1,&sece,&sn,&sym1,&syme,&st1,&end,&dsym1,&dsyme,&dst1));
  
  massert(!label_got_symbols(v1,sec1,sece,sym1,syme,got));

  massert(memory=load_memory(sec1,sece,v1,&got,&gote));

  massert(!relocate_symbols(sym1,syme,sec1,sece,st1));
  
  massert(!find_init_address(sym1,syme,sec1,sece,sn,st1,&init_address));
  
  massert(!relocate_code(v1,sec1,sece,sym1,got,gote));
  
  massert(!fseek(fp,end,SEEK_SET));
  data=feof(fp) ? 0 : read_fasl_vector(faslfile);
  
  massert(!un_mmap(v1,ve));
  close_stream(faslfile);
  
#ifdef CLEAR_CACHE
  CLEAR_CACHE;
#endif
  
  init_address-=(ul)memory->cfd.cfd_start;
  call_init(init_address,memory,data,0);
  
  if(symbol_value(sLAload_verboseA)!=Cnil)
    printf("start address -T %p ",memory->cfd.cfd_start);
  
  return(memory->cfd.cfd_size);
  
}

#include "sfasli.c"
