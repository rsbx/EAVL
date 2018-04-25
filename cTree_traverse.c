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
#include "EAVL_cTree.h"

#define CHECKS_AVAILABLE	EAVLc_CHECKS_AVAILABLE

#include "cTree_internal.h"
#include "cTree_traverse.h"

#include "callback_internal.h"
#include "checks_internal.h"
#include "eavl_internal.h"
#include "naming_internal.h"
#include "pathe_internal.h"
#include "cTree.h"


int PRIVATE(traverse)(
		EAVLc_node_t*		node,
		EAVL_dir_t		dir,
		order_mask_t		interests,
		unsigned int		safe,
		EAVLc_cbTraverse_t	callback,
		void*			cbtdata,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata
		)
	{
	EAVLc_node_t*		prev;
	EAVL_dir_t		other;
	order_mask_t		coverage;
	unsigned int		pathlen = 0;
	int			result = EAVL_OK;

	if (!interests || interests != (interests & (ORDER_MASK_PRE | ORDER_MASK_IN | ORDER_MASK_POST)))
		{
		return EAVL_ERROR;
		}

	prev = node;			// Not NULL and not node child
	PATHE_SET_SELECT(pathlen++, cbpathe, cbdata, NULL, safe);
	other = EAVL_DIR_OTHER(dir);

	while (callback && node)
		{
		EAVLc_node_t*		next;
		EAVLc_node_t*		T0 = EAVLc_GET_CHILD(node, other);
		EAVLc_node_t*		T1 = EAVLc_GET_CHILD(node, dir);
		EAVLc_node_t*		up;
		unsigned int		limiter = 0;

		if (T1 && prev == T1)
			{
			coverage = interests & ORDER_MASK_POST;
			next = NULL;
			}
		else if (T0 && prev && prev != T0)
			{
			coverage = interests & ORDER_MASK_PRE;
			next = T0;
			}
		else
			{
			coverage = interests
					&
						(
						ORDER_MASK_IN
						| ((T0) ? 0 : ORDER_MASK_PRE)
						| ((T1) ? 0 : ORDER_MASK_POST)
						)
					;
			next = T1;
			}

		PATHE_GET_SELECT(pathlen-1, cbpathe, cbdata, up, safe);

		if (coverage)
			{
			CB_TRAVERSE(node, coverage, safe, callback, cbtdata, limiter, result);
			}

		prev = node;
		if (next && !limiter)
			{
			PATHE_SET_SELECT(pathlen++, cbpathe, cbdata, node, safe);
			node = next;
			}
		else
			{
			pathlen--;
			node = up;
			}
		}

	return result;
	}


/* cTree_traverse.c */
