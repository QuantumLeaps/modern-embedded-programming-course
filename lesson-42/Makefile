##############################################################################
# Product: Makefile for QP/C for Windows and POSIX *HOSTS*
# Last updated for version 6.4.0
# Last updated on  2019-01-09
#
#                    Q u a n t u m  L e a P s
#                    ------------------------
#                    Modern Embedded Software
#
# Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Alternatively, this program may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GNU General Public License and are specifically designed for
# licensees interested in retaining the proprietary status of their code.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Contact information:
# https://www.state-machine.com
# mailto:info@state-machine.com
##############################################################################
#
# examples of invoking this Makefile:
# building configurations: Debug (default), Release, and Spy
# make
# make CONF=rel
# make CONF=spy
# make clean   # cleanup the build
# make CONF=spy clean   # cleanup the build
#
# NOTE:
# To use this Makefile on Windows, you will need the GNU make utility, which
# is included in the QTools collection for Windows, see:
#    http://sourceforge.net/projects/qpc/files/QTools/
#

#-----------------------------------------------------------------------------
# project name:
#
PROJECT := qhsmtst

#-----------------------------------------------------------------------------
# project directories:
#

# list of all source directories used by this project
VPATH := . \

# list of all include directories needed by this project
INCLUDES := -I. \

# location of the QP/C framework (if not provided in an env. variable)
ifeq ($(QPC),)
QPC := c:/qp/qpc
endif

#-----------------------------------------------------------------------------
# project files:
#

# C source files...
C_SRCS := \
	main.c \
	qhsmtst.c

# C++ source files...
CPP_SRCS :=

LIB_DIRS  :=
LIBS      :=

# defines...
# QP_API_VERSION controls the QP API compatibility; 9999 means the latest API
DEFINES   := -DQP_API_VERSION=9999

ifeq (,$(CONF))
	CONF := dbg
endif

#-----------------------------------------------------------------------------
# add QP/C framework (depends on the OS this Makefile runs on):
#
ifeq ($(OS),Windows_NT)

# NOTE:
# For Windows hosts, you can choose:
# - the single-threaded QP/C port (win32-qv) or
# - the multithreaded QP/C port (win32).
#
QP_PORT_DIR := $(QPC)/ports/win32-qv
#QP_PORT_DIR := $(QPC)/ports/win32
LIB_DIRS += -L$(QP_PORT_DIR)/$(CONF)
LIBS     += -lqp -lws2_32

else

# NOTE:
# For POSIX hosts (Linux, MacOS), you can choose:
# - the single-threaded QP/C port (win32-qv) or
# - the multithreaded QP/C port (win32).
#
QP_PORT_DIR := $(QPC)/ports/posix-qv
#QP_PORT_DIR := $(QPC)/ports/posix

C_SRCS += \
	qep_hsm.c \
	qep_msm.c \
	qf_act.c \
	qf_actq.c \
	qf_defer.c \
	qf_dyn.c \
	qf_mem.c \
	qf_ps.c \
	qf_qact.c \
	qf_qeq.c \
	qf_qmact.c \
	qf_time.c \
	qf_port.c

QS_SRCS := \
	qs.c \
	qs_64bit.c \
	qs_rx.c \
	qs_fp.c \
	qs_port.c

LIBS += -lpthread

endif

#============================================================================
# Typically you should not need to change anything below this line

VPATH    += $(QPC)/src/qf $(QP_PORT_DIR)
INCLUDES += -I$(QPC)/include -I$(QPC)/src -I$(QP_PORT_DIR)

#-----------------------------------------------------------------------------
# GNU toolset:
#
# NOTE:
# GNU toolset (MinGW) is included in the QTools collection for Windows, see:
#     http://sourceforge.net/projects/qpc/files/QTools/
# It is assumed that %QTOOLS%\bin directory is added to the PATH
#
CC    := gcc
CPP   := g++
LINK  := gcc    # for C programs
#LINK  := g++   # for C++ programs

#-----------------------------------------------------------------------------
# basic utilities (depends on the OS this Makefile runs on):
#
ifeq ($(OS),Windows_NT)
	MKDIR      := mkdir
	RM         := rm
	TARGET_EXT := .exe
