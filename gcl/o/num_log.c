
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
	Logical operations on number
*/
#define NEED_MP_H
#define EXPORT_GMP
#include <stdlib.h>
#include "include.h"
#include "num_include.h"

/*FIXME 64*/
#define int fixnum
   
#ifdef GMP
#include "gmp_num_log.c"
#else
#include "pari_num_log.c"
#endif


static int
ior_op(int i, int j)
{
	return(i | j);
}

static int
xor_op(int i, int j)
{
	return(i ^ j);
}


/* #define IF1(a_) BF(1,a_,f,b) */

/* IF1(bitcount) */
/* IF1(popcount) */
/* IF1(bitlength) */
/* BF(2,sizeinbase,f,b,f) */
/* IF1(get_si) */
/* IF1(get_ui) */
/* IF1(sgn) */

/* BF(1,fac_ui,b,f) */
/* BF(1,fib_ui,b,f) */

/* BF(3,powm,b,b,b,b) */
/* BF(3,powm_ui,b,b,f,b) */
/* BF(2,tdiv_qr,m,b,b) */

/* #define BF1(a_) BF(1,a_,b,b) */

/* BF1(com) */
/* BF1(sqrt) */
/* BF1(neg) */

/* BF(2,cmp,f,b,b) */

/* #define BF2(a_) BF(2,a_,b,b,b) */

/* BF2(invert) */
/* BF2(remove) */
/* BF2(add) */
/* BF2(mul) */
/* BF2(sub) */
/* BF2(and) */
/* BF2(ior) */
/* BF2(xor) */
/* BF2(gcd) */
/* BF2(lcm) */
/* BF2(divexact) */

/* BF(2,tstbit,f,b,f) */
/* BF(2,jacobi,f,b,b) */

/* #define BF2I(a_) BF(2,a_,b,b,f) */

/* BF2I(root) */
/* BF2I(divexact_ui) */
/* BF2I(gcd_ui) */
/* BF2I(bin_ui) */
/* BF2I(lcm_ui) */
/* BF2I(sub_ui) */
/* BF2I(add_ui) */
/* BF2I(mul_ui) */
/* BF2I(mul_si) */
/* BF2I(mul_2exp) */
/* BF2I(fdiv_q_2exp) */


#define BI(n_)\
  DEFUN(#n_,fixnum,Join(fS,n_),SI,1,1,NONE,II,OO,OO,OO,(fixnum x),"") {\
\
  RETURN1(Join(__builtin_,n_)(x));\
\
}

BI(clzl)
BI(ctzl)
BI(ffsl)
BI(parityl)
BI(popcountl)


DEFUN("SHFT",object,fSshft,SI,2,2,NONE,OO,IO,OO,OO,(object x,fixnum y),"") {

  object u=new_bignum();

  ENSURE_MP(x,1);
  shifti(MP(u),MP(x),y);
  RETURN1(normalize_big(u));

}


DEFUN("LOGCB1",object,fSlogcb1,SI,1,1,NONE,OO,OO,OO,OO,(object x),"") {

  object u=new_bignum();

  ENSURE_MP(x,1);
  mpz_com(MP(u),MP(x));
  RETURN1(normalize_big(u));

}

#define B2OP(n_,b_)						\
DEFUN(#n_ "B2",object,Join(Join(fSlog,n_),b2),SI,3,3,NONE,OO,OO,OO,OO,(object x,object y,object z),"") { \
\
  object u=new_bignum();\
\
  ENSURE_MP(x,1);\
  ENSURE_MP(y,2);\
  Join(mpz_,b_)(MP(u),MP(x),MP(y));\
  if (z!=Cnil) mpz_com(MP(u),MP(u));\
  RETURN1(normalize_big(u));\
\
}

B2OP(AND,and)
B2OP(IOR,ior)
B2OP(XOR,xor)



static int
and_op(int i, int j)
{
	return(i & j);
}

static int
eqv_op(int i, int j)
{
	return(~(i ^ j));
}

static int
nand_op(int i, int j)
{
	return(~(i & j));
}

static int
nor_op(int i, int j)
{
	return(~(i | j));
}

static int
andc1_op(int i, int j)
{
	return((~i) & j);
}

static int
andc2_op(int i, int j)
{
	return(i & (~j));
}

static int
orc1_op(int i, int j)
{
	return((~i) | j);
}

static int
orc2_op(int i, int j)
{
	return(i | (~j));
}

static int
b_clr_op(int i, int j)
{
	return(0);
}

