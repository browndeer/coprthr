ENABLE_NCPU = 1
ENABLE_ATIGPU = 0
USE_BDT_BUILTINS = 0

LIB_NAME = libocl

#DEFS += -DXCL_DEBUG 
#DEFS += -DXCL_WARN

#DEFS += -DUSE_FAST_SETJMP	# ONLY USE THIS IF YOU KNOW WHAT YOU ARE DOING


INSTALL_BIN_DIR ?= /usr/local/browndeer/bin
INSTALL_LIB_DIR ?= /usr/local/browndeer/lib
INSTALL_INCLUDE_DIR ?= /usr/local/browndeer/include
INSTALL_MAN_DIR ?= /usr/share/man

#ATISTREAMSDK = /usr/local/atistream/atistream_sdk-2.0/
ATISTREAMSDK = /usr/local/atistream/


XCL_OBJS = xcl_platform.o xcl_device.o xcl_context.o xcl_command_queue.o \
	xcl_enqueue.o xcl_event.o xcl_program.o xcl_kernel.o xcl_memobj.o \
	xcl_flush.o

IMP_OBJS = platform.o device.o context.o command_queue.o event.o \
	environment.o \
	program.o kernel.o memobj.o \
	cmdsched.o cmdcall.o cmdcall_x86_64.o \
	elf_cl.o compiler_x86_64.o \
	vcore.o fast_setjmp.o \
	watermark.o


RT_OBJS = __elfcl_rt.o fast_setjmp.o

RT_BC = __vcore_rt.bc bdt_builtins_x86_64.bc


DEFS += -DATISTREAMSDK=\"$(ATISTREAMSDK)\" 
DEFS += -DINSTALL_LIB_DIR=\"$(INSTALL_LIB_DIR)\" 

ifeq ($(ENABLE_ATIGPU),1)
DEFS += -DENABLE_ATIGPU 
INCS += -I$(ATISTREAMSDK)/include
LIBS += -laticalrt -laticalcl
IMP_OBJS += compiler_atigpu.o cmdcall_atigpu.o ircompiler_atigpu.o
endif

ifeq ($(ENABLE_NCPU),1)
DEFS += -DENABLE_NCPU
endif

ifeq ($(USE_BDT_BUILTINS),1)
DEFS += -DUSE_BDT_BUILTINS
endif


######################################################################

TOPDIR ?= ../../

INCS += -I. -I$(TOPDIR)/include -I/usr/local/include/libelf

LIBS += -lfl -lpthread -L/usr/local/lib -lelf -ldl

OBJS = $(XCL_OBJS) $(IMP_OBJS)

DEFS += \
	-DINSTALL_BIN_DIR=\"$(INSTALL_BIN_DIR)\" \
	-DINSTALL_LIB_DIR=\"$(INSTALL_LIB_DIR)\" \
	-DINSTALL_INCLUDE_DIR=\"$(INSTALL_INCLUDE_DIR)\" 

#CFLAGS += -O1 -fPIC -fbounds-check -g
CFLAGS += -O1 -fPIC -funroll-loops -fstrict-aliasing


#YACC = yacc
YACC = bison
YACCFLAGS =  -y -v 

LEX = flex
LEXFLAGS = 

XCLNM_YYPREFIX = __xclnm_

all: $(LIB_NAME).so $(RT_OBJS) $(RT_BC) ati_builtins_x86_64_patch.bc

.PHONY: check_root clean install uninsstall

.SUFFIXES:
.SUFFIXES: .l .y .c .S .s .o

$(LIB_NAME).so: $(OBJS) $(RT_OBJS)
	$(CC) $(CFLAGS) $(INCS) \
		-shared -Wl,-soname,$(LIB_NAME).so -o $(LIB_NAME).so  $(OBJS) $(LIBS)

__vcore_rt.bc: __vcore_rt.c
#	llvm-gcc -c -emit-llvm $(DEFS) $(INCS) -o __vcore_rt.bc __vcore_rt.c
	clang -c -emit-llvm $(DEFS) $(INCS) -o __vcore_rt.bc __vcore_rt.c

bdt_builtins_x86_64.bc: bdt_builtins_x86_64.c
	clang -c -emit-llvm $(DEFS) $(INCS) -o bdt_builtins_x86_64.bc \
		bdt_builtins_x86_64.c

ati_builtins_x86_64_patch.bc: ati_builtins_x86_64_patch.ll
	llvm-as -f ati_builtins_x86_64_patch.ll

.c.o:
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -c $<

.s.o:
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -c $<

.S.o:
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -c $<

check_root:
ifneq ($(shell whoami),root)
	$(error install must be done as root)
endif

install: check_root
	test -d $(INSTALL_INCLUDE_DIR) || install -m 755 -d $(INSTALL_INCLUDE_DIR)
	test -d $(INSTALL_LIB_DIR) || install -m 755 -d $(INSTALL_LIB_DIR)
	test -d $(INSTALL_BIN_DIR) || install -m 755 -d $(INSTALL_BIN_DIR)
	install -m 755 $(LIB_NAME).so $(INSTALL_LIB_DIR)
	install -m 755 $(RT_OBJS) $(INSTALL_LIB_DIR)
	install -m 755 $(RT_BC) $(INSTALL_LIB_DIR)
	install -m 755 ati_builtins_x86_64_patch.bc $(INSTALL_LIB_DIR)
	install -m 755 vcore.h $(INSTALL_INCLUDE_DIR)

uninstall: check_root
	rm -f $(INSTALL_LIB_DIR)/$(LIB_NAME).so
	rm -f $(addprefix $(INSTALL_LIB_DIR)/,$(RT_OBJS))
	rm -f $(addprefix $(INSTALL_LIB_DIR)/,$(RT_BC))
	rm -f $(addprefix $(INSTALL_INCLUDE_DIR)/,vcore.h)

clean:
	rm -f *.o *.so
	rm -f *.bc


