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


#include <stddef.h>

#include "EAVL_cTree.h"

#define CHECKS_AVAILABLE	EAVLc_CHECKS_AVAILABLE

#include "cTree.h"
#include "cTree_internal.h"
#include "cTree_traverse.h"

#include "callback_internal.h"
#include "checks_internal.h"
#include "context_internal.h"
#include "eavl_internal.h"
#include "naming_internal.h"
#include "pathe_internal.h"
#include "treeload_internal.h"


int PUBLIC(Tree_Init)(
		EAVLc_tree_t*		tree,
		EAVLc_tree_t*		existing,
		EAVLc_cbset_t*		cbset
		)
	{
	CHECK_PARAM_NON_NULL(tree);

	if (existing && existing->cbset
			&& existing->cbset->compare
			&& existing->cbset->dup
			)
		{
		tree->cbset = existing->cbset;
		tree->root = existing->root;
		REFS_INC(existing->root);
		}
	else if (cbset && cbset->compare && cbset->dup)
		{
		tree->cbset = cbset;
		tree->root = NULL;
		}
	else
		{
		return EAVL_ERROR_PARAMETER;
		}

	tree->common.contexts = NULL;
	tree->common.associations = 0;

	return EAVL_OK;
	}


#define NODE_DUP(NODE, PARENT, CB, CBDATA)				\
	do								\
		{							\
		if (GET_REFS((NODE)))					\
			{						\
			EAVLc_node_t*		ND_T0;			\
			EAVLc_node_t*		ND_T1;			\
			EAVL_dir_t		ND_bal;			\
									\
			CB_DUP((NODE), (CB), (CBDATA), ND_T0);		\
			NODE_INIT(ND_T0);				\
			SET_CHILD((PARENT), ND_T0, SID((PARENT), (NODE)));	\
			REFS_DEC((NODE));				\
									\
			ND_T1 = GET_CHILD((NODE), 0);			\
			SET_CHILD(ND_T0, ND_T1, 0);			\
			REFS_INC(ND_T1);				\
			ND_T1 = GET_CHILD((NODE), 1);			\
			SET_CHILD(ND_T0, ND_T1, 1);			\
			REFS_INC(ND_T1);				\
									\
			ND_bal = GET_BAL((NODE));			\
			SET_BAL(ND_T0, ND_bal);				\
									\
			(NODE) = ND_T0;					\
			}						\
		} while (0)


static int PRIVATE(tree_split)(
		EAVLc_node_t**		rootp,
		EAVLc_node_t**		targetp,
		unsigned int		pathlen,
		EAVLc_cbDup_t		dup,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata
		)
	{
	if (pathlen && GET_REFS(*rootp))
		{
		EAVLc_node_t		fakeroot;

		NODE_INIT(&fakeroot);
		SET_CHILD(&fakeroot, *rootp, DIR_LEFT);

		NODE_DUP(*rootp, &fakeroot, dup, cbdata);
		if (pathlen == 1)
			{
			*targetp = *rootp;
			}
		else
			{
			PATHE_SET_SAFE(1, cbpathe, cbdata, *rootp);
			}
		}

	if (pathlen > 1)
		{
		EAVLc_node_t*		curr;
		EAVLc_node_t*		prev;
		unsigned int		pathpos = 2;

		curr = *rootp;
		while (pathpos < pathlen)
			{
			prev = curr;
			PATHE_GET_SAFE(pathpos, cbpathe, cbdata, curr);
			NODE_DUP(curr, prev, dup, cbdata);
			PATHE_SET_SAFE(pathpos, cbpathe, cbdata, curr);
			pathpos++;
			}

		prev = curr;
		NODE_DUP(*targetp, prev, dup, cbdata);
		}

	return EAVL_OK;
	}


int PUBLIC(Split)(
		EAVLc_context_t*	context,
		EAVLc_node_t**		nodep
		)
	{
	EAVLc_node_t*		target;
	int			result;

	CHECK_PARAM_NON_NULL(context);

	CHECK_STD_PRE(context, context->tree, 1);

	target = context->recent;

	result = PRIVATE(tree_split)(
			&context->tree->root,
			&target,
			context->pathlen,
			context->tree->cbset->dup,
			context->cbpathe,
			context->common.cbdata
			);
	if (result == EAVL_OK)
		{
		if (context->recent != target)
			{
			CONTEXT_RESET_ALL(context);
			CONTEXT_SET(context, target, context->pathlen, 0);
			if (nodep)
				{
				*nodep = target;
				}
			}
		}
	else if (result == EAVL_CALLBACK)
		{
		CONTEXT_RESET_ALL(context);
		CONTEXT_SET(context, target, context->pathlen, 1);
		}
	else
		{
		CONTEXT_RESET_ALL(context);
		CONTEXT_RESET(context, 0);
		}

	CHECK_STD_POST(context->tree, context);

	return result;
	}


