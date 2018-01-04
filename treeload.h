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


#ifndef _TREELOAD_H
#define _TREELOAD_H 1


#define LOAD_INIT(NODES, NODE, CB)					\
		(*(CB))((NODES), (NODE))

#define LOAD_SETCHILD(NODES, PARENT, CHILD, DIR, CB)			\
		(*(CB))((NODES), (PARENT), (CHILD), (DIR))

#define LOAD_SETBAL(NODES, NODE, BAL, CB)				\
		(*(CB))((NODES), (NODE), (BAL))

#define LOAD_FIXUP(NODES, NODE, CB, CBCB, DATA, RES)			\
	do								\
		{							\
		if ((CBCB))						\
			{						\
			int		NFres;				\
									\
			NFres = (*(CB))((NODES), (NODE), (CBCB), (DATA));	\
			if (NFres != EAVL_CB_OK)			\
				{					\
				(CBCB) = NULL;				\
				}					\
			if (NFres != EAVL_CB_OK && NFres != EAVL_CB_FINISHED)	\
				{					\
				(RES) = EAVL_ERROR_CALLBACK;		\
				}					\
			}						\
		} while (0)


#endif	/* _TREELOAD_H */
