#
# Makefile for libstdcl 
#

### select default OpenCL implementation if not already defined
USE_OPENCL_INCS ?= -I/usr/local/atistream/include
USE_OPENCL_LIB ?= -L/usr/local/atistream/lib/x86_64 -lOpenCL

### enable build of debug version if not already defined
ENABLE_DEBUG_LIBSTDCL ?= 1


DEFS += -DSTDCL_WARN 


INSTALL_INCLUDE_DIR ?= /usr/local/browndeer/include
INSTALL_BIN_DIR ?= /usr/local/browndeer/bin
INSTALL_LIB_DIR ?= /usr/local/browndeer/lib
INSTALL_MAN_DIR ?= /usr/share/man


DEFS += -DENABLE_CLEXPORT ### XXX experimental

ifeq ($(ENABLE_CLGL),1)
DEFS += -DENABLE_CLGL ### XXX experimental
endif

############################################################################
##### DO NOT MODIFY BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING #####
############################################################################

VPATH = .:./man/man1:./man/man3

INCS += $(USE_OPENCL_INCS)
LIBS += $(USE_OPENCL_LIB)
LIBS += -lpthread -ldl

OBJS = clinit.o clcontext.o clfcn.o clmalloc.o clsched.o clarg.o

OBJS_D = $(subst .o,_d.o,$(OBJS)) 

HDRS = stdcl.h clinit.h clfcn.h clcontext.h clsched.h clmalloc.h clarg.h \
	clmalloc_allocator.h clvector.h

MAN3 = claddr.3 clmalloc.3 clndrange_set2d.3 clarg_set.3 clmattach.3 \
	clndrange_set3d.3 clarg_set_global.3 clmdetach.3 clndrange_set.3 \
	clarg_set_local.3 clmsync.3 clopen.3 clclose.3 clndrange_init1d.3 \
	clsizeofmem.3 clerror.3 clndrange_init2d.3 clsym.3 clfork.3 \
	clndrange_init3d.3 clwait.3 clfree.3 clndrange_init.3 stdcl.3 \
	clload.3 clndrange_set1d.3

INSTALL_INCS = $(HDRS)
INSTALL_LIBS = libstdcl.so libstdcl_d.so

#CC = gcc44
CCFLAGS += -g -O2 -fPIC

TARGET = libstdcl.so $(addsuffix .gz,$(MAN3))
ifeq ($(ENABLE_DEBUG_LIBSTDCL),1)
TARGET += libstdcl_d.so 
endif

all: $(TARGET)

.phony: install uninstall clean distclean check_root debug

libstdcl.so: $(OBJS)
	$(CC) $(CCFLAGS) -shared -Wl,-soname=libstdcl.so \
		-o libstdcl.so $(OBJS) $(LIBS)

libstdcl_d.so: $(OBJS_D)
	$(CC) $(CCFLAGS) -shared -Wl,-soname=libstdcl_d.so \
		-o libstdcl_d.so $(OBJS_D) $(LIBS)

.SUFFIXES:
.SUFFIXES: .c .o

#%.o:
#	$(CC) $(CCFLAGS) $(DEFS) $(INCS) -c $*.c

.c.o:
	$(CC) $(CCFLAGS) $(DEFS) $(INCS) -c $<

%_d.o:
	$(CC) $(CCFLAGS) $(DEFS) -DSTDCL_DEBUG $(INCS) -c $*.c -o $*_d.o

%.3.gz:
	-gzip -f -c $*.3 > $*.3.gz


check_root: 
ifneq ($(shell whoami),root)
	$(error install must be done as root)
endif


#install: check_root all
install: check_root $(INSTALL_INCS) $(INSTALL_LIBS) $(addsuffix .gz,$(MAN3))
	test -d $(INSTALL_INCLUDE_DIR) || install -m 755 -d $(INSTALL_INCLUDE_DIR)
	test -d $(INSTALL_LIB_DIR) || install -m 755 -d $(INSTALL_LIB_DIR)
	test -d $(INSTALL_BIN_DIR) || install -m 755 -d $(INSTALL_BIN_DIR)
#	install -m 644 $(addprefix ./include/,$(INSTALL_INCS)) $(INSTALL_INCLUDE_DIR)
	install -m 644 $(INSTALL_INCS) $(INSTALL_INCLUDE_DIR)
	install -m 644 $(INSTALL_LIBS) $(INSTALL_LIB_DIR)
	install -m 644 $(addsuffix .gz,$(MAN3)) $(INSTALL_MAN_DIR)/man3
#	@echo -e "\a\n\n*** IMPORTANT ***\n"
#	@echo -e "Please ensure that " $(INSTALL_BIN_DIR) " is added to PATH"
#	@echo -e "if it is not in the standard search path for executables."
#	@echo -e "For example:"
#	@echo -e "   set path = ($(INSTALL_BIN_DIR) \$$path)\n"
#	@echo -e "Please ensure that $(INSTALL_LIB_DIR) is added to " \
#		"LD_LIBRARY_PATH"
#	@echo -e "if it is not in the standard search path for libraries."
#	@echo -e "For example:"
#	@echo -e "   setenv LD_LIBRARY_PATH $(INSTALL_LIB_DIR):\$$LD_LIBRARY_PATH\n"
#	@echo -e "When compiling programs that use libstcl include the following:" 
#	@echo -e "   INCS += -I$(INSTALL_INC_DIR)" 
#	@echo -e "   LIBS += -L$(INSTALL_LIB_DIR) -lstdcl" 
#	@echo -e "\n"

uninstall: check_root
	rm -f $(addprefix $(INSTALL_INC_DIR)/,$(INSTALL_INCS)) 
	rm -f $(addprefix $(INSTALL_LIB_DIR)/,$(INSTALL_LIBS)) 
	rm -f $(addprefix $(INSTALL_BIN_DIR)/,$(INSTALL_BINS)) 
	rm -f $(addprefix $(INSTALL_MAN_DIR)/man3/,$(addsuffix .gz,$(MAN3)))


clean:
	rm -f *.o 
	rm -f *.1.gz *.3.gz
	rm -f _interceptor.h 
	rm -f *.so


