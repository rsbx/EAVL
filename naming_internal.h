/*\
***  EAVL_Tree: Embedded AVL Tree
\*/

/*\
*#*  Copyright (c) 2012, 2017, Raymond S Brand
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


#ifndef _NAMING_INTERNAL_H
#define _NAMING_INTERNAL_H 1


/*
** Prototypes for the internal API.
*/
#define PUBLIC(NAME)	JOIN3(LIBRARY, PREFIX, NAME)

#define PRIVATE(NAME)	JOIN8(						\
		LIBRARY,						\
		PREFIX,							\
		NAME,							\
		LIB_VERSION_API,					\
		LIB_VERSION_FEATURE,					\
		LIB_VERSION_PATCH,					\
		LIB_VERSION_LOCAL,					\
		LIB_VERSION_BUILD					\
		)

#define FOREIGN(PREFIX, NAME)	JOIN8(					\
		LIBRARY,						\
		PREFIX,							\
		NAME,							\
		LIB_VERSION_API,					\
		LIB_VERSION_FEATURE,					\
		LIB_VERSION_PATCH,					\
		LIB_VERSION_LOCAL,					\
		LIB_VERSION_BUILD					\
		)


#define JOIN3(L, P, N)		JOIN3_1(L, P, N)
#define JOIN3_1(L, P, N)	L ## P ## N

#define JOIN8(LIBRARY, PREFIX, NAME, V1, V2, V3, V4, V5)		\
		JOIN8_1(LIBRARY, PREFIX, NAME, V1, V2, V3, V4, V5)

#define JOIN8_1(LIBRARY, PREFIX, NAME, V1, V2, V3, V4, V5)		\
		LIBRARY ## PREFIX ## internal_ ## NAME ## _		\
		## A ## V1						\
		## F ## V2						\
		## P ## V3						\
		## L ## V4						\
		## B ## V5


#endif	/* _NAMING_INTERNAL_H */