static int
b_set_op(int i, int j)
{
	return(-1);
}

static int
b_1_op(int i, int j)
{
	return(i);
}

static int
b_2_op(int i, int j)
{
	return(j);
}

static int
b_c1_op(int i, int j)
{
	return(~i);
}

static int
b_c2_op(int i, int j)
{
	return(~j);
}

#ifdef NEVER
int (*intLogOps)()[16]= {
  b_clr_op,  /* 0 */
  b_and_op,  /* 01 */
  b_andc2_op,  /* 02 */
  b_1_op,  /* 03 */
  b_andc1_op,  /* 04 */
  b_2_op,  /* 05 */
  b_xor_op,  /* 06 */
  b_ior_op,  /* 07 */
  b_nor_op,  /* 010 */
  b_eqv_op,  /* 011 */
  b_c2_op,  /* 012 */
  b_orc2_op,  /* 013 */
  b_c1_op,  /* 014 */
  b_orc1_op,  /* 015 */
  b_nand_op,  /* 016 */
  b_set_op,  /* 017 */
}
#endif


static int
fix_bitp(object x, int p)
{
	if (p > LOG_WORD_SIZE-2) {		/* fix = sign + bit0-30 */
		if (fix(x) < 0)
			return(1);
		else
			return(0);
	}
	return((fix(x) >> p) & 1);
}	

static int
count_int_bits(int x)
{
	int	i, count;

	count = 0;
	for (i=0; i <= (LOG_WORD_SIZE-1); i++) count += ((x >> i) & 1);
	return(count);
}

static int
count_bits(object x)
{
	int i, count=0;

	if (type_of(x) == t_fixnum) {
		i = fix(x);
		if (i < 0) i = ~i;
		count = count_int_bits(i);
	} else if (type_of(x) == t_bignum)
	  {
	    count= MP_BITCOUNT(MP(x));
	  }
	else 
		FEwrong_type_argument(sLinteger, x);
	return(count);
}

/*
	double_shift(h, l, w, hp, lp) shifts the int h & l ( 31 bits)
	w bits to left ( w > 0) or to right ( w < 0).
	result is returned in *hp and *lp.
*/



object
shift_integer(object x, fixnum w) { 
  if (type_of(x) == t_fixnum) { 
    if (w <= 0){   
      w = -w;
      if (w >= LOG_WORD_SIZE || w<0 /*most-negative-fixnum*/) 
	return small_fixnum(fix(x) < 0 ? -1 :0);
      else
	return make_fixnum (fix(x) >> (w));
    }
    MPOP(return, shifti,SI_TO_MP(fix(x),big_fixnum1),w);
  }
  else
    if (type_of(x) == t_bignum) {
      MPOP(return,shifti,MP(x),w);
    }
  FEwrong_type_argument(sLinteger, x);
  return(Cnil);
}
	

static fixnum
int_bit_length(fixnum i) {
  fixnum j;
  for (j=LOG_WORD_SIZE-1;j>=0 && !((i>>j)&1);j--);
  return j+1;
}

	
/* 	count = 0; */
/* 	for (j = 0; j <= (LOG_WORD_SIZE-1) ; j++) */
/* 		if (((i >> j) & 1) == 1) count = j + 1; */
/* 	return(count); */
/* } */



LFD(Llogior)(void)
{
	object  x;
	int	narg, i;
	int	ior_op(int i, int j);

	narg = vs_top - vs_base;
	for (i = 0; i < narg; i++)
		check_type_integer(&vs_base[i]);
	if (narg == 0) {
		vs_top = vs_base;
		vs_push(small_fixnum(0));
		return;
	}
	if (narg == 1)
		return;
	x = log_op(ior_op,mp_ior_op);
	vs_top = vs_base;
	vs_push(x);
}

LFD(Llogxor)(void)
{
	object  x;
	int	narg, i;
	int	xor_op(int i, int j);

	narg = vs_top - vs_base;
	for (i = 0; i < narg; i++)
		check_type_integer(&vs_base[i]);
	if (narg == 0) {
		vs_top = vs_base;
		vs_push(small_fixnum(0));
		return;
	}
	if (narg == 1) return;
	x = log_op(xor_op,mp_xor_op);
	vs_top = vs_base;
	vs_push(x);
}

