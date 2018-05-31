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
#include "EAVL_sTree.h"

#define CHECKS_AVAILABLE	EAVLs_CHECKS_AVAILABLE

//#include "sTree_checks.h"
#include "sTree_internal.h"

#include "callback_internal.h"
#include "checks_internal.h"
#include "eavl_internal.h"
#include "naming_internal.h"
#include "pathe_internal.h"
#include "sTree.h"


unsigned int	PUBLIC(Checks_Available) = EAVLs_CHECKS_AVAILABLE & EAVL_CHECK_ALL;
unsigned int	PUBLIC(Checks_Enabled) = 0;


#if CHECKS_AVAILABLE & EAVL_CHECK_CONTEXT


int PRIVATE(Path_match)(
		EAVLs_context_t*	context
		)
	{
	EAVLs_node_t*		node;
	EAVLs_cbPathe_t		cbpathe;
	void*			cbdata;
	unsigned int		i;

	if (!context->recent || !context->pathlen
			|| !context->tree || !context->tree->root
			)
		{
		return EAVL_ERROR_CONTEXT;
		}

	cbpathe = context->cbpathe;
	cbdata = context->common.cbdata;

	node = context->recent;
	for (i=context->pathlen-1; i; i--)
		{
		EAVLs_node_t*		T;

		PATHE_GET_DANGER(i, cbpathe, cbdata, T);

		if (node != GET_CHILD(T, SID(T, node)))
			{
			return EAVL_ERROR_CONTEXT;
			}

		node = T;
		}

	if (node != context->tree->root)
		{
		return EAVL_ERROR_CONTEXT;
		}

	PATHE_GET_DANGER(0, cbpathe, cbdata, node);
	if (node)
		{
		return EAVL_ERROR_CONTEXT;
		}

	return EAVL_OK;
	}


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CONTEXT */


#if CHECKS_AVAILABLE & EAVL_CHECK_TREE


static int PRIVATE(Validate_Tree_Recurse)(
		EAVLs_node_t*		node,
		EAVLs_node_t*		fenceL,
		EAVLs_node_t*		fenceR,
		EAVLs_cbCompare_t	compare,
		EAVLs_cbVerify_t*	verifyp,
		void*			cbdata,
		int*			heightp,
		EAVLs_node_t**		myinterval
		)
	{
	int			result;
	int			height[2] = {0, 0};
	unsigned int		i;
	EAVL_dir_t		baldir;
	EAVLs_node_t*		T;
	EAVLs_node_t*		myfence[2];
	EAVLs_node_t*		stinterval[2];

	myfence[0] = fenceL;
	myfence[1] = fenceR;

	myinterval[0] = node;
	myinterval[1] = node;

	for (i=0; i<2; i++)
		{
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
		EAVLs_context_t*	context,
		EAVLs_tree_t*		tree
		)
	{
	int			result = EAVL_OK;
	int			height;
	EAVLs_cbVerify_t	verify = tree->cbset->verify;
	EAVLs_node_t*		stinterval[2];

	if (tree->root)
		{
		result = PRIVATE(Validate_Tree_Recurse)(
				tree->root,
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


#endif	/* EAVLs_CHECKS_AVAILABLE & EAVL_CHECK_TREE */


/* sTree_check.c */
