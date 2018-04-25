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


#ifndef _PTREE_H
#define _PTREE_H 1


#include "naming_internal.h"


#define NODE_INIT(NODE)							\
	do								\
		{							\
		(NODE)->EAVLnode.child[0] = 0;				\
		(NODE)->EAVLnode.child[1] = 0;				\
		(NODE)->parent = NULL;					\
		} while (0)

#define NODE_CLEAR_ACTUAL(NODE)						\
	do								\
		{							\
		(NODE)->EAVLnode.child[0] = -1u;			\
		(NODE)->EAVLnode.child[1] = -1u;			\
		(NODE)->parent = (EAVLp_node_t*)-1;			\
		} while (0)

#define NODE_FIXUP(NODE, FORCE, FIXUP, CBDATA)				\
	do								\
		{							\
		CB_FIXUP(						\
				(NODE),					\
				GET_CHILD((NODE), 0),			\
				GET_CHILD((NODE), 1),			\
				(FORCE),				\
				(FIXUP),				\
				(CBDATA)				\
				);					\
		} while (0)


#define GET_CHILD(NODE, DIR)		EAVLp_GET_CHILD((NODE), (DIR))
#define GET_BAL(NODE)			EAVLp_GET_BAL((NODE))

#define GET_PARENT(NODE)		EAVLp_GET_PARENT((NODE))

#define SID(PARENT, CHILD)		((CHILD) == GET_CHILD((PARENT), DIR_RIGHT))

#define SET_CHILDONLY(NODE, CHILD, DIR)	EAVL_SET_CHILD(&(NODE)->EAVLnode, (CHILD), (DIR))
#define SET_BAL(NODE, BAL)		EAVL_SET_BAL(&(NODE)->EAVLnode, (BAL))

#define SET_PARENTONLY(NODE, PARENT)					\
	do								\
		{							\
		(NODE)->parent = (PARENT);				\
		} while (0)

#define SET_CHILD(PARENT, CHILD, DIR)					\
	do								\
		{							\
		SET_CHILDONLY((PARENT), (CHILD), (DIR));		\
		if ((CHILD))						\
			{						\
			SET_PARENTONLY((CHILD), (PARENT));		\
			}						\
		} while (0)

#define INTREE(CONTEXT, RES)						\
	do								\
		{							\
		EAVLp_node_t*		dummy;				\
									\
		(RES) = (EAVL_OK == PRIVATE(find)(			\
				(CONTEXT)->tree->root,			\
				EAVL_FIND_EQ,				\
				(CONTEXT)->tree->cbset->compare,	\
				(CONTEXT)->common.cbdata,		\
				NULL,					\
				(CONTEXT)->recent,			\
				&dummy					\
				));					\
		} while (0)

#define RECENT_OK(CONTEXT)	(1)

#define RECENT_SET(CONTEXT, NODE, POS, NO_TRUNCATE)			\
	do								\
		{							\
		(CONTEXT)->recent = (NODE);				\
		} while (0)


#endif	/* _PTREE_H */
