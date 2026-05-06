/*
 * implement stack functions for dc
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2005, 2006, 2008, 2012, 2016-2017
 * Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* This module is the only one that knows what stacks (both the
 * regular evaluation stack and the named register stacks)
 * look like.
 */

#include <stdio.h>
#include <stdlib.h>
#include "dc.h"
#include "dc-regdef.h"

/* an oft-used error message: */
#define Empty_Stack	dc_warn("stack empty")


/* simple linked-list implementation suffices: */
struct dc_list {
	dc_data value;
	struct dc_array *array;	/* opaque */
	struct dc_list *link;
};
typedef struct dc_list dc_list;

/* the anonymous evaluation stack */
static dc_list *dc_stack=NULL;

/* the named register stacks */
typedef dc_list *dc_listp;
static dc_listp dc_register[DC_REGCOUNT];


/* allocate a new dc_list item */
static dc_list *
dc_alloc(void)
{
	dc_list *result;

	result = dc_malloc(sizeof *result);
	result->value.dc_type = DC_UNINITIALIZED;
	result->array = NULL;
	result->link = NULL;
	return result;
}

/* Check that the top N values on the stack are numeric:
 * if so, then copy them off into the result array and return DC_SUCCESS,
 * else emit a diagnostic and return DC_FAIL.
 * Regardless, r[] might be modified, but the stack will not be.
 */
static int
dc_get_N_numbers(int n, dc_data r[])
{
	struct dc_list *p;

	for (p=dc_stack; 0<n--; p=p->link) {
		if (!p){
			Empty_Stack;
			return DC_FAIL;
		}
		if (p->value.dc_type != DC_NUMBER){
			dc_warn("non-numeric value");
			return DC_FAIL;
		}
		*r++ = p->value;
	}
	return DC_SUCCESS;
}


/* Check that there is one number on top of the stack,
 * then call op with the popped number.
 * Construct a dc_data value from the dc_num returned
 * by op and push it on the stack.
 * If the op call doesn't return DC_SUCCESS,
 * then leave the stack unmodified.
 */
void
dc_monop(int (*op)(dc_num, int, dc_num *), int kscale)
{
	dc_data a;
	dc_data r;

	if (dc_get_N_numbers(1, &a) != DC_SUCCESS)
		return;
	if ((*op)(a.v.number, kscale, &r.v.number) == DC_SUCCESS){
		dc_discard_TOS();
		r.dc_type = DC_NUMBER;
		dc_push(r);
	}
}

/* check that there are two numbers on top of the stack,
 * then call op with the popped numbers.  Construct a dc_data
 * value from the dc_num returned by op and push it
 * on the stack.
 * If the op call doesn't return DC_SUCCESS, then leave the stack
 * unmodified.
 */
void
dc_binop(int (*op)(dc_num, dc_num, int, dc_num *), int kscale)
{
	dc_data a[2];
	dc_data r;

	if (dc_get_N_numbers(2, a) != DC_SUCCESS)
		return;
	if ((*op)(a[1].v.number, a[0].v.number, kscale, &r.v.number) == DC_SUCCESS){
		dc_discard_TOS();
		dc_discard_TOS();
		r.dc_type = DC_NUMBER;
		dc_push(r);
	}
}

/* Binary operation with two return values.
 * Check that there are two numbers on top of the stack,
 * then call op with the popped numbers.  Construct two dc_data
 * values from the dc_num's returned by op and push them
 * on the stack.
 * If the op call doesn't return DC_SUCCESS, then leave the stack
 * unmodified.
 */
void
dc_binop2(int (*op)(dc_num, dc_num, int, dc_num *, dc_num *), int kscale)
{
	dc_data a[2];
	dc_data r0, r1;

	if (dc_get_N_numbers(2, a) != DC_SUCCESS)
		return;
	if ((*op)(a[1].v.number, a[0].v.number,
				kscale, &r0.v.number, &r1.v.number) == DC_SUCCESS){
		dc_discard_TOS();
		dc_discard_TOS();
		r0.dc_type = DC_NUMBER;
		dc_push(r0);
		r1.dc_type = DC_NUMBER;
		dc_push(r1);
	}
}

/* check that there are three numbers on top of the stack,
 * then call op with the popped numbers.  Construct a dc_data
 * value from the dc_num returned by op and push it
 * on the stack.
 * If the op call doesn't return DC_SUCCESS, then leave the stack
 * unmodified.
 */
void
dc_triop(int (*op)(dc_num, dc_num, dc_num, int, dc_num *), int kscale)
{
	dc_data a[3];
	dc_data r;

	if (dc_get_N_numbers(3, a) != DC_SUCCESS)
		return;
	if ((*op)(a[2].v.number, a[1].v.number, a[0].v.number,
				kscale, &r.v.number) == DC_SUCCESS){
		dc_discard_TOS();
		dc_discard_TOS();
		dc_discard_TOS();
		r.dc_type = DC_NUMBER;
		dc_push(r);
	}
}

