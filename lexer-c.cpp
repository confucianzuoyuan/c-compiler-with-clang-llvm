extern "C"
{
#include "clang-c/Index.h"
}

#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <fstream>

using namespace llvm;

static cl::opt<std::string>
    FileName(cl::Positional, cl::desc("Input file"), cl::Required);

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv, "My tokenizer\n");
    CXIndex index = clang_createIndex(1, 0);

    std::ifstream in_file(FileName.c_str(), std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int file_size = in_file.tellg();
    // Parse the source file into a translation unit
    CXTranslationUnit translationUnit = clang_parseTranslationUnit(
        index,
        FileName.c_str(),        // Assume the name of the file is example.c
        nullptr, 0,              // Command line args and number of args
        nullptr, 0,              // Unsaved files and number of unsaved files
        CXTranslationUnit_None); // Options

    if (translationUnit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        exit(-1);
    }

    // lexer
    CXFile file = clang_getFile(translationUnit, FileName.c_str());
    CXSourceLocation loc_start =
        clang_getLocationForOffset(translationUnit, file, 0);
    CXSourceLocation loc_end =
        clang_getLocationForOffset(translationUnit, file, file_size);
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

    clang_disposeTokens(translationUnit, tokens, numTokens);
    clang_disposeTranslationUnit(translationUnit);
    return 0;
}
