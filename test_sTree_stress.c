/*\
***  EAVL_Tree: Embedded AVL Tree
\*/

/*\
*#*  Copyright (c) 2018, Raymond S Brand
*#*  All rights reserved.
*#*
*#*  Redistribution and use in source and binary forms, with or without
*#*  modification, are permitted provided that the following conditions
*#*  are met:
*#*
*#*   * Redistributions of source code must retain the above copyright
*#*     notice, this list of conditions and the following disclaimer.
*#*
*#*   * Redistributions in binary form must reproduce the above copyright
*#*     notice, this list of conditions and the following disclaimer in
*#*     the documentation and/or other materials provided with the
*#*     distribution.
*#*
*#*   * Redistributions in source or binary form must carry prominent
*#*     notices of any modifications.
*#*
*#*   * Neither the name of the Raymond S Brand nor the names of its
*#*     contributors may be used to endorse or promote products derived
*#*     from this software without specific prior written permission.
*#*
*#*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*#*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*#*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*#*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*#*  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*#*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*#*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*#*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*#*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*#*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*#*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*#*  POSSIBILITY OF SUCH DAMAGE.
\*/


#define _XOPEN_SOURCE	1000

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "EAVL_sTree.h"

#include "container_of.h"


#ifndef QUIET_UNUSED
#define QUIET_UNUSED(var)	var = var
#endif	/* QUIET_UNUSED */

#ifndef MAX
#define MAX(A, B)		(((A) < (B)) ? (B) : (A))
#endif

#ifndef MIN
#define MIN(A, B)		(((B) < (A)) ? (B) : (A))
#endif


#define MASK_PRE		(1u<<0)
#define MASK_IN			(1u<<1)
#define MASK_POST		(1u<<2)

#define BIT_PRESENT		(1u<<0)
#define BIT_PROCESSED		(1u<<1)

#define TREE_PRINT_STR_SAME	"     "
#define TREE_PRINT_STR_DIFF	"|    "
#define TREE_PRINT_STR_BEND	"++ "


typedef struct node		node_t;
typedef struct tree_track	tree_track_t;
typedef struct master_track	master_track_t;
typedef struct pathestore	pathestore_t;
typedef struct cbdata		cbdata_t;
typedef uint32_t		hash_t;
typedef uint32_t		hash_ctx_t;
typedef	void			hash_data_t;
typedef struct params		params_t;
typedef unsigned int		order_mask_t;
typedef int (cb_t)(EAVLs_node_t* node, order_mask_t cover, void* cbdata);
typedef struct stats		stats_t;


struct node
	{
	unsigned int		key;
	unsigned int		height;
	unsigned int		weight;
	unsigned int		value;
	unsigned int		sum;
	EAVLs_node_t		node;
	hash_t			hash;
	};

#define NODE_INIT(NODE)							\
	do								\
		{							\
		NODE_CLEAR((NODE));					\
		(NODE)->height	= 0;					\
		(NODE)->weight	= 0;					\
		(NODE)->value	= 0;					\
		} while (0)

#define NODE_CLEAR(NODE)						\
	do								\
		{							\
		(NODE)->key	= -1u;					\
		(NODE)->height	= -1u;					\
		(NODE)->weight	= -1u;					\
		(NODE)->value	= -1u;					\
		(NODE)->sum	= -1u;					\
		hash_clear(&(NODE)->hash);				\
		} while (0)

struct pathestore
	{
	EAVLs_pathelement_t*	elements;
	unsigned int		count;
	unsigned char		bad_pathe;
	};

struct cbdata
	{
	tree_track_t*		tracker;
	pathestore_t*		pathestore;
	params_t*		params;
	};

#define CBDATA_INIT(CBDATA)						\
	do								\
		{							\
		(CBDATA)->tracker	= NULL;				\
		(CBDATA)->pathestore	= NULL;				\
		(CBDATA)->params	= NULL;				\
		} while (0)

struct tree_track
	{
	master_track_t*		tracker;
	EAVLs_tree_t		tree;
	EAVLs_context_t		context;
	cbdata_t*		cbdata;
	unsigned char*		presence;
	EAVLs_node_t**		shuffle;
	unsigned int*		position;
	EAVLs_node_t**		order;
	unsigned int		count;
	unsigned int		weight;
	unsigned int		sum;
	unsigned int		found;
	hash_t			hash;
	};

#define TREE_TRACK_CLEAR(TRACKER)					\
	do								\
		{							\
		(TRACKER)->tracker	= NULL;				\
		(TRACKER)->presence	= NULL;				\
		(TRACKER)->cbdata	= NULL;				\
		(TRACKER)->shuffle	= NULL;				\
		(TRACKER)->position	= NULL;				\
		(TRACKER)->order	= NULL;				\
		(TRACKER)->count	= -1u;				\
		(TRACKER)->weight	= -1u;				\
		(TRACKER)->sum		= -1u;				\
		(TRACKER)->found	= -1u;				\
		} while (0)

struct master_track
	{
	unsigned int		total;
	EAVLs_node_t**		nodep;
	EAVLs_cbset_t		cbset;
	tree_track_t		tracker;
	};

#define MASTER_TRACK_CLEAR(MASTER)					\
	do								\
		{							\
		(MASTER)->total		= -1u;				\
		(MASTER)->nodep		= NULL;				\
		(MASTER)->cbset.compare	= NULL;				\
		(MASTER)->cbset.fixup	= NULL;				\
		(MASTER)->cbset.verify	= NULL;				\
		TREE_TRACK_CLEAR(&(MASTER)->tracker);			\
		} while (0)

struct stats
	{
	unsigned int		find;
	unsigned int		insert;
	unsigned int		remove;
	unsigned int		first;
	unsigned int		next;
	unsigned int		load;
	unsigned int		clear;
	unsigned int		fixup;
	unsigned int		total;
	};

#define STATS_INIT(STATS)						\
	do								\
		{							\
		(STATS)->find		= 0;				\
		(STATS)->insert		= 0;				\
		(STATS)->remove		= 0;				\
		(STATS)->first		= 0;				\
		(STATS)->next		= 0;				\
		(STATS)->load		= 0;				\
		(STATS)->clear		= 0;				\
		(STATS)->fixup		= 0;				\
		(STATS)->total		= 0;				\
		} while (0)

struct params
	{
	unsigned int		random_seed;
	unsigned int		size;
	unsigned int		iterations;
	unsigned int		checks_desired;
	unsigned char		bad_pathe;
	unsigned char		timing;
	unsigned int		verbose;
	time_t			now;
	};


