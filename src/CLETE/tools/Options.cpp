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
// Functions:
// flagOption
// stringOption
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------

#include <string>
#include <stdlib.h>
using std::string;

#include "Options.h"

bool flagOption(int argc,char ** argv,const string &option)
{
  // Scan through the arguments.

  int i;
  for (i=1;i<argc;++i)
  {
    if (string(argv[i]) == string(option))
      return true;
  }

  return false;
}

string stringOption(int argc,char ** argv,
		    const string &option,const string &def)
{
  // Scan through the arguments.

  int i;
  for (i=1;i<argc;++i)
  {
    if (string(argv[i]) == string(option))
    {
      if (i+1<argc)
	return string(argv[i+1]);
    }
  }

  return def;
}

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Options.cpp,v $   $Author: sa_smith $
// $Revision: 1.6 $   $Date: 2000/08/03 00:18:02 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
