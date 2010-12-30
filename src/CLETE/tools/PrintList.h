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
// Class:
// **CLASSNAME**
//-----------------------------------------------------------------------------

#ifndef _PRINTLIST_H
#define _PRINTLIST_H

//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Overview: 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Typedefs:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Forward Declarations:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Full Description:
//
//-----------------------------------------------------------------------------

template<class OSTR,class PROTOTYPE,class LIST1>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1)
{
  typename LIST1::const_iterator i1;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    prototype.print(ostr,*i1);
  }
}

template<class OSTR,class PROTOTYPE,class LIST1,class LIST2>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1,
	  const LIST2& list2)
{
  typename LIST1::const_iterator i1;
  typename LIST2::const_iterator i2;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    for (i2 = list2.begin();i2 != list2.end(); ++i2)
    {
      prototype.print(ostr,*i1,*i2);
    }
  }
}

template<class OSTR,class PROTOTYPE,class LIST1,class LIST2,class LIST3>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1,
	  const LIST2& list2,const LIST3& list3)
{
  typename LIST1::const_iterator i1;
  typename LIST2::const_iterator i2;
  typename LIST3::const_iterator i3;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    for (i2 = list2.begin();i2 != list2.end(); ++i2)
    {
      for (i3 = list3.begin();i3 != list3.end(); ++i3)
      {
	prototype.print(ostr,*i1,*i2,*i3);
      }
    }
  }
}

template<class OSTR,class PROTOTYPE,
  class LIST1,class LIST2,class LIST3,class LIST4>
void
printList(OSTR& ostr,const PROTOTYPE& prototype,const LIST1& list1,
	  const LIST2& list2,const LIST3& list3,const LIST4& list4)
{
  typename LIST1::const_iterator i1;
  typename LIST2::const_iterator i2;
  typename LIST3::const_iterator i3;
  typename LIST4::const_iterator i4;
  for (i1 = list1.begin();i1 != list1.end(); ++i1)
  {
    for (i2 = list2.begin();i2 != list2.end(); ++i2)
    {
      for (i3 = list3.begin();i3 != list3.end(); ++i3)
      {
	for (i4 = list4.begin();i4 != list4.end(); ++i4)
	{
	  prototype.print(ostr,*i1,*i2,*i3,*i4);
	}
      }
    }
  }
}

#endif     // _PRINTLIST_H

//////////////////////////////////////////////////////////////////////

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PrintList.h,v $   $Author: swhaney $
// $Revision: 1.5 $   $Date: 2000/01/04 02:24:05 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
