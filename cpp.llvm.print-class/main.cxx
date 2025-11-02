// clang-print-class-plugin: A Clang plugin that prints a given C++ class
// and, immediately after its declaration, prints any out-of-line method
// definitions that belong to that class (so you see implementations grouped
// together with the class declaration).
//
// Files contained below (single file for simplicity):
// - plugin.cpp           (the plugin source)
// - CMakeLists.txt       (CMake to build the plugin)
// - test.cpp             (example input)
//
// Build & run instructions are included at the bottom of this file.

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace llvm;

namespace {

    class PrintClassConsumer : public ASTConsumer {
      public:
        PrintClassConsumer(CompilerInstance& CI, StringRef ClassName)
            : CI(CI), TargetName(ClassName.str()) {}

        void HandleTranslationUnit(ASTContext& Ctx) override {
            TranslationUnitDecl* TU = Ctx.getTranslationUnitDecl();

            // Find the CXXRecordDecl(s) with the requested name.
            SmallVector<CXXRecordDecl*, 4> matches;
            for (auto* D : TU->decls()) {
                if (auto* C = dyn_cast<CXXRecordDecl>(D)) {
                    if (C->getName() == TargetName) {
                        matches.push_back(C);
                    }
                }
            }

            if (matches.empty()) {
                CI.getDiagnostics().Report(DiagnosticsEngine::Warning)
                    << "print-class plugin: no class named '" << TargetName
                    << "' found in TU";
                return;
            }

            // Prepare printing policy from the compiler's LangOptions.
            PrintingPolicy Policy(Ctx.getLangOpts());
            Policy.TerseOutput = false;

            raw_ostream& out = outs();

            for (auto* C : matches) {
                // Print the class declaration (this prints the declaration text
                // as it would appear in source for that declaration node).
                C->print(out, Policy);
                out << "\n";

                // Now find any method definitions that belong to this class but
                // are *out-of-line* (i.e. defined outside the class body). We
                // iterate the translation unit to find
                // FunctionDecl/CXXMethodDecls that have this class as their
                // parent and are definitions.

                for (auto* D : TU->decls()) {
                    if (auto* FD = dyn_cast<FunctionDecl>(D)) {
                        // We want CXXMethodDecl specifically and a definition.
                        if (auto* MD = dyn_cast<CXXMethodDecl>(FD)) {
                            if (MD->getParent() == C &&
                                MD->isThisDeclarationADefinition()) {
                                // If the declaration is actually inside the
                                // class body (i.e. the "in-class" definition),
                                // the MethodDecl *is* located nested inside the
                                // CXXRecordDecl and will typically not appear
                                // separately as a top-level TU decl; but many
                                // out-of-line definitions *are* top-level
                                // FunctionDecls in the TU, so this catches
                                // those.

                                // Avoid printing the same method twice: skip if
                                // this method is already defined inline inside
                                // the class (isOutOfLine returns true for
                                // out-of-line definitions).
                                if (!MD->isOutOfLine())
                                    continue;

                                // Print the full function definition using the
                                // printing policy.
                                MD->print(out, Policy);
                                out << "\n";
                            }
                        }
                    }
                }

                out << "// ---- end of class " << TargetName << " ----\n\n";
            }
        }

      private:
        CompilerInstance& CI;
        std::string TargetName;
    };

    class PrintClassAction : public PluginASTAction {
      public:
        std::unique_ptr<ASTConsumer>
        CreateASTConsumer(CompilerInstance& CI, StringRef /*InFile*/) override {
            return std::make_unique<PrintClassConsumer>(CI, ClassName);
        }

        bool ParseArgs(const CompilerInstance& CI,
                       const std::vector<std::string>& args) override {
            for (auto& arg : args) {
                // Accept either "class=Foo" or just "Foo" for convenience.
                if (arg.starts_with("class="))
                    ClassName = arg.substr(strlen("class="));
                else
                    ClassName = arg;
            }

            if (ClassName.empty()) {
                CI.getDiagnostics().Report(DiagnosticsEngine::Warning)
                    << "print-class plugin: no class name provided; use "
                       "-plugin-arg-print-class=class=Foo";
                return false; // fail parsing
            }
            return true;
        }

        bool HandleTopLevelDecl(DeclGroupRef DG) { return true; }

      private:
        std::string ClassName;
    };

} // namespace

static FrontendPluginRegistry::Add<PrintClassAction>
    X("print-class", "print a class and its out-of-line method definitions");

// -----------------------------
// CMakeLists.txt (use this to build the plugin)
// -----------------------------
/*
cmake_minimum_required(VERSION 3.13)
project(clang-print-class-plugin)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_DIR}")
message(STATUS "Found Clang ${Clang_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(print-class-plugin SHARED plugin.cpp)

target_include_directories(print-class-plugin PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

# Link against the Clang libraries we used; adjust if link errors occur.
target_link_libraries(print-class-plugin PRIVATE
  clangAST
  clangFrontend
  clangTooling
  clangBasic
  clangSerialization
)

# Some environments require these flags from llvm-config.
llvm_map_components_to_libnames(llvm_libs support core)
target_link_libraries(print-class-plugin PRIVATE ${llvm_libs})
*/

// -----------------------------
// test.cpp (example input)
// -----------------------------
/*
#include <iostream>

class Foo {
public:
  Foo();
  void say() const;
  void definedInline() { std::cout << "inline\n"; }
};

Foo::Foo() {}

void Foo::say() const { std::cout << "hello\n"; }

int main() {
  Foo f;
  f.say();
  f.definedInline();
}
*/

// -----------------------------
// Build & run (short guide)
// -----------------------------
/*
1) Create build dir, configure, build:

   mkdir build && cd build
   cmake ..
   make -j

   This should produce libprint-class-plugin.so (name may vary by platform).

2) Run the plugin with clang's -cc1 interface. Example:

   clang -cc1 -load ./libprint-class-plugin.so -plugin print-class \
         -plugin-arg-print-class -class=Foo test.cpp

   Alternative argument styles that some clang builds accept:

   clang -cc1 -load ./libprint-class-plugin.so -plugin print-class \
         -plugin-arg-print-class class=Foo test.cpp

Notes:
 - Use `clang -cc1` because the plugin interface is part of clang's frontend
   API. If your clang installation or packaging differs, you may need to
   adjust paths or use the clang binary that matches the headers/libs you
   built against.
 - If you see undefined symbol errors at link/load time, ensure the plugin
   is built against the *same* Clang/LLVM version as the clang you're
   invoking.
 - You can also compile and load against libclang (or use libtooling) but
   using CMake + find_package(Clang) usually simplifies the configuration.
*/