else ifeq ($(OSTYPE),cygwin)
	MKDIR      := mkdir -p
	RM         := rm -f
	TARGET_EXT := .exe
else
	MKDIR      := mkdir -p
	RM         := rm -f
	TARGET_EXT :=
endif

#-----------------------------------------------------------------------------
# build configurations...

ifeq (rel, $(CONF)) # Release configuration ..................................

BIN_DIR := build_rel
# gcc options:
CFLAGS  = -c -O3 -fno-pie -std=c11 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES) -DNDEBUG

CPPFLAGS = -c -O3 -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES) -DNDEBUG

else ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR := build_spy

C_SRCS   += $(QS_SRCS)
VPATH    += $(QPC)/src/qs

# gcc options:
CFLAGS  = -c -g -O -fno-pie -std=c11 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES) -DQ_SPY

CPPFLAGS = -c -g -O -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES) -DQ_SPY

else # default Debug configuration .........................................

BIN_DIR := build

# gcc options:
CFLAGS  = -c -g -O -fno-pie -std=c11 -pedantic -Wall -Wextra -W \
	$(INCLUDES) $(DEFINES)

CPPFLAGS = -c -g -O -fno-pie -std=c++11 -pedantic -Wall -Wextra \
	-fno-rtti -fno-exceptions \
	$(INCLUDES) $(DEFINES)

endif  # .....................................................................

ifndef GCC_OLD
	LINKFLAGS := -no-pie
endif

#-----------------------------------------------------------------------------
C_OBJS       := $(patsubst %.c,%.o,   $(C_SRCS))
CPP_OBJS     := $(patsubst %.cpp,%.o, $(CPP_SRCS))

TARGET_EXE   := $(BIN_DIR)/$(PROJECT)$(TARGET_EXT)
C_OBJS_EXT   := $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT   := $(patsubst %.o,%.d, $(C_OBJS_EXT))
CPP_OBJS_EXT := $(addprefix $(BIN_DIR)/, $(CPP_OBJS))
CPP_DEPS_EXT := $(patsubst %.o,%.d, $(CPP_OBJS_EXT))

# create $(BIN_DIR) if it does not exist
ifeq ("$(wildcard $(BIN_DIR))","")
$(shell $(MKDIR) $(BIN_DIR))
endif

#-----------------------------------------------------------------------------
# rules
#

all: $(TARGET_EXE)

$(TARGET_EXE) : $(C_OBJS_EXT) $(CPP_OBJS_EXT)
	$(CC) $(CFLAGS) $(QPC)/include/qstamp.c -o $(BIN_DIR)/qstamp.o
	$(LINK) $(LINKFLAGS) $(LIB_DIRS) -o $@ $^ $(BIN_DIR)/qstamp.o $(LIBS)

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

.PHONY : clean show

# include dependency files only if our goal depends on their existence
ifneq ($(MAKECMDGOALS),clean)
  ifneq ($(MAKECMDGOALS),show)
-include $(C_DEPS_EXT) $(CPP_DEPS_EXT)
  endif
endif

.PHONY : clean show

clean :
	-$(RM) $(BIN_DIR)/*.o \
	$(BIN_DIR)/*.d \
	$(TARGET_EXE)

show :
	@echo PROJECT      = $(PROJECT)
	@echo TARGET_EXE   = $(TARGET_EXE)
	@echo VPATH        = $(VPATH)
	@echo C_SRCS       = $(C_SRCS)
	@echo CPP_SRCS     = $(CPP_SRCS)
	@echo C_DEPS_EXT   = $(C_DEPS_EXT)
	@echo C_OBJS_EXT   = $(C_OBJS_EXT)
	@echo C_DEPS_EXT   = $(C_DEPS_EXT)
	@echo CPP_DEPS_EXT = $(CPP_DEPS_EXT)
	@echo CPP_OBJS_EXT = $(CPP_OBJS_EXT)
	@echo LIB_DIRS     = $(LIB_DIRS)
	@echo LIBS         = $(LIBS)
	@echo DEFINES      = $(DEFINES)