LFD(Llogand)(void)
{
	object  x;
	int	narg, i;
	int	and_op(int i, int j);

	narg = vs_top - vs_base;
	for (i = 0; i < narg; i++)
		check_type_integer(&vs_base[i]);
	if (narg == 0) {
		vs_top = vs_base;
		vs_push(small_fixnum(-1));
		return;
	}
	if (narg == 1) return;
	x = log_op(and_op,mp_and_op);
	vs_top = vs_base;
	vs_push(x);
}

LFD(Llogeqv)(void)
{
	object  x;
	int	narg, i;
	int	eqv_op(int i, int j);

	narg = vs_top - vs_base;
	for (i = 0; i < narg; i++)
		check_type_integer(&vs_base[i]);
	if (narg == 0) {
		vs_top = vs_base;
		vs_push(small_fixnum(-1));
		return;
	}
	if (narg == 1) return;
	x = log_op(eqv_op,mp_eqv_op);
	vs_top = vs_base;
	vs_push(x);
}

LFD(Lboole)(void)
{
	object  x;
	object	o;
	int	(*op)()=NULL;
	void	(*mp_op)() = (void *) 0;

	check_arg(3);
	check_type_integer(&vs_base[0]);
	check_type_integer(&vs_base[1]);
	check_type_integer(&vs_base[2]);
	o = vs_base[0];

	switch(fixint(o)) {
		case BOOLCLR:	op = b_clr_op; mp_op = mp_b_clr_op;	break;
		case BOOLSET:	op = b_set_op; mp_op = mp_b_set_op;	break;
		case BOOL1:	op = b_1_op; mp_op = mp_b_1_op;	break;
		case BOOL2:	op = b_2_op; mp_op = mp_b_2_op;	break;
		case BOOLC1:	op = b_c1_op; mp_op =mp_b_c1_op;	break;
		case BOOLC2:	op = b_c2_op; mp_op =mp_b_c2_op;	break;
		case BOOLAND:	op = and_op; mp_op = mp_and_op;	break;
		case BOOLIOR:	op = ior_op; mp_op = mp_ior_op;	break;
		case BOOLXOR:	op = xor_op; mp_op = mp_xor_op;	break;
		case BOOLEQV:	op = eqv_op; mp_op = mp_eqv_op;	break;
		case BOOLNAND:	op = nand_op; mp_op =mp_nand_op;	break;
		case BOOLNOR:	op = nor_op; mp_op = mp_nor_op;	break;
		case BOOLANDC1:	op = andc1_op; mp_op = mp_andc1_op;	break;
		case BOOLANDC2:	op = andc2_op; mp_op = mp_andc2_op;	break;
		case BOOLORC1:	op = orc1_op; mp_op =mp_orc1_op;	break;
		case BOOLORC2:	op = orc2_op; mp_op =mp_orc2_op;	break;
		default:
			FEerror("~S is an invalid logical operator.",
				1, o);
	}
	vs_base++;
	x = log_op(op,mp_op);
	vs_base--;
	vs_top = vs_base;
	vs_push(x);
}

LFD(Llogbitp)(void)
{
	object	x, p;
	int	i;

	check_arg(2);
	check_type_non_negative_integer(&vs_base[0]);
	check_type_integer(&vs_base[1]);
	p = vs_base[0];
	x = vs_base[1];
	if (type_of(p) == t_fixnum)
		if (type_of(x) == t_fixnum)
			i = fix_bitp(x, fix(p));
		else
			i = big_bitp(x, fix(p));
	else if (big_sign(p) < 0)
			i = 0;
		/*
		   bit position represented by bignum is out of
		   our address space. So, result is returned
		   according to sign of integer.
		*/

	else if (type_of(x) == t_fixnum)
		if (fix(x) < 0)
			i = 1;
		else
			i = 0;
	else if (big_sign(x) < 0)
			i = 1;
		else
			i = 0;

	vs_top = vs_base;
	if (i)
		vs_push(Ct);
	else
		vs_push(Cnil);
}

DEFUN("ASH",object,fLash,LISP,2,2,NONE,OO,OO,OO,OO,(object x,object y),"") {

  int w,sign_x;
  
  check_type_integer(&x);
  check_type_integer(&y);

  if (type_of(y) == t_fixnum) {
    w = fix(y);
    RETURN1(shift_integer(x,w));
  } 

  if (x==small_fixnum(0))
    RETURN1(x);

  if (type_of(x) == t_fixnum)
    sign_x=fix(x)>0 ? 1 : -1;
  else
    sign_x = big_sign(x);
  if (big_sign(y) < 0)
    RETURN1(sign_x < 0 ? small_fixnum(-1) : small_fixnum(0));
  else
    FEerror("Insufficient memory.", 0);
  RETURN1(Cnil);
}

