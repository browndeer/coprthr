/* OperatorDescriptor.h
 *
 * Copyright (c) 2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This is a derivative work based on PETE-2.1.0, produced in compliance
 * with the original ACL license reproduced below, and which should not be
 * confused with the original software.
 */

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
// ----------------------------------------------------------------------
// $RCSfile: OperatorDescriptor.h,v $   $Author: sa_smith $
// $Revision: 1.7 $   $Date: 2000/08/03 00:18:02 $
// ----------------------------------------------------------------------

/* DAR */

#ifndef _OPERATORDESCRIPTOR_H
#define _OPERATORDESCRIPTOR_H

#include <iostream>

using std::ostream;
using std::endl;

#include <string>

using std::string;

#include "DescriptorBase.h"

class OperatorDescriptor: public DescriptorBase<5> {
public:

  //---------------------------------------------------------------------------
  // Constructors.

  OperatorDescriptor()
  { }

  OperatorDescriptor(const string &tag, const string &func,
		     const string &expr, const string &arg = "", 
				const string& strexpr = "" )
  {
    addData(0, tag);
    addData(1, func);
    addData(2, expr);
    addData(3, arg);
    addData(4, strexpr);
  }

  OperatorDescriptor(const OperatorDescriptor &model)
  : DescriptorBase<5>(model)
  { }

  //---------------------------------------------------------------------------
  // Trivial destructor. 
  
  ~OperatorDescriptor() { }

  //---------------------------------------------------------------------------
  // Copy-assignment operator: just copy members. 

  OperatorDescriptor &operator=(const OperatorDescriptor &rhs)
  {
    DescriptorBase<5>::operator=(rhs);
    
    return *this;
  }
  
  //---------------------------------------------------------------------------
  // Return strings/info. 

  const string tag(bool full = true) const
  {
    if (full)
      return str(0);
    else
      return str(0).substr(0, str(0).find('<'));
  }

  const string &function() const
  {
    return str(1);
  }

  const string &expression() const
  {
    return str(2);
  }

  const string &argDef() const
  {
    return str(3);
  }

  const string& strexpr() const
  {
    return str(4);
  }

  bool templateArgs() const
  {
    return argDef().size() != 0;
  }

};

inline ostream &operator<<(ostream &os, const OperatorDescriptor &o)
{
  os << "TAG  = " << o.tag() << endl;
  os << "FUNC = " << o.function() << endl;
  os << "EXPR = " << o.expression() << endl;
  os << "ARG  = " << o.argDef() << endl;
  os << "STREXPR  = " << o.strexpr() << endl;
  
  return os;
}
  
#endif // _OPERATORDESCRIPTOR_H

