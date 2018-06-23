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
#include <stdlib.h>

#include "EAVL.h"
#include "EAVL_pTree.h"

#define CHECKS_AVAILABLE	EAVLp_CHECKS_AVAILABLE

//#include "pTree_checks.h"

#include "checks_internal.h"
#include "eavl_internal.h"
#include "naming_internal.h"
#include "pTree.h"


unsigned int	PUBLIC(Checks_Available) = EAVLp_CHECKS_AVAILABLE & EAVL_CHECK_ALL;
unsigned int	PUBLIC(Checks_Enabled) = 0;


#if CHECKS_AVAILABLE & EAVL_CHECK_TREE


static int PRIVATE(validate_tree_recurse)(
		EAVLp_node_t*		node,
		EAVLp_node_t*		parent,
		EAVL_dir_t		dir,
		int*			heightp,
		EAVLp_node_t**		leftp,
		EAVLp_cbCompare_t	compare,
		EAVLp_cbVerify_t*	verifyp,
		void*			cbdata
		)
	{
	int			result;
	int			height[2] = {0, 0};
	EAVL_dir_t		bal_node;
	EAVL_dir_t		bal_height;
	EAVLp_node_t*		T;
	EAVL_dir_t		cmp;

	if (GET_PARENT(node) != parent || (parent && GET_CHILD(parent, dir) != node))
		{
		return EAVL_ERROR_TREE;
		}

	T = GET_CHILD(node, DIR_LEFT);
	if (T)
		{
		result = PRIVATE(validate_tree_recurse)(
				T,
				node,
				DIR_LEFT,
				&height[DIR_LEFT],
				leftp,
				compare,
				verifyp,
				cbdata
				);
		if (result != EAVL_OK)
			{
			return result;
			}
		}

	if (*leftp)
		{
		CB_COMPARE(NULL, *leftp, node, compare, cbdata, cmp);
		if (cmp != DIR_RIGHT)
			{
			return EAVL_ERROR_COMPARE;
			}
		}

	*leftp = node;

	T = GET_CHILD(node, DIR_RIGHT);
	if (T)
		{
		result = PRIVATE(validate_tree_recurse)(
				T,
				node,
				DIR_RIGHT,
				&height[DIR_RIGHT],
				leftp,
				compare,
				verifyp,
				cbdata
				);
		if (result != EAVL_OK)
			{
			return result;
			}
		}

	bal_height = (height[0] == height[1])
			? DIR_NEITHER
			: ((height[0] > height[1])
				? DIR_LEFT
				: DIR_RIGHT
			);
	bal_node = GET_BAL(node);

	if (bal_height != bal_node || abs(height[0]-height[1]) > 1)
		{
		return EAVL_ERROR_TREE;
		}

	*heightp = MAX(height[0], height[1])+1;

	CB_VERIFY(
			node,
			GET_CHILD(node, DIR_LEFT),
			GET_CHILD(node, DIR_RIGHT),
			*verifyp,
			cbdata
			);

	return EAVL_OK;
	}


int PRIVATE(Validate_Tree)(
		EAVLp_context_t*	context,
		EAVLp_tree_t*		tree
		)
	{
	int			result = EAVL_OK;
	int			height;
	EAVLp_cbVerify_t	verify = tree->cbset->verify;
	EAVLp_node_t*		left = NULL;

	if (tree->root)
		{
		result = PRIVATE(validate_tree_recurse)(
				tree->root,
				NULL,
				0,
				&height,
				&left,
				tree->cbset->compare,
				&verify,
				context->common.cbdata
				);
		}

	return result;
	}


#endif	/* EAVLp_CHECKS_AVAILABLE & EAVL_CHECK_TREE */


/* pTree_check.c */