LFD(Llogcount)(void)
{
	object	x;
	int	i;

	check_arg(1);
	check_type_integer(&vs_base[0]);
	x = vs_base[0];
	i = count_bits(x);
	vs_top = vs_base;
	vs_push(make_fixnum(i));
}

LFD(Linteger_length)(void)
{
	object	x;
	int count=0, i;

	check_arg(1);
	x = vs_base[0];
	if (type_of(x) == t_fixnum) {
		i = fix(x);
		if (i < 0) i = ~i;
		count = int_bit_length(i);
	} else if (type_of(x) == t_bignum) 
	  count = MP_SIZE_IN_BASE2(MP(x));
	else
	      	FEwrong_type_argument(sLinteger, x);
	vs_top = vs_base;
	vs_push(make_fixnum(count));
}

/* #define W_SIZE (8*sizeof(int)) */
/* static object */
/* bitand(object a, object b, object c) */
/* { int d= a->bv.bv_fillp; */
/*   int *ap,*bp,*cp; */
/*   d=(d+W_SIZE-1)/W_SIZE; */
/*   ap= (int *)(a->bv.bv_self); */
/*   bp= (int *)(b->bv.bv_self); */
/*   cp= (int *)(c->bv.bv_self); */
/*   while (--d >= 0) */
/*     { *cp++ = *bp++ & *ap++; */
/*     } */
/*   return c; */
/* } */

/* static object */
/* bitior(object a, object b, object c) */
/* { int *ap,*cp,*bp, d= a->bv.bv_fillp; */
/*   d=(d+W_SIZE-1)/W_SIZE; */
/*    ap= (int *)((a->bv.bv_self)); */
/*    bp= (int *)(b->bv.bv_self); */
/*    cp= (int *)(c->bv.bv_self); */
/*   while (--d >= 0) */
/*     { *cp++ = *bp++ | *ap++; */
/*     } */
/*   return c; */
/* } */

/* Note in order to be equal we assume that the part above the
   fill pointer is 0 up to the next word */

