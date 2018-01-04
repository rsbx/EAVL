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

#include "EAVL.h"

#define CHECKS_AVAILABLE	EAVL_CHECKS_AVAILABLE

//#include "context.h"
#include "context_internal.h"

#include "eavl_internal.h"
#include "naming_internal.h"
#include "pTree_internal.h"


#if EAVL_CHECKS_AVAILABLE & EAVL_CHECK_CONTEXT


static EAVL_dir_t FOREIGN(context_, node_cmp)(
		void*			ref_value,
		EAVL_context_node_t*	ref_node,
		EAVL_context_node_t*	node,
		void*			data
		)
	{
	uintptr_t	nodev = (uintptr_t)node;
	uintptr_t	refv = (uintptr_t)ref_node;

	QUIET_UNUSED(ref_value);
	QUIET_UNUSED(data);

	return (nodev == refv)
			? EAVL_CMP_SAME
			: (nodev < refv)
				? EAVL_CMP_LEFT
				: EAVL_CMP_RIGHT
			;
	}


int FOREIGN(context_, find)(
		EAVL_context_node_t*		root,
		EAVL_context_node_t*		node
		)
	{
	EAVL_pnode_t*		dummy;

	return FOREIGN(p_, find)(
			root,
			EAVL_FIND_EQ,
			&FOREIGN(context_, node_cmp),
			NULL,
			NULL,
			node,
			&dummy
			);
	}


void FOREIGN(context_, insert)(
		EAVL_context_node_t**		rootp,
		EAVL_context_node_t*		context
		)
	{
	EAVL_pnode_t*		dummy;

	(void) FOREIGN(p_, insert)(
			rootp,
			context,
			&FOREIGN(context_, node_cmp),
			NULL,
			NULL,
			&dummy
			);

	return;
	}


void FOREIGN(context_, remove)(
		EAVL_context_node_t**	rootp,
		EAVL_context_node_t*	context
		)
	{
	if (EAVL_OK == FOREIGN(context_, find)(*rootp, context))
		{
		(void) FOREIGN(p_, remove)(rootp, context, NULL, NULL, NULL);
		}

	return;
	}


#endif	/* EAVL_CHECKS_AVAILABLE & EAVL_CHECK_CONTEXT */


/* context.c */
