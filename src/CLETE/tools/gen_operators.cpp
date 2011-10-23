/* gen_operators.cpp
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
// RCSfile: MakeOperators.cpp,v Author: sa_smith 
// Revision: 1.18 Date: 2000/08/03 00:18:02 
// ----------------------------------------------------------------------

/* DAR */

#include "ClassDescriptor.h"
#include "Header.h"
#include "OperatorDescriptor.h"
#include "Parser.h"
#include "PrintOperators.h"
#include "PrintFunctions.h"
#include "PrintList.h"
#include "Options.h"
#include "PeteOps.h"

#include <fstream>

using std::ifstream;
using std::ofstream;

#include <iostream>

using std::cout;
using std::endl;
using std::ostream;

#include <string>

using std::string;

#include <vector>

using std::vector;
using std::copy;
using std::back_inserter;

#include <map>

using std::map;

#if defined(macintosh)
#include <console.h>
#endif

int main(int argc, char *argv[])
{
#if defined(macintosh)
  argc = ccommand(&argv);
#endif

  if (flagOption(argc,argv,"--help") || flagOption(argc,argv,"--pete-help"))
  {
    cout << "gen_operators produces global functions for C++\n";
    cout << "operators (+ - * etc.) that create expression trees.\n";
    cout << "Global assignment operators may be produced as well.\n";
    cout << "This function can also produce operator tag structs.\n\n";

    cout << "Options:\n";
    cout << "--help:           Print this message.\n";
    cout << "--pete-help:      Print this message.\n";
    cout << "--classes file:   Read the class descriptors from file.\n";
    cout << "                  If no class file is provided, then\n";
    cout << "                  no operators or assignment operators\n";
    cout << "                  are produced.\n";
    cout << "--o file:         Name of file to write operator info to.\n";
    cout << "                  If not specified, output goes to the\n";
    cout << "                  terminal.\n";
    cout << "--operators file: Read the operator descriptors from\n";
    cout << "                  file.\n";
    cout << "                  If no operator file is provided, then\n";
    cout << "                  the standard set of PETE operators is\n";
    cout << "                  used (most of the C++ operators).\n";
    cout << "--pete-ops:       Add the set of PETE operators to those\n";
    cout << "                  input from the operator file.\n";
    cout << "--guard string:   Use string for the include guard\n";
    cout << "                  (defaults to GENERATED_OPERATORS_H).\n";
    cout << "--scalars:        If this flag is present, only generate\n";
    cout << "                  operators involving user-defined scalars.";
    cout << "--extra-classes:  If this flag is present, only generate\n";
    cout << "                  operators involving the extraClasses.\n";
    cout << "--no-expression:  If this flag is present, don't generate\n";
    cout << "                  operators involving Expression<T>\n";
    cout << "--assign-ops:     If this flag is present, generate the\n";
    cout << "                  assignment operators that call\n";
    cout << "                  evaluate().\n";
    cout << "--op-tags:        If this flag is present, generate the\n";
    cout << "                  operator tag structs\n";
    cout << "--no-shift-guard: If this flag is present, put no guards\n";
    cout << "                  around the scalar << class and \n";
    cout << "                  scalar >> class operators (they can\n";
    cout << "                  get confused with stream operations).\n\n";

    cout << "These two options are used internally by PETE:\n";
    cout << "--insert-op:      Used to build the file\n";
    cout << "                  src/tools/PeteOps.cpp.\n";
//    cout << "--lanl-boilerplate:  Includes the standard ACL header and\n";
//    cout << "                  footer." << endl;
    return 0;
  }

  vector<string> filesUsed;
  filesUsed.push_back("gen_operators");

  bool printOperators = flagOption(argc,argv,"--classes");
  string cls = stringOption(argc,argv,"--classes","");
  string ofile = stringOption(argc, argv, "--o", "");
  string guardDef(printOperators ?
		  "GENERATED_OPERATORS_H" : "OPERATOR_TAGS_H");
  string includeGuard = stringOption(argc,argv,"--guard",guardDef);
  bool justScalars = flagOption(argc,argv,"--scalars");
  bool justExtraClasses = flagOption(argc,argv,"--extra-classes");
  bool expression = !flagOption(argc,argv,"--no-expression");
  bool assignment = flagOption(argc,argv,"--assign-ops");
  bool printTags = flagOption(argc,argv,"--op-tags");
  bool shiftGuard = !flagOption(argc,argv,"--no-shift-guard");
  bool insertOp = flagOption(argc,argv,"--insert-op");
  bool addPeteOps = (!flagOption(argc,argv,"--operators")) ||
    flagOption(argc,argv,"--pete-ops");
  bool lanlBoilerplate = flagOption(argc,argv,"--lanl-boilerplate");

  string prefix, suffix;

  // Input the operator descriptors.
  
  map<string, vector<OperatorDescriptor> > mOps;
  map<string, vector<OperatorDescriptor> > inputOps;  

  if (flagOption(argc,argv,"--operators"))
  {
    string ops = stringOption(argc,argv,"--operators","");
    ifstream fOps(ops.c_str());
    filesUsed.push_back(ops);
    PInsist1(!!fOps, "ERROR: couldn't open operator description file \""
	    "%s\"", ops.c_str());

    Parser<OperatorDescriptor> pOps(fOps, ops, mOps);
    pOps.addKeyword("TAG");
    pOps.addKeyword("FUNCTION");
    pOps.addKeyword("EXPR");
    pOps.addKeyword("ARG");
    pOps.addKeyword("STREXPR");

    pOps.parse();
    prefix = pOps.prefixText();
    suffix = pOps.suffixText();
  }
  inputOps = mOps;
 
//// XXX
//vector<OperatorDescriptor> tmp(mOps["unaryOps"]);
//for(int i=0; i< tmp.size(); i++) std::cout<<tmp[i];
 
  if (addPeteOps)
  {
    peteOps(mOps);
  }
  
  // Create vectors for unary operators.
  
  vector<OperatorDescriptor> unaryOps(mOps["unaryOps"]);
  copy(mOps["unaryBoolOps"].begin(), mOps["unaryBoolOps"].end(),
       back_inserter(unaryOps));
  copy(mOps["unarySpecialOps"].begin(), mOps["unarySpecialOps"].end(),
       back_inserter(unaryOps));
  
  vector<OperatorDescriptor> &unaryCastOps = mOps["unaryCastOps"];

  // Create vectors for binary operators.
  
  vector<OperatorDescriptor> binaryOps(mOps["binaryOps"]);
  copy(mOps["binaryBoolOps"].begin(), mOps["binaryBoolOps"].end(),
       back_inserter(binaryOps));
  //  copy(mOps["binaryLeftOps"].begin(), mOps["binaryLeftOps"].end(),
  //       back_inserter(binaryOps));
  copy(mOps["binarySpecialOps"].begin(), mOps["binarySpecialOps"].end(),
       back_inserter(binaryOps));
    
  vector<OperatorDescriptor> binaryLeftOps(mOps["binaryLeftOps"]);

  // Create reference for trinary operators.
  
  vector<OperatorDescriptor> &trinaryOps = mOps["trinaryOps"];
    
  // Create vector for assignment operators.
  
  vector<OperatorDescriptor> assignOps(mOps["assignOp"]);
  copy(mOps["binaryAssignOps"].begin(),
       mOps["binaryAssignOps"].end(),
       back_inserter(assignOps));
  copy(mOps["binaryAssignBoolOps"].begin(),
       mOps["binaryAssignBoolOps"].end(),
       back_inserter(assignOps));

  // Input the class descriptors.

  map<string, vector<ClassDescriptor> > mClasses;  

  vector<ClassDescriptor> classes;
  vector<ClassDescriptor> extraClasses;
  vector<ClassDescriptor> scalars;
  vector<ClassDescriptor> generalT;
  vector<ClassDescriptor> userClasses;
  vector<ClassDescriptor> expressionClass;

  if (printOperators)
  {
    ifstream fClasses(cls.c_str());
    filesUsed.push_back(cls);
    if (justScalars)
    {
      filesUsed.push_back("(Only operations with scalars were printed.)");
    }

    PInsist1(!!fClasses, "ERROR: couldn't open class description file \""
	    "%s\"", cls.c_str());
    
    Parser<ClassDescriptor> pClasses(fClasses, cls, mClasses);

    pClasses.addKeyword("ARG");
    pClasses.addKeyword("CLASS");

    pClasses.parse();
    if (prefix.size() != 0)
      prefix += "\n\n";
    if (suffix.size() != 0)
      suffix += "\n\n";
    prefix += pClasses.prefixText();
    suffix += pClasses.suffixText();
  
    classes = mClasses["classes"];
    extraClasses = mClasses["extraClasses"];
    scalars = mClasses["scalars"];

    userClasses = classes;

    if (expression)
    {
      expressionClass.push_back(ClassDescriptor("class T[n]",
						"Expression<T[n]>"));
      classes.push_back(ClassDescriptor("class T[n]",
					"Expression<T[n]>"));
    }

    if (!justScalars)
      {
	scalars.push_back(ClassDescriptor("class T[n]","T[n]"));
      }
  }

  generalT.push_back(ClassDescriptor("class T[n]","T[n]"));

  // Set up streams for printing.
  
  ostream *ofl = &cout;
  if (ofile != string(""))
    {
      ofl = new ofstream(ofile.c_str());

      PInsist1(ofl != NULL, "ERROR: couldn't open class description file \""
	       "%s\"", ofile.c_str());
      PInsist1(!!*ofl, "ERROR: couldn't open class description file \""
	       "%s\"", ofile.c_str());
    }
        
  // Print header.
  
  printHeader(*ofl,includeGuard,filesUsed,lanlBoilerplate,prefix);

  // The following code is used when we generate PeteOps.cpp from
  // PeteOps.in.  Users should never use the --insert-ops option.

  if (insertOp)
  {
    *ofl << "#include \"OperatorDescriptor.h\"" << endl
	 << "#include <vector>" << endl
	 << "#include <map>" << endl
	 << "#include <string>" << endl
	 << "using std::map;" << endl
	 << "using std::vector;" << endl
	 << "using std::string;" << endl
	 << endl;
    *ofl << endl
	 << "void peteOps(map<string,"
	 << "vector<OperatorDescriptor> > &m)" << endl
	 << "{" << endl;

    map<string,vector<OperatorDescriptor> >::iterator opvec;

    for(opvec = mOps.begin();opvec != mOps.end();++opvec)
    {
      printList(*ofl,InsertOp(opvec->first),opvec->second);
    }

    *ofl << "}" << endl;
  }

  // Print operator tags.
  
  if (printTags)
  {
    printList(*ofl,UnaryOp(),           inputOps["unaryOps"]);
    printList(*ofl,UnaryBoolOp(),       inputOps["unaryBoolOps"]);
    printList(*ofl,UnaryCastOp(),       inputOps["unaryCastOps"]);
    printList(*ofl,UnarySpecialOp(),    inputOps["unarySpecialOps"]);
    printList(*ofl,BinaryOp(),          inputOps["binaryOps"]);
    printList(*ofl,BinaryBoolOp(),      inputOps["binaryBoolOps"]);
    printList(*ofl,BinaryLeftOp(),      inputOps["binaryLeftOps"]);
    printList(*ofl,BinarySpecialOp(),   inputOps["binarySpecialOps"]);
    printList(*ofl,BinaryAssignOp(),    inputOps["binaryAssignOps"]);
    printList(*ofl,BinaryAssignOp(),    inputOps["assignOp"]);
    printList(*ofl,BinaryAssignBoolOp(),inputOps["binaryAssignBoolOps"]);
    printList(*ofl,TrinaryOp(),         inputOps["trinaryOps"]);
  }

  // Print operators.

  if (printOperators)
  {
    if (!justScalars)
    {
      if (!justExtraClasses)
      {
	printList(*ofl,UnaryFunction(),unaryOps,userClasses);
	printList(*ofl,UnaryCastFunction(),unaryCastOps,userClasses);
	printList(*ofl,BinaryFunction(),binaryOps,userClasses,userClasses);
	printList(*ofl,BinaryFunction(),binaryLeftOps,userClasses,userClasses);
	printList(*ofl,BinaryFunction(),binaryOps,
		  userClasses, expressionClass);
	printList(*ofl,BinaryFunction(),binaryLeftOps,
		  userClasses, expressionClass);
	printList(*ofl,BinaryFunction(),binaryOps,
		  expressionClass, userClasses);
	printList(*ofl,BinaryFunction(),binaryLeftOps,
		  expressionClass, userClasses);
      }
      else
      {
	printList(*ofl,UnaryFunction(),unaryOps,extraClasses);
	printList(*ofl,UnaryCastFunction(),unaryCastOps,extraClasses);
	printList(*ofl,BinaryFunction(),binaryOps,extraClasses,extraClasses);
	printList(*ofl,BinaryFunction(),binaryLeftOps,extraClasses,
		  extraClasses);
	printList(*ofl,BinaryFunction(),binaryOps,classes,extraClasses);
	printList(*ofl,BinaryFunction(),binaryLeftOps,classes,extraClasses);
	printList(*ofl,BinaryFunction(),binaryOps,extraClasses,classes);
	printList(*ofl,BinaryFunction(),binaryLeftOps,extraClasses,classes);
      }
    }

    if (!justExtraClasses)
    {
      printList(*ofl,BinaryFunction(),binaryOps,userClasses,scalars);
      printList(*ofl,BinaryFunction(),binaryLeftOps,userClasses,scalars);
      printList(*ofl,BinaryFunction(),binaryOps,scalars,userClasses);
    }
    else
    {
      printList(*ofl,BinaryFunction(),binaryOps,extraClasses,scalars);
      printList(*ofl,BinaryFunction(),binaryLeftOps,extraClasses,scalars);
      printList(*ofl,BinaryFunction(),binaryOps,scalars,extraClasses);
    }

    // The following flag covers the common situation where you define
    // ostream << class.  Some compilers define cout to be a class that
    // inherits from ostream, so the compiler would use the PETE shift
    // operators T << class which defines shift for scalars and the user
    // class.  Since this shift operration is pretty bizzare, and stream
    // output is pretty common, the default behaviour of PETE is to turn
    // off the operators that allow for scalar << container and
    // scalar << expression.

    if (shiftGuard)
    {
      *ofl << "#ifdef PETE_ALLOW_SCALAR_SHIFT" << endl;
    }
    if (!justExtraClasses)
    {
      printList(*ofl,BinaryFunction(),binaryLeftOps,scalars,userClasses);
    }
    else
    {
      printList(*ofl,BinaryFunction(),binaryLeftOps,scalars,extraClasses);
    }
    if (shiftGuard)
    {
      *ofl << "#endif // PETE_ALLOW_SCALAR_SHIFT" << endl;
    }

    if (!justScalars)
    {
      if (!justExtraClasses)
      {
	printList(*ofl,TrinaryFunction(),trinaryOps,userClasses,
		  generalT,generalT);
      }
      else
      {
	printList(*ofl,TrinaryFunction(),trinaryOps,extraClasses,generalT,
		  generalT);
      }
    }


    // Operators involving expression are guarded to make it easy to combine
    // operator files for different classes.  It's possible to generate
    // files that you can combine by using --no-expression for one of them, but
    // this approach is simpler.  Thanks to J. Striegnitz,
    // Research Center Juelich for coming up with this approach.

    if (expression)
    {
      *ofl << "#ifndef PETE_EXPRESSION_OPERATORS" << endl;
      *ofl << "#define PETE_EXPRESSION_OPERATORS" << endl;
      if (printOperators)
      {
	if (!justScalars)
	{
	  if (!justExtraClasses)
	  {
	    printList(*ofl,UnaryFunction(),unaryOps,expressionClass);
	    printList(*ofl,UnaryCastFunction(),unaryCastOps,expressionClass);
	    printList(*ofl,BinaryFunction(),binaryOps,
		      expressionClass, expressionClass);
	    printList(*ofl,BinaryFunction(),binaryLeftOps,
		      expressionClass, expressionClass);
	  }
	}
	
	if (!justExtraClasses)
	{
	  printList(*ofl,BinaryFunction(),binaryOps,expressionClass,scalars);
	  printList(*ofl,BinaryFunction(),binaryLeftOps,
		    expressionClass, scalars);
	  printList(*ofl,BinaryFunction(),binaryOps,scalars,expressionClass);
	}
	
	if (shiftGuard)
	{
	  *ofl << "#ifdef PETE_ALLOW_SCALAR_SHIFT" << endl;
	}
	if (!justExtraClasses)
	{
	  printList(*ofl,BinaryFunction(),binaryLeftOps,scalars,
		    expressionClass);
	}
	if (shiftGuard)
	{
	  *ofl << "#endif // PETE_ALLOW_SCALAR_SHIFT" << endl;
	}

	if (!justScalars)
	{
	  if (!justExtraClasses)
	  {
	    printList(*ofl,TrinaryFunction(),trinaryOps,expressionClass,
		      generalT,generalT);
	  }
	}
      }
      *ofl << "#endif  // PETE_EXPRESSION_OPERATORS" << endl;
    }
  }

  // Print assignment operators.
  
  if (assignment)
  {
    if (!justExtraClasses)
    {
      printList(*ofl,AssignFunctionForClass(),assignOps,userClasses);
    }
    else
    {
      printList(*ofl,AssignFunctionForClass(),assignOps,extraClasses);
    }
  }

  // Print footer.
  
  printFooter(*ofl,includeGuard,lanlBoilerplate,suffix);
  
  // Clean up.
  
  if (ofile != string(""))
    delete ofl;
}

// ACL:rcsinfo
// ----------------------------------------------------------------------
// $RCSfile: MakeOperators.cpp,v $   $Author: sa_smith $
// $Revision: 1.18 $   $Date: 2000/08/03 00:18:02 $
// ----------------------------------------------------------------------
// ACL:rcsinfo
