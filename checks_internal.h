/*\
***  EAVL_Tree: Embedded AVL Tree
\*/

/*\
*#*  Copyright (c) 2017, Raymond S Brand
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


#ifndef _CHECKS_INTERNAL_H
#define _CHECKS_INTERNAL_H 1


#include "callback_internal.h"
#include "naming_internal.h"


#define CHECK_STD_PRE(CONTEXT, TREE, REQ)				\
	do								\
		{							\
		CHECK_TREE(CONTEXT, TREE);				\
		CHECK_CONTEXT(CONTEXT, REQ);				\
		} while (0)


#define CHECK_STD_POST(TREE, CONTEXT)					\
	do								\
		{							\
		goto check_std_post;					\
		check_std_post:						\
		CHECK_TREE(CONTEXT, TREE);				\
		CHECK_CONTEXT(CONTEXT, 0);				\
		} while (0)


#define RESULT(RESULT)							\
	do								\
		{							\
		result = (RESULT);					\
		goto check_std_post;					\
		} while (0)


#define RETURN	return result


#if CHECKS_AVAILABLE & EAVL_CHECK_PARAM


#define CHECK_NODE_ALIGN(NODE)						\
	do								\
		{							\
		if ((PUBLIC(Checks_Enabled) & EAVL_CHECK_PARAM)		\
				&& (((uintptr_t)(NODE)) & 0x1)		\
				)					\
			{						\
			return EAVL_ERROR_ALIGNMENT;			\
			}						\
		} while (0)


#define CHECK_NODES_ALIGN(COUNT, NODE)					\
	do								\
		{							\
		unsigned int		CNA_i;				\
									\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_PARAM)		\
			{						\
			for (CNA_i=0; CNA_i<(COUNT); CNA_i++)		\
				{					\
				CHECK_NODE_ALIGN((NODE)[CNA_i]);	\
				}					\
			}						\
		} while (0)


#define CHECK_PARAM_NON_NULL(PARAM)					\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_PARAM)		\
			{						\
			if ((PARAM) == NULL)				\
				{					\
				return EAVL_ERROR_PARAMETER;		\
				}					\
			}						\
		} while (0)


#define CHECK_PARAM_DIR(DIR)						\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_PARAM)		\
			{						\
			if ((DIR) > EAVL_DIR_RIGHT)			\
				{					\
				return EAVL_ERROR_PARAMETER;		\
				}					\
			}						\
		} while (0)


#define CHECK_PARAM_REL(REL)						\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_PARAM)		\
			{						\
			if ((REL) > EAVL_FIND_GT)			\
				{					\
				return EAVL_ERROR_PARAMETER;		\
				}					\
			}						\
		} while (0)


#define CHECK_PARAM_ORDER(ORDER)					\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_PARAM)		\
			{						\
			if ((ORDER) > EAVL_ORDER_POST)			\
				{					\
				return EAVL_ERROR_PARAMETER;		\
				}					\
			}						\
		} while (0)


#else


#define CHECK_NODE_ALIGN(NODE)		((void)0)
#define CHECK_NODES_ALIGN(COUNT, NODE)	((void)0)
#define CHECK_PARAM_NON_NULL(PARAM)	((void)0)
#define CHECK_PARAM_DIR(DIR)		((void)0)
#define CHECK_PARAM_REL(REL)		((void)0)
#define CHECK_PARAM_ORDER(ORDER)	((void)0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_PARAM */


#if CHECKS_AVAILABLE & EAVL_CHECK_TREE


int PRIVATE(Validate_Tree)(
		PUBLIC(context_t)*	context,
		PUBLIC(tree_t)*		tree
		);

#define CHECK_TREE(CONTEXT, TREE)					\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_TREE)		\
			{						\
			int		check_error;			\
									\
			check_error = PRIVATE(Validate_Tree)(CONTEXT, TREE);	\
			if (check_error)				\
				{					\
				return check_error;			\
				}					\
			}						\
		} while (0)

#define NODE_CLEAR(NODE)						\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_TREE)		\
			{						\
			NODE_CLEAR_ACTUAL((NODE));			\
			}						\
		} while (0)


#else


#define CHECK_TREE(CONTEXT, TREE)	((void)0)
#define NODE_CLEAR(NODE)		((void)0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_TREE */


#if CHECKS_AVAILABLE & EAVL_CHECK_ORDER


#define CHECK_NODES_ORDER(CONTEXT, COUNT, NODES)			\
	do								\
		{							\
		if (PUBLIC(Checks_Enabled) & EAVL_CHECK_ORDER)		\
			{						\
			PUBLIC(cbCompare_t)*	compare;		\
			void*			cbdata;			\
			unsigned int		CNO_i;			\
			EAVL_dir_t		CNO_cmp;		\
									\
			compare = (CONTEXT)->tree->cbset->compare;	\
			cbdata = (CONTEXT)->common.cbdata;		\
									\
			for (CNO_i=1; CNO_i<COUNT; CNO_i++)		\
				{					\
				CB_COMPARE(				\
					NULL,				\
					NODES[CNO_i-1],			\
					NODES[CNO_i],			\
					compare,			\
					cbdata,				\
					CNO_cmp				\
					);				\
									\
				if (CNO_cmp != EAVL_CMP_RIGHT)		\
					{				\
					return EAVL_ERROR_COMPARE;	\
					}				\
				}					\
			}						\
		} while (0)


#else


#define CHECK_NODES_ORDER(TREE, COUNT, NODES)	((void)0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_ORDER */


#endif	/* _CHECKS_INTERNAL_H */