typedef struct
	{
	EAVLc_cbRelease_t	cbrelease;
	void*			cbdata;
	}			tc_cbdata_t;


static int PRIVATE(cb_tree_clear)(
		EAVLc_node_t*		node,
		order_mask_t		cover,
		unsigned int		safe,
		void*			cbdata
		)
	{
	if (safe)
		{
		return (GET_REFS(node))
				? EAVL_CB_LIMIT
				: EAVL_CB_OK
				;
		}

	if (GET_REFS(node))
		{
		REFS_DEC(node);
		return EAVL_CB_LIMIT;
		}

	if (cover & ORDER_MASK_POST)
		{
		return (*((tc_cbdata_t*)cbdata)->cbrelease)(
				node,
				((tc_cbdata_t*)cbdata)->cbdata
				);
		}

	return EAVL_CB_OK;
	}


int PUBLIC(Clear)(
		EAVLc_context_t*	context,
		EAVLc_cbRelease_t	cbrelease
		)
	{
	tc_cbdata_t		tccbdata;
	EAVLc_cbPathe_t		cbpathe;
	unsigned int		i = 1;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_STD_PRE(context, context->tree, 0);

	CONTEXT_RESET_ALL(context);

	if (!context->tree->root || !cbrelease)
		{
		context->tree->root = NULL;
		CONTEXT_RESET(context, 0);
		RESULT(EAVL_OK);
		}

	cbpathe = context->cbpathe;
	tccbdata.cbrelease = cbrelease;
	tccbdata.cbdata = context->common.cbdata;

	do
		{
		result = PRIVATE(traverse)(
				context->tree->root,
				EAVL_DIR_LEFT,
				ORDER_MASK_PRE | ORDER_MASK_POST,
				i,
				&PRIVATE(cb_tree_clear),
				&tccbdata,
				cbpathe,
				context->common.cbdata
				);
		} while (!result && i--);

	if (result != EAVL_CALLBACK)
		{
		context->tree->root = NULL;
		CONTEXT_RESET(context, 0);
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(Release)(
		EAVLc_tree_t*		tree
		)
	{
	CHECK_PARAM_NON_NULL(tree);

	if (tree->common.associations)
		{
		return EAVL_ERROR_CONTEXT;
		}
	if (tree->root)
		{
		return EAVL_ERROR_TREE;
		}

	return EAVL_OK;
	}


int PUBLIC(Context_Init)(
		EAVLc_context_t*	context,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata
		)
	{
	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(cbpathe);

	context->tree = NULL;
	context->common.self = &context->common;
	context->cbpathe = cbpathe;
	context->common.cbdata = cbdata;
	context->recent = NULL;
	context->pathlen = 0;

	return EAVL_OK;
	}


int PUBLIC(Context_Associate)(
		EAVLc_context_t*	context,
		EAVLc_tree_t*		tree
		)
	{
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(tree);

	if (context->tree)
		{
		RESULT(EAVL_ERROR_CONTEXT);
		}

	context->tree = tree;
	tree->common.associations++;

	CONTEXT_RESET(context, 0);

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(Context_Disassociate)(
		EAVLc_context_t*	context
		)
	{
	EAVLc_tree_t*		tree;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);

	tree = context->tree;

	CHECK_STD_PRE(context, context->tree, 0);

	CONTEXT_RESET(context, 0);

	context->tree = NULL;
	tree->common.associations--;

	CHECK_TREE(context, tree);

	RETURN;
	}


/*
**				_cmp_
**	Rel			0 2 1
**	LT	0	000	0 1 1
**	LE	1	001	0 2 1
**	EQ	2	010	0 2 1
**	GE	3	011	0 2 1
**	GT	4	100	0 0 1
*/
#define CMP_REL_MAP(REL, CMP)						\
		(((CMP) != EAVL_CMP_SAME || ((REL) & 0x03))		\
				? (CMP)					\
				: (((REL) == EAVL_FIND_LT)		\
					? EAVL_CMP_RIGHT		\
					: EAVL_CMP_LEFT			\
					)				\
				)


int PRIVATE(find)(
		EAVLc_node_t*		node,
		EAVL_rel_t		rel,
		EAVLc_cbCompare_t	compare,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata,
		void*			ref_value,
		EAVLc_node_t*		ref_node,
		EAVLc_node_t**		resultp,
		unsigned int*		pathlenp
		)
	{
	EAVLc_node_t*		left = NULL;
	EAVLc_node_t*		right = NULL;
	EAVL_dir_t		cmp;
	unsigned int		pathlen = 0;

	PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, NULL);

	while (node)
		{
		CB_COMPARE(ref_value, ref_node, node, compare, cbdata, cmp);
		switch (CMP_REL_MAP(rel, cmp))
			{
			case EAVL_CMP_SAME:
				*resultp = node;
				*pathlenp = pathlen;
				return EAVL_OK;

			case EAVL_CMP_LEFT:
				left = node;
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, node);
				node = GET_CHILD(node, DIR_RIGHT);
				continue;

			case EAVL_CMP_RIGHT:
				right = node;
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, node);
				node = GET_CHILD(node, DIR_LEFT);
				continue;
			}
		}

	node = (rel == EAVL_FIND_EQ)
			? node
			: (rel < EAVL_FIND_EQ)
				? left
				: right
			;
	*resultp = node;
	*pathlenp = pathlen;

	return (node) ? EAVL_OK : EAVL_NOTFOUND;
	}


