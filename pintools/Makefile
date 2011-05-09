##
## PIN tools
##

##############################################################
#
# Here are some things you might want to configure
#
##############################################################

PIN_HOME=../..
PIN_KIT=../../../..

TARGET_COMPILER?=gnu
ifdef OS
    ifeq (${OS},Windows_NT)
        TARGET_COMPILER=ms
    endif
endif

##############################################################
#
# include *.config files
#
##############################################################

ifeq ($(TARGET_COMPILER),gnu)
	TARGET=ia32
    include ../../makefile.gnu.config
    CXXFLAGS ?= -g -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT) -I../watchpoint_system
endif

ifeq ($(TARGET_COMPILER),ms)
    include ../../makefile.ms.config
    DBG?=
endif

##############################################################
#
# Tools sets
#
##############################################################


#TOOL_ROOTS = rerun_tls grace_tls grace_omp taint
TOOL_ROOTS = rerun_tls
STATIC_TOOL_ROOTS =

TOOLS = $(TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))
STATIC_TOOLS = $(STATIC_TOOL_ROOTS:%=$(OBJDIR)%$(SATOOL_SUFFIX))
SPECIAL_TOOLS = $(SPECIAL_TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))
APPS = $(APP_NAMES:%=$(OBJDIR)%$(EXEEXT))

##############################################################
#
# build rules
#
##############################################################

all: tools apps
tools: $(TOOLS) $(STATIC_TOOLS) $(SPECIAL_TOOLS)
apps: $(APPS)
test: $(TOOL_ROOTS:%=%.test) $(STATIC_TOOL_ROOTS:%=%.test) $(SPECIAL_TOOL_ROOTS:%=%.test)


##############################################################
#
# build rules
#
##############################################################

$(APPS): $(OBJDIR)make-directory

$(OBJDIR)make-directory:
	mkdir -p $(OBJDIR)
	touch $(OBJDIR)make-directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o : %.cpp $(OBJDIR)make-directory
	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $<

$(TOOLS): $(PIN_LIBNAMES)

$(TOOLS): %$(PINTOOL_SUFFIX) : %.o
	${PIN_LD} $(PIN_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $< ${PIN_LPATHS} $(PIN_LIBS) $(DBG)

$(STATIC_TOOLS): $(PIN_LIBNAMES)

$(STATIC_TOOLS): %$(SATOOL_SUFFIX) : %.o
	${PIN_LD} $(PIN_SALDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $< ${PIN_LPATHS} $(SAPIN_LIBS) $(DBG)

## cleaning
clean:
	-rm -rf $(OBJDIR) *.out *.tested *.failed *.makefile.copy *.out.*.*
