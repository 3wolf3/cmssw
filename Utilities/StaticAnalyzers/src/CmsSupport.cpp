//===--- CmsSupport.cpp - Provides support functions ------------*- C++ -*-===//
//
// by Shahzad Malik MUZAFFAR [ Shahzad.Malik.MUZAFFAR@cern.ch ]
//
//===----------------------------------------------------------------------===//
#include <clang/Basic/FileManager.h>
#include <clang/StaticAnalyzer/Core/Checker.h>
#include <clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h>
#include <clang/StaticAnalyzer/Core/BugReporter/BugType.h>
#include <llvm/ADT/SmallString.h>
#include "llvm/Support/raw_ostream.h"
#include "CmsSupport.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>


using namespace clangcms;
using namespace clang;
using namespace llvm;
bool support::isCmsLocalFile(const char* file)
{
  static char* LocalDir=0;
  static int DirLen=-1;
  if (DirLen==-1)
  {
    DirLen=0;
    LocalDir = getenv ("LOCALRT");
    if (LocalDir!=NULL) DirLen=strlen(LocalDir);
  }
  if ((DirLen==0) || (strncmp(file,LocalDir,DirLen)!=0) || (strncmp(&file[DirLen],"/src/",5)!=0)) return false;
  return true;
}


// This is a wrapper around NamedDecl::getQualifiedNameAsString.
// It produces more qualified output to distinguish several cases
// which would otherwise be ambiguous.
std::string support::getQualifiedName(const clang::NamedDecl &d) {
  std::string ret;
  const DeclContext *ctx = d.getDeclContext();
  if (ctx->isFunctionOrMethod() && isa<NamedDecl>(ctx))
  {
    // This is a local variable.
    // d.getQualifiedNameAsString() will return the unqualifed name for this
    // but we want an actual qualified name so we can distinguish variables
    // with the same name but that are in different functions.
    ret = getQualifiedName(*cast<NamedDecl>(ctx)) + "::" + d.getNameAsString();
  }
  else
  {
    ret = d.getQualifiedNameAsString();
  }

  if (const FunctionDecl *fd = dyn_cast<FunctionDecl>(&d))
  {
    // This is a function. getQualifiedNameAsString will return a string
    // like "ANamespace::AFunction". To this we append the list of parameters
    // so that we can distinguish correctly between
    // void ANamespace::AFunction(int);
    // and
    // void ANamespace::AFunction(float);
    ret += "(";
    const FunctionType *ft = fd->getType()->castAs<FunctionType>();
    if (const FunctionProtoType *fpt = dyn_cast<FunctionProtoType>(ft))
    {
      unsigned num_params = fd->getNumParams();
      for (unsigned i = 0; i < num_params; ++i) {
        if (i)
          ret += ", ";
        ret += fd->getParamDecl(i)->getType().getAsString();
      }

      if (fpt->isVariadic()) {
        if (num_params > 0)
          ret += ", ";
        ret += "...";
      }
    }
    ret += ")";
    if (ft->isConst())
      ret += " const";
  }

  return ret;
}


bool support::isSafeClassName(const std::string &cname) {

  static const std::vector<std::string> names = {
    "std::atomic",
    "struct std::atomic",
    "std::__atomic_",
    "std::mutex",
    "std::recursive_mutex",
    "boost::thread_specific_ptr",
    "class std::atomic",
    "class std::__atomic_",
    "class std::mutex",
    "class std::recursive_mutex",
    "class boost::thread_specific_ptr",
    "tbb::",
    "class tbb::",
    "edm::AtomicPtrCache",
    "class edm::AtomicPtrCache"
    "std::once_flag",
    "struct std::once_flag",
    "boost::<anonymous namespace>::extents"
  };
  
  for (auto& name: names)
  	if ( cname.substr(0,name.length()) == name ) return true;	
  return false;
}

bool support::isDataClass(const std::string & name) {
	std::string buf;
	llvm::raw_string_ostream os(buf);
	clang::FileSystemOptions FSO;
	clang::FileManager FM(FSO);
	const char * lPath = std::getenv("LOCALRT");
	const char * rPath = std::getenv("CMSSW_RELEASE_BASE");
	std::string lname(""); 
	std::string rname(""); 
	std::string iname(""); 
	if ( lPath != NULL && rPath != NULL ) {
		lname = std::string(lPath);
		rname = std::string(rPath);
	}
		
	std::string tname("/tmp/classes.txt");
	std::string fname1 = lname + tname;
	if (!FM.getFile(fname1)) {
		llvm::errs()<<"\n\nChecker cannot find classes.txt. Run \"USER_LLVM_CHECKERS='-enable-checker optional.ClassDumperCT -enable-checker optional.ClassDumperFT scram b checker to create $LOCALRT/tmp/classes.txt.\n\n\n";
		exit(1);
		}
	if ( FM.getFile(fname1) ) 
		iname = fname1;
	os <<"class '"<< name <<"'";
	std::ifstream ifile;
	ifile.open(iname.c_str(),std::ifstream::in);
	std::string line;
	while (std::getline(ifile,line)) {
		if ( line == os.str() ) return true;
	}
	return false;
}

bool support::isInterestingLocation(const std::string & name) {
	if ( name[0] == '<' && name.find(".h")==std::string::npos ) return false;
	if ( name.find("/test/") != std::string::npos ) return false;
	return true;
}

bool support::isKnownThrUnsafeFunc(const std::string &fname ) {
	static const std::vector<std::string> names = {
		"TGraph::Fit(const char *,", 
		"TGraph2D::Fit(const char *,", 
		"TH1::Fit(const char *,", 
		"TMultiGraph::Fit(const char *,", 
		"TTable::Fit(const char *,", 
		"TTree::Fit(const char *,", 
		"TTreePlayer::Fit(const char *,"
	};
	for (auto& name: names)
  		if ( fname.substr(0,name.length()) == name ) return true;	
	return false;
}
