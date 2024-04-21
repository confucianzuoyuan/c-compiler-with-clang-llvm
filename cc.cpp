#include <clang-c/Index.h>
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <fstream>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/PreprocessorOptions.h>

#include <llvm/IR/Module.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>

using namespace llvm;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << std::endl;
        std::cout << "    --emit-sema c语言文件名" << std::endl;
        std::cout << "    --emit-tokens c语言文件名" << std::endl;
        std::cout << "    --emit-ast c语言文件名" << std::endl;
        std::cout << "    --emit-ir c语言文件名" << std::endl;
        return 0;
    }
    CXIndex index = clang_createIndex(0, 0);

    std::ifstream in_file(argv[2], std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int file_size = in_file.tellg();

    // Parse the source file into a translation unit
    CXTranslationUnit translationUnit = clang_parseTranslationUnit(
        index,
        argv[2],                 // Assume the name of the file is example.c
        nullptr, 0,              // Command line args and number of args
        nullptr, 0,              // Unsaved files and number of unsaved files
        CXTranslationUnit_None); // Options

    if (translationUnit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        exit(-1);
    }

    if (strcmp(argv[1], "--emit-sema") == 0)
    {
        /// 如果有语义分析错误，打印错误
        unsigned diagnosticCount = clang_getNumDiagnostics(translationUnit);
        for (unsigned i = 0; i < diagnosticCount; ++i)
        {
            CXDiagnostic diagnostic = clang_getDiagnostic(translationUnit, i);
            CXString category = clang_getDiagnosticCategoryText(diagnostic);
            CXString message = clang_getDiagnosticSpelling(diagnostic);
            int severity = clang_getDiagnosticSeverity(diagnostic);
            CXSourceLocation loc = clang_getDiagnosticLocation(diagnostic);
            CXString fName;
            unsigned line = 0, col = 0;
            clang_getPresumedLocation(loc, &fName, &line, &col);
            std::cout << "Severity: " << severity << " File: "
                      << clang_getCString(fName) << " Line: "
                      << line << " Col: " << col << " Category: \""
                      << clang_getCString(category) << "\" Message: "
                      << clang_getCString(message) << std::endl;
            clang_disposeString(fName);
            clang_disposeString(message);
            clang_disposeString(category);
            clang_disposeDiagnostic(diagnostic);
        }
        std::cout << std::endl;
    }

    // 词法分析
    if (strcmp(argv[1], "--emit-tokens") == 0)
    {
        CXFile file = clang_getFile(translationUnit, argv[2]);
        CXSourceLocation loc_start =
            clang_getLocationForOffset(translationUnit, file, 0);
        CXSourceLocation loc_end =
            clang_getLocationForOffset(translationUnit, file, 60);
        CXSourceRange range = clang_getRange(loc_start, loc_end);
        unsigned numTokens = 0;
        CXToken *tokens = NULL;
        clang_tokenize(translationUnit, range, &tokens, &numTokens);
        for (unsigned i = 0; i < numTokens; ++i)
        {
            enum CXTokenKind kind = clang_getTokenKind(tokens[i]);
            CXString name = clang_getTokenSpelling(translationUnit, tokens[i]);
            switch (kind)
            {
            case CXToken_Punctuation:
                std::cout << "PUNCTUATION(" << clang_getCString(name) << ") ";
                break;
            case CXToken_Keyword:
                std::cout << "KEYWORD(" << clang_getCString(name) << ") ";
                break;
            case CXToken_Identifier:
                std::cout << "IDENTIFIER(" << clang_getCString(name) << ") ";
                break;
            case CXToken_Literal:
                std::cout << "COMMENT(" << clang_getCString(name) << ") ";
                break;
            default:
                std::cout << "UNKNOWN(" << clang_getCString(name) << ") ";
                break;
            }
            clang_disposeString(name);
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // A lambda function to recursively print the AST nodes
    auto printAST = [](CXCursor c, CXCursor parent, CXClientData clientData) -> CXChildVisitResult
    {
        if (clang_Location_isFromMainFile(clang_getCursorLocation(c)) == 0)
            return CXChildVisit_Continue;

        CXString cursorKindName = clang_getCursorKindSpelling(clang_getCursorKind(c));
        CXString cursorSpelling = clang_getCursorSpelling(c);

        std::cout << clang_getCString(cursorKindName) << " " << clang_getCString(cursorSpelling) << std::endl;

        clang_disposeString(cursorKindName);
        clang_disposeString(cursorSpelling);

        return CXChildVisit_Recurse;
    };

    // Visit all the nodes in the AST starting from the root cursor
    // Get the root cursor of the translation unit
    // 打印AST语法分析树
    if (strcmp(argv[1], "--emit-ast") == 0)
    {
        CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);
        clang_visitChildren(rootCursor, printAST, nullptr);
    }

    // Clean up
    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);

    if (strcmp(argv[1], "--emit-ir") == 0)
    {

        /// 生成llvm ir
        // Setup custom diagnostic options.
        IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_opts(new clang::DiagnosticOptions());
        diag_opts->ShowColors = 1;

        // Setup custom diagnostic consumer.
        //
        // We configure the consumer with our custom diagnostic options and set it
        // up that diagnostic messages are printed to stderr.
        std::unique_ptr<clang::DiagnosticConsumer> diag_print =
            std::make_unique<clang::TextDiagnosticPrinter>(llvm::errs(), diag_opts.get());

        // Create custom diagnostics engine.
        //
        // The engine will NOT take ownership of the DiagnosticConsumer object.
        auto diag_eng = std::make_unique<clang::DiagnosticsEngine>(
            nullptr /* DiagnosticIDs */, diag_opts, diag_print.get(),
            false /* own DiagnosticConsumer */);

        // Create compiler instance.
        clang::CompilerInstance cc;

        // Setup compiler invocation.
        //
        // We are only passing a single argument, which is the pseudo file name for
        // our code `code_fname`. We will be remapping this pseudo file name to an
        // in-memory buffer via the preprocessor options below.
        //
        // The CompilerInvocation is a helper class which holds the data describing
        // a compiler invocation (eg include paths, code generation options,
        // warning flags, ..).
        if (!clang::CompilerInvocation::CreateFromArgs(cc.getInvocation(),
                                                       ArrayRef<const char *>({argv[2]}),
                                                       *diag_eng))
        {
            std::puts("Failed to create CompilerInvocation!");
            return 1;
        }

        // Setup a TextDiagnosticPrinter printer with our diagnostic options to
        // handle diagnostic messaged.
        //
        // The compiler will NOT take ownership of the DiagnosticConsumer object.
        cc.createDiagnostics(diag_print.get(), false /* own DiagnosticConsumer */);

        // Create in-memory readonly buffer with pointing to our C code.
        std::ifstream t(argv[2]);
        t.seekg(0, std::ios::end);
        size_t size = t.tellg();
        std::string code_input(size, ' ');
        t.seekg(0);
        t.read(&code_input[0], size);
        std::unique_ptr<MemoryBuffer> code_buffer =
            MemoryBuffer::getMemBuffer(code_input);
        // Configure remapping from pseudo file name to in-memory code buffer
        // code_fname -> code_buffer.
        //
        // Ownership of the MemoryBuffer object is moved, except we would set
        // `RetainRemappedFileBuffers = 1` in the PreprocessorOptions.
        cc.getPreprocessorOpts().addRemappedFile(argv[2], code_buffer.release());

        // Create action to generate LLVM IR.
        //
        // If created with default arguments, the EmitLLVMOnlyAction will allocate
        // an owned LLVMContext and free it once the action goes out of scope.
        //
        // To keep the context after the action goes out of scope, either pass a
        // LLVMContext (borrowed) when creating the EmitLLVMOnlyAction or call
        // takeLLVMContext() to move ownership out of the action.
        clang::EmitLLVMOnlyAction action;
        // Run action against our compiler instance.
        if (!cc.ExecuteAction(action))
        {
            std::puts("Failed to run EmitLLVMOnlyAction!");
            return 1;
        }

        // Take generated LLVM IR module and print to stdout.
        if (auto mod = action.takeModule())
        {
            mod->print(llvm::outs(), nullptr);
        }
    }

    return 0;
}