/* Check that there are two numbers on top of the stack; if not,
 * then return false (zero) immediately, without modifying the stack.
 * Otherwise pop the two numbers and call dc_compare with them;
 * if the result matches the comparison designated by cmpop,
 * return cmpval, otherwise return the logical negation of cmpval.
 */
int
dc_cmpop(int cmpop, int cmpval)
{
	int result;
	dc_data a[2];

	if (dc_get_N_numbers(2, a) != DC_SUCCESS)
		return 0;
	result = dc_compare(a[0].v.number, a[1].v.number);
	dc_discard_TOS();
	dc_discard_TOS();
	if (result<0 && cmpop=='<')
		return cmpval;
	if (result==0 && cmpop=='=')
		return cmpval;
	if (result>0 && cmpop=='>')
		return cmpval;
	return ! cmpval;
}


/* initialize the register stacks to their initial values */
void
dc_register_init(void)
{
	int i;

	for (i=0; i<DC_REGCOUNT; ++i)
		dc_register[i] = NULL;
}

/* clear the evaluation stack */
void
dc_clear_stack(void)
{
	dc_list *n;
	dc_list *t;

	for (n=dc_stack; n; n=t){
		t = n->link;
		if (n->value.dc_type == DC_NUMBER)
			dc_free_num(&n->value.v.number);
		else if (n->value.dc_type == DC_STRING)
			dc_free_str(&n->value.v.string);
		else
			dc_garbage("in stack", -1);
		dc_array_free(n->array);
		free(n);
	}
	dc_stack = NULL;
}

/* push a value onto the evaluation stack */
void
dc_push(dc_data value)
{
	dc_list *n = dc_alloc();

	if (value.dc_type!=DC_NUMBER && value.dc_type!=DC_STRING)
		dc_garbage("in data being pushed", -1);
	n->value = value;
	n->link = dc_stack;
	dc_stack = n;
}

/* push a value onto the named register stack */
void
dc_register_push(int stackid, dc_data value)
{
	dc_list *n = dc_alloc();

	stackid = regmap(stackid);
	n->value = value;
	n->link = dc_register[stackid];
	dc_register[stackid] = n;
}

/* set *result to the value on the top of the evaluation stack */
/* The caller is responsible for duplicating the value if it
 * is to be maintained as anything more than a transient identity.
 *
 * DC_FAIL is returned if the stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise.
 *
 * A diagnostic message is emitted if the stack is empty.
 */
int
dc_top_of_stack(dc_data *result)
{
	if (!dc_stack){
		Empty_Stack;
		return DC_FAIL;
	}
	if (dc_stack->value.dc_type!=DC_NUMBER
			&& dc_stack->value.dc_type!=DC_STRING)
		dc_garbage("at top of stack", -1);
	*result = dc_stack->value;
	return DC_SUCCESS;
}

/* Set *result to the value on the top of the evaluation stack, provided
 * said value is numeric; errval is placed in *result if the numeric value
 * will not fit in a C int.  The stack is always left unmodified.
 * DC_FAIL is returned (and *result unchanged) if the stack is empty
 * or the top of stack is not numeric; DC_SUCCESS is returned otherwise.
 */
int
dc_int_from_TOS(int *result, int errval)
{
	if (!dc_stack || dc_stack->value.dc_type!=DC_NUMBER)
		return DC_FAIL;
	*result = dc_num2int(dc_stack->value.v.number, errval);
	return DC_SUCCESS;
}

/* set *result to a dup of the value on the top of the named register stack,
 * or 0 (zero) if the stack is empty */
/*
 * DC_FAIL is returned if an internal bug is detected
 * DC_SUCCESS is returned otherwise
 */
int
dc_register_get(int regid, dc_data *result)
{
	dc_list *r;

	regid = regmap(regid);
	r = dc_register[regid];
	if (!r || r->value.dc_type==DC_UNINITIALIZED){
		*result = dc_int2data(0);
	}else{
		*result = dc_dup(r->value);
	}
	return DC_SUCCESS;
}

/* set the top of the named register stack to the indicated value */
/* If the named stack is empty, craft a stack entry to enter the
 * value into.
 */
void
dc_register_set(int regid, dc_data value)
{
	dc_list *r;

	regid = regmap(regid);
	r = dc_register[regid];
	if (!r)
		dc_register[regid] = dc_alloc();
	else if (r->value.dc_type == DC_NUMBER)
		dc_free_num(&r->value.v.number);
	else if (r->value.dc_type == DC_STRING)
		dc_free_str(&r->value.v.string);
	else if (r->value.dc_type == DC_UNINITIALIZED)
		;
	else
		dc_garbage("", regid);
	dc_register[regid]->value = value;
}

