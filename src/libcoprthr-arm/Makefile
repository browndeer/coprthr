### 
### Makefile for libcoprthr
###

prefix=/usr/local/browndeer
exec_prefix=${prefix}

### this useless define is needed to silence useless autoconf warning
datarootdir=${prefix}/share

LIBELF_INC=-I/usr/local/include
LIBELF_LIB=-L/usr/local/lib -lelf

#ENABLE_NCPU=
#ENABLE_ATIGPU=@ENABLE_ATIGPU@
#USE_BDT_BUILTINS=@ENABLE_BDT_BUILTINS@

#DEFS += -DXCL_DEBUG 
#DEFS += -DXCL_WARN

#DEFS += -DUSE_FAST_SETJMP	# ONLY USE THIS IF YOU KNOW WHAT YOU ARE DOING

### select default install paths 
INSTALL_BIN_DIR=${exec_prefix}/bin
INSTALL_LIB_DIR=${exec_prefix}/lib
INSTALL_INCLUDE_DIR=${prefix}/include
INSTALL_MAN_DIR=${datarootdir}/man

#ATISTREAMSDK = /usr/local/atistream/atistream_sdk-2.0/
#ATISTREAMSDK = /usr/local/atistream/
#ATISTREAMSDK=@AMD_SDK@
#ATISTREAMSDK21=@ATI_SDK_21@

############################################################################
##### DO NOT MODIFY BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING #####
############################################################################

XCL_OBJS = xcl_platform.o xcl_device.o xcl_context.o xcl_command_queue.o \
	xcl_enqueue.o xcl_event.o xcl_program.o xcl_kernel.o xcl_memobj.o \
	xcl_flush.o

IMP_OBJS = platform.o device.o context.o command_queue.o event.o \
	environment.o \
	program.o kernel.o memobj.o \
	cmdsched.o cmdcall.o cmdcall_arm.o \
	elf_cl.o compiler_arm.o \
	vcore.o \
	watermark.o


RT_OBJS = __vcore_rt.o



#DEFS += -DATISTREAMSDK=\"$(ATISTREAMSDK)\" 
#DEFS += -DATISTREAMSDK21=\"$(ATISTREAMSDK21)\" 
DEFS += -DINSTALL_LIB_DIR=\"$(INSTALL_LIB_DIR)\" 

#ifeq ($(ENABLE_ATIGPU),1)
#DEFS += -DENABLE_ATIGPU 
#INCS += -I$(ATISTREAMSDK)/include
#LIBS += -laticalrt -laticalcl
#IMP_OBJS += compiler_atigpu.o cmdcall_atigpu.o ircompiler_atigpu.o
#endif

#ifeq (,1)
DEFS += -DENABLE_NCPU
#endif

#ifeq (@ENABLE_BDT_BUILTINS@,1)
#DEFS += -DUSE_BDT_BUILTINS
#endif

#ifeq (1,1)
#DEFS += -DXCL_DEBUG 
#endif

######################################################################

TOPDIR ?= ../../

INCS += -I. -I$(TOPDIR)/include 

#ifeq (freebsd,$(patsubst freebsd%,freebsd,linux-gnu))
#INCS += -I/usr/local/include
#else
#INCS += -I/usr/local/include/libelf
#endif
INCS += $(LIBELF_INC)

LIBS += $(LIBELF_LIB)

#LIBS += -lfl -L/usr/local/lib -lelf  -lpthread -lrt
LIBS += -lfl  -lpthread -lrt
ifeq (freebsd,$(patsubst freebsd%,freebsd,linux-gnu))
LIBS += -lc 
else
LIBS += -ldl 
endif


OBJS = $(XCL_OBJS) $(IMP_OBJS)

OBJS_D = $(subst .o,_d.o,$(OBJS))

DEFS += \
	-DINSTALL_BIN_DIR=\"$(INSTALL_BIN_DIR)\" \
	-DINSTALL_LIB_DIR=\"$(INSTALL_LIB_DIR)\" \
	-DINSTALL_INCLUDE_DIR=\"$(INSTALL_INCLUDE_DIR)\" 

