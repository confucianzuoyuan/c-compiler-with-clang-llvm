#include <clang-c/Index.h>
#include <iostream>

int main()
{
    // Create an index with excludeDeclarationsFromPCH = 1, displayDiagnostics = 0
    CXIndex index = clang_createIndex(1, 0);

    // Parse the source file into a translation unit
    CXTranslationUnit translationUnit = clang_parseTranslationUnit(
        index,
        "example.c",             // Assume the name of the file is example.c
        nullptr, 0,              // Command line args and number of args
        nullptr, 0,              // Unsaved files and number of unsaved files
        CXTranslationUnit_None); // Options

    if (translationUnit == nullptr)
    {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        exit(-1);
    }

    // Get the root cursor of the translation unit
    CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);
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

    // lexer
    CXFile file = clang_getFile(translationUnit, "example.c");
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
    clang_visitChildren(rootCursor, printAST, nullptr);

    // Clean up
    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);

    return 0;
}
