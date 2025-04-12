/*========================================================================= *
* @file parser.y
*
* @brief: This file is VSOP Parser Implementation.
*
* @authors: jamaa JAIR & Ayoub Assaoud
* @date  : 23-3-2025
* @projet: INFO0085 : Implementing a VSOP Compiler
*
* ========================================================================= */

/*========================================================================= *
* ========================= HERE WE START ================================= *
* ========================================================================= */

%{
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include "AST.hpp"
#include "semantic_analyzer.cpp"

// External functions and variables declarations
void yyerror(const char *s);
int yylex(void);
std::unique_ptr<ASTNode> root;  // Root of the AST
extern FILE *yyin;              // Input file
extern char* yytext;            // Current lexeme
extern char *fileName;          // Name of the input file
extern void initialize_dict();  // Initialize dictionary of tokens
extern bool lexer_debug_mode;   // Debug mode flag for lexer
extern int yyline;              // Current line number
extern int yycolumn;            // Current column number
extern char* error_message;     // Error message from lexer

// structure to hold a list of expressions
struct ExprList {
    std::vector<std::unique_ptr<Expr>> exprs;
};

// structure to hold a list of formal parameters
struct FormalList {
    std::vector<std::unique_ptr<Formal>> formals;
};

/**
 * Reports a syntax error with file, line and column information
 * @param message Error message to display
 * @param line Line number where the error occurred
 * @param column Column number where the error occurred
 */
void reportSyntaxError(std::string message, unsigned int line, unsigned int column) {
        std::cerr << fileName << ":" << line << ":" << column 
                  << ": syntax error: "<< message << std::endl;
        exit(1); // Exit the program with an error code
    }
%}

// Define the start symbol of the grammar
%start program

// Define the union for token values
%union {
    int num;                      // For number tokens
    void* node;                   // For AST nodes
    char* str;                    // For string tokens
    bool boolean;                 // For boolean tokens
    struct ExprList* expr_list;   // For expression lists
    struct FormalList* formal_list; // For formal parameter lists
    struct {
        int line_error;           // Line where error occurred
        int column_error;         // Column where error occurred
    } error_location;
    struct {
        char* str;
        unsigned int line;
        unsigned int column;
    } loc;
}

%debug
%locations
%token <num> NUMBER               // Integer literal
%token CLASS EXTENDS IF THEN ELSE WHILE DO LET IN NEW NOT SELF UNIT BOOL INT32 OR AND ISNULL STRING TRUE FALSE
%token LBRACE RBRACE LPAR RPAR COLON SEMICOLON COMMA EQUAL LOWER LOWER_EQUAL ASSIGN DOT PLUS MINUS TIMES DIV POW GREATER
%token UMINUS                     // Unary minus operator (for precedence)
%token <error_location> ERROR     // Error token with location information

// Non-terminal symbols and their associated types
%nterm <node> field class_body field_assign classDecl program expr Method block formal classDeclList
%nterm <expr_list> block_body args expr_list
%nterm <formal_list> formals
%nterm <str> type extends_or_not
%token <loc> OBJECT_IDENTIFIER
%token <loc> TYPE_IDENTIFIER
%token <str> STR TRUE_TYPE FALSE_TYPE

// Operator precedence and associativity rules
%precedence IF THEN WHILE DO LET IN
%precedence ELSE
%precedence EMPTY
%precedence RPAR

%right ASSIGN
%left AND
%right NOT
%left EQUAL LOWER LOWER_EQUAL GREATER
%left PLUS MINUS
%left TIMES DIV
%right UMINUS ISNULL
%right POW
%left DOT


%%

/* Grammar Rules */

/* Program is a list of class declarations */
program:
    classDeclList {
        root = std::unique_ptr<ASTNode>(static_cast<ASTNode*>($1));
    };

/* A list of class declarations */
classDeclList:
    classDecl {
        Program* prog = new Program();
        prog->addClass(std::unique_ptr<ClassNode>(static_cast<ClassNode*>($1)));
        $$ = prog;
    }
    | classDeclList classDecl {
        static_cast<Program*>($1)->addClass(std::unique_ptr<ClassNode>(static_cast<ClassNode*>($2)));
        $$ = $1;
    };

/* A class declaration */
classDecl:
    CLASS TYPE_IDENTIFIER extends_or_not LBRACE class_body RBRACE {
    $$ = new ClassNode(std::string($2.str), std::string($3), &static_cast<ClassNode*>($5)->getFields(), &static_cast<ClassNode*>($5)->getMethods(), $2.column, $2.line);
    };

