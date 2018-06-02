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

#include "EAVL_pTree.h"

#define CHECKS_AVAILABLE	EAVLp_CHECKS_AVAILABLE

#include "pTree.h"
#include "pTree_internal.h"

#include "callback_internal.h"
#include "checks_internal.h"
#include "context_internal.h"
#include "eavl_internal.h"
#include "naming_internal.h"
#include "treeload_internal.h"


int PUBLIC(Tree_Init)(
		EAVLp_tree_t*		tree,
		EAVLp_tree_t*		existing,
		EAVLp_cbset_t*		cbset
		)
	{
	CHECK_PARAM_NON_NULL(tree);

	if (existing && existing->cbset
			&& existing->cbset->compare
			)
		{
		tree->cbset = existing->cbset;
		}
	else if (cbset && cbset->compare)
		{
		tree->cbset = cbset;
		}
	else
		{
		return EAVL_ERROR_PARAMETER;
		}

	tree->root = NULL;
	tree->common.contexts = NULL;
	tree->common.associations = 0;

	return EAVL_OK;
	}


int PUBLIC(Clear)(
		EAVLp_context_t*	context,
		EAVLp_cbRelease_t	noderelease
		)
	{
	EAVLp_node_t*		curr;
	EAVLp_node_t*		prev;
	EAVLp_node_t*		node;
	void*			cbdata;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_STD_PRE(context, context->tree, 0);

	curr = context->tree->root;
	context->tree->root = NULL;
	cbdata = context->common.cbdata;

	CONTEXT_RESET_ALL(context);

	if (!curr || !noderelease)
		{
		CONTEXT_RESET(context, 0);
		RESULT(EAVL_OK);
		}

	/* First(LEFT, POST) */
	while ((prev = GET_CHILD(curr, DIR_RIGHT)) || (prev = GET_CHILD(curr, DIR_LEFT)))
		{
		curr = prev;
		}

	while (noderelease && curr)
		{
		node = curr;	/* save for callback */

		/* Next(LEFT, POST) */
		prev = curr;
		curr = GET_PARENT(curr);

		if (curr && prev == GET_CHILD(curr, DIR_RIGHT) && GET_CHILD(curr, DIR_LEFT))
			{
			curr = GET_CHILD(curr, DIR_LEFT);

			while ((prev = GET_CHILD(curr, DIR_RIGHT)) || (prev = GET_CHILD(curr, DIR_LEFT)))
				{
				curr = prev;
				}
			}

		NODE_CLEAR(node);
		CB_RELEASE(node, noderelease, cbdata);
		}

	CONTEXT_RESET(context, 0);

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(Release)(
		EAVLp_tree_t*		tree
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
		EAVLp_context_t*	context,
		void*			cbdata
		)
	{
	CHECK_PARAM_NON_NULL(context);

	context->tree = NULL;
	context->common.self = &context->common;
	context->common.cbdata = cbdata;
	context->recent = NULL;

	return EAVL_OK;
	}


int PUBLIC(Context_Associate)(
		EAVLp_context_t*	context,
		EAVLp_tree_t*		tree
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
		EAVLp_context_t*	context
		)
	{
	EAVLp_tree_t*		tree;
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
		EAVLp_node_t*		node,
		EAVL_rel_t		rel,
		EAVLp_cbCompare_t	compare,
		void*			cbdata,
		void*			ref_value,
		EAVLp_node_t*		ref_node,
		EAVLp_node_t**		resultp
		)
	{
	EAVLp_node_t*		left = NULL;
	EAVLp_node_t*		right = NULL;
	EAVL_dir_t		cmp;

	while (node)
		{
		CB_COMPARE(ref_value, ref_node, node, compare, cbdata, cmp);
		switch (CMP_REL_MAP(rel, cmp))
			{
			case EAVL_CMP_SAME:
				*resultp = node;
				return EAVL_OK;

			case EAVL_CMP_LEFT:
				left = node;
				node = GET_CHILD(node, DIR_RIGHT);
				continue;

			case EAVL_CMP_RIGHT:
				right = node;
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

	return (node) ? EAVL_OK : EAVL_NOTFOUND;
	}


int PUBLIC(Find)(
		EAVLp_context_t*	context,
		EAVL_rel_t		rel,
		EAVLp_cbCompare_t	compare,
		void*			ref_value,
		EAVLp_node_t*		ref_node,
		EAVLp_node_t**		resultp
		)
	{
	EAVLp_node_t*		node;
	int			result = EAVL_NOTFOUND;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_REL(rel);

	CHECK_STD_PRE(context, context->tree, 0);

	if (!compare)
		{
		compare = context->tree->cbset->compare;
		}

	result = PRIVATE(find)(
			context->tree->root,
			rel,
			compare,
			context->common.cbdata,
			ref_value,
			ref_node,
			&node
			);

	if (result == EAVL_OK)
		{
		CONTEXT_SET(context, node, 0, 0);
		*resultp = node;
		}
	else
		{
		CONTEXT_RESET(context, 0);
		}

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


int PUBLIC(First)(
		EAVLp_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLp_node_t**		resultp
		)
	{
	EAVLp_node_t*		curr;
	EAVLp_node_t*		next;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_DIR(dir);
	CHECK_PARAM_ORDER(order);

	CHECK_STD_PRE(context, context->tree, 0);

	curr = context->tree->root;
	if (!curr)
		{
		CONTEXT_RESET(context, 0);
		RESULT(EAVL_NOTFOUND);
		}

	switch (order)
		{
		case EAVL_ORDER_IN:
			/* find extreme node in direction opposit of motion */
			dir = DIR_OTHER(dir);
			while ((next = GET_CHILD(curr, dir)))
				{
				curr = next;
				}
			break;

		case EAVL_ORDER_PRE:
			break;

		case EAVL_ORDER_POST:
			while ((next = GET_CHILD(curr, DIR_OTHER(dir))) || (next = GET_CHILD(curr, dir)))
				{
				curr = next;
				}
			break;
		}

	if (curr)
		{
		CONTEXT_SET(context, curr, 0, 0);
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
		EAVLp_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLp_node_t**		resultp
		)
	{
	EAVLp_node_t*		curr;
	EAVLp_node_t*		prev;
	EAVL_dir_t		other = DIR_OTHER(dir);
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_DIR(dir);
	CHECK_PARAM_ORDER(order);

	CHECK_STD_PRE(context, context->tree, 1);

	curr = context->recent;

	switch (order)
		{
		case EAVL_ORDER_IN:
			/* try to move DIR */
			prev = GET_CHILD(curr, dir);
			if (prev)
				{
				/* find OTHER-most node */
				curr = prev;
				while ((prev = GET_CHILD(curr, other)))
					{
					curr = prev;
					}
				}
			else
				{
				do
					{
					/* move UP */
					prev = curr;
					curr = GET_PARENT(curr);
					} while (curr && prev != GET_CHILD(curr, other));
				}
			break;

		case EAVL_ORDER_PRE:
			/* try to move OTHER(DIR) if not move DIR */
			if ((prev = GET_CHILD(curr, other)) || (prev = GET_CHILD(curr, dir)))
				{
				curr = prev;
				break;
				}

			/* move UP until an unvisited DIR */
			do
				{
				prev = curr;
				curr = GET_PARENT(curr);
				} while (curr && (!GET_CHILD(curr, dir) || prev == GET_CHILD(curr, dir)));

			if (curr)
				{
				curr = GET_CHILD(curr, dir);
				}
			break;

		case EAVL_ORDER_POST:
			/* OTHER(DIR) and DIR sub-trees already visited but not PARENT */
			prev = curr;
			curr = GET_PARENT(curr);

			if (curr && prev == GET_CHILD(curr, other) && GET_CHILD(curr, dir))
				{
				/* go DIR and find OTHER(DIR)-most leaf */
				curr = GET_CHILD(curr, dir);

				while ((prev = GET_CHILD(curr, other)) || (prev = GET_CHILD(curr, dir)))
					{
					curr = prev;
					}
				}
			break;
		}

	if (curr)
		{
		CONTEXT_SET(context, curr, 0, 0);
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
		EAVLp_context_t*	context
		)
	{
	EAVLp_node_t*		curr;
	EAVLp_cbFixup_t		fixup;
	void*			cbdata;
	int			result = EAVL_OK;

	CHECK_PARAM_NON_NULL(context);

	CHECK_CONTEXT(context, 1);

	curr = context->recent;
	fixup = context->tree->cbset->fixup;
	cbdata = context->common.cbdata;

	while (fixup && curr)
		{
		NODE_FIXUP(curr, 0, fixup, cbdata);
		curr = GET_PARENT(curr);
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
			((EAVLp_node_t**)nodep)[parentindex],
			((EAVLp_node_t**)nodep)[childindex],
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
			((EAVLp_node_t**)nodep)[nodeindex],
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
	EAVLp_cbFixup_t		cbfixup = (EAVLp_cbFixup_t)fixup;

	CB_FIXUP(
			((EAVLp_node_t**)nodep)[nodeindex],
			GET_CHILD(((EAVLp_node_t**)nodep)[nodeindex], DIR_LEFT),
			GET_CHILD(((EAVLp_node_t**)nodep)[nodeindex], DIR_RIGHT),
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
		EAVLp_context_t*	context,
		unsigned int		count,
		EAVLp_node_t**		nodes
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
		EAVLp_node_t*		P,	// Never NULL
		EAVLp_node_t*		A,	// Never NULL
		EAVLp_node_t*		B	// Never NULL
		)
	{
	EAVLp_node_t*		T;
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
		EAVLp_node_t*		P,	// Never NULL
		EAVLp_node_t*		A,	// Never NULL
		EAVLp_node_t*		B,	// Never NULL; A->child[dir]
		EAVLp_node_t*		C	// Never NULL; B->child[OTHER(dir)]
		)
	{
	EAVLp_node_t*		T;
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


int PRIVATE(insert)(
		EAVLp_node_t**		rootp,
		EAVLp_node_t*		new_node,
		EAVLp_cbCompare_t	compare,
		EAVLp_cbFixup_t		fixup,
		void*			cbdata,
		EAVLp_node_t**		resultp
		)
	{
	EAVLp_node_t*		curr = *rootp;
	EAVLp_node_t*		prev;
	EAVLp_node_t		root;
	EAVL_dir_t		dir;
	int			result = EAVL_OK;

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
				curr = GET_CHILD(curr, dir);
				continue;
			}
		}

	// curr === NULL
	// prev === parent of the new node; not NULL
	// dir === which child gets the new node

	NODE_INIT(new_node);
	NODE_FIXUP(new_node, 1, fixup, cbdata);
	SET_CHILD(prev, new_node, dir);

	NODE_INIT(&root);
	SET_CHILD(&root, *rootp, DIR_LEFT);

	curr = new_node;

	while (prev != &root)
		{
		EAVLp_node_t*		parent;
		EAVL_dir_t		other;
		EAVL_dir_t		bal;

		parent = GET_PARENT(prev);
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
			EAVLp_node_t*		T;

			if (GET_BAL(curr) != other)	// Case: 3
				{
				T = curr;
				PRIVATE(rotate_single)(dir, parent, prev, curr);
				NODE_FIXUP(prev, 1, fixup, cbdata);
				NODE_FIXUP(curr, 1, fixup, cbdata);
				}
			else				// Case: 4
				{
				T = GET_CHILD(curr, other);
				PRIVATE(rotate_double)(dir, parent, prev, curr, T);
				NODE_FIXUP(prev, 1, fixup, cbdata);
				NODE_FIXUP(curr, 1, fixup, cbdata);
				NODE_FIXUP(T, 1, fixup, cbdata);
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
		prev = GET_PARENT(prev);
		}

	curr = GET_CHILD(&root, DIR_LEFT);
	SET_PARENTONLY(curr, NULL);
	*rootp = curr;
	*resultp = new_node;

	return result;
	}


int PUBLIC(Insert)(
		EAVLp_context_t*	context,
		EAVLp_node_t*		new_node,
		EAVLp_node_t**		resultp
		)
	{
	int			result;

	CHECK_PARAM_NON_NULL(context);
	CHECK_PARAM_NON_NULL(resultp);
	CHECK_PARAM_NON_NULL(new_node);

	CHECK_NODE_ALIGN(new_node);
	CHECK_STD_PRE(context, context->tree, 0);

	result = PRIVATE(insert)(
			&context->tree->root,
			new_node,
			context->tree->cbset->compare,
			context->tree->cbset->fixup,
			context->common.cbdata,
			resultp
			);

	if (result == EAVL_OK)
		{
		CONTEXT_RESET_ALL(context);
		CONTEXT_SET(context, *resultp, 0, 0);
		}
	else if (result == EAVL_EXISTS)
		{
		CONTEXT_SET(context, *resultp, 0, 0);
		}
	else
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


int PRIVATE(remove)(
		EAVLp_node_t**		rootp,
		EAVLp_node_t*		del_node,
		EAVLp_cbFixup_t		fixup,
		void*			cbdata,
		EAVLp_node_t**		nodep
		)
	{
	EAVLp_node_t*		prev;
	EAVLp_node_t*		del_node_parent = NULL;
	EAVLp_node_t*		T;
	EAVLp_node_t		root;
	EAVL_dir_t		dir = GET_BAL(del_node) & 0x1u;
	EAVL_dir_t		other = DIR_OTHER(dir);
	int			result = EAVL_OK;

	NODE_INIT(&root);

	if (GET_CHILD(del_node, other))
		{
		EAVLp_node_t*		swap_node;
		EAVLp_node_t*		swap_node_parent;

		// Two children
		// swap with adjacent on long side or LEFT: Adjacent(bal & 0x1)

		prev = del_node;
		swap_node = GET_CHILD(del_node, dir);	// Long or LEFT side

		while ((T = GET_CHILD(swap_node, other)))
			{
			swap_node = T;
			}

		SET_CHILD(&root, *rootp, DIR_LEFT);

		swap_node_parent = GET_PARENT(swap_node);
		del_node_parent = GET_PARENT(del_node);
		SWAP_NODES(del_node, del_node_parent, swap_node, swap_node_parent, T);

		if (del_node_parent == &root)
			{
			*rootp = swap_node;
			}
		}
	else
		{
		// One child or no children
		SET_CHILD(&root, *rootp, DIR_LEFT);
		}

	// del_node now has 1 or no children
	//	child will be "dir" child: (GET_BAL(del_node) & 0x1)

	T = GET_CHILD(del_node, dir);
	prev = GET_PARENT(del_node);

	NODE_CLEAR(del_node);
	if (nodep)
		{
		*nodep = del_node;
		}

	if (&root == prev)
		{
		*rootp = T;
		if (T)
			{
			SET_PARENTONLY(T, NULL);
			}
		return result;
		}
	else
		{
		dir = SID(prev, del_node);
		SET_CHILD(prev, T, dir);
		}

	// dir === direction of removed node
	// prev === ancestor of removed node; not NULL

	while (prev != &root)
		{
		EAVLp_node_t*		parent;
		EAVL_dir_t		bal;

		bal = GET_BAL(prev);
		other = DIR_OTHER(dir);
		parent = GET_PARENT(prev);

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
			EAVLp_node_t*		B;
			EAVLp_node_t*		S;

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
			prev = GET_PARENT(prev);
			}
		}

	while (fixup && prev != &root)
		{
		NODE_FIXUP(prev, 0, fixup, cbdata);
		prev = GET_PARENT(prev);
		}

	T = GET_CHILD(&root, DIR_LEFT);
	*rootp = T;
	SET_PARENTONLY(T, NULL);

	return result;
	}


int PUBLIC(Remove)(
		EAVLp_context_t*	context,
		EAVLp_node_t**		nodep
		)
	{
	int			result;

	CHECK_PARAM_NON_NULL(context);

	CHECK_STD_PRE(context, context->tree, 1);

	result = PRIVATE(remove)(
			&context->tree->root,
			context->recent,
			context->tree->cbset->fixup,
			context->common.cbdata,
			nodep
			);

	CONTEXT_RESET_ALL(context);
	CONTEXT_RESET(context, 0);

	CHECK_STD_POST(context->tree, context);

	RETURN;
	}


/* eavl_pTree.c */
