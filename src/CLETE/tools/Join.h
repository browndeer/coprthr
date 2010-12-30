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

#ifndef _JOIN_H
#define _JOIN_H

#include <string>

using std::string;

//-----------------------------------------------------------------------------
// joinWithComma(string,string,...)
//
// Joins a bunch of strings together with commas between them.  If any of the
// strings are empty then the redundant commas are eliminated.  This routine
// is used to produce the list of template arguments given lists for each
// function argument (some of which may be empty).
//-----------------------------------------------------------------------------

extern string joinWithComma(const string& a,const string& b);

extern string joinWithComma(const string& a,const string& b,
			  const string& c);
  
#endif // _JOIN_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: Join.h,v $   $Author: sa_smith $
// $Revision: 1.2 $   $Date: 2000/08/03 00:18:02 $
// ----------------------------------------------------------------------
// ACL:rcsinfo