#define PARAMS_SET_DEFAULT(PBLOCK)					\
	do								\
		{							\
		(PBLOCK)->random_seed		= 0;			\
		(PBLOCK)->size			= 0;			\
		(PBLOCK)->iterations		= 0;			\
		(PBLOCK)->checks_desired	= 0;			\
		(PBLOCK)->bad_pathe		= 0;			\
		(PBLOCK)->timing		= 0;			\
		(PBLOCK)->verbose		= 0;			\
		(PBLOCK)->now			= 0;			\
		} while (0)


static int hash_init(
		hash_ctx_t*		ctx
		)
	{
	*ctx = 0;

	return 0;
	}


static int hash_clear(
		hash_t*			hash
		)
	{
	*hash = -1u;

	return 0;
	}


static int hash_update(
		hash_ctx_t*		ctx,
		hash_data_t*		data,
		size_t			count
		)
	{
	uint32_t		crc = ~*ctx;
	unsigned char*		current = (unsigned char*)data;

	static uint32_t lut[16] =
			{
			0x00000000,0x1DB71064,0x3B6E20C8,0x26D930AC,
			0x76DC4190,0x6B6B51F4,0x4DB26158,0x5005713C,
			0xEDB88320,0xF00F9344,0xD6D6A3E8,0xCB61B38C,
			0x9B64C2B0,0x86D3D2D4,0xA00AE278,0xBDBDF21C
			};

	while (count--)
		{
		crc = lut[(crc ^  *current      ) & 0x0F] ^ (crc >> 4);
		crc = lut[(crc ^ (*current >> 4)) & 0x0F] ^ (crc >> 4);
		current++;
		}

	*ctx = ~crc;

	return 0;
	}


static int hash_final(
		hash_ctx_t*		ctx,
		hash_t*			hash
		)
	{
	*hash = *ctx;
	*ctx = -1u;

	return 0;
	}


static int hash_cmp(
		hash_t*			hash0,
		hash_t*			hash1
		)
	{
	return *hash0 != *hash1;
	}


static void hash_copy(
		hash_t*			old,
		hash_t*			new
		)
	{
	*new = *old;
	}


#define CALLBACK(NODE, COVER, CBP, CBDATA, LIMITER, RES)		\
	do								\
		{							\
		if ((*CBP))						\
			{						\
			int		CBres;				\
									\
			CBres = (**(CBP))((NODE), (COVER), (CBDATA));	\
			switch (CBres)					\
				{					\
				case EAVL_CB_OK:			\
					break;				\
									\
				case EAVL_CB_FINISHED:			\
					(*CBP) = NULL;			\
					break;				\
									\
				case EAVL_CB_LIMIT:			\
					(LIMITER) = 1;			\
					break;				\
									\
				default:				\
					(*CBP) = NULL;			\
					(RES) = CBres;			\
				}					\
			}						\
		} while (0)


#define RWALK3(NODE, DIR, WANT, START, COMPARE, CB, CBDATA, RES)	\
	do								\
		{							\
		void*		RW3_start = (START);			\
		cb_t*		RW3_cb = (CB);				\
									\
		(RES) = rwalk3(						\
				(NODE),					\
				(DIR),					\
				(WANT),					\
				&RW3_start,				\
				(COMPARE),				\
				&RW3_cb,				\
				(CBDATA)				\
				);					\
		} while (0)


static int rwalk3(
		EAVLs_node_t*		node,
		EAVL_dir_t		dir,
		order_mask_t		want,
		void**			start,
		EAVLs_cbCompare_t	compare,
		cb_t**			cbp,
		void*			cbdata
		)
	{
	EAVLs_node_t*		T0;
	EAVLs_node_t*		T1;
	EAVL_dir_t		cmp = EAVL_CMP_SAME;
	int			limiter = 0;
	int			error = 0;

	if (!node || !*cbp || !(want & (MASK_PRE | MASK_IN | MASK_POST)))
		{
		return 0;
		}

	if (compare && *start)
		{
		cmp = (*compare)(*start, NULL, node, cbdata);
		}

	T0 = EAVLs_GET_CHILD(node, EAVL_DIR_OTHER(dir));
	T1 = EAVLs_GET_CHILD(node, dir);

	if (!error && *cbp && T0 && (want & MASK_PRE) && (!*start || (cmp == EAVL_CMP_SAME)))
		{
		CALLBACK(node, MASK_PRE, cbp, cbdata, limiter, error);
		*start = NULL;
		cmp = EAVL_CMP_SAME;
		}

	if (!error && *cbp && !limiter && T0)
		{
		if (!*start || ((cmp == EAVL_CMP_SAME) && (want & MASK_PRE)))
			{
			error = rwalk3(T0, dir, want, start, NULL, cbp, cbdata);
			}
		else if (cmp == dir)
			{
			error = rwalk3(T0, dir, want, start, compare, cbp, cbdata);
			}
		}

	if (!error && *cbp
			&& (
				(want & MASK_IN)
				|| (!T0 && (want & MASK_PRE))
				|| (!T1 && (want & MASK_POST))
				)
			&& (!*start || (cmp == EAVL_CMP_SAME))
			)
		{
		CALLBACK(
				node,
				want & (
					MASK_IN
					| ((T0) ? 0u : MASK_PRE)
					| ((T1) ? 0u : MASK_POST)
					),
				cbp,
				cbdata,
				limiter,
				error
				);
		*start = NULL;
		cmp = EAVL_CMP_SAME;
		}

	if (!error && *cbp && !limiter && T1)
		{
		if (!*start || ((cmp == EAVL_CMP_SAME) && (want & (MASK_PRE | MASK_IN))))
			{
			error = rwalk3(T1, dir, want, start, NULL, cbp, cbdata);
			}
		else if (cmp == EAVL_DIR_OTHER(dir))
			{
			error = rwalk3(T1, dir, want, start, compare, cbp, cbdata);
			}
		}

	if (!error && *cbp && T1 && (want & MASK_POST) && (!*start || (cmp == EAVL_CMP_SAME)))
		{
		CALLBACK(node, MASK_POST, cbp, cbdata, limiter, error);
		*start = NULL;
		cmp = EAVL_CMP_SAME;
		}

	return error;
	}


static void destroy_pathestore(
		pathestore_t*		pathestore
		)
	{
	if (pathestore->elements)
		{
		free(pathestore->elements);
		}
	free(pathestore);
	}


static pathestore_t* create_pathestore(
		unsigned char		bad
		)
	{
	pathestore_t*		store;

	if (!(store = (pathestore_t*)malloc(sizeof(pathestore_t))))
		{
		return NULL;
		}

	store->elements = NULL;
	store->count = 0;
	store->bad_pathe = bad;

	return store;
	}


