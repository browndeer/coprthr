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

#ifndef _PASSERT_H
#define _PASSERT_H

//-----------------------------------------------------------------------------
// Classes:
// PAssert macro, PInsist macro, CTAssert struct
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CTAssert(bool c) is a compile-time assertion macro.
// PAssert(bool c) is a run-time assertion macro.
// PInsist(bool c, char *m) is a run-time insistence, with an explanatory
// message.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Forward Declarations:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// assertion: exception notification class for assertions.
//
// This class should really be derived from std::runtime_error, but
// unfortunately we don't have good implementation of the library standard
// yet, on compilers other than KCC.  So, this class will keep with the
// "what" method evidenced in the standard, but dispense with inheriting from
// classes for which we don't have implementations...
// 
//-----------------------------------------------------------------------------

namespace Pete {

class Assertion /* : std::runtime_error */
{
  char *msg_m;
  char *file_m;
  int line_m;
public:
  Assertion(const char *msg, const char *file, int line);
  Assertion(const Assertion &a);
  ~Assertion() { delete[] msg_m; delete [] file_m; }
  Assertion &operator=(const Assertion &a);
  const char *what() const { return msg_m; }
  const char *file() const { return file_m; }
  int line() const { return line_m; }
  
  template<class OS>
  void print(OS &os) const
  {
    os << "### PETE Assertion Failure ###\n";
    os << "### " << what() << "\n";
    os << "### File " << file() << "; Line " << line();
  }
};

//
// This is the function called in the assert/insist macros.
//

void toss_cookies(const char *msg, const char *file, int line ...);

} // namespace Pete

//-----------------------------------------------------------------------------
//
// CTAssert: CTAssert is a compile-time assert.
//
// That is, it tests the condition at compile time and if it is false
// you get a compile error that it can't find CTAssert<false>::test().
// 
// If NOCTAssert is defined, CTAssert will revert to the equivalent of PAssert.
// To turn off the test completely, define NOPAssert as well.
//
//-----------------------------------------------------------------------------

#if defined(NOCTAssert)

#if defined(NOPAssert)

#define CTAssert(c)

#else

#define CTAssert(c) if (c) {} else Pete::toss_cookies(#c, __FILE__, __LINE__)

#endif

#else

#define CTAssert(c) PeteCTAssert<(c)>::test()

template<bool B> struct PeteCTAssert {};

template<> struct PeteCTAssert<true> { static void test() {} };

#endif


//-----------------------------------------------------------------------------
//
// PAssert: Run-time assertion
//
// Now we define a run time assertion mechanism.  We will call it "PAssert",
// to reflect the idea that this is for use in Pete per se, recognizing that
// there are numerous other assertion facilities in use in client codes.
//
// The PAssert macro is intended to be used for validating preconditions
// which must be true in order for following code to be correct, etc.  For
// example, PAssert( x > 0. ); y = sqrt(x);  If the assertion fails, the code
// should just bomb.  Philosophically, it should be used to feret out bugs in
// preceding code, making sure that prior results are within reasonable
// bounds before proceeding to use those results in further computation, etc.
//
//-----------------------------------------------------------------------------

#if defined NOPAssert
#define PAssert(c) 
#else
#define PAssert(c) if (c) {} else Pete::toss_cookies(#c, __FILE__, __LINE__)
#endif


//-----------------------------------------------------------------------------
//
// PInsist:
//
// The PInsist macros are like the PAssert macro, but they provide the
// opportunity to specify an instructive message.  The idea here is that you
// should use Insist for checking things which are more or less under user
// control.  If the user makes a poor choice, we "insist" that it be
// corrected, providing a corrective hint.
//
//-----------------------------------------------------------------------------

#define PInsist(c,m) if (c) {} else Pete::toss_cookies(m, __FILE__, __LINE__)
#define PInsist1(c,m,a1)                                                      \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1)
#define PInsist2(c,m,a1,a2)                                                   \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1,a2)
#define PInsist3(c,m,a1,a2,a3)                                                \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1,a2,a3)
#define PInsist4(c,m,a1,a2,a3,a4)                                             \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1,a2,a3,a4)
#define PInsist5(c,m,a1,a2,a3,a4,a5)                                          \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1,a2,a3,a4,a5)
#define PInsist6(c,m,a1,a2,a3,a4,a5,a6)                                       \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1,a2,a3,a4,a5,a6)
#define PInsist7(c,m,a1,a2,a3,a4,a5,a6,a7)                                    \
  if (c) {} else Pete::toss_cookies(m,__FILE__,__LINE__,a1,a2,a3,a4,a5,a6,a7)


//-----------------------------------------------------------------------------
// NOTE:  We provide a way to eliminate assertions, but not insistings.  The
// idea is that PAssert is used to perform sanity checks during program
// development, which you might want to eliminate during production runs for
// performance sake.  PInsist is used for things which really really must be
// true, such as "the file must've been opened", etc.  So, use PAssert for
// things which you want taken out of production codes (like, the check might
// inhibit inlining or something like that), but use PInsist for those things
// you want checked even in a production code.
//-----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////

#endif // _PASSERT_H

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: PAssert.h,v $   $Author: swhaney $
// $Revision: 1.1 $   $Date: 2000/01/04 02:24:05 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