int PUBLIC(Find)(
		EAVLc_context_t*	context,
		EAVL_rel_t		rel,
		EAVLc_cbCompare_t	compare,
		void*			ref_value,
		EAVLc_node_t*		ref_node,
		EAVLc_node_t**		resultp
		)
	{
	EAVLc_node_t*		node;
	unsigned int		pathlen;
	int			result;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_REL(rel);

	CHECK_STD_PRE(context, context->tree, 0);

	CONTEXT_RESET(context, 1);

	if (!compare)
		{
		compare = context->tree->cbset->compare;
		}

	result = PRIVATE(find)(
			context->tree->root,
			rel,
			compare,
			context->cbpathe,
			context->common.cbdata,
			ref_value,
			ref_node,
			&node,
			&pathlen
			);

	if (result == EAVL_OK)
		{
		CONTEXT_SET(context, node, pathlen, 0);
		*resultp = node;
		}
	else
		{
		CONTEXT_RESET(context, (result == EAVL_CALLBACK));
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(First)(
		EAVLc_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLc_node_t**		resultp
		)
	{
	EAVLc_node_t*		curr;
	EAVLc_node_t*		next;
	EAVLc_cbPathe_t		cbpathe;
	void*			cbdata;
	unsigned int		pathlen = 0;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_DIR(dir);
	CHECK_PARAM_ORDER(order);

	CHECK_STD_PRE(context, context->tree, 0);

	CONTEXT_RESET(context, 1);

	cbpathe = context->cbpathe;
	cbdata = context->common.cbdata;

	curr = context->tree->root;
	if (!curr)
		{
		CONTEXT_RESET(context, 0);
		RESULT(EAVL_NOTFOUND);
		}
	PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, NULL);

	switch (order)
		{
		case EAVL_ORDER_IN:
			/* find extreme node in direction opposit of motion */
			dir = DIR_OTHER(dir);
			while ((next = GET_CHILD(curr, dir)))
				{
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
				curr = next;
				}
			break;

		case EAVL_ORDER_PRE:
			break;

		case EAVL_ORDER_POST:
			while ((next = GET_CHILD(curr, DIR_OTHER(dir))) || (next = GET_CHILD(curr, dir)))
				{
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
				curr = next;
				}
			break;
		}

	if (curr)
		{
		CONTEXT_SET(context, curr, pathlen, 0);
		*resultp = curr;
		}
	else
		{
		CONTEXT_RESET(context, 0);
		result = EAVL_NOTFOUND;
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(Next)(
		EAVLc_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLc_node_t**		resultp
		)
	{
	EAVLc_node_t*		curr;
	EAVLc_node_t*		prev;
	EAVLc_cbPathe_t		cbpathe;
	void*			cbdata;
	EAVL_dir_t		other = DIR_OTHER(dir);
	unsigned int		pathlen;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_DIR(dir);
	CHECK_PARAM_ORDER(order);

	CHECK_STD_PRE(context, context->tree, 1);

	curr = context->recent;
	pathlen = context->pathlen;
	cbpathe = context->cbpathe;
	cbdata = context->common.cbdata;

	switch (order)
		{
		case EAVL_ORDER_IN:
			/* try to move DIR */
			prev = GET_CHILD(curr, dir);
			if (prev)
				{
				/* find OTHER-most node */
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
				curr = prev;
				while ((prev = GET_CHILD(curr, other)))
					{
					PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
					curr = prev;
					}
				}
			else
				{
				do
					{
					/* move UP */
					prev = curr;
					PATHE_GET_SAFE(--pathlen, cbpathe, cbdata, curr);
					} while (curr && prev != GET_CHILD(curr, other));
				}
			break;

		case EAVL_ORDER_PRE:
			/* try to move OTHER(DIR) if not move DIR */
			if ((prev = GET_CHILD(curr, other)) || (prev = GET_CHILD(curr, dir)))
				{
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
				curr = prev;
				break;
				}

			/* move UP until an unvisited DIR */
			do
				{
				prev = curr;
				PATHE_GET_SAFE(--pathlen, cbpathe, cbdata, curr);
				} while (curr && (!GET_CHILD(curr, dir) || prev == GET_CHILD(curr, dir)));

			if (curr)
				{
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
				curr = GET_CHILD(curr, dir);
				}
			break;

		case EAVL_ORDER_POST:
			/* OTHER(DIR) and DIR sub-trees already visited but not PARENT */
			prev = curr;
			PATHE_GET_SAFE(--pathlen, cbpathe, cbdata, curr);

			if (curr && prev == GET_CHILD(curr, other) && GET_CHILD(curr, dir))
				{
				/* go DIR and find OTHER(DIR)-most leaf */
				PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
				curr = GET_CHILD(curr, dir);

				while ((prev = GET_CHILD(curr, other)) || (prev = GET_CHILD(curr, dir)))
					{
					PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, curr);
					curr = prev;
					}
				}
			break;
		}

	if (curr)
		{
		CONTEXT_SET(context, curr, pathlen, 0);
		*resultp = curr;
		}
	else
		{
		CONTEXT_RESET(context, 0);
		result = EAVL_NOTFOUND;
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(Fixup)(
		EAVLc_context_t*	context
		)
	{
	EAVLc_node_t*		curr;
	EAVLc_cbFixup_t		fixup;
	EAVLc_cbPathe_t		cbpathe;
	void*			cbdata;
	unsigned int		pathlen;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);

	CHECK_CONTEXT(context, 1);

	curr = context->recent;
	pathlen = context->pathlen;
	fixup = context->tree->cbset->fixup;
	cbpathe = context->cbpathe;
	cbdata = context->common.cbdata;

	while (fixup && curr)
		{
		NODE_FIXUP(curr, 0, fixup, cbdata);
		PATHE_GET_SAFE(--pathlen, cbpathe, cbdata, curr);
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


static void PRIVATE(load_setchild)(
		void**			nodep,
		unsigned int		parentindex,
		unsigned int		childindex,
		EAVL_dir_t		dir
		)
	{
	SET_CHILD(
			((EAVLc_node_t**)nodep)[parentindex],
			((EAVLc_node_t**)nodep)[childindex],
			dir
			);
	}


static void PRIVATE(load_setbal)(
		void**			nodep,
		unsigned int		nodeindex,
		EAVL_dir_t		bal
		)
	{
	SET_BAL(
			((EAVLc_node_t**)nodep)[nodeindex],
			bal
			);
	}


static int PRIVATE(load_fixup)(
		void**			nodep,
		unsigned int		nodeindex,
		FOREIGN(_, load_cbFixup_t)	fixup,
		void*			cbdata
		)
	{
	EAVLc_cbFixup_t		cbfixup = (EAVLc_cbFixup_t)fixup;

	CB_FIXUP(
			((EAVLc_node_t**)nodep)[nodeindex],
			GET_CHILD(((EAVLc_node_t**)nodep)[nodeindex], DIR_LEFT),
			GET_CHILD(((EAVLc_node_t**)nodep)[nodeindex], DIR_RIGHT),
			1,
			cbfixup,
			cbdata
			);

	return EAVL_CB_OK;
	}


static FOREIGN(_, load_cbset_t) PRIVATE(load_cbset) =
	{
	&PRIVATE(load_setchild),
	&PRIVATE(load_setbal),
	&PRIVATE(load_fixup)
	};


int PUBLIC(Load)(
		EAVLc_context_t*	context,
		unsigned int		count,
		EAVLc_node_t**		nodes
		)
	{
	unsigned int		rootindex;
	unsigned int		i;
	int			result;

	CHECK_PARAM_NON_NULL(context);

	CHECK_STD_PRE(context, context->tree, 0);
	if (context->tree->root)
		{
		RESULT(EAVL_ERROR_PARAMETER);
		}

	if (!count)
		{
		RESULT(EAVL_OK);
		}

	CHECK_PARAM_NON_NULL(nodes);
	CHECK_NODES_ALIGN(count, nodes);
	CHECK_NODES_ORDER(context, count, nodes);

	for (i=0; i<count; i++)
		{
		NODE_INIT(nodes[i]);
		NODE_FIXUP(nodes[i], 1, context->tree->cbset->fixup, context->common.cbdata);
		}

	result = FOREIGN(_, load)(
			&rootindex,
			count,
			(void**)nodes,
			&PRIVATE(load_cbset),
			(FOREIGN(_, load_cbFixup_t))context->tree->cbset->fixup,
			context->common.cbdata
			);

	if (result == EAVL_OK)
		{
		context->tree->root = nodes[rootindex];
		}

	CONTEXT_RESET_ALL(context);
	CONTEXT_RESET(context, 0);

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


static void PRIVATE(rotate_single)(
		EAVL_dir_t		dir,
		EAVLc_node_t*		P,	// Never NULL
		EAVLc_node_t*		A,	// Never NULL
		EAVLc_node_t*		B	// Never NULL
		)
	{
	EAVLc_node_t*		T;
	EAVL_dir_t		other = DIR_OTHER(dir);
	EAVL_dir_t		asid = SID(P, A);

	T = GET_CHILD(B, other);
	SET_CHILD(A, T, dir);

	SET_CHILD(B, A, other);

	SET_CHILD(P, B, asid);

	if (GET_BAL(B) == DIR_NEITHER)
		{
		SET_BAL(A, dir);
		SET_BAL(B, other);
		}
	else
		{
		SET_BAL(A, DIR_NEITHER);
		SET_BAL(B, DIR_NEITHER);
		}
	}


static void PRIVATE(rotate_double)(
		EAVL_dir_t		dir,
		EAVLc_node_t*		P,	// Never NULL
		EAVLc_node_t*		A,	// Never NULL
		EAVLc_node_t*		B,	// Never NULL; A->child[dir]
		EAVLc_node_t*		C	// Never NULL; B->child[OTHER(dir)]
		)
	{
	EAVLc_node_t*		T;
	EAVL_dir_t		other = DIR_OTHER(dir);
	EAVL_dir_t		asid = SID(P, A);

	T = GET_CHILD(C, dir);
	SET_CHILD(B, T, other);

	SET_CHILD(C, B, dir);

	T = GET_CHILD(C, other);
	SET_CHILD(A, T, dir);

	SET_CHILD(C, A, other);

	SET_CHILD(P, C, asid);

	if (GET_BAL(C) == dir)
		{
		SET_BAL(A, other);
		SET_BAL(B, DIR_NEITHER);
		}
	else if (GET_BAL(C) == DIR_NEITHER)
		{
		SET_BAL(A, DIR_NEITHER);
		SET_BAL(B, DIR_NEITHER);
		}
	else	// C->baldir == other
		{
		SET_BAL(A, DIR_NEITHER);
		SET_BAL(B, dir);
		}
	SET_BAL(C, DIR_NEITHER);
	}


static int PRIVATE(insert)(
		EAVLc_node_t**		rootp,
		EAVLc_node_t*		new_node,
		EAVLc_cbCompare_t	compare,
		EAVLc_cbFixup_t		fixup,
		EAVLc_cbDup_t		dup,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata,
		EAVLc_node_t**		resultp,
		unsigned int*		pathlenp
		)
	{
	EAVLc_node_t*		curr = *rootp;
	EAVLc_node_t*		prev;
	EAVLc_node_t		root;
	EAVL_dir_t		dir;
	unsigned int		pathlensave;
	int			result = EAVL_OK;

	*pathlenp = 0;
	PATHE_SET_SAFE((*pathlenp)++, cbpathe, cbdata, NULL);

	if (!curr)
		{
		NODE_INIT(new_node);
		NODE_FIXUP(new_node, 1, fixup, cbdata);
		*rootp = new_node;
		*resultp = new_node;
		return EAVL_OK;
		}

	while (curr)
		{
		prev = curr;
		CB_COMPARE(NULL, curr, new_node, compare, cbdata, dir);
		switch (dir)
			{
			case EAVL_CMP_SAME:
				*resultp = curr;
				return EAVL_EXISTS;
				break;

			case EAVL_CMP_LEFT:
			case EAVL_CMP_RIGHT:
				PATHE_SET_SAFE((*pathlenp)++, cbpathe, cbdata, curr);
				curr = GET_CHILD(curr, dir);
				continue;
			}
		}

	result = PRIVATE(tree_split)(
			rootp,
			&prev,
			*pathlenp-1,
			dup,
			cbpathe,
			cbdata
			);
	if (result != EAVL_OK)
		{
		return result;
		}
	PATHE_SET_SAFE((*pathlenp)-1, cbpathe, cbdata, prev);

	// curr === NULL
	// prev === parent of the new node; not NULL
	// dir === which child gets the new node

	NODE_INIT(new_node);
	NODE_FIXUP(new_node, 1, fixup, cbdata);
	SET_CHILD(prev, new_node, dir);
	pathlensave = *pathlenp;

	NODE_INIT(&root);
	SET_CHILD(&root, *rootp, DIR_LEFT);
	PATHE_SET_SAFE(0, cbpathe, cbdata, &root);

	(*pathlenp)--;

	curr = new_node;

	while (prev != &root)
		{
		EAVLc_node_t*		parent;
		EAVL_dir_t		other;
		EAVL_dir_t		bal;

		PATHE_GET_DANGER(--(*pathlenp), cbpathe, cbdata, parent);
		bal = GET_BAL(prev);
		dir = SID(prev, curr);
		other = DIR_OTHER(dir);

		if (bal == DIR_NEITHER)			// Cases: 1,5
			{
			SET_BAL(prev, dir);
			NODE_FIXUP(prev, 1, fixup, cbdata);
			}
		else if (bal == other)			// Case: 2
			{
			SET_BAL(prev, DIR_NEITHER);
			NODE_FIXUP(prev, 1, fixup, cbdata);
			prev = parent;
			break;
			}
		else	// bal == dir			// Cases: 3,4
			{
			EAVLc_node_t*		T;

			if (GET_BAL(curr) != other)	// Case: 3
				{
				T = curr;
				PRIVATE(rotate_single)(dir, parent, prev, curr);
				NODE_FIXUP(prev, 1, fixup, cbdata);
				NODE_FIXUP(curr, 1, fixup, cbdata);
				pathlensave--;
				PATHE_SHIFT((*pathlenp)+2, pathlensave, cbpathe, cbdata);
				}
			else				// Case: 4
				{
				T = GET_CHILD(curr, other);
				PRIVATE(rotate_double)(dir, parent, prev, curr, T);
				NODE_FIXUP(prev, 1, fixup, cbdata);
				NODE_FIXUP(curr, 1, fixup, cbdata);
				NODE_FIXUP(T, 1, fixup, cbdata);
				pathlensave--;
				PATHE_SHIFT((*pathlenp)+2, pathlensave, cbpathe, cbdata);
				PATHE_SET_DANGER((*pathlenp)+1, cbpathe, cbdata, T);
				if (T == new_node)
					{
					pathlensave--;
					}
				else
					{
					EAVL_dir_t		cmp;

					CB_COMPARE(NULL, T, new_node, compare, cbdata, cmp);
					PATHE_SET_DANGER((*pathlenp)+2, cbpathe, cbdata,
							(cmp != dir)
							? prev
							: curr
							);
					}
				}

			prev = parent;
			break;
			}

		curr = prev;
		prev = parent;
		}

	while (fixup && prev != &root)
		{
		NODE_FIXUP(prev, 0, fixup, cbdata);
		PATHE_GET_DANGER(--(*pathlenp), cbpathe, cbdata, prev);
		}

	curr = GET_CHILD(&root, DIR_LEFT);
	PATHE_SET_DANGER(0, cbpathe, cbdata, NULL);
	*pathlenp = pathlensave;
	*rootp = curr;
	*resultp = new_node;

	return result;
	}


int PUBLIC(Insert)(
		EAVLc_context_t*	context,
		EAVLc_node_t*		new_node,
		EAVLc_node_t**		resultp
		)
	{
	unsigned int		pathlen;
	int			result;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_NON_NULL(new_node);

	CHECK_NODE_ALIGN(new_node);
	CHECK_STD_PRE(context, context->tree, 0);

	CONTEXT_RESET(context, 1);

	result = PRIVATE(insert)(
			&context->tree->root,
			new_node,
			context->tree->cbset->compare,
			context->tree->cbset->fixup,
			context->tree->cbset->dup,
			context->cbpathe,
			context->common.cbdata,
			resultp,
			&pathlen
			);

	if (result == EAVL_OK)
		{
		CONTEXT_RESET_ALL(context);
		CONTEXT_SET(context, *resultp, pathlen, 0);
		}
	else if (result == EAVL_EXISTS)
		{
		CONTEXT_SET(context, *resultp, pathlen, 0);
		}
	else if (result != EAVL_CALLBACK)
		{
		CONTEXT_RESET_ALL(context);
		CONTEXT_RESET(context, 0);
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


#define SWAP_NODES(A, pA, B, pB, T)					\
	/* A, pA, B, and pB are never NULL */				\
	/* A is NOT a child of B */					\
	do								\
		{							\
		EAVL_dir_t		TdirA = SID((pA), (A));		\
		EAVL_dir_t		TdirB = SID((pB), (B));		\
									\
		(T) = GET_CHILD((A), 0);				\
		SET_CHILD((A), GET_CHILD((B), 0), 0);			\
		if ((T) != (B))						\
			{						\
			SET_CHILD((B), (T), 0);				\
			}						\
									\
		(T) = GET_CHILD((A), 1);				\
		SET_CHILD((A), GET_CHILD((B), 1), 1);			\
		if ((T) != (B))						\
			{						\
			SET_CHILD((B), (T), 1);				\
			}						\
									\
		SET_CHILD((pA), (B), TdirA);				\
		SET_CHILD(((pB) != (A)) ? (pB) : (B), (A), TdirB);	\
									\
		TdirA = GET_BAL((A));					\
		SET_BAL((A), GET_BAL((B)));				\
		SET_BAL((B), TdirA);					\
		} while (0)


static int PRIVATE(remove_split)(
		EAVLc_node_t**		rootp,
		EAVLc_node_t**		delpointp,
		unsigned int		pathlen,
		EAVLc_cbDup_t		dup,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata
		)
	{
	EAVLc_node_t*		curr;
	EAVLc_node_t*		prev;
	EAVL_dir_t		dir;
	int			result = EAVL_OK;

	result = PRIVATE(tree_split)(
			rootp,
			delpointp,
			pathlen,
			dup,
			cbpathe,
			cbdata
			);
	if (result != EAVL_OK)
		{
		return result;
		}

	pathlen--;
	PATHE_GET_SAFE(pathlen, cbpathe, cbdata, curr);
	prev = *delpointp;

	// dir === direction of removed node
	// curr === ancestor of removed node; not NULL
	// prev === GET_CHILD(curr, dir);

	while (curr)
		{
		EAVL_dir_t		other;
		EAVL_dir_t		bal;

		dir = SID(curr, prev);
		other = DIR_OTHER(dir);
		bal = GET_BAL(curr);

		if (bal == dir)				// Case: 1
			{
			// Do nothing
			}
		else if (bal == DIR_NEITHER)		// Case: 2
			{
			break;
			}
		else	// bal == other			// Cases: 3,4,5
			{
			EAVLc_node_t*		B;
			EAVLc_node_t*		C;

			B = GET_CHILD(curr, other);
			bal = GET_BAL(B);
			NODE_DUP(B, curr, dup, cbdata);
			if (bal != dir)			// Cases: 3,4
				{
//				PRIVATE(rotate_single)(other, parent, curr, B);
//				NODE_DUP(B, curr, dup, cbdata);
				}
			else				// Case: 5
				{
//				C = GET_CHILD(B, dir);
//				PRIVATE(rotate_double)(other, parent, curr, B, C);
//				NODE_DUP(B, curr, dup, cbdata);

				C = GET_CHILD(B, dir);
				NODE_DUP(C, B, dup, cbdata);
				}

			if (bal == DIR_NEITHER)		// Case: 4
				{
				break;
				}
			}

		prev = curr;
		PATHE_GET_SAFE(--pathlen, cbpathe, cbdata, curr);
		}

	return result;
	}


static int PRIVATE(remove)(
		EAVLc_node_t**		rootp,
		EAVLc_node_t**		del_nodep,
		unsigned int		pathlen,
		EAVLc_cbFixup_t		fixup,
		EAVLc_cbDup_t		dup,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata,
		EAVLc_node_t**		nodep
		)
	{
	EAVLc_node_t*		prev;
	EAVLc_node_t*		del_node_parent = NULL;
	EAVLc_node_t*		T;
	EAVLc_node_t		root;
	EAVL_dir_t		dir = GET_BAL(*del_nodep) & 0x1u;
	EAVL_dir_t		other = DIR_OTHER(dir);
	unsigned int		del_node_pathlen = pathlen;
	int			result = EAVL_OK;

	NODE_INIT(&root);

	if (GET_CHILD(*del_nodep, other))
		{
		EAVLc_node_t*		swap_node;
		EAVLc_node_t*		swap_node_parent;

		// Two children
		// swap with adjacent on long side or LEFT: Adjacent(bal & 0x1)

		PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, *del_nodep);
		swap_node = GET_CHILD(*del_nodep, dir);	// Long or LEFT side

		while ((T = GET_CHILD(swap_node, other)))
			{
			PATHE_SET_SAFE(pathlen++, cbpathe, cbdata, swap_node);
			swap_node = T;
			}

		result = PRIVATE(remove_split)(
				rootp,
				&swap_node,
				pathlen,
				dup,
				cbpathe,
				cbdata
				);
		// Fix context->recent
		PATHE_GET_SAFE(del_node_pathlen, cbpathe, cbdata, *del_nodep);

		if (result != EAVL_OK)
			{
			return result;
			}

		SET_CHILD(&root, *rootp, DIR_LEFT);
		PATHE_SET_SAFE(0, cbpathe, cbdata, &root);

		PATHE_GET_DANGER(pathlen-1, cbpathe, cbdata, swap_node_parent);
		PATHE_GET_DANGER(del_node_pathlen-1, cbpathe, cbdata, del_node_parent);
		SWAP_NODES(*del_nodep, del_node_parent, swap_node, swap_node_parent, T);
		PATHE_SET_DANGER(del_node_pathlen, cbpathe, cbdata, swap_node);

		if (del_node_parent == &root)
			{
			*rootp = swap_node;
			}
		}
	else
		{
		result = PRIVATE(remove_split)(
				rootp,
				del_nodep,
				pathlen,
				dup,
				cbpathe,
				cbdata
				);
		if (result != EAVL_OK)
			{
			return result;
			}

		// One child or no children
		SET_CHILD(&root, *rootp, DIR_LEFT);
		PATHE_SET_SAFE(0, cbpathe, cbdata, &root);
		}

	// del_node now has 1 or no children
	//	child will be "dir" child: (GET_BAL(del_node) & 0x1)

	T = GET_CHILD(*del_nodep, dir);
	PATHE_GET_DANGER(pathlen-1, cbpathe, cbdata, prev);
	if (&root == prev)
		{
		*rootp = T;
		PATHE_SET_DANGER(0, cbpathe, cbdata, NULL);
		return result;
		}

	dir = SID(prev, *del_nodep);
	SET_CHILD(prev, T, dir);
	NODE_CLEAR(*del_nodep);
	if (nodep)
		{
		*nodep = *del_nodep;
		}

	pathlen--;

	// dir === direction of removed node
	// prev === ancestor of removed node; not NULL

	while (prev != &root)
		{
		EAVLc_node_t*		parent;
		EAVL_dir_t		bal;

		bal = GET_BAL(prev);
		other = DIR_OTHER(dir);
		PATHE_GET_DANGER(--pathlen, cbpathe, cbdata, parent);

		if (prev == del_node_parent)
			{
			del_node_parent = NULL;
			}

		if (bal == dir)				// Case: 1
			{
			SET_BAL(prev, DIR_NEITHER);
			NODE_FIXUP(prev, 1, fixup, cbdata);
			}
		else if (bal == DIR_NEITHER)		// Case: 2
			{
			SET_BAL(prev, other);
			NODE_FIXUP(prev, 1, fixup, cbdata);
			prev = parent;
			break;
			}
		else	// bal == other			// Cases: 3,4,5
			{
			EAVLc_node_t*		B;
			EAVLc_node_t*		S;

			B = GET_CHILD(prev, other);
			bal = GET_BAL(B);
			if (bal != dir)			// Cases: 3,4
				{
				S = B;
				PRIVATE(rotate_single)(other, parent, prev, B);
				NODE_FIXUP(prev, 1, fixup, cbdata);
				NODE_FIXUP(B, 1, fixup, cbdata);
				}
			else				// Case: 5
				{
				S = GET_CHILD(B, dir);

				PRIVATE(rotate_double)(other, parent, prev, B, S);
				NODE_FIXUP(prev, 1, fixup, cbdata);
				NODE_FIXUP(B, 1, fixup, cbdata);
				NODE_FIXUP(S, 1, fixup, cbdata);
				}
			prev = S;

			if (bal == DIR_NEITHER)		// Case: 4
				{
				prev = parent;
				break;
				}
			}

		dir = SID(parent, prev);
		prev = parent;
		}

	if (del_node_parent)
		{
		while (prev != &root && prev != del_node_parent)
			{
			NODE_FIXUP(prev, 1, fixup, cbdata);
			PATHE_GET_DANGER(--pathlen, cbpathe, cbdata, prev);
			}
		}

	while (fixup && prev != &root)
		{
		NODE_FIXUP(prev, 0, fixup, cbdata);
		PATHE_GET_DANGER(--pathlen, cbpathe, cbdata, prev);
		}

	T = GET_CHILD(&root, DIR_LEFT);
	*rootp = T;
	PATHE_SET_DANGER(0, cbpathe, cbdata, NULL);

	return result;
	}


int PUBLIC(Remove)(
		EAVLc_context_t*	context,
		EAVLc_node_t**		nodep
		)
	{
	int			result;

	CHECK_PARAM_NON_NULL(context);

	CHECK_STD_PRE(context, context->tree, 1);

	result = PRIVATE(remove)(
			&context->tree->root,
			&context->recent,
			context->pathlen,
			context->tree->cbset->fixup,
			context->tree->cbset->dup,
			context->cbpathe,
			context->common.cbdata,
			nodep
			);

	if (result != EAVL_CALLBACK)
		{
		CONTEXT_RESET_ALL(context);
		CONTEXT_RESET(context, 0);
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


/* eavl_cTree.c */