/* pop from the evaluation stack
 *
 * DC_FAIL is returned if the stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise
 */
int
dc_pop(dc_data *result)
{
	dc_list *r;

	r = dc_stack;
	if (!r || r->value.dc_type==DC_UNINITIALIZED){
		Empty_Stack;
		return DC_FAIL;
	}
	if (r->value.dc_type!=DC_NUMBER && r->value.dc_type!=DC_STRING)
		dc_garbage("at top of stack", -1);
	*result = r->value;
	dc_stack = r->link;
	dc_array_free(r->array);
	free(r);
	return DC_SUCCESS;
}

/* pop from the named register stack
 *
 * DC_FAIL is returned if the named stack is empty (and *result unchanged),
 * DC_SUCCESS is returned otherwise
 */
int
dc_register_pop(int stackid, dc_data *result)
{
	dc_list *r;

	stackid = regmap(stackid);
	r = dc_register[stackid];
	if (!r || r->value.dc_type==DC_UNINITIALIZED){
		fprintf(stderr, "%s: stack register ", progname);
		dc_show_id(stderr, stackid, " is empty\n");
		return DC_FAIL;
	}
	if (r->value.dc_type!=DC_NUMBER && r->value.dc_type!=DC_STRING)
		dc_garbage(" stack", stackid);
	*result = r->value;
	dc_register[stackid] = r->link;
	dc_array_free(r->array);
	free(r);
	return DC_SUCCESS;
}


/* cyclically rotate the "n" topmost elements of the stack;
 *   negative "n" rotates forward (topmost element becomes n-th deep)
 *   positive "n" rotates backward (topmost element becomes 2nd deep)
 *
 * If stack depth is less than "n", whole stack is rotated
 * (without raising an error).
 */
void
dc_stack_rotate(int n)
{
	dc_list *p; /* becomes bottom of sub-stack */
	dc_list *r; /* predecessor of "p" */
	int absn = n<0 ? -n : n;

	/* always do nothing for empty stack or degenerate rotation depth */
	if (!dc_stack || absn < 2)
		return;
	/* find bottom of rotation sub-stack */
	r = NULL;
	for (p=dc_stack; p->link && --absn>0; p=p->link)
		r = p;
	/* if stack has only one element, treat rotation as no-op */
	if (!r)
		return;
	/* do the rotation, in appropriate direction */
	if (n > 0) {
		r->link = p->link;
		p->link = dc_stack;
		dc_stack = p;
	} else {
		dc_list *new_tos = dc_stack->link;
		dc_stack->link = p->link;
		p->link = dc_stack;
		dc_stack = new_tos;
	}
}


/* tell how many entries are currently on the evaluation stack */
int
dc_tell_stackdepth(void)
{
	dc_list *n;
	int depth=0;

	for (n=dc_stack; n; n=n->link)
		++depth;
	return depth;
}


/* discards the value on top of stack */
void
dc_discard_TOS(void)
{
	dc_data value;

	if (dc_pop(&value) != DC_SUCCESS) {
		/*ignore; dc_pop gave diagnostic*/
	} else if (value.dc_type == DC_NUMBER){
		dc_free_num(&value.v.number);
	} else if (value.dc_type == DC_STRING) {
		dc_free_str(&value.v.string);
	} else {
		dc_garbage("in discard_TOS", -1);
		/*NOTREACHED*/
	}
}

/* return the length of the indicated data value;
 * deallocate the value when done
 *
 * The definition of a datum's length is deligated to the
 * appropriate module.
 */
int
dc_tell_length(dc_data value)
{
	int length;

	if (value.dc_type == DC_NUMBER){
		length = dc_numlen(value.v.number);
		dc_free_num(&value.v.number);
	} else if (value.dc_type == DC_STRING) {
		length = (int) dc_strlen(value.v.string);
		dc_free_str(&value.v.string);
	} else {
		dc_garbage("in tell_length", -1);
		/*NOTREACHED*/
		length = 0;	/*just to suppress spurious compiler warnings*/
	}
	return length;
}



/* print out all of the values on the evaluation stack */
void
dc_printall(int obase)
{
	dc_list *n;

	for (n=dc_stack; n; n=n->link)
		dc_print(n->value, obase, DC_WITHNL, DC_KEEP);
}




/* get the current array head for the named array */
struct dc_array *
dc_get_stacked_array(int array_id)
{
	dc_list *r = dc_register[regmap(array_id)];
	return r ? r->array : NULL;
}

/* set the current array head for the named array */
void
dc_set_stacked_array(int array_id, struct dc_array *new_head)
{
	dc_list *r;

	array_id = regmap(array_id);
	r = dc_register[array_id];
	if (!r)
		r = dc_register[array_id] = dc_alloc();
	r->array = new_head;
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
