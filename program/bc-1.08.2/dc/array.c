/*
 * implement arrays for dc
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2006, 2008, 2017, 2018
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

/* This module is the only one that knows what arrays look like. */
/* Arrays are now stored as a dynamically sized hash table;
 * this allows for sparse arrays with constant-time lookups.
 *
 * For now we are continuing the legacy constraint of integer
 * keys, though a reasonable future direction would be to allow
 * any dc value (numeric or string) to be hashed, giving associative
 * arrays...
 */

#include <stdlib.h>
#include <string.h> /* memset */
#include "dc.h"
#include "dc-regdef.h"

/* store entries in a hash table with linear probing */
#define HASH_LOAD(n) (n/10*8)

typedef unsigned long hash_num;

typedef struct {
	int Index;
	dc_data value;
} hashtable_value;

struct dc_array {
	hash_num nentries;
	hash_num rehash_thresh;
	const hash_num *prime;
	hashtable_value *values; /* array with *prime elements */
};

static const hash_num hash_primes[] = {
	61, 4093, 32771, 262147, 1048573, 4194301,
	16777213, 67108859, 268435459, 1073741827, 4294967291,
	0
};

static hashtable_value *
hashtable_values_allocate(hash_num n)
{
	size_t s = n * sizeof(hashtable_value);
	if (s/n != sizeof(hashtable_value)) /* multiplication overflowed */
		dc_memfail();
	hashtable_value *p = dc_malloc(s);
	memset(p, 0, s);
	return p;
}

static void
free_value(dc_data v, int array_id)
{
	switch (v.dc_type) {
	case DC_NUMBER: dc_free_num(&v.v.number); break;
	case DC_STRING: dc_free_str(&v.v.string); break;
	case DC_UNINITIALIZED: /*ignore (no-op)*/; break;
	default: dc_garbage("in array", array_id); break;
	}
}

/* find slot matching key, otherwise empty slot to insert into */
static hashtable_value *
hashtable_slot(struct dc_array *table, int key)
{
	hash_num n = (hash_num)key % *table->prime;
	hashtable_value *p = table->values + n;
	hashtable_value *end_of_table = table->values + *table->prime;

	while (p->value.dc_type != DC_UNINITIALIZED) {
		if (p->Index == key)
			break;
		if (end_of_table <= ++p)
			p = table->values;
	}
	return p;
}


/* initialize the arrays */
void
dc_array_init(void)
{
	/* no-op, in this implementation */
}

/* store value into array_id[Index] */
void
dc_array_set(int array_id, int Index, dc_data value)
{
	struct dc_array *cur = dc_get_stacked_array(array_id);

	if ( ! cur ) {
		/* allocate initial (empty) hash table */
		cur = dc_malloc(sizeof *cur);
		cur->nentries = 0;
		cur->prime = &hash_primes[0];
		cur->rehash_thresh = HASH_LOAD(*cur->prime);
		cur->values = hashtable_values_allocate(*cur->prime);
		dc_set_stacked_array(array_id, cur);
	}

	if (cur->rehash_thresh <= cur->nentries) {
		/* re-hash into a larger hash table */
		hashtable_value *oldtable = cur->values;
		hash_num oldprime = *cur->prime;
		hash_num newprime = *++cur->prime;
		if ( ! newprime ) {
			dc_warn("prime table exhausted (!); re-compile with more entries");
			exit(EXIT_FAILURE);
		}
		cur->values = hashtable_values_allocate(newprime);
		cur->rehash_thresh = HASH_LOAD(newprime);

		hashtable_value *p = oldtable;
		for (; oldprime--; ++p)
			if (p->value.dc_type != DC_UNINITIALIZED)
				*hashtable_slot(cur, p->Index) = *p;
		free(oldtable);
	}

	/* okay, actually add/replace the entry now... */
	hashtable_value *slot = hashtable_slot(cur, Index);
	if (slot->value.dc_type == DC_UNINITIALIZED)
		++cur->nentries;
	else
		free_value(slot->value, array_id);
	slot->value = value;
	slot->Index = Index;
}

/* retrieve a dup of a value from array_id[Index] */
/* A zero value is returned if the specified value is unintialized. */
dc_data
dc_array_get(int array_id, int Index)
{
	struct dc_array *cur = dc_get_stacked_array(array_id);
	if ( ! cur )
		return dc_int2data(0);
	hashtable_value *slot = hashtable_slot(cur, Index);
	if (slot->value.dc_type == DC_UNINITIALIZED)
		return dc_int2data(0);
	return dc_dup(slot->value);
}

/* free an array chain */
void
dc_array_free(struct dc_array *arr)
{
	if (!arr)
		return;
	hash_num nentry = *arr->prime;
	hashtable_value *p = arr->values;
	for (; nentry--; ++p)
		free_value(p->value, -1);
	free(arr->values);
	free(arr);
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
