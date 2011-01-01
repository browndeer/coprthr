
include Makefile.choices

#BUILD_LIBS = libstdcl libocl
BUILD_TOOLS = cltrace xclnm 

#export INSTALL_BIN_DIR = /usr/local/browndeer/bin
#export INSTALL_LIB_DIR = /usr/local/browndeer/lib
#export INSTALL_INCLUDE_DIR = /usr/local/browndeer/include
#export INSTALL_MAN_DIR = /usr/share/man


SUBDIRS = ./src/libstdcl ./src/libocl ./src/CLETE
#SUBDIRS += $(addprefix ./libs/,$(BUILD_LIBS))
SUBDIRS += $(addprefix ./tools/,$(BUILD_TOOLS)) 

export TOPDIR = $(CURDIR)

all: $(SUBDIRS) 

.PHONY: subdirs $(SUBDIRS) clean install uninstall test quicktest

subdirs: $(SUBDIRS)

$(SUBDIRS):
	make -C $@ $(MAKEFLAGS) $(MAKECMDGOALS)

clean: subdirs
#	make -C ./test $(MAKEFLAGS) $(MAKECMDGOALS)
	make -C ./test $(MAKECMDGOALS)

install: subdirs
	@echo -e "\a\n\n*** IMPORTANT ***\n"
	@echo -e "Please ensure that " $(INSTALL_BIN_DIR) " is added to PATH"
	@echo -e "if it is not in the standard search path for executables."
	@echo -e "For example:"
	@echo -e "   set path = ($(INSTALL_BIN_DIR) \$$path)\n"
	@echo -e "Please ensure that $(INSTALL_LIB_DIR) is added to " \
		"LD_LIBRARY_PATH"
	@echo -e "if it is not in the standard search path for libraries."
	@echo -e "For example:"
	@echo -e "   setenv LD_LIBRARY_PATH $(INSTALL_LIB_DIR):\$$LD_LIBRARY_PATH\n"
	@echo -e "In order to test the installation, type:"
	@echo -e "   make test\n"
	@echo -e "Testing can take several minutes, for a quick test type:"
	@echo -e "   make quicktest\n"
	@echo -e "When compiling programs that use libocl include the following:"
	@echo -e "   INCS += -I$(INSTALL_INCLUDE_DIR)"
	@echo -e "   LIBS += -L$(INSTALL_LIB_DIR) -locl\n"
	@echo -e "When compiling programs that use libstcl include the following:"
	@echo -e "   INCS += -I$(INSTALL_INCLUDE_DIR)"
	@echo -e "   LIBS += -L$(INSTALL_LIB_DIR) -lstdcl\n"

uninstall: subdirs

test: 
	make -C ./test $(MAKEFLAGS) $(MAKECMDGOALS)

quicktest: 
	make -C ./test $(MAKEFLAGS) $(MAKECMDGOALS)