/* Parent can be empty (defaults to Object) */
extends_or_not:
    EXTENDS TYPE_IDENTIFIER {
        $$ = $2.str;
    }
    | /* empty */ {
        $$ = (char*)"Object";
    };

/* Class body contains fields and methods */
class_body:
    /* empty */ { 
        std::vector<std::unique_ptr<FieldNode>>* fields = new std::vector<std::unique_ptr<FieldNode>>();
        std::vector<std::unique_ptr<MethodNode>>* methods = new std::vector<std::unique_ptr<MethodNode>>();
        $$ = new ClassNode("", "", fields, methods);
         }
    | field class_body  {
        static_cast<ClassNode*>($2)->addField(std::unique_ptr<FieldNode>(static_cast<FieldNode*>($1)));
        $$ = $2;
    }
    | Method class_body {
        static_cast<ClassNode*>($2)->addMethod(std::unique_ptr<MethodNode>(static_cast<MethodNode*>($1)));
        $$ = $2;
    };

/* Field declaration with optional initialization */
field:
    OBJECT_IDENTIFIER COLON type field_assign SEMICOLON {
        $$ = (new FieldNode(std::string($1.str), Type($3),
        $1.column, $1.line,
        std::unique_ptr<Expr>(static_cast<Expr*>($4))
        ));
    };

/* Optional field initialization */
field_assign:
    /* empty */ {
        $$ = nullptr;
    }
    | ASSIGN expr {
        $$ = $2;
    };

/* Method declaration with or without parameters */
Method:
    OBJECT_IDENTIFIER LPAR formals RPAR COLON type block {
        // Method with parameters
        std::vector<std::unique_ptr<Formal>> params;
        for (auto& formal : $3->formals) {
            params.push_back(std::move(formal));
        }
        delete $3; // Freedom Memomry (Ayoub don't forget to free memory pleaaase ;) )
        
        $$ = new MethodNode(std::string($1.str), Type($6), std::move(params), std::unique_ptr<Block>(static_cast<Block*>($7)), $1.column, $1.line);
    }
    | OBJECT_IDENTIFIER LPAR RPAR COLON type block {
        // Method without parameters
        $$ = new MethodNode(std::string($1.str), Type($5), std::unique_ptr<Block>(static_cast<Block*>($6)), $1.column, $1.line);
    }
    | OBJECT_IDENTIFIER LPAR formals RPAR COLON type {
        // Error: Method declaration without implementation
        reportSyntaxError("Method without implementationt !", yyline, yycolumn);
    }
    | OBJECT_IDENTIFIER LPAR formals COLON type block {
        // Error: Unclosed argument list
        reportSyntaxError("Unclosed argument list of the Method !", yyline, yycolumn);
    };

/* Block of expressions */
block:
    LBRACE block_body RBRACE {
        Block* blk = new Block();
        for (auto& expr : $2->exprs) {
            blk->addExpr(std::move(expr));
        }
        delete $2;
        $$ = blk;
    }
    | LBRACE RBRACE {
        $$ = new Block();
    };

/* Body of a block, containing expressions separated by semicolons */
block_body:
    expr {
        ExprList* list = new ExprList();
        list->exprs.push_back(std::unique_ptr<Expr>(static_cast<Expr*>($1)));
        $$ = list;
    }
    | block_body SEMICOLON expr {
        $1->exprs.push_back(std::unique_ptr<Expr>(static_cast<Expr*>($3)));
        $$ = $1;
    };

/* List of formal parameters */
formals:
    formal {
        FormalList* list = new FormalList();
        list->formals.push_back(std::unique_ptr<Formal>(static_cast<Formal*>($1)));
        $$ = list;
    }
    | formals COMMA formal {
        $1->formals.push_back(std::unique_ptr<Formal>(static_cast<Formal*>($3)));
        $$ = $1;
    };

/* Formal parameter */
formal:
    OBJECT_IDENTIFIER COLON type {
        $$ = static_cast<Expr*>(new Formal(std::string($1.str), Type($3)));
    };

/* Type specification */
type:
    TYPE_IDENTIFIER {
        $$ = $1.str;
    }
    | INT32 {
        $$ = (char*)"int32";
    }
    | BOOL {
        $$ = (char*)"bool";
    }
    | STRING {
        $$ = (char*)"string";
    }
    | UNIT {
        $$ = (char*)"unit";
    }
    | { 
        reportSyntaxError("Invalid Type !", yyline, yycolumn);
    };

