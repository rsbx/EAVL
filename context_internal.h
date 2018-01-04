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


#ifndef _CONTEXT_INTERNAL_H
#define _CONTEXT_INTERNAL_H 1

#include "EAVL.h"

#include "naming_internal.h"


#if CHECKS_AVAILABLE & EAVL_CHECK_CONTEXT


int FOREIGN(context_, find)(
		EAVL_context_node_t*	root,
		EAVL_context_node_t*	node
		);

void FOREIGN(context_, insert)(
		EAVL_context_node_t**	rootp,
		EAVL_context_node_t*	context
		);

void FOREIGN(context_, remove)(
		EAVL_context_node_t**	rootp,
		EAVL_context_node_t*	context
		);


#define CONTEXT_SET(CONTEXT, NODE, POS, NO_TRUNCATE)			\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_CONTEXT)	\
			{						\
			if ((NODE))					\
				{					\
				FOREIGN(context_, insert)(		\
					&(CONTEXT)->tree->common.contexts,	\
					&(CONTEXT)->common.node		\
					);				\
				}					\
			else						\
				{					\
				FOREIGN(context_, remove)(		\
					&(CONTEXT)->tree->common.contexts,	\
					&(CONTEXT)->common.node		\
					);				\
				}					\
			}						\
		RECENT_SET((CONTEXT), (NODE), (POS), (NO_TRUNCATE));	\
		} while (0)

#define CHECK_CONTEXT(CONTEXT, REQ)					\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_CONTEXT)	\
			{						\
			int		inTree = 0;			\
			int		isLinked;			\
									\
			if (&context->common != context->common.self	\
					|| !context->tree		\
					|| !RECENT_OK((CONTEXT))	\
					)				\
				{					\
				return EAVL_ERROR_CONTEXT;		\
				}					\
									\
			isLinked = (EAVL_OK == FOREIGN(context_, find)(	\
					context->tree->common.contexts,	\
					&context->common.node		\
					));				\
									\
			if ((REQ) && !isLinked)				\
				{					\
				return EAVL_ERROR_CONTEXT;		\
				}					\
									\
			if (context->recent && isLinked)		\
				{					\
				INTREE(context, inTree);		\
				}					\
									\
			if (isLinked && !inTree)			\
				{					\
				return EAVL_ERROR_CONTEXT;		\
				}					\
			}						\
		} while (0)


#else


#define CONTEXT_SET(CONTEXT, NODE, POS, NO_TRUNCATE)			\
	RECENT_SET((CONTEXT), (NODE), (POS), (NO_TRUNCATE));

#define CHECK_CONTEXT(CONTEXT, REQ)	((void)0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CONTEXT */


#define CONTEXT_RESET(CONTEXT, NO_TRUNCATE)				\
	CONTEXT_SET((CONTEXT), NULL, 0, (NO_TRUNCATE))

#define CONTEXT_RESET_ALL(CONTEXT)					\
	do								\
		{							\
		(CONTEXT)->tree->common.contexts = NULL;		\
		} while (0)


#endif	/* _CONTEXT_INTERNAL_H */