static EAVLs_pathelement_t* ecb_pathe(
		unsigned int		index,
		unsigned int		param,
		void*			cbdata
		)
	{
	pathestore_t*		pathestore;

	if (!cbdata || !(pathestore = ((cbdata_t*)cbdata)->pathestore))
		{
		printf("WARNING: This indicates corruption of some sort\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		return NULL;
		}

//	printf("Pathe:: %4u  %4u  (%u)\n", index, param, pathestore->count);
	if (index == -1u)	// set size
		{
		unsigned int		count = param;
		void*			temp = NULL;

//		printf("Pathe-set_size:: %4d  %4u  (%u)\n",
//				index, param, pathestore->count);fflush(NULL);
		if (pathestore->count < param)
			{
			printf("Pathe-set_size:: %4d  %4u  (%u)\n",
					index, param, pathestore->count);
			fflush(NULL);
			exit(1);
			}

		if (!count && pathestore->elements)
			{
			free((void*)pathestore->elements);
			temp = NULL;}
		if (count)
			{
			if (pathestore->elements)
				{
				temp = realloc(
						pathestore->elements,
						sizeof(EAVLs_pathelement_t)*count
						);
				}
			else
				{
				temp = malloc(sizeof(EAVLs_pathelement_t)*count);
				}
			}
		if (!temp)
			{
			count = 0;
			}

		pathestore->elements = (EAVLs_pathelement_t*)temp;
		pathestore->count = count;

		return NULL;
		}

	if (param != 0)		// shift down 1
		{
//		printf("Pathe-shift::    %4u  %4u  (%u)\n",
//				index, param, pathestore->count);fflush(NULL);

		if (index == 0 || pathestore->count <= param || param < index)
			{
			printf("WARNING: This indicates corruption of some sort\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			exit(1);
			return NULL;
			}

		memmove(
				&pathestore->elements[index-1],
				&pathestore->elements[index],
				sizeof(EAVLs_pathelement_t)*(param-index+1)
				);

		return NULL;
		}

	if (index >= pathestore->count)
		{
		unsigned int		count = index+1;
		void*			temp;

//		printf("Pathe-expand::   %4u  %4u  (%u)\n",
//				index, param, pathestore->count);fflush(NULL);

		temp = realloc(
				pathestore->elements,
				sizeof(EAVLs_pathelement_t)*count
				);

		if (!temp)
			{
			printf("WARNING: This MAY cause a SEGFAULT!\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			exit(1);
			return NULL;
			}

		pathestore->elements = (EAVLs_pathelement_t*)temp;
		pathestore->count = count;

		if (pathestore->bad_pathe)
			{
			return NULL;
			}
		}

	return &pathestore->elements[index];
	}


static void print_pathestore(
		pathestore_t*		pathestore
		)
	{
	unsigned int		i;

	for (i=0; i<pathestore->count; i++)
		{
		printf("PathEStore::  %3u  %p\n",
				i, (void*)pathestore->elements[i]);
		}
	printf("\n");
	}


static void destroy_cbData(
		cbdata_t*		cbdata
		)
	{
	if (cbdata->pathestore)
		{
		destroy_pathestore(cbdata->pathestore);
		}

	free(cbdata);
	}


static cbdata_t* create_cbData(
		params_t*		params
		)
	{
	cbdata_t*		cbdata;

	if (!(cbdata = (cbdata_t*)malloc(sizeof(cbdata_t))))
		{
		return NULL;
		}

	CBDATA_INIT(cbdata);

	if (!(cbdata->pathestore = create_pathestore(params->bad_pathe)))
		{
		destroy_cbData(cbdata);
		return NULL;
		}

	cbdata->params = params;

	return cbdata;
	}


static int node_track(
		EAVLs_node_t*		node,
		tree_track_t*		tracker
		)
	{
	unsigned int		k;

	k = container_of(node, node_t, node)->key;
	if (tracker->presence[k] & BIT_PRESENT)
		{
		printf("ERROR: node_track: already tracked\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	tracker->presence[k] |= BIT_PRESENT;
	tracker->position[k] = tracker->count;
	tracker->shuffle[tracker->count] = node;
	tracker->count++;
	tracker->tracker->total++;

	return 0;
	}


static int node_untrack(
		EAVLs_node_t*		node,
		tree_track_t*		tracker
		)
	{
	unsigned int		k;
	unsigned int		p;

	k = container_of(node, node_t, node)->key;
	if (!(tracker->presence[k] & BIT_PRESENT))
		{
		printf("ERROR: node_untrack: was not tracked\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	tracker->presence[k] &= (unsigned char)~BIT_PRESENT;

	p = tracker->position[k];
	if (container_of(tracker->shuffle[p], node_t, node)->key != k)
		{
		printf("ERROR: node_untrack: key and shuffle location mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	tracker->shuffle[p] = tracker->shuffle[tracker->count-1];
	tracker->position[container_of(tracker->shuffle[p], node_t, node)->key] = p;
	tracker->count--;
	tracker->tracker->total--;

	return 0;
	}


static void print_recent(
		tree_track_t*		tracker
		)
	{
	printf("Recent::  %p->%u  pathlen:%u\n",
			(void*)tracker->context.recent,
			(tracker->context.recent) ? container_of(tracker->context.recent, node_t, node)->key : -1u,
			tracker->context.pathlen
			);
	}


static void tree_print(
		char*			prefix,
		char*			end,
		char*			prefix2,
		int			dir,
		EAVLs_node_t*		root
		)
	{
	node_t*			T;
	char*			last;
	char*			nend;

	if (!root)
		{
		printf("%s%c---(nil)\n", prefix, TREE_PRINT_STR_BEND[dir]);

		return;
		}

	T = container_of(root, node_t, node);

	nend = stpcpy(end, prefix2);

	if (root && EAVLs_GET_CHILD(root, EAVL_DIR_RIGHT))
		{
		if (dir != EAVL_DIR_LEFT)
			{
			last = TREE_PRINT_STR_SAME;
			}
		else
			{
			last = TREE_PRINT_STR_DIFF;
			}
		tree_print(
				prefix,
				nend,
				last,
				EAVL_DIR_RIGHT,
				EAVLs_GET_CHILD(root, EAVL_DIR_RIGHT)
				);
		printf("%s%s%s\n", prefix, last, TREE_PRINT_STR_DIFF);
		}

	printf("%s%c---(%u)",
			prefix, TREE_PRINT_STR_BEND[dir],
			T->key);
//	printf("[a:%p  L:%p  R:%p  r:%lu  b:%u]",
//			(void *)root,
//			(void *)EAVLs_GET_CHILD(root, 0),
//			(void *)EAVLs_GET_CHILD(root, 1),
//			EAVLs_GET_BAL(root)
//			);
	printf("[a:%p b:%u]", (void*)root, EAVLs_GET_BAL(root));
	printf("[h:%u w:%u v:%u s:%u H:0x%08X]",
			T->height,
			T->weight,
			T->value,
			T->sum,
			T->hash
			);
	printf("\n");

	if (root && EAVLs_GET_CHILD(root, EAVL_DIR_LEFT))
		{
		if (dir != EAVL_DIR_RIGHT)
			{
			last = TREE_PRINT_STR_SAME;
			}
		else
			{
			last = TREE_PRINT_STR_DIFF;
			}
		printf("%s%s%s\n", prefix, last, TREE_PRINT_STR_DIFF);
		tree_print(
				prefix,
				nend,
				last,
				EAVL_DIR_LEFT,
				EAVLs_GET_CHILD(root, EAVL_DIR_LEFT)
				);
		}

	*end = '\0';
	}


static void print_tree(
		EAVLs_node_t*		root
		)
	{
	char*			buffer;

	buffer = malloc(65536);

	printf("\n");
	*buffer = '\0';
	tree_print(buffer, buffer, "", EAVL_DIR_NEITHER, root);
	printf("\n");

	free(buffer);
	}


static EAVL_dir_t ecb_cmp(
		void*			ref_value,
		EAVLs_node_t*		ref_node,
		EAVLs_node_t*		node,
		void*			cbdata
		)
	{
	unsigned int *valp = &container_of(node, node_t, node)->key;
	unsigned int *refp = (unsigned int *)ref_value;

	QUIET_UNUSED(cbdata);

	if (ref_node)
		{
		refp = &container_of(ref_node, node_t, node)->key;
		}

	return (*valp == *refp)
			? EAVL_CMP_SAME
			: (*valp < *refp)
				? EAVL_CMP_LEFT
				: EAVL_CMP_RIGHT
			;
	}


#define CBNODEPRINT(NODE, PREFIX)					\
	do								\
		{							\
		if ((NODE))						\
			{						\
			printf("%s%5d  %p", (PREFIX),			\
				container_of((NODE), node_t, node)->key,	\
				(void*)(NODE)				\
				);					\
			}						\
		else							\
			{						\
			printf("%s     ", (PREFIX));			\
			}						\
		} while (0)


#define CBNODEPRINTHW(NODE)						\
	do								\
		{							\
		printf("(%d:%d:%d:%d)",					\
			((NODE)						\
				? container_of((NODE), node_t, node)->height	\
				: 0					\
				),					\
			((NODE)						\
				? container_of((NODE), node_t, node)->weight	\
				: 0					\
				),					\
			((NODE)						\
				? container_of((NODE), node_t, node)->sum	\
				: 0					\
				),					\
			((NODE)						\
				? container_of((NODE), node_t, node)->count	\
				: 0					\
				)					\
			);						\
		} while (0)


static int ecb_verify(
		EAVLs_node_t*		eavl_node,
		EAVLs_node_t*		childL,
		EAVLs_node_t*		childR,
		void*			cbdata
		)
	{
	hash_t			hash = 0;
	hash_ctx_t		hctx;

//	printf("Verify::  ");
//	CBNODEPRINT(eavl_node, "");
//	CBNODEPRINTHW(eavl_node);
//	CBNODEPRINT(childL, "  L:");
//	CBNODEPRINTHW(childL);
//	CBNODEPRINT(childR, "  R:");
//	CBNODEPRINTHW(childR);
//	printf("\n");

	if (container_of(eavl_node, node_t, node)->height !=
			1 + MAX(
				((childL)
					? container_of(childL, node_t, node)->height
					: 0
					),
				((childR)
					? container_of(childR, node_t, node)->height
					: 0
					)
				)
			)
		{
printf("%s:%u\n", __FILE__, __LINE__);fflush(NULL);
		return EAVL_ERROR;
		}
	if (container_of(eavl_node, node_t, node)->weight !=
			1
			+ ((childL)
				? container_of(childL, node_t, node)->weight
				: 0
				)
			+ ((childR)
				? container_of(childR, node_t, node)->weight
				: 0
				)
			)
		{
printf("%s:%u\n", __FILE__, __LINE__);fflush(NULL);
		return EAVL_ERROR;
		}
	if (container_of(eavl_node, node_t, node)->sum !=
			1
			+ container_of(eavl_node, node_t, node)->value
			+ ((childL)
				? container_of(childL, node_t, node)->sum
				: 0
				)
			+ ((childR)
				? container_of(childR, node_t, node)->sum
				: 0
				)
			)
		{
printf("%s:%u\n", __FILE__, __LINE__);fflush(NULL);
		return EAVL_ERROR;
		}
	if (!((cbdata_t*)cbdata)->params->timing)
		{
		hash_init(&hctx);
		hash_update(
				&hctx,
				&container_of(eavl_node, node_t, node)->key,
				sizeof(unsigned int)
				);
		hash_update(
				&hctx,
				&container_of(eavl_node, node_t, node)->value,
				sizeof(unsigned int)
				);
		hash_update(
				&hctx,
				(childL)
					? &container_of(childL, node_t, node)->hash
					: &hash,
				sizeof(hash_t)
				);
		hash_update(
				&hctx,
				(childR)
					? &container_of(childR, node_t, node)->hash
					: &hash,
				sizeof(hash_t)
				);
		hash_final(&hctx, &hash);
		if (hash_cmp(&container_of(eavl_node, node_t, node)->hash, &hash))
			{
printf("%s:%u\n", __FILE__, __LINE__);fflush(NULL);
			return EAVL_ERROR;
			}
		}

	return EAVL_CB_OK;
	return EAVL_CB_FINISHED;
	}


static int ecb_fixup(
		EAVLs_node_t*		eavl_node,
		EAVLs_node_t*		childL,
		EAVLs_node_t*		childR,
		void*			cbdata
		)
	{
	node_t			temp_node;
	hash_t			hash = 0;
	hash_ctx_t		hctx;

	NODE_INIT(&temp_node);

//	printf("ecb_fixup::  ");
//	CBNODEPRINT(eavl_node, "");
//	CBNODEPRINTHW(eavl_node);
//	CBNODEPRINT(childL, "  L:");
//	CBNODEPRINTHW(childL);
//	CBNODEPRINT(childR, "  R:");
//	CBNODEPRINTHW(childR);
//	printf("\n");

	temp_node.height =
			1 + MAX(
				((childL)
					? container_of(childL, node_t, node)->height
					: 0
					),
				((childR)
					? container_of(childR, node_t, node)->height
					: 0
					)
				)
			;
	temp_node.weight =
			1
			+ ((childL)
				? container_of(childL, node_t, node)->weight
				: 0
				)
			+ ((childR)
				? container_of(childR, node_t, node)->weight
				: 0
				)
			;
	temp_node.sum =
			1
			+ container_of(eavl_node, node_t, node)->value
			+ ((childL)
				? container_of(childL, node_t, node)->sum
				: 0
				)
			+ ((childR)
				? container_of(childR, node_t, node)->sum
				: 0
				)
			;
	if (!((cbdata_t*)cbdata)->params->timing)
		{
		hash_init(&hctx);
		hash_update(
				&hctx,
				&container_of(eavl_node, node_t, node)->key,
				sizeof(unsigned int)
				);
		hash_update(
				&hctx,
				&container_of(eavl_node, node_t, node)->value,
				sizeof(unsigned int)
				);
		hash_update(
				&hctx,
				(childL)
					? &container_of(childL, node_t, node)->hash
					: &hash,
				sizeof(hash_t)
				);
		hash_update(
				&hctx,
				(childR)
					? &container_of(childR, node_t, node)->hash
					: &hash,
				sizeof(hash_t)
				);
		hash_final(
				&hctx,
				&temp_node.hash
				);
		}

	if (container_of(eavl_node, node_t, node)->height == temp_node.height
			&& container_of(eavl_node, node_t, node)->weight == temp_node.weight
			&& container_of(eavl_node, node_t, node)->sum == temp_node.sum
			&& (((cbdata_t*)cbdata)->params->timing || !hash_cmp(&container_of(eavl_node, node_t, node)->hash, &temp_node.hash))
			)
		{
		return EAVL_CB_FINISHED;
		}

	container_of(eavl_node, node_t, node)->height = temp_node.height;
	container_of(eavl_node, node_t, node)->weight = temp_node.weight;
	container_of(eavl_node, node_t, node)->sum = temp_node.sum;
	hash_copy(&temp_node.hash, &container_of(eavl_node, node_t, node)->hash);

	return EAVL_CB_OK;
	}


static int ecb_release(
		EAVLs_node_t*		node,
		void*			cbdata
		)
	{
//	printf("Release::  ");
//	CBNODEPRINT(node, "");
//	printf("\n");

	node_untrack(node, ((cbdata_t*)cbdata)->tracker);
	NODE_CLEAR(container_of(node, node_t, node));

	free(container_of(node, node_t, node));

	return EAVL_CB_OK;
	}


static int cb_verify(
		EAVLs_node_t*		node,
		order_mask_t		cover,
		void*			cbdata
		)
	{
	tree_track_t*		tracker;
	EAVLs_node_t*		childL;
	EAVLs_node_t*		childR;
	EAVLs_cbCompare_t	compare;
	unsigned int		i;

	QUIET_UNUSED(cover);

	tracker = ((cbdata_t*)cbdata)->tracker;

	i = tracker->position[container_of(node, node_t, node)->key];
	if (i >= tracker->count)
		{
		printf("ERROR: cb_verify: position >= count\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	if (tracker->shuffle[i] != node)
		{
		printf("ERROR: cb_verify: shuffle[position] != node\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	i = container_of(node, node_t, node)->key;
	if (!(tracker->presence[i] & BIT_PRESENT))
		{
		printf("ERROR: cb_verify: !presence[key]\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	childL = EAVLs_GET_CHILD(node, EAVL_DIR_LEFT);
	childR = EAVLs_GET_CHILD(node, EAVL_DIR_RIGHT);
	compare = tracker->tracker->cbset.compare;

	if ((childL && (*compare)(NULL, node, childL, cbdata) != EAVL_CMP_LEFT)
			|| (childR && (*compare)(NULL, node, childR, cbdata) != EAVL_CMP_RIGHT)
			)
		{
		printf("ERROR: cb_verify: Improper node relationship\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	return ecb_verify(node, childL, childR, cbdata);
	}


static int tree_verify(
		tree_track_t*		tracker,
		params_t*		params
		)
	{
	cbdata_t*		cbdata;
	int			error;

	if (!tracker)
		{
		return -1;
		}

	if (!(cbdata = create_cbData(params)))
		{
		printf("create_cbData() FAILED\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}
	cbdata->tracker = tracker;

//	RWALK2(EAVLs_TREE_ROOT(&tracker->tree), EAVL_DIR_RIGHT, MASK_PRE, &cb_verify, cbdata, error);
	RWALK3(EAVLs_TREE_ROOT(&tracker->tree), EAVL_DIR_RIGHT, MASK_PRE, NULL, NULL, &cb_verify, cbdata, error);

	destroy_cbData(cbdata);

//	print_tree(EAVLs_TREE_ROOT(&tracker->tree));

	return error;
	}


static node_t* new_node(
		unsigned int		key,
		tree_track_t*		tracker,
		params_t*		params
		)
	{
	node_t*			node;

	QUIET_UNUSED(tracker);
	QUIET_UNUSED(params);

	if (!(node = malloc(sizeof(node_t))))
		{
		return NULL;
		}

	NODE_INIT(node);
	node->key = key;

	return node;
	}


static int get_uint(
		unsigned int*		uintp,
		char*			str
		)
	{
	unsigned long int	t;
	char*			endc;

	errno = 0;
	t = strtoul(str, &endc, 0);

	if (errno || (endc == str) || (*endc != '\0'))
		{
		return 1;
		}

	*uintp = (unsigned int)t;
	return 0;
	}


#define FREE_NONNULL(POINTER)						\
	do								\
		{							\
		if ((POINTER))						\
			{						\
			free((POINTER));				\
			}						\
		} while (0)


static void destroy_mtrack(
		master_track_t*		mtrack
		)
	{
	FREE_NONNULL(mtrack->nodep);
	FREE_NONNULL(mtrack->tracker.presence);
	FREE_NONNULL(mtrack->tracker.shuffle);
	FREE_NONNULL(mtrack->tracker.position);
	FREE_NONNULL(mtrack->tracker.order);

	if (mtrack->tracker.cbdata)
		{
		destroy_cbData(mtrack->tracker.cbdata);
		}

	MASTER_TRACK_CLEAR(mtrack);

	free(mtrack);
	}


static master_track_t* create_mtrack(
		params_t*		params
		)
	{
	master_track_t*		mtrack;
	int			error;

	if (!(mtrack = malloc(sizeof(master_track_t))))
		{
		return NULL;
		}
	MASTER_TRACK_CLEAR(mtrack);

	mtrack->total = 0;

	mtrack->cbset.compare	= &ecb_cmp;
	mtrack->cbset.fixup	= &ecb_fixup;
	mtrack->cbset.verify	= &ecb_verify;

	mtrack->tracker.tracker = mtrack;
	mtrack->tracker.count = 0;

	if (!(mtrack->nodep = malloc(params->size*sizeof(EAVLs_node_t*)))
			|| !(mtrack->tracker.presence	= malloc(params->size))
			|| !(mtrack->tracker.shuffle	= malloc(params->size*sizeof(EAVLs_node_t*)))
			|| !(mtrack->tracker.position	= malloc(params->size*sizeof(unsigned int)))
			|| !(mtrack->tracker.order	= malloc(params->size*sizeof(EAVLs_node_t*)))
			)
		{
		printf("ERROR: malloc:\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		destroy_mtrack(mtrack);
		return NULL;
		}

	memset(mtrack->tracker.presence, 0, params->size);

	if ((error = EAVLs_Tree_Init(&mtrack->tracker.tree, NULL, &mtrack->cbset)))
		{
		printf("ERROR: Tree_Init: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		destroy_mtrack(mtrack);
		return NULL;
		}

	if (!(mtrack->tracker.cbdata = create_cbData(params)))
		{
		printf("ERROR: create_cbDatat\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		destroy_mtrack(mtrack);
		return NULL;
		}

	mtrack->tracker.cbdata->tracker = &mtrack->tracker;

	if ((error = EAVLs_Context_Init(&mtrack->tracker.context, &ecb_pathe, mtrack->tracker.cbdata)))
		{
		printf("ERROR: Context_Init: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		destroy_mtrack(mtrack);
		return NULL;
		}

	if ((error = EAVLs_Context_Associate(&mtrack->tracker.context, &mtrack->tracker.tree)))
		{
		printf("ERROR: Context_Associate: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		destroy_mtrack(mtrack);
		return NULL;
		}

	return mtrack;
	}


static int check_tree_track(
		master_track_t*		mtrack,
		tree_track_t*		tracker,
		params_t*		params
		)
	{
	EAVLs_node_t*		root;
	unsigned int		i;

	if (mtrack != tracker->tracker)
		{
		printf("ERROR: master_track mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	if (&tracker->tree != EAVLs_CONTEXT_TREE(&tracker->context))
		{
		printf("ERROR: context tree mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	if (tracker->cbdata != tracker->context.common.cbdata)
		{
		printf("ERROR: context cbdata mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	root = EAVLs_TREE_ROOT(&tracker->tree);
	if ((root && !tracker->count) || (!root && tracker->count))
		{
		printf("ERROR: tree root and tracker count mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	for (i=0; i<params->size; i++)
		{
		tracker->presence[i] &= BIT_PRESENT;
		}
	for (i=0; i<tracker->count; i++)
		{
		unsigned int		k;

		if (!tracker->shuffle[i])
			{
			printf("ERROR: NULL shuffle position\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		k = container_of(tracker->shuffle[i], node_t, node)->key;
		if (k >= params->size)
			{
			printf("ERROR: node key >= size: %u\n", k);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		if (i != tracker->position[k])
			{
			printf("ERROR: shuffle position mismatch\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		if (!(tracker->presence[k] & BIT_PRESENT))
			{
			printf("ERROR: BIT_PRESENT mismatch\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		if (tracker->presence[k] & BIT_PROCESSED)
			{
			printf("ERROR: shuffle entries not unique\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		tracker->presence[k] |= BIT_PROCESSED;
		}

	return 0;
	}


static int check_mtrack(
		master_track_t*		mtrack,
		params_t*		params
		)
	{
	int			error;

	if ((error = check_tree_track(mtrack, &mtrack->tracker, params)))
		{}

	return error;
	}


#define CBTT_PRINT_KEY(NODE, COVERED)					\
	do								\
		{							\
		if ((COVERED))						\
			{						\
			printf("  %5u",	container_of((NODE), node_t, node)->key);	\
			}						\
		else							\
			{						\
			printf("%10s", "");				\
			}						\
		} while (0)


static int cb_enumerate(
		EAVLs_node_t*		node,
		order_mask_t		cover,
		void*			cbdata
		)
	{
	tree_track_t*		tracker;

	QUIET_UNUSED(cover);

//	printf("found[%u]: %u  0x%x\n", __LINE__, container_of(node, node_t, node)->key, cover);

	tracker = ((cbdata_t*)cbdata)->tracker;

	if (tracker->found >= tracker->count)
		{
		printf("ERROR: found > count\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	if (tracker->order[tracker->found++] != node)
		{
		printf("ERROR: enumeration mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	return EAVL_OK;
	}


static int test_enumerate(
		master_track_t*		mtrack,
		stats_t*		stats,
		params_t*		params
		)
	{
	tree_track_t*		tracker;
	EAVLs_context_t*	context;
	EAVLs_node_t*		node;
	EAVL_dir_t		dir;
	EAVL_order_t		order;
	order_mask_t		want;
	void*			cbdata;
	unsigned int		f;
	unsigned int		i;
	int			error;

	if (params->verbose > 2)
		{
		printf("### test_enumerate::\n");
		}

	tracker = &mtrack->tracker;
	context = &tracker->context;
	cbdata = tracker->cbdata;

	i = (unsigned int)random();
	dir = (i & 0x1) ? EAVL_DIR_RIGHT : EAVL_DIR_LEFT;
	i >>= 1;
	switch (i & 0x3)
		{
		case 0:
			order = EAVL_ORDER_PRE;
			want = MASK_PRE;
			break;

		case 1:
			order = EAVL_ORDER_POST;
			want = MASK_POST;
			break;

		default:
			order = EAVL_ORDER_IN;
			want = MASK_IN;
			/* break; */
		}
	i >>= 2;
	i &= 0x3;

	if (params->verbose > 3)
		{
		printf("\td:%u o:%o m:%u  f:%u\n", dir, order, want, !i);
		}

	if (!i)
		{
		f = 0;
		do
			{
			error = EAVLs_First(context, dir, order, &node);
			} while (error == EAVL_CALLBACK);
		stats->first++;
		stats->total++;
		while (error == EAVL_OK)
			{
			tracker->order[f++] = node;
			if (params->verbose > 4)
				{
				printf("found[%u}: %u\n", __LINE__, container_of(node, node_t, node)->key);
				}

			do
				{
				error = EAVLs_Next(context, dir, order, &node);
				} while (error == EAVL_CALLBACK);
			stats->next++;
			stats->total++;
			}
		if (error != EAVL_NOTFOUND)
			{
			printf("ERROR: Node_%s: %d\n", (f) ? "Next" : "First", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		error = EAVL_OK;
		if (f != tracker->count)
			{
			printf("ERROR: test_enumerate: found != count\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}

		if (!params->timing)
			{
			tracker->found = 0;
			RWALK3(EAVLs_TREE_ROOT(&tracker->tree), dir, want, NULL, NULL, &cb_enumerate, cbdata, error);
			if (error)
				{
				printf("ERROR: rwalk: %d\n", error);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				return -1;
				}
			if (tracker->found != f)
				{
				printf("ERROR: test_enumerate: found(%u) != First_Next(%u)\n", tracker->found, f);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				return -1;
				}
			}
		}
	else
		{
		if (tracker->count)
			{
			i = (unsigned int)random()%tracker->count;
			i = container_of(tracker->shuffle[i], node_t, node)->key;
			}
		else
			{
			i = -2u;
			}
		if (params->verbose > 3)
			{
			printf("\ti:%u\n", i);
			}

		f = 0;
		do
			{
			error = EAVLs_Find(context, EAVL_FIND_EQ, NULL, &i, NULL, &node);
			} while (error == EAVL_CALLBACK);
		stats->find++;
		stats->total++;
		while (error == EAVL_OK)
			{
			tracker->order[f++] = node;
			if (params->verbose > 4)
				{
				printf("found[%u]: %u\n", __LINE__, container_of(node, node_t, node)->key);
				}

			do
				{
				error = EAVLs_Next(context, dir, order, &node);
				} while (error == EAVL_CALLBACK);
			stats->next++;
			stats->total++;
			}
		if (error != EAVL_NOTFOUND)
			{
			printf("ERROR: Node_%s: %d\n", (tracker->found) ? "Next" : "First", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		error = EAVL_OK;

		if (!params->timing)
			{
			tracker->found = 0;
			RWALK3(EAVLs_TREE_ROOT(&tracker->tree), dir, want, &i, mtrack->cbset.compare, &cb_enumerate, cbdata, error);
			if (error)
				{
				printf("ERROR: rwalk: %d\n", error);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				return -1;
				}
			if (tracker->found != f)
				{
				printf("ERROR: test_enumerate: found(%u) != Find_Next(%u)\n", tracker->found, f);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				return -1;
				}
			}
		}

	return error;
	}


static int test_insertdelete(
		master_track_t*		mtrack,
		stats_t*		stats,
		params_t*		params
		)
	{
	tree_track_t*		tracker;
	EAVLs_context_t*	context;
	EAVLs_node_t*		node;
	EAVLs_node_t*		dummy;
	node_t*			T;
	unsigned int		i;
	int			error;

	if (params->verbose > 2)
		{
		printf("### test_insertdelete::\n");
		}

	tracker = &mtrack->tracker;
	context = &tracker->context;

	i = (unsigned int)random() % params->size;

	do
		{
		error = EAVLs_Find(context, EAVL_FIND_EQ, NULL, &i, NULL, &node);
		} while (error == EAVL_CALLBACK);
	stats->find++;
	stats->total++;
	if (error != EAVL_OK && error != EAVL_NOTFOUND)
		{
		printf("ERROR: Node_Find: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

//printf("%u\n", i);fflush(NULL);
	if (((tracker->presence[i] & BIT_PRESENT) && error != EAVL_OK)
			|| (!(tracker->presence[i] & BIT_PRESENT) && error == EAVL_OK)
			)
		{
		printf("ERROR: Node_Find result and presence mismatch\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	if (error == EAVL_NOTFOUND)
		{
		if (params->verbose > 3)
			{
			printf("\tinsert: %u\n", i);
			}
		if (!(T = new_node(i, tracker, params)))
			{
			printf("ERROR: new_node FAILED\n");
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}
		do
			{
			error = EAVLs_Insert(context, &T->node, &dummy);
			} while (error == EAVL_CALLBACK);
		stats->insert++;
		stats->total++;
		if (error != EAVL_OK)
			{
			printf("ERROR: Node_Insert: %d\n", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}

		node_track(&T->node, tracker);
		}
	else
		{
		if (params->verbose > 3)
			{
			printf("\tremove: %u\n", container_of(node, node_t, node)->key);
			}
		do
			{
			error = EAVLs_Remove(context, &node);
			} while (error == EAVL_CALLBACK);
		stats->remove++;
		stats->total++;
		if (error != EAVL_OK)
			{
			printf("ERROR: Node_Remove: %d\n", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			return -1;
			}

		node_untrack(node, tracker);
		NODE_CLEAR(container_of(node, node_t, node));
		free(container_of(node, node_t, node));
		}

	return error;
	}


static int test_update(
		master_track_t*		mtrack,
		stats_t*		stats,
		params_t*		params
		)
	{
	tree_track_t*		tracker;
	EAVLs_context_t*	context;
	EAVLs_node_t*		node;
	unsigned int		i;
	int			error;

	if (params->verbose > 2)
		{
		printf("### test_update::\n");
		}

	tracker = &mtrack->tracker;
	context = &tracker->context;

	i = (unsigned int)random() % tracker->count;
	i = container_of(tracker->shuffle[i], node_t, node)->key;

//printf("%u\n", i);fflush(NULL);
	do
		{
		error = EAVLs_Find(context, EAVL_FIND_EQ, NULL, &i, NULL, &node);
		} while (error == EAVL_CALLBACK);
	if (error != EAVL_OK)
		{
		printf("ERROR: Node_Find(%u): %d\n", i, error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	stats->find++;
	stats->total++;

	container_of(node, node_t, node)->value++;
	do
		{
		error = EAVLs_Fixup(context);
		} while (error == EAVL_CALLBACK);
	if (error != EAVL_OK)
		{
		printf("ERROR: Node_Fixup: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}
	stats->fixup++;
	stats->total++;

	return error;
	}


static int test_iterate(
		stats_t*		stats,
		params_t*		params
		)
	{
	master_track_t*		mtrack;
	unsigned int		i;
	unsigned int		r;
	int			error = 0;

	srandom(params->random_seed);

	if (!(mtrack = create_mtrack(params)))
		{
		printf("ERROR: create_mtrack\n");
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	if ((error = check_mtrack(mtrack, params)))
		{
		printf("ERROR: check_mtrack: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		destroy_mtrack(mtrack);
		return -1;
		}

	for (i=0; i<params->iterations; i++)
		{
		r = (unsigned int)random() & ((mtrack->tracker.count) ? 0x1f : 0x07);
//		r = (unsigned int)random() & (0x07);
		switch(r)
			{
			case 0:
			case 1:
			case 2:
//				print_tree(EAVLs_TREE_ROOT(&mtrack->tracker.tree));
				if ((error = test_enumerate(mtrack, stats, params)))
					{
					printf("ERROR: test_enumerate: %d\n", error);
					printf("\t%s:%u\n", __FILE__, __LINE__);
					print_tree(EAVLs_TREE_ROOT(&mtrack->tracker.tree));
					print_recent(&mtrack->tracker);
					print_pathestore(mtrack->tracker.cbdata->pathestore);
					destroy_mtrack(mtrack);
					return error;
					}
				break;

			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
//				print_tree(EAVLs_TREE_ROOT(&mtrack->tracker.tree));
				if ((error = test_insertdelete(mtrack, stats, params)))
					{
					printf("ERROR: test_insertdelete: %d\n", error);
					printf("\t%s:%u\n", __FILE__, __LINE__);
					print_tree(EAVLs_TREE_ROOT(&mtrack->tracker.tree));
					print_recent(&mtrack->tracker);
					print_pathestore(mtrack->tracker.cbdata->pathestore);
					destroy_mtrack(mtrack);
					return error;
					}
//				print_tree(EAVLs_TREE_ROOT(&mtrack->tracker.tree));
				break;

			default:
				if ((error = test_update(mtrack, stats, params)))
					{
					printf("ERROR: test_update: %d\n", error);
					printf("\t%s:%u\n", __FILE__, __LINE__);
//					print_tree(EAVLs_TREE_ROOT(&mtrack->tracker.tree));
					destroy_mtrack(mtrack);
					return error;
					}
				/* break; */
			}

		if (!params->timing)
			{
			if ((error = tree_verify(&mtrack->tracker, params)))
				{
				printf("ERROR: tree_verify: %d\n", error);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				destroy_mtrack(mtrack);
				return error;
				}
			}
		if (params->verbose)
			{
			time_t			now;

			now = time(NULL);
			if (now != params->now || i == (params->iterations-1) || params->verbose > 1)
				{
				params->now = now;
				printf("%10u %9u %9u %9u %9u %9u %9u %9u%c",
						i+1,
						mtrack->tracker.count,
						mtrack->total,
						mtrack->tracker.count - mtrack->total,
						stats->find,
						stats->fixup,
						stats->insert,
						stats->remove,
						(params->verbose > 1) ? '\n' : '\r'
						);
				fflush(NULL);
				}
			}

		if ((error = check_mtrack(mtrack, params)))
			{
			printf("ERROR: check_mtrack: %d\n", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			destroy_mtrack(mtrack);
			return -1;
			}
		}

	do
		{
		error = EAVLs_Clear(&mtrack->tracker.context, &ecb_release);
		} while (error == EAVL_CALLBACK);
	if (error != EAVL_OK)
		{
		printf("ERROR: Tree_Clear: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	if (mtrack->total)
		{
		printf("ERROR: Un-free'd nodes: %d\n", mtrack->total);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		return -1;
		}

	destroy_mtrack(mtrack);
	return error;
	}


static void usage(
		char*			progname
		)
	{
	printf("Usage: %s [-h] [-P | -p] [-t] [-v]\n"
			"\t\t[-c <checks mask>] [-r <seed>]\n"
			"\t\t<size> <iterations>\n", progname);
	}


#define GET_UINT(VAR, OPTARG, ARGV)					\
	do								\
		{							\
		unsigned int		GTt;				\
									\
		if (get_uint((&GTt), (OPTARG)))				\
			{						\
			usage(*(ARGV));					\
			exit(1);					\
			}						\
		(VAR) = GTt;						\
		} while (0)


int main(int argc, char **argv)
	{
	params_t		params;
	stats_t			stats;
	int			c;
	int			result = 0;

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	STATS_INIT(&stats);

	PARAMS_SET_DEFAULT(&params);

	opterr = 0;
	while ((c = getopt(argc, argv, ":hPptvc:r:")) != -1)
		{
		switch(c)
			{
			case 'h':
				usage(*argv);
				exit(0);
				break;

			case 'P':
				params.bad_pathe = 0;
				break;

			case 'p':
				params.bad_pathe = 1;
				break;

			case 't':
				params.timing = 1;
				break;

			case 'v':
				params.verbose++;
				break;

			case 'c':
				GET_UINT(params.checks_desired, optarg, argv);
				break;

			case 'r':
				GET_UINT(params.random_seed, optarg, argv);
				break;

			default:
				usage(*argv);
				exit(1);
			}
		}

	if (optind+2 != argc)
		{
		usage(*argv);
		exit(1);
		}

	GET_UINT(params.size, argv[optind++], argv);
	GET_UINT(params.iterations, argv[optind++], argv);

	if (params.verbose)
		{
		printf("Size:              %8u\n", params.size);
		printf("Iterations:        %8u\n", params.iterations);
		printf("Checks avail:        0x%04x\n", EAVLs_Checks_Available);
		printf("Checks enabled:      0x%04x\n", EAVLs_Checks_Available & params.checks_desired);
		printf("Random seed:     0x%08x\n", params.random_seed);
		printf("Bad pathe:         %8s\n",
				(params.bad_pathe) ? "TRUE" : "FALSE");
		printf("Timing:            %8s\n",
				(params.timing) ? "TRUE" : "FALSE");
		printf("Verbose:           %8s(%u)\n",
				(params.verbose) ? "TRUE" : "FALSE", params.verbose);
		printf("\n");
		}

	EAVLs_Checks_Enabled = EAVLs_Checks_Available & params.checks_desired;

	result = test_iterate(&stats, &params);

	if (params.verbose == 1)
		{
		printf("\n");
		}

	printf("\n");
	printf("find:    %8u\n", stats.find);
	printf("insert:  %8u\n", stats.insert);
	printf("remove:  %8u\n", stats.remove);
	printf("first:   %8u\n", stats.first);
	printf("next:    %8u\n", stats.next);
	printf("load:    %8u\n", stats.load);
	printf("clear:   %8u\n", stats.clear);
	printf("fixup:   %8u\n", stats.fixup);
	printf("\n");
	printf("total:   %8u\n", stats.total);

	return result;
	}
