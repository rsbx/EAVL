# (Not so) simple make file

#  Copyright (c) 2017, Raymond S Brand
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#   * Redistributions in source or binary form must carry prominent
#     notices of any modifications.
#
#   * Neither the name of the Raymond S Brand nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.


LIB		= EAVL

PREFIX_PTREE	= p_
PREFIX_STREE	= s_
PREFIX_CTREE	= c_
PREFIX_COMMON	= _

VERSION_API	= WIP
VERSION_FEATURE	= 0
VERSION_PATCH	= 0
VERSION_LOCAL	= 0
VERSION_BUILD	= 6
VERSION_SPECIAL	= Strawberry Mongoose

CHECKS_LIB	= 255
CHECKS_PTREE	= $(CHECKS_LIB)
CHECKS_STREE	= $(CHECKS_LIB)
CHECKS_CTREE	= $(CHECKS_LIB)
CHECKS_COMMON	= ($(CHECKS_PTREE) | $(CHECKS_STREE) | $(CHECKS_CTREE))


CMDS		:= test_pTree test_pTree_stress
CMDS		+= test_sTree test_sTree_badpathe test_sTree_stress
CMDS		+= test_cTree test_cTree_badpathe test_cTree_stress
CMD_SRCS	:= $(CMDS:%=%.c)

LIB_SO		:= lib$(LIB).so
LIB_NAME	:= $(LIB_SO).$(VERSION_API).$(VERSION_FEATURE)
LIB_FILE	:= $(LIB_NAME).$(VERSION_PATCH).$(VERSION_LOCAL).$(VERSION_BUILD)

LIB_PTREE_SRCS	:= pTree.c pTree_checks.c
LIB_STREE_SRCS	:= sTree.c sTree_checks.c 
LIB_CTREE_SRCS	:= cTree.c cTree_checks.c cTree_traverse.c
LIB_COMMON_SRCS	:= context.c treeload.c

LIB_PTREE_OBJS	:= $(LIB_PTREE_SRCS:%.c=%.o)
LIB_STREE_OBJS	:= $(LIB_STREE_SRCS:%.c=%.o)
LIB_CTREE_OBJS	:= $(LIB_CTREE_SRCS:%.c=%.o)
LIB_COMMON_OBJS	:= $(LIB_COMMON_SRCS:%.c=%.o)

LIB_SRCS	:= $(LIB_PTREE_SRCS) $(LIB_STREE_SRCS) $(LIB_CTREE_SRCS) $(LIB_COMMON_SRCS)
LIB_OBJS	:= $(LIB_SRCS:%.c=%.o)

ALL_SRCS	:= $(CMD_SRCS) $(LIB_SRCS)
ALL_OBJS	:= $(ALL_SRCS:%.c=%.o)

TARGETS		:= $(CMDS) $(LIB_SO) $(LIB_NAME) $(LIB_FILE)


all:	$(TARGETS)

CC	= colorgcc
LD	= ld
LN	= ln -s
RM	= rm -f

CFLAGS	+= -O3 -Wall -Werror
CFLAGS	+= -std=c99
CFLAGS	+= -pedantic
CFLAGS	+= -Wextra -Wunused -Wuninitialized -Wundef -Wshadow -Wconversion
CFLAGS	+= -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations

$(LIB_OBJS):CFLAGS += -fpic
$(LIB_OBJS):CFLAGS += -DLIB_VERSION_API=$(VERSION_API)
$(LIB_OBJS):CFLAGS += -DLIB_VERSION_FEATURE=$(VERSION_FEATURE)
$(LIB_OBJS):CFLAGS += -DLIB_VERSION_PATCH=$(VERSION_PATCH)
$(LIB_OBJS):CFLAGS += -DLIB_VERSION_LOCAL=$(VERSION_LOCAL)
$(LIB_OBJS):CFLAGS += -DLIB_VERSION_BUILD=$(VERSION_BUILD)
$(LIB_OBJS):CFLAGS += "-DLIB_VERSION_SPECIAL=\"$(VERSION_SPECIAL)\""
$(LIB_OBJS):CFLAGS += "-DLIBRARY=$(LIB)"

$(LIB_PTREE_OBJS):CFLAGS += "-D$(LIB)$(PREFIX_PTREE)CHECKS_AVAILABLE=$(CHECKS_PTREE)"
$(LIB_PTREE_OBJS):CFLAGS += -DPREFIX=$(PREFIX_PTREE)

$(LIB_STREE_OBJS):CFLAGS += "-D$(LIB)$(PREFIX_STREE)CHECKS_AVAILABLE=$(CHECKS_STREE)"
$(LIB_STREE_OBJS):CFLAGS += -DPREFIX=$(PREFIX_STREE)

$(LIB_CTREE_OBJS):CFLAGS += "-D$(LIB)$(PREFIX_CTREE)CHECKS_AVAILABLE=$(CHECKS_CTREE)"
$(LIB_CTREE_OBJS):CFLAGS += -DPREFIX=$(PREFIX_CTREE)

$(LIB_COMMON_OBJS):CFLAGS += "-D$(LIB)$(PREFIX_COMMON)CHECKS_AVAILABLE=$(CHECKS_COMMON)"
$(LIB_COMMON_OBJS):CFLAGS += -DPREFIX=$(PREFIX_COMMON)


test_pTree:	test_pTree.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_pTree_stress:	test_pTree_stress.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_sTree:	test_sTree.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_sTree_badpathe:	test_sTree_badpathe.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_sTree_stress:	test_sTree_stress.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_cTree:	test_cTree.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_cTree_badpathe:	test_cTree_badpathe.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@

test_cTree_stress:	test_cTree_stress.o $(LIB_SO) $(LIB_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -l$(LIB) $(LDLIBS) -o $@


$(LIB_SO):	$(LIB_FILE)
	@echo "\$$(LN) $@"
	@$(RM) $@
	@$(LN) $(LIB_FILE) $@

$(LIB_NAME):	$(LIB_FILE)
	@echo "\$$(LN) $@"
	@$(RM) $@
	@$(LN) $(LIB_FILE) $@

$(LIB_FILE):	$(LIB_OBJS)
	@echo "\$$(LD) $@"
	@$(RM) $(LIB_NAME) $(LIB_SO)
	@$(LD) -shared -soname $(LIB_NAME) -o $(LIB_FILE) -lc $^


DEPS	:= $(ALL_SRCS:%.c=.%.d)

.%.d: %.c Makefile
	@echo "\$$(CC) -MM $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCS) -MM -MF $@ -MQ $(<:%.c=%.o) -MQ '$@' $<

%.o: %.c .%.d
	@echo "\$$(CC) $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ -c $<


clean::
	@$(RM) $(TARGETS) $(ALL_OBJS) $(DEPS) $(LIB_SO).*

symbols::	$(LIB_FILE)
	@nm -D $(LIB_FILE)

symbols_all::	$(LIB_FILE)
	@readelf -Ws $(LIB_FILE)

-include $(DEPS)
