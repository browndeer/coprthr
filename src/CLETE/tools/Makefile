# ACL:license
#  ----------------------------------------------------------------------
#  This software and ancillary information (herein called "SOFTWARE")
#  called PETE (Portable Expression Template Engine) is
#  made available under the terms described here.  The SOFTWARE has been
#  approved for release with associated LA-CC Number LA-CC-99-5.
#  
#  Unless otherwise indicated, this SOFTWARE has been authored by an
#  employee or employees of the University of California, operator of the
#  Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
#  the U.S. Department of Energy.  The U.S. Government has rights to use,
#  reproduce, and distribute this SOFTWARE. The public may copy, distribute,
#  prepare derivative works and publicly display this SOFTWARE without 
#  charge, provided that this Notice and any statement of authorship are 
#  reproduced on all copies.  Neither the Government nor the University 
#  makes any warranty, express or implied, or assumes any liability or 
#  responsibility for the use of this SOFTWARE.
#  
#  If SOFTWARE is modified to produce derivative works, such modified
#  SOFTWARE should be clearly marked, so as not to confuse it with the
#  version available from LANL.
#  
#  For more information about PETE, send e-mail to pete@acl.lanl.gov,
#  or visit the PETE web page at http://www.acl.lanl.gov/pete/.
#  ----------------------------------------------------------------------
# ACL:license

ifndef PETEARCH
PETEARCH = LINUXEGCS
endif

PROJECTROOT = $(shell cd ../../..; pwd)
include $(PROJECTROOT)/config/$(PETEARCH)

default: gen_operators

gen_operators: gen_operators.cpp ClassDescriptor.h Header.h \
	OperatorDescriptor.h Parser.h PrintFunctions.h PrintOperators.h \
	PrintList.h Options.cpp Options.h PeteOps.cpp PeteOps.h \
	PAssert.cpp PAssert.h DescriptorBase.h Join.h Join.cpp
	$(CXX) -o gen_operators $(CXXINC) $(CXXARG) gen_operators.cpp \
	Options.cpp PAssert.cpp PeteOps.cpp Join.cpp

newpeteops: gen_operators
	gen_operators --operators PeteOps.in --guard "" --insert-op \
	--lanl-boilerplate --o PeteOps.cpp

fixpeteops:
	/home/software/bin/aclsrcfix --label=license --insert=../../LICENSE \
	PeteOps.cpp

peteheaders: OperatorTags.h

OperatorTags.h: gen_operators
	gen_operators --guard PETE_PETE_OPERATORTAGS_H --op-tags \
	--operators PeteOps.in --lanl-boilerplate --o OperatorTags.h

fixheaders:
	/home/software/bin/aclsrcfix --label=license --insert=../../LICENSE \
	OperatorTags.h

installheaders: OperatorTags.h
	cp OperatorTags.h ../PETE

clean:
	rm -f gen_operators
	rm -f OperatorTags.h
	rm -f *.o
	rm -rf ti_files
	rm -rf ii_files

# ACL:rcsinfo
#  ----------------------------------------------------------------------
#  $RCSfile: makefile,v $   $Author: swhaney $
#  $Revision: 1.18 $   $Date: 2000/01/18 18:35:56 $
#  ----------------------------------------------------------------------
# ACL:rcsinfo
