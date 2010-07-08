
TESTS = test_stdcl test_ocl

##### DO NOT EDIT BELOW THIS LINE #####

SUBDIRS = 
SUBDIRS += $(TESTS)

export TOPDIR = $(CURDIR)

all: $(SUBDIRS) 

.PHONY: subdirs $(SUBDIRS) clean test quicktest

subdirs: $(SUBDIRS)

$(SUBDIRS):
#	make -C $@ $(MAKEFLAGS) $(MAKECMDGOALS)
	make -C $@ $(MAKECMDGOALS)

clean: subdirs
#	make -C ./test $(MAKEFLAGS) $(MAKECMDGOALS)

test: subdirs
#	make -C ./test $(MAKEFLAGS) $(MAKECMDGOALS)

quicktest: subdirs
#	make -C ./test $(MAKEFLAGS) $(MAKECMDGOALS)

