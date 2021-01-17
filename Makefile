#
# Copyright (C) 2021 Linzhi Ltd.
#
# This work is licensed under the terms of the MIT License.
# A copy of the license can be found in the file COPYING.txt
#

NAME = kunai

CFLAGS = -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
         -Wmissing-prototypes -Wmissing-declarations \
	 -I../libcommon
override CFLAGS += $(CFLAGS_OVERRIDE)
LDLIBS = -L../$(OBJDIR)libcommon -lcommon -lmosquitto -lpthread

OBJS = $(NAME).o daemon.o mqtt.o run.o cfg.o

include Makefile.c-common


.PHONY:		all spotless

all::		| $(OBJDIR:%/=%)
all::		$(OBJDIR)$(NAME)

arm:
		$(MAKE) CC=arm-linux-cc OBJDIR=arm/ all

$(OBJDIR:%/=%):
		mkdir -p $@

$(OBJDIR)$(NAME): $(OBJS_IN_OBJDIR)

spotless::
		rm -f $(NAME)
		rm -rf arm
