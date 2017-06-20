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


#ifndef _EAVL_INTERNAL_H
#define _EAVL_INTERNAL_H 1


#include "EAVL.h"


#ifndef QUIET_UNUSED
#define QUIET_UNUSED(var)	var = var
#endif	/* QUIET_UNUSED */


/*
** Setup some shorter names
*/
#define DIR_LEFT		EAVL_DIR_LEFT
#define DIR_RIGHT		EAVL_DIR_RIGHT
#define DIR_NEITHER		EAVL_DIR_NEITHER
#define DIR_OTHER(DIR)		EAVL_DIR_OTHER(DIR)


#define MAX(A, B)		( ((A) >= (B)) ? (A) : (B) )
#define MIN(A, B)		( ((A) <= (B)) ? (A) : (B) )


#define EAVL_SET_LOW(A, BIT)	(EAVL_ADDR(A) | ((BIT) & 0x1u))

#define EAVL_SET_CHILD(NODE, CHILD, DIR)				\
	do								\
		{							\
		(NODE)->child[(DIR)] = (EAVL_node_spec_t)EAVL_SET_LOW(	\
				(CHILD),				\
				EAVL_GET_LOW((NODE)->child[(DIR)])	\
				);					\
		} while (0)

#define EAVL_SET_BAL(NODE, BAL)						\
	do								\
		{							\
		EAVL_dir_t setbal_temp = ((BAL)+1)%3;			\
									\
		(NODE)->child[0] = (EAVL_node_spec_t)EAVL_SET_LOW(	\
				(NODE)->child[0],			\
				(setbal_temp)>>0			\
				);					\
		(NODE)->child[1] = (EAVL_node_spec_t)EAVL_SET_LOW(	\
				(NODE)->child[1],			\
				(setbal_temp)>>1			\
				);					\
		} while (0)


#if DIR_LEFT != EAVL_DIR_LEFT || DIR_LEFT != EAVL_CMP_LEFT
	#error DIR_LEFT/EAVL_DIR_LEFT/EAVL_CMP_LEFT inconsistency
#endif

#if DIR_RIGHT != EAVL_DIR_RIGHT || DIR_RIGHT != EAVL_CMP_RIGHT
	#error DIR_RIGHT/EAVL_DIR_RIGHT/EAVL_CMP_RIGHT inconsistency
#endif

#if DIR_NEITHER != EAVL_DIR_NEITHER || DIR_NEITHER != EAVL_CMP_SAME
	#error DIR_NEITHER/EAVL_DIR_NEITHER/EAVL_CMP_SAME inconsistency
#endif


#endif	/* _EAVL_INTERNAL_H */
