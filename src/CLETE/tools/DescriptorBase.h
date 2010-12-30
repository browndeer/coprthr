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

#ifndef _DESCRIPTORBASE_H
#define _DESCRIPTORBASE_H

#include <algorithm>
#include <string>

using std::string;
using std::copy;

#include "PAssert.h"

template<int N>
class DescriptorBase {
public:

  //---------------------------------------------------------------------------
  // Constructors: default and copy constructor.

  DescriptorBase()
  { }
  
  DescriptorBase(const DescriptorBase<N> &model)
  {
    copy(model.strs_m, model.strs_m + N, strs_m);
  }

  //---------------------------------------------------------------------------
  // Trivial destructor. 
  
  ~DescriptorBase() { }

  //---------------------------------------------------------------------------
  // Adds some data to this descriptor.

  void addData(int snum, const string &data)
  {
    PAssert(snum >= 0 && snum < N);

    strs_m[snum] = data;
  }

  //---------------------------------------------------------------------------
  // Copy-assignment operator: just copy members. 

  DescriptorBase &operator=(const DescriptorBase &rhs)
  {
    copy(rhs.strs_m, rhs.strs_m + N, strs_m);
    
    return *this;
  }
  
  //---------------------------------------------------------------------------
  // Return strings. 

  const string &str(int n) const
  {
    PAssert(n >= 0 && n < N);

    return strs_m[n];
  }
  
private:

  string strs_m[N];

};
  
#endif // _DESCRIPTORBASE_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: DescriptorBase.h,v $   $Author: sa_smith $
// $Revision: 1.3 $   $Date: 2000/08/03 00:18:02 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