/* static int */
/* bvequal(object a, object b) */
/* { int *ap,*bp, d= a->bv.bv_fillp; */
/*   d=(d+W_SIZE-1)/W_SIZE; */
/*  ap= (int *)(a->bv.bv_self); */
/*  bp= (int *)(b->bv.bv_self); */
/*   while (--d >= 0) */
/*     { if (*ap++ != *bp++) return 1; */
/*     } */
/*   return 0; */
/* } */

  
DEFUN("BIT-ARRAY-OP",object,fSbit_array_op,SI,4,4,NONE,OO,OO,OO,OO,
	  (object o,object x,object y,object r),"") {

/* LFD(siLbit_array_op)(void) */
/* { */
	int i, j, n, d;
	object  r0=Cnil;
	int (*op)()=NULL;
	bool replace = FALSE;
	int xi, yi, ri;
	char *xp, *yp, *rp;
	int xo, yo, ro;
/* 	object *base = vs_base; */

/* 	check_arg(4); */
/* 	o = vs_base[0]; */
/* 	x = vs_base[1]; */
/* 	y = vs_base[2]; */
/* 	r = vs_base[3]; */
	if (type_of(x) == t_bitvector) {
		d = x->bv.bv_dim;
		xp = x->bv.bv_self;
		xo = BV_OFFSET(x);
		if (type_of(y) != t_bitvector)
			goto ERROR;
		if (d != y->bv.bv_dim)
			goto ERROR;
		yp = y->bv.bv_self;
		yo = BV_OFFSET(y);
		if (r == Ct)
			r = x;
		if (r != Cnil) {
			if (type_of(r) != t_bitvector)
				goto ERROR;
			if (r->bv.bv_dim != d)
				goto ERROR;
			i = (r->bv.bv_self - xp)*CHAR_SIZE + (BV_OFFSET(r) - xo);
			if ((i > 0 && i < d) || (i < 0 && -i < d)) {
				r0 = r;
				r = Cnil;
				replace = TRUE;
				goto L1;
			}
			i = (r->bv.bv_self - yp)*CHAR_SIZE + (BV_OFFSET(r) - yo);
			if ((i > 0 && i < d) || (i < 0 && -i < d)) {
				r0 = r;
				r = Cnil;
				replace = TRUE;
			}
		}
	L1:
		if (r == Cnil)
		  r=fSmake_vector(sLbit,d,Cnil,Cnil,Cnil,0,Cnil,Cnil);
	} else {
		if (type_of(x) != t_array)
			goto ERROR;
		if ((enum aelttype)x->a.a_elttype != aet_bit)
			goto ERROR;
		d = x->a.a_dim;
		xp = x->bv.bv_self;
		xo = BV_OFFSET(x);
		if (type_of(y) != t_array)
			goto ERROR;
		if ((enum aelttype)y->a.a_elttype != aet_bit)
			goto ERROR;
		if (x->a.a_rank != y->a.a_rank)
			goto ERROR;
		yp = y->bv.bv_self;
		yo = BV_OFFSET(y);
		for (i = 0;  i < x->a.a_rank;  i++)
			if (x->a.a_dims[i] != y->a.a_dims[i])
				goto ERROR;
		if (r == Ct)
			r = x;
		if (r != Cnil) {
			if (type_of(r) != t_array)
				goto ERROR;
			if ((enum aelttype)r->a.a_elttype != aet_bit)
				goto ERROR;
			if (r->a.a_rank != x->a.a_rank)
				goto ERROR;
			for (i = 0;  i < x->a.a_rank;  i++)
				if (r->a.a_dims[i] != x->a.a_dims[i])
					goto ERROR;
			i = (r->bv.bv_self - xp)*CHAR_SIZE + (BV_OFFSET(r) - xo);
			if ((i > 0 && i < d) || (i < 0 && -i < d)) {
				r0 = r;
				r = Cnil;
				replace = TRUE;
				goto L2;
			} 
			i = (r->bv.bv_self - yp)*CHAR_SIZE + (BV_OFFSET(r) - yo);
			if ((i > 0 && i < d) || (i < 0 && -i < d)) {
				r0 = r;
				r = Cnil;
				replace = TRUE;
			}
		}
	L2:
		if (r == Cnil) {
		  object b;
		  struct cons *p=ZALLOCA((x->a.a_rank+1)*sizeof(struct cons));
		  p=(void *)(((unsigned long)((void *)p+sizeof(struct cons)-1))&~(sizeof(struct cons)-1));
		  if (x->a.a_rank) {
		    object b1;

		    b=(object)p;
		    for (b1=b,i=0;i<x->a.a_rank;i++,b1=b1->c.c_cdr) {
		      set_type_of(b1,t_cons); 
		      b1->c.c_car=make_fixnum(x->a.a_dims[i]);
		      b1->c.c_cdr=i<x->a.a_rank-1 ? (object)++p : Cnil;
		    }
		  } else
		    b=Cnil;

		  r = fSmake_array1(sLbit,Cnil,small_fixnum(0),Cnil,0,b);

		  /* 		  object b[F_ARG_LIMIT]; */
		  /* 		  b[0]=Cnil; */
		  /* 		  for (i = 0;  i < x->a.a_rank;  i++) */
		  /* 		    b[i] = (make_fixnum(x->a.a_dims[i])); */
		  /* 		  r=Iapply_fun_n1(fSmake_array1,5,x->a.a_rank ? x->a.a_rank : 1, */
		  /* 			       aet_bit, */
		  /* 			       Cnil, */
		  /* 			       small_fixnum(0), */
		  /* 			       Cnil, */
		  /* 			       Cnil, */
		  /*				 b); */

		}
	}
	rp = r->bv.bv_self;
	ro = BV_OFFSET(r);
	switch(fixint(o)) {
		case BOOLCLR:	op = b_clr_op;	break;
		case BOOLSET:	op = b_set_op;	break;
		case BOOL1:	op = b_1_op;	break;
		case BOOL2:	op = b_2_op;	break;
		case BOOLC1:	op = b_c1_op;	break;
		case BOOLC2:	op = b_c2_op;	break;
		case BOOLAND:	op = and_op;	break;
		case BOOLIOR:	op = ior_op;	break;
		case BOOLXOR:	op = xor_op;	break;
		case BOOLEQV:	op = eqv_op;	break;
		case BOOLNAND:	op = nand_op;	break;
		case BOOLNOR:	op = nor_op;	break;
		case BOOLANDC1:	op = andc1_op;	break;
		case BOOLANDC2:	op = andc2_op;	break;
		case BOOLORC1:	op = orc1_op;	break;
		case BOOLORC2:	op = orc2_op;	break;
		default:
			FEerror("~S is an invalid logical operator.", 1, o);
	}

#define	set_high(place, nbits, value) \
	((place)=(((place)&~(-0400>>(nbits)))|((value)&(-0400>>(nbits)))))

#define	set_low(place, nbits, value) \
	((place)=(((place)&(-0400>>(8-(nbits))))|((value)&~(-0400>>(8-(nbits))))))

#define	extract_byte(integer, pointer, index, offset) \
	(integer) = (pointer)[(index)+1] & 0377; \
	(integer) = ((pointer)[index]<<(offset))|((integer)>>(8-(offset)))

#define	store_byte(pointer, index, offset, value) \
	set_low((pointer)[index], 8-(offset), (value)>>(offset)); \
	set_high((pointer)[(index)+1], offset, (value)<<(8-(offset)))

	if (xo == 0 && yo == 0 && ro == 0) {
		for (n = d/8, i = 0;  i < n;  i++)
			rp[i] = (*op)(xp[i], yp[i]);
		if ((j = d%8) > 0)
			set_high(rp[n], j, (*op)(xp[n], yp[n]));
		if (!replace) {
		  RETURN1(r);
/* 			vs_top = vs_base = base; */
/* 			vs_push(r); */
/* 			return; */
		}
	} else {
		for (n = d/8, i = 0;  i <= n;  i++) {
			extract_byte(xi, xp, i, xo);
			extract_byte(yi, yp, i, yo);
			if (i == n) {
				if ((j = d%8) == 0)
					break;
				extract_byte(ri, rp, n, ro);
				set_high(ri, j, (*op)(xi, yi));
			} else
				ri = (*op)(xi, yi);
			store_byte(rp, i, ro, ri);
		}
		if (!replace) {
		  RETURN1(r);
/* 			vs_top = vs_base = base; */
/* 			vs_push(r); */
/* 			return; */
		}
	}
	rp = r0->bv.bv_self;
	ro = BV_OFFSET(r0);
	for (n = d/8, i = 0;  i <= n;  i++) {
		if (i == n) {
			if ((j = d%8) == 0)
				break;
			extract_byte(ri, rp, n, ro);
			set_high(ri, j, r->bv.bv_self[n]);
		} else
			ri = r->bv.bv_self[i];
		store_byte(rp, i, ro, ri);
	}
	RETURN1(r0);
/* 	vs_top = vs_base = base; */
/* 	vs_push(r0); */
/* 	return; */

ERROR:
	FEerror("Illegal arguments for bit-array operation.", 0);
	RETURN1(Cnil);
}

