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

#include "treeload.h"
#include "treeload_internal.h"

#include "eavl_internal.h"
#include "naming_internal.h"


/*
** Builds an AVL tree from a sorted array of node pointers from the leafs and
** working towards the root (bottom-up).  No recursion and no stack.
**
**	O(n) work
**	O(1) extra space
**
** This describes the top-down build method.  The bottom-up is the reverse:
** (1) Start with a single span of node (pointers) with an assumed extra node
**	at the end.
** (2) Each level of the (final) tree has twice as many spans as the adjacent
**	level close to the root.
** (3) Find the mid-point of each span in a level and add it to the tree
**	before moving to the next level farther from the root.
** (4) Calculate the mid-point of a span so that it will only be the last node
**	in spans in levels farther away from the root.  This is the trick.
*/
int PRIVATE(load)(
		unsigned int*		rootindex,
		unsigned int		count,
		void**			nodep,
		FOREIGN(_, load_cbset_t)*	cbset,
		FOREIGN(_, load_cbFixup_t)	fixup,
		void*			cbdata
		)
	{
	unsigned int		delta;

	unsigned int		lower_ints = 1;
	unsigned int		lower_base;
	unsigned int		lower_mask;
	unsigned int		lower_shift = 0;

	unsigned int		upper_ints;
	unsigned int		upper_base;
	unsigned int		upper_mask;
	unsigned int		upper_shift;

	unsigned int		i;
	int			result = EAVL_OK;

	if (((count+1)<<1) < count)
		{
		return EAVL_ERROR_PARAMETER;
		}

	for (i=0; i<count; i++)
		{
		LOAD_INIT(nodep, i, cbset->init);
		LOAD_FIXUP(nodep, i, cbset->fixup, fixup, cbdata, result);
		}

	while ((lower_ints<<1) < (count+1))
		{
		lower_ints <<= 1;
		lower_shift++;
		}

	delta = (count+1) & (lower_ints-1);

	lower_base = (delta) ? 1 : 2;
	lower_mask = lower_ints-1;

	upper_ints = lower_ints>>1;
	upper_base = lower_base<<1;
	upper_shift = lower_shift-1;
	upper_mask = upper_ints-1;

	while (upper_ints)
		{
		unsigned int		upper_start = 0;
		unsigned int		upper_next = 0;
		unsigned int		upper_fract = 0;
		unsigned int		upper_size;
		unsigned int		upper_mid;

		unsigned int		lower_start = 0;
		unsigned int		lower_next = 0;
		unsigned int		lower_fract = 0;
		unsigned int		lower_size;
		unsigned int		lower_mid;

		unsigned int		next_fract = 0;

		for (i=0; i<upper_ints; i++)
			{
			unsigned int		j;

			upper_size = upper_base;
			upper_fract += delta;
			upper_size += upper_fract >> upper_shift;
			upper_fract &= upper_mask;
			upper_next += upper_size;

			upper_mid = upper_start + lower_base + ((lower_fract + delta) >> lower_shift) - 1;

			for (j=0; j<2; j++)
				{
				lower_size = lower_base;
				lower_fract += delta;
				lower_size += lower_fract >> lower_shift;
				lower_fract &= lower_mask;
				lower_next += lower_size;

				if (lower_size > 1)
					{
					lower_mid = lower_start;

					if (lower_size > 2)
						{
						lower_mid += (lower_base>>1) + ((next_fract + delta) >> (lower_shift+1)) - 1;
						}

					LOAD_SETCHILD(nodep, upper_mid, lower_mid, j, cbset->setchild);
					}

				next_fract += delta;
				/* next_fract &= (lower_ints<<1)-1; */
				next_fract += delta;
				next_fract &= (lower_ints<<1)-1;

				lower_start = lower_next;
				}

			if (upper_size == lower_size<<1)
				{
				/* The 2 lower spans are the same size so the
				** 2 subtrees will have the same height.
				*/
				LOAD_SETBAL(nodep, upper_mid, DIR_NEITHER, cbset->setbal);
				}
			else
				{
				/* The only for the subtrees to have different
				** heights is for the lighter subtree (shorter
				** span) to be "full".  The other subtree will
				** be taller and the last row will have only 1
				** node.  A full subtree will have 2^k-1 nodes.
				*/
				unsigned int		smaller = MIN(upper_size>>1, lower_size);

				if (smaller & (smaller-1))
					{
					/* smaller is not power of 2 */
					LOAD_SETBAL(nodep, upper_mid, DIR_NEITHER, cbset->setbal);
					}
				else
					{
					LOAD_SETBAL(nodep, upper_mid, smaller == (lower_size) ? DIR_LEFT : DIR_RIGHT, cbset->setbal);
					}
				}

			LOAD_FIXUP(nodep, upper_mid, cbset->fixup, fixup, cbdata, result);

			upper_start = upper_next;
			}

		lower_ints = upper_ints;	/* lower_ints >>= 1;	*/
		lower_base = upper_base;	/* lower_base <<= 1;	*/
		lower_mask = upper_mask;	/* lower_mask = lower_ints-1; */
		lower_shift = upper_shift;	/* lower_shift--;	*/

		upper_ints >>= 1;
		upper_base <<= 1;
		upper_mask = upper_ints-1;
		upper_shift--;
		}

	i = 0 + (lower_base>>1) + (delta>>1) - 1;
	LOAD_FIXUP(nodep, i, cbset->fixup, fixup, cbdata, result);
	*rootindex = i;

	return result;
	}


/* common.c */
