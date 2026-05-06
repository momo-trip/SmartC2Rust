/*
 * prototypes of all externally visible dc functions
 *
 * Copyright (C) 1994, 1997, 1998, 2003, 2008, 2012, 2013, 2017, 2022, 2024
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

/* note: this file is presumed to be #include'd from "dc.h",
 * after all of the the relevant types have been defined
 */

#include <stdio.h> /* for FILE* */

extern void dc_array_init(void);
extern void dc_math_init(void);
extern void dc_register_init(void);
extern void dc_string_init(void);
extern void input_cleanup(void);

extern size_t dc_strlen(dc_str);
extern const char *dc_str2charp(dc_str);
extern const char *dc_system(const char *);
extern void *dc_malloc(size_t);
extern struct dc_array *dc_get_stacked_array(int);

extern void dc_monop(int (*)(dc_num, int, dc_num *), int);
extern void dc_binop(int (*)(dc_num, dc_num, int, dc_num *), int);
extern void dc_binop2(int (*)(dc_num, dc_num, int, dc_num *, dc_num *), int);
extern void dc_triop(int (*)(dc_num, dc_num, dc_num, int, dc_num *), int);

extern void dc_array_free(struct dc_array *);
extern void dc_array_set(int, int, dc_data);
extern void dc_clear_stack(void);
extern void dc_discard_TOS(void);
extern void dc_dump_num(dc_num);
extern void dc_flush_stdout(int);
extern void dc_free_num(dc_num *);
extern void dc_free_str(dc_str *);
extern void dc_garbage(const char *, int);
extern void dc_memfail(void);
extern void dc_out_num(dc_num, int, dc_discard);
extern void dc_out_str(dc_str, dc_discard);
extern void dc_print(dc_data, int, dc_newline, dc_discard);
extern void dc_printall(int);
extern void dc_push(dc_data);
extern void dc_register_push(int, dc_data);
extern void dc_register_set(int, dc_data);
extern void dc_set_interactive_flag(int);
extern void dc_set_stacked_array(int, struct dc_array *);
extern void dc_show_id(FILE *, int, const char *);
extern void dc_stack_rotate(int);
extern void dc_warn(const char *);

extern int  dc_cmpop(int, int);
extern int  dc_compare(dc_num, dc_num);
extern int  dc_evalfile(FILE *);
extern int  dc_evalstr(const char *);
extern int  dc_getchar(int);
extern int  dc_int_from_TOS(int *, int);
extern int  dc_num2int(dc_num, int);
extern int  dc_numlen(dc_num);
extern int  dc_pop(dc_data *);
extern int  dc_register_get(int, dc_data *);
extern int  dc_register_pop(int, dc_data *);
extern int  dc_tell_length(dc_data);
extern int  dc_tell_scale(dc_num);
extern int  dc_tell_stackdepth(void);
extern int  dc_top_of_stack(dc_data *);
extern int  dc_TOS_is_number(dc_data *);

extern dc_data dc_array_get(int, int);
extern dc_data dc_dup(dc_data);
extern dc_data dc_dup_num(dc_num);
extern dc_data dc_dup_str(dc_str);
extern dc_data dc_getnum(int (*)(void), int, int *);
extern dc_data dc_int2data(int);
extern dc_data dc_makestring(const char *, size_t, const char *);
extern dc_data dc_readline(void);
extern dc_data dc_readstring(int (*)(void), int , int, const char *);

extern int dc_add(dc_num, dc_num, int, dc_num *);
extern int dc_div(dc_num, dc_num, int, dc_num *);
extern int dc_divrem(dc_num, dc_num, int, dc_num *, dc_num *);
extern int dc_exp(dc_num, dc_num, int, dc_num *);
extern int dc_modexp(dc_num, dc_num, dc_num, int, dc_num *);
extern int dc_mul(dc_num, dc_num, int, dc_num *);
extern int dc_rem(dc_num, dc_num, int, dc_num *);
extern int dc_sqrt(dc_num, int, dc_num *);
extern int dc_sub(dc_num, dc_num, int, dc_num *);