CCFLAGS += -O0 -fPIC -fbounds-check -g
#CCFLAGS += -O1 -fPIC 
CCFLAGS_VCORE_RT = -O0 -fPIC -funroll-loops -fstrict-aliasing -g
#CCFLAGS_VCORE_RT = -O1 -fPIC -funroll-loops -fstrict-aliasing


#YACC = yacc
YACC = bison
YACCCFLAGS =  -y -v 

LEX = flex
LEXFLAGS = 

XCLNM_YYPREFIX = __xclnm_

TARGET = libcoprthr.so $(RT_OBJS) 
#ifeq ($(ENABLE_DEBUG_LIBCOPRTHR),1)
TARGET += libcoprthr_d.so
#endif

all: $(TARGET) 

.PHONY: clean install uninstall

.SUFFIXES:
.SUFFIXES: .l .y .S .s .c .o


.c.o:
	$(CC) $(CCFLAGS) $(DEFS) $(INCS) -c $<

.s.o:
	$(CC) $(CCFLAGS) $(DEFS) $(INCS) -c $<

.S.o:
	$(CC) $(CCFLAGS) $(DEFS) $(INCS) -c $<

%_d.o:
	$(CC) $(CCFLAGS) $(DEFS) -DXCL_DEBUG $(INCS) -c $*.c -o $*_d.o


libcoprthr.so: $(OBJS) $(RT_OBJS)
	$(CC) $(CCFLAGS) $(INCS) \
		-shared -Wl,-soname,libcoprthr.so -o libcoprthr.so  $(OBJS) $(LIBS)

libcoprthr_d.so: $(OBJS_D) $(RT_OBJS)
	$(CC) $(CCFLAGS) $(INCS) \
		-shared -Wl,-soname,libcoprthr_d.so -o libcoprthr_d.so  $(OBJS_D) $(LIBS)

#__vcore_rt.bc: __vcore_rt.c
#	clang -c -emit-llvm $(DEFS) $(INCS) -o __vcore_rt.bc __vcore_rt.c

__vcore_rt.o: __vcore_rt.c
	$(CC) -c $(CCFLAGS_VCORE_RT) $(DEFS) $(INCS) -o __vcore_rt.o __vcore_rt.c




#check_root:
#ifneq ($(shell whoami),root)
#	$(error install must be done as root)
#endif

debug:
	export ENABLE_DEBUG=1
	make -f Makefile all


#install: check_root
install: 
	test -d $(INSTALL_INCLUDE_DIR) || install -m 755 -d $(INSTALL_INCLUDE_DIR)
	test -d $(INSTALL_LIB_DIR) || install -m 755 -d $(INSTALL_LIB_DIR)
	test -d $(INSTALL_BIN_DIR) || install -m 755 -d $(INSTALL_BIN_DIR)
	install -m 755 libcoprthr.so $(INSTALL_LIB_DIR)
	install -m 755 libcoprthr_d.so $(INSTALL_LIB_DIR)
	install -m 755 $(RT_OBJS) $(INSTALL_LIB_DIR)
#	install -m 755 $(RT_BC) $(INSTALL_LIB_DIR)
	install -m 755 vcore.h $(INSTALL_INCLUDE_DIR)
	install -m 755 __libcoprthr.h $(INSTALL_INCLUDE_DIR)
	test -d /etc/OpenCL/vendors || install -m 755 -d /etc/OpenCL/vendors
	echo $(INSTALL_LIB_DIR)/libcoprthr.so > /etc/OpenCL/vendors/coprthr.icd
#	ln -sf $(INSTALL_LIB_DIR)/libcoprthr.so $(INSTALL_LIB_DIR)/libOpenCL.so

uninstall: 
	rm -f $(INSTALL_LIB_DIR)/libcoprthr.so $(INSTALL_LIB_DIR)/libcoprthr_d.so
	rm -f $(addprefix $(INSTALL_LIB_DIR)/,$(RT_OBJS))
	rm -f $(addprefix $(INSTALL_INCLUDE_DIR)/,vcore.h)

clean:
	rm -f *.o *.so

distclean: clean


