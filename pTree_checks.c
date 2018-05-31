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


#if EAVLp_CHECKS_AVAILABLE & EAVL_CHECK_TREE




static int PRIVATE(Validate_Tree_Recurse)(
		EAVLp_node_t*		node,
		EAVLp_node_t*		parent,
		EAVL_dir_t		dir,
		EAVLp_node_t*		fenceL,
		EAVLp_node_t*		fenceR,
		EAVLp_cbCompare_t	compare,
		EAVLp_cbVerify_t*	verifyp,
		void*			cbdata,
		int*			heightp,
		EAVLp_node_t**		myinterval
		)
	{
	int			result;
	int			height[2] = {0, 0};
	unsigned int		i;
	EAVL_dir_t		baldir;
	EAVLp_node_t*		T;
	EAVLp_node_t*		myfence[2];
	EAVLp_node_t*		stinterval[2];

	if (GET_PARENT(node) != parent || (parent && GET_CHILD(parent, dir) != node))
		{
		return EAVL_ERROR_TREE;
		}

	myfence[0] = fenceL;
	myfence[1] = fenceR;

	myinterval[0] = node;
	myinterval[1] = node;

	for (i=0; i<2; i++)
		{
		T = GET_CHILD(node, i);
		if (T)
			{
			EAVL_dir_t		cmp;

			CB_COMPARE(NULL, node, T, compare, cbdata, cmp);
			if (i != cmp)
				{
				return EAVL_ERROR_COMPARE;
				}

			result = PRIVATE(Validate_Tree_Recurse)(
					T,
					node,
					i,
					(i) ? node : fenceL,
					(i) ? fenceR : node,
					compare,
					verifyp,
					cbdata,
					&height[i],
					stinterval
					);
			if (result != EAVL_OK)
				{
				return result;
				}

			myinterval[i] = stinterval[i];
			myfence[i] = stinterval[1-i];
			}

		T = myfence[i];
		if (T)
			{
			EAVL_dir_t		cmp;

			CB_COMPARE(NULL, node, T, compare, cbdata, cmp);
			if (i != cmp)
				{
				return EAVL_ERROR_COMPARE;
				}
			}
		}

	baldir = (height[0] == height[1])
			? 2
			: ((height[0] > height[1])
				? 0
				: 1
			);

	if (baldir != GET_BAL(node) || abs(height[0]-height[1]) > 1)
		{
		return EAVL_ERROR_TREE;
		}

	*heightp = MAX(height[0], height[1])+1;

	CB_VERIFY(
			node,
			GET_CHILD(node, 0),
			GET_CHILD(node, 1),
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
	EAVLp_node_t*		stinterval[2];

	if (tree->root)
		{
		result = PRIVATE(Validate_Tree_Recurse)(
				tree->root,
				NULL,
				0,
				NULL,
				NULL,
				tree->cbset->compare,
				&verify,
				context->common.cbdata,
				&height,
				stinterval
				);
		}

	return result;
	}


#endif	/* EAVLp_CHECKS_AVAILABLE & EAVL_CHECK_TREE */


/* pTree_check.c */
