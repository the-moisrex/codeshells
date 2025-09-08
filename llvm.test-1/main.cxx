import standard;

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"

using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory MyToolCategory("my-tool options");

int main(int argc, const char **argv) {
  // CommonOptionsParser::create will parse arguments and create a
  // CompilationDatabase.
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser& OptionsParser = ExpectedParser.get();

  // Use OptionsParser.getCompilations() and OptionsParser.getSourcePathList()
  // to retrieve CompilationDatabase and the list of input file paths.
  return 0;
}