/* Expressions */
expr: 
    /* If-then construct */
    IF expr THEN expr {
        $$ = static_cast<Expr*>(new Conditional(
            std::unique_ptr<Expr>(static_cast<Expr*>($2)),
            std::unique_ptr<Expr>(static_cast<Expr*>($4)),
            nullptr
        ));
    }
    /* If-then-else construct */
    | IF expr THEN expr ELSE expr {
        $$ = static_cast<Expr*>(new Conditional(
            std::unique_ptr<Expr>(static_cast<Expr*>($2)),
            std::unique_ptr<Expr>(static_cast<Expr*>($4)),
            std::unique_ptr<Expr>(static_cast<Expr*>($6))
        ));
    }
    /* Error handling for conditional expressions */
    | IF error THEN expr {
        reportSyntaxError("Invalid expression in IF condition", yyline, yycolumn);
    }
    | IF expr THEN error {
        reportSyntaxError("Invalid expression in THEN clause", yyline, yycolumn);
    }
    | IF expr THEN expr ELSE error {
        reportSyntaxError("Invalid expression in ELSE clause", yyline, yycolumn);
    }
    | IF error SEMICOLON {
        reportSyntaxError("Malformed IF statement", yyline, yycolumn);
    }
    /* While loop */
    | WHILE expr DO expr {
        $$ = static_cast<Expr*>(new WhileLoop(
            std::unique_ptr<Expr>(static_cast<Expr*>($2)),
            std::unique_ptr<Expr>(static_cast<Expr*>($4))
        ));
    }
    /* Error handling for while loops */
    | WHILE error DO expr {
        reportSyntaxError("Invalid expression in WHILE condition", yyline, yycolumn);
    }
    | WHILE expr DO error {
        reportSyntaxError("Invalid expression in WHILE body", yyline, yycolumn);
    }
    | WHILE error SEMICOLON {
        reportSyntaxError("Malformed WHILE statement", yyline, yycolumn);
    }
    /* Let binding without initialization */
    | LET OBJECT_IDENTIFIER COLON type IN expr {
        $$ = static_cast<Expr*>(new Let(
            std::string($2.str), 
            Type($4), 
            nullptr, 
            std::unique_ptr<Expr>(static_cast<Expr*>($6))
        ));
    }
    /* Let binding with initialization */
    | LET OBJECT_IDENTIFIER COLON type ASSIGN expr IN expr {
        $$ = static_cast<Expr*>(new Let(
            std::string($2.str), 
            Type($4),
            $2.column, $2.line,
            std::unique_ptr<Expr>(static_cast<Expr*>($6)), 
            std::unique_ptr<Expr>(static_cast<Expr*>($8))
        ));
    }
    /* Error handling for let bindings */
    | LET OBJECT_IDENTIFIER COLON error IN expr {
        reportSyntaxError("Invalid type in LET declaration", yyline, yycolumn);
    }
    | LET OBJECT_IDENTIFIER COLON type ASSIGN error IN expr {
        reportSyntaxError("Invalid initialization expression in LET", yyline, yycolumn);
    }
    | LET OBJECT_IDENTIFIER COLON type ASSIGN expr IN error {
        reportSyntaxError("Invalid body expression in LET", yyline, yycolumn);
    }
    /* Assignment */
    | OBJECT_IDENTIFIER ASSIGN expr {
        $$ = static_cast<Expr*>(new Assign($1.str, $1.column, $1.line, std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    } 
    
    /* Unary Operations */ 
    | NOT expr {
        $$= static_cast<Expr*>(new UnOp("not",std::unique_ptr<Expr>(static_cast<Expr*>($2))));
    }
    | MINUS expr %prec UMINUS{
        $$= static_cast<Expr*>(new UnOp("-",std::unique_ptr<Expr>(static_cast<Expr*>($2))));
    }
    | ISNULL expr {
        $$= static_cast<Expr*>(new UnOp("isnull",std::unique_ptr<Expr>(static_cast<Expr*>($2))));
    } 
    
    /* Binary Operations */    
    | expr EQUAL expr {
        $$ = static_cast<Expr*>(new BinaryOperation("=", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr LOWER expr {
        $$ = static_cast<Expr*>(new BinaryOperation("<", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr LOWER_EQUAL expr {
        $$ = static_cast<Expr*>(new BinaryOperation("<=", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr PLUS expr {
        $$ = static_cast<Expr*>(new BinaryOperation("+", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr MINUS expr {
        $$ = static_cast<Expr*>(new BinaryOperation("-", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr TIMES expr {
        $$ = static_cast<Expr*>(new BinaryOperation("*", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr DIV expr {
        $$ = static_cast<Expr*>(new BinaryOperation("/", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr POW expr {
        $$ = static_cast<Expr*>(new BinaryOperation("^", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    | expr AND expr {
        $$ = static_cast<Expr*>(new BinaryOperation("and", std::unique_ptr<Expr>(static_cast<Expr*>($1)), std::unique_ptr<Expr>(static_cast<Expr*>($3))));
    }
    
    /* Method Call */
    | OBJECT_IDENTIFIER LPAR args RPAR {
        // Simple method call on self
        std::vector<std::unique_ptr<Expr>> arguments;
        if ($3) {
            arguments = std::move($3->exprs);
            delete $3;
        }
        $$ = static_cast<Expr*>(new Call(
        std::string($1.str),
        std::move(arguments),
        std::unique_ptr<Expr>(new Self("self")),
        $1.column, $1.line
    ));
    }
    | expr DOT OBJECT_IDENTIFIER LPAR args RPAR {
        // Method call on an object
        std::vector<std::unique_ptr<Expr>> arguments;
        if ($5) {
            arguments = std::move($5->exprs);
            delete $5;
        }
        $$ = static_cast<Expr*>(new Call(
            std::string($3.str),
            std::move(arguments),
            std::unique_ptr<Expr>(static_cast<Expr*>($1)),
            $3.column, $3.line
        ));
    } 
    
    /* Object instantiation */
    | NEW TYPE_IDENTIFIER {
        $$ = static_cast<Expr*>(new New(std::string($2.str)));
    }
    
    /* Variable reference */
    | OBJECT_IDENTIFIER {
        $$ = static_cast<Expr*>(new ObjectIdentifier($1.str, $1.column, $1.line));
    }
    
    /* Self reference */
    | SELF {
        $$ = static_cast<Expr*>(new Self("self"));
    } 
    
    /* Literals */
    | NUMBER {
        $$ = static_cast<Expr*>(new IntegerLiteral($1));
    }
    | STR {
        $$ = static_cast<Expr*>(new StringLiteral($1));
    }
    | TRUE_TYPE {
        $$ = static_cast<Expr*>(new BooleanLiteral(true));
    }
    | FALSE_TYPE {
        $$ = static_cast<Expr*>(new BooleanLiteral(false));
    }
    
    /* Empty parentheses - unit value */
    | LPAR RPAR {
        $$ = static_cast<Expr*>(new Parenthesis());
    }
    
    /* Parenthesized expression */
    | LPAR expr RPAR {
        $$ = $2;
    }
    
    /* Block expression */
    | block {
        $$ = $1;
    };

/* Method argument list */
args:
    /* empty */ {
        $$ = nullptr;
    }
    | expr_list {
        $$ = $1;
    }
    ;

/* List of expressions */
expr_list:
    expr {
        auto list = new ExprList();
        list->exprs.push_back(std::unique_ptr<Expr>(static_cast<Expr*>($1)));
        $$ = list;
    }
    | expr_list COMMA expr {
        $1->exprs.push_back(std::unique_ptr<Expr>(static_cast<Expr*>($3)));
        $$ = $1;
    }
    ;

%%

/**
 * Function called when a syntax error is detected
 * @param s Error message
 */
void yyerror(const char *s) {
    std::cerr << fileName << ":" << yyline << ":" << yycolumn
              << ": Syntax error: " << s << std::endl;
    exit(1);
}

/**
 * Main function 
 */
int main(int argc, char **argv) {
    // Check command line arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " -p|-l|-c <source_code_file>\n";
        return 1;
    }
    
    fileName = argv[2];
    initialize_dict();
    
    // Directly append the content of "Object.vsop" to the input file
    std::string tempFileName = std::string(argv[2]) + "_tempo";
    FILE* tempFile = fopen(tempFileName.c_str(), "w+"); // Open in write mode and truncate if it exists
    if (!tempFile) {
        std::cerr << "Error: Can't create temporary file " << tempFileName << std::endl;
        return 1;
    }

    // Copy the content of the input file to the temporary file
    FILE* inputFile = fopen(argv[2], "r");
    if (!inputFile) {
        std::cerr << "Error: Can't open file " << argv[2] << std::endl;
        fclose(tempFile);
        return 1;
    }
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFile)) > 0) {
        fwrite(buffer, 1, bytesRead, tempFile);
    }
    fclose(inputFile);

    // Append the hardcoded content of "Object.vsop" to the temporary file
    const char* objectVsopContent = R""(
        class Object {
            print(s : string) : Object { (* print s on stdout, then return self*) self}
            printBool(b: bool) : Object { (* print b on stdout, then return self *) self}
            printInt32(i: int32) : Object { (* print i on stdout, then return self *) self}
            inputLine() : string {
                (* read one line from stdin, return "" in case of error *) ""}
            inputBool() : bool {
                (* read one boolean value from stdin, exit with error message in case of error *) true}
            inputInt32() : int32 {
                (* read one integer from stdin, exit with error message in case of error *) 0}
        })"";
    fwrite(objectVsopContent, 1, strlen(objectVsopContent), tempFile);
    fclose(tempFile);

    // Reopen the temporary file as the input file
    yyin = fopen(tempFileName.c_str(), "r");
    if (!yyin) {
        std::cerr << "Error: Can't open temporary file " << tempFileName << std::endl;
        return 1;
    }

    // Process based on the mode argument (-p, -l, or -c)
    if (strcmp(argv[1], "-l") == 0) {

        yyin = fopen(argv[2], "r");
        if (!yyin) {
            std::cerr << "Error: Can't open temporary file " << tempFileName << std::endl;
            return 1;
        }

        // Lexical analysis mode only
        lexer_debug_mode = true;
        int token;
        while ((token = yylex()) != 0) { } // No need to print anything, printing is done during lexing
    }
    else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-p") == 0) {
        yyin = fopen(tempFileName.c_str(), "r");
        if (!yyin) {
            std::cerr << "Error: Can't open temporary file " << tempFileName << std::endl;
            return 1;
        }
        lexer_debug_mode = false;
        int token;
        
        // First pass: ensure lexing is done without errors
        while ((token = yylex()) != 0) {
            if (token == ERROR) {
                std::cerr << fileName << ":" << yylval.error_location.line_error << ":" 
                          << yylval.error_location.column_error << ": lexical error : " 
                          << error_message << std::endl;
                exit(1);
            }
        }
        // Second pass: syntactic analysis
        yycolumn = 1; yyline = 1;
        rewind(yyin);
        if (!yyparse()) {
            // Successful parsing
            if (root) {
                // Load Object.vsop and extend the AST
                // FILE* objectFile = fopen("Object.vsop", "r");
                // if (objectFile) {
                //     yyin = objectFile;
                //     yycolumn = 1; yyline = 1;
                //     if (!yyparse()) {
                //         // Successfully parsed Object.vsop
                //         fclose(objectFile);
                //     } else {
                //         std::cerr << "Error parsing Object.vsop!" << std::endl;
                //         fclose(objectFile);
                //         return EXIT_FAILURE;
                //     }
                // } else {
                //     std::cerr << "Error: Can't Open File Object.vsop" << std::endl;
                //     return EXIT_FAILURE;
                // }

                if (strcmp(argv[1], "-c") == 0) {

                    // add 
                    SemanticAnalyzer* analyzer = new SemanticAnalyzer(std::string(fileName));
                    analyzer->analyze(static_cast<Program*>(root.get()));
                    // std::cout << "analyzer->isAccepted : "<< analyzer->isAccepted << std::endl;
                    
                    if (analyzer->isAccepted == true)
                        std::cout << root->toString2() << std::endl;
                    else
                        return EXIT_FAILURE;
                } else if (strcmp(argv[1], "-p") == 0) {
                    std::cout << root->toString() << std::endl;
                }
            } else {
                std::cerr << "Error: AST is empty!" << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Parsing Error!" << std::endl;
            return EXIT_FAILURE;
        }
    }
    else {
        std::cerr << "Invalid Option: " << argv[1] << "\nUsage: " << argv[0] 
                  << " -l|-p|-c <source_code_file>\n";  
    }
    
    // Clean up
    fclose(yyin);
    return EXIT_SUCCESS;
}

/*========================================================================= *
* ========================= HERE WE FINISH ================================ *
* ========================================================================= */