void
gcl_init_num_log(void)
{
/*  	int siLbit_array_op(void); */

	make_constant("BOOLE-CLR", make_fixnum(BOOLCLR));
	make_constant("BOOLE-SET", make_fixnum(BOOLSET));
	make_constant("BOOLE-1", make_fixnum(BOOL1));
	make_constant("BOOLE-2", make_fixnum(BOOL2));
	make_constant("BOOLE-C1", make_fixnum(BOOLC1));
	make_constant("BOOLE-C2", make_fixnum(BOOLC2));
	make_constant("BOOLE-AND", make_fixnum(BOOLAND));
	make_constant("BOOLE-IOR", make_fixnum(BOOLIOR));
	make_constant("BOOLE-XOR", make_fixnum(BOOLXOR));
	make_constant("BOOLE-EQV", make_fixnum(BOOLEQV));
	make_constant("BOOLE-NAND", make_fixnum(BOOLNAND));
	make_constant("BOOLE-NOR", make_fixnum(BOOLNOR));
	make_constant("BOOLE-ANDC1", make_fixnum(BOOLANDC1));
	make_constant("BOOLE-ANDC2", make_fixnum(BOOLANDC2));
	make_constant("BOOLE-ORC1", make_fixnum(BOOLORC1));
	make_constant("BOOLE-ORC2", make_fixnum(BOOLORC2));

	make_function("LOGIOR", Llogior);
	make_function("LOGXOR", Llogxor);
	make_function("LOGAND", Llogand);
	make_function("LOGEQV", Llogeqv);
	make_function("BOOLE", Lboole);
	make_function("LOGBITP", Llogbitp);
/* 	make_function("ASH", Lash); */
	make_function("LOGCOUNT", Llogcount);
	make_function("INTEGER-LENGTH", Linteger_length);

	sLbit = make_ordinary("BIT");
/* 	make_si_function("BIT-ARRAY-OP", siLbit_array_op); */
}

