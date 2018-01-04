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


#ifndef _EAVL_H
#define _EAVL_H 1


#include <stdint.h>


/*
** Success return values:
*/
#define EAVL_OK			(0)	/* Success			*/
#define EAVL_EXISTS		(1)	/* Node equal to existing node	*/
#define EAVL_NOTFOUND		(2)	/* Node/value not found		*/
#define EAVL_CALLBACK		(3)	/* Retryable callback failure	*/

/*
** Error return values:
*/
#define EAVL_ERROR		(-1)	/* Unspecified error		*/
#define EAVL_ERROR_PARAMETER	(-2)	/* Invalid parameter value	*/
#define EAVL_ERROR_CONTEXT	(-3)	/* Context invalid		*/
#define EAVL_ERROR_TREE		(-4)	/* Tree structure inconsistency	*/
#define EAVL_ERROR_ALIGNMENT	(-5)	/* Parameter misaligned		*/
#define EAVL_ERROR_COMPARE	(-6)	/* Unexpected comparison result	*/
#define EAVL_ERROR_CALLBACK	(-7)	/* Non-retryable callback fail	*/

#define EAVL_ERROR_BUILD	(-12)	/* System/build incompatability	*/
#define EAVL_ERROR_IMPLEMENT	(-13)	/* Not implemented		*/


/*
** Callback success return values:
*/
#define EAVL_CB_OK		EAVL_OK	/* Success			*/
#define	EAVL_CB_FINISHED	(1)	/* Discontinue calling callback	*/
#define EAVL_CB_LIMIT		(2)	/* Do not visit node children	*/
#define EAVL_CB_CALLBACK	(3)	/* Retryable callback failure	*/

/*
** Callback error return values:
*/
#define EAVL_CB_ERROR		(-1)	/* Failed; check cbdata		*/


/*
** Balance and traversal direction values:
*/
#define EAVL_DIR_LEFT		(0)
#define EAVL_DIR_RIGHT		(1)
#define EAVL_DIR_NEITHER	(2)
#define EAVL_DIR_OTHER(DIR)	(1-(DIR))


/*
** Traversal order values:
*/
#define EAVL_ORDER_PRE		(0)
#define EAVL_ORDER_IN		(1)
#define EAVL_ORDER_POST		(2)
#define EAVL_ORDER_INVERSE(METHOD)	(2-(METHOD))


/*
** Compare result values:
*/
#define EAVL_CMP_LEFT		EAVL_DIR_LEFT
#define EAVL_CMP_RIGHT		EAVL_DIR_RIGHT
#define EAVL_CMP_SAME		EAVL_DIR_NEITHER


/*
** Node find relativity values:
*/
#define EAVL_FIND_LT		(0)
#define EAVL_FIND_LE		(1)
#define EAVL_FIND_EQ		(2)
#define EAVL_FIND_GE		(3)
#define EAVL_FIND_GT		(4)


/*
** CHECK values:
**
**	Useful for debugging EAVL and client code.
**
** Warning:
**	Although the the main tree manipulation code is non-recursive,
**	the code that these flags enable may be recursive with an
**	expected recursion depth proportional to log2(tree node count).
*/
#define EAVL_CHECK_TREE		(1u<<0)	/* Validate tree before each op	*/
#define EAVL_CHECK_CONTEXT	(1u<<1)	/* Validate context consistency	*/
#define EAVL_CHECK_ORDER	(1u<<2)	/* Validate node ordering also	*/
#define EAVL_CHECK_PARAM	(1u<<3)	/* Validate parameters		*/
#define EAVL_CHECK_CALLBACK	(1u<<4)	/* Validate callback results	*/

#define EAVL_CHECK_ALL	((1u<<5)-1)	/* Enable all available checks	*/


typedef uintptr_t		EAVL_node_spec_t;
typedef struct
	{
	EAVL_node_spec_t	child[2];
	}			EAVL_node_t;
typedef struct EAVL_pnode	EAVL_pnode_t;
struct EAVL_pnode
	{
	EAVL_node_t		EAVLnode;
	EAVL_pnode_t*		parent;
	};
typedef EAVL_pnode_t		EAVL_context_node_t;
typedef struct EAVL_context_common	EAVL_context_common_t;
typedef struct
	{
	EAVL_context_node_t*	contexts;
	uintptr_t		associations;
	}			EAVL_tree_common_t;
struct EAVL_context_common
	{
	EAVL_context_node_t	node;
	void*			cbdata;
	EAVL_context_common_t*	self;
	};
typedef unsigned int		EAVL_dir_t;
typedef unsigned int		EAVL_order_t;
typedef unsigned int		EAVL_rel_t;


#define EAVL_ADDR(ADDR)		((uintptr_t)(ADDR) & ~(uintptr_t)0x1u)
#define EAVL_NODE(A)		((EAVL_node_t *)EAVL_ADDR(A))
#define EAVL_GET_LOW(ADDR)	((uintptr_t)(ADDR) & (uintptr_t)0x1u)

#define EAVL_GET_CHILD(NODE, DIR)	EAVL_NODE((NODE)->child[(DIR)])

#define EAVL_GET_BAL(NODE)		((EAVL_dir_t)(((EAVL_GET_LOW(	\
						(NODE)->child[0]	\
						)			\
					| EAVL_GET_LOW(			\
						(NODE)->child[1]	\
						)<<1)+2) % 3		\
					))


#endif	/* _EAVL_H */
