// -*- C++ -*-
// ACL:license
// ----------------------------------------------------------------------
// This software and ancillary information (herein called "SOFTWARE")
// called PETE (Portable Expression Template Engine) is
// made available under the terms described here.  The SOFTWARE has been
// approved for release with associated LA-CC Number LA-CC-99-5.
// 
// Unless otherwise indicated, this SOFTWARE has been authored by an
// employee or employees of the University of California, operator of the
// Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
// the U.S. Department of Energy.  The U.S. Government has rights to use,
// reproduce, and distribute this SOFTWARE. The public may copy, distribute,
// prepare derivative works and publicly display this SOFTWARE without 
// charge, provided that this Notice and any statement of authorship are 
// reproduced on all copies.  Neither the Government nor the University 
// makes any warranty, express or implied, or assumes any liability or 
// responsibility for the use of this SOFTWARE.
// 
// If SOFTWARE is modified to produce derivative works, such modified
// SOFTWARE should be clearly marked, so as not to confuse it with the
// version available from LANL.
// 
// For more information about PETE, send e-mail to pete@acl.lanl.gov,
// or visit the PETE web page at http://www.acl.lanl.gov/pete/.
// ----------------------------------------------------------------------
// ACL:license
//-----------------------------------------------------------------------------
// PAssert non-template definitions.
//-----------------------------------------------------------------------------

#include "PAssert.h"

//-----------------------------------------------------------------------------
// Use stdio.h instead of cstdio. Justification:
//
//   o Intel doesn't put these in the std:: namespace, 
//     and writing varargs wrapper functions doesn't
//     sound like fun. 
//   o Ultimately we should switch to ostringstreams,
//     but currently EGCS doesn't have these. 
//   o Finally, this is a .cmpl.cpp files, so using stdio.h
//     doesn't pollute the global namespace of users of PAssert.
//
// Note that stdio.h (and other C header files) is required, but
// deprecated, by the '98 version of the standard. 
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if !PETE_EXCEPTIONS
# include <stdlib.h>
#endif


namespace Pete {

//-----------------------------------------------------------------------------
// Definitions of methods in Assertion class.  The constructor stores
// the assertion message in a char buffer.
//-----------------------------------------------------------------------------

Assertion::Assertion(const char *msg, const char *file, int line)
{
  msg_m = new char[strlen(msg) + 1];
  strcpy(msg_m, msg);
  file_m = new char[strlen(file) + 1];
  strcpy(file_m, file);
  line_m = line;
}

Assertion::Assertion(const Assertion &a)
{
  msg_m = new char[strlen(a.what())+1];
  strcpy(msg_m, a.what());
  file_m = new char[strlen(a.file())+1];
  strcpy(file_m, a.file());
  line_m = a.line();
}

Assertion &Assertion::operator=(const Assertion &a)
{
  msg_m = new char[strlen(a.what())+1];
  strcpy(msg_m, a.what());
  file_m = new char[strlen(a.file())+1];
  strcpy(file_m, a.file());
  line_m = a.line();

  return *this;
}

//-----------------------------------------------------------------------------
// Function to perform the task of actually throwing an assertion, from a
// PAssert situation.
//-----------------------------------------------------------------------------

void toss_cookies(const char *msg, const char *file, int line ...)
{
  va_list ap;
  va_start(ap, line);
  char buf[256];
  vsprintf(buf, msg, ap);
  va_end(ap);

  Assertion a(buf, file, line);

#if PETE_EXCEPTIONS
  throw a;
#else
  fprintf(stderr, "### PETE Assertion Failure ###\n");
  fprintf(stderr, "### %s\n", a.what());
  fprintf(stderr, "### File %s; Line %d.\n", a.file(), a.line());
  abort();
#endif
}

} // namespace Pete

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PAssert.cpp,v $   $Author: swhaney $
// $Revision: 1.1 $   $Date: 2000/01/04 02:24:05 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
