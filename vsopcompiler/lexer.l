 /*
 * VSOP Language Lexer
 * This file defines the lexical rules for tokenizing VSOP source code
 */
%{
    #include "parser.hpp"
    #include <iostream>              /* for input/output */
    #include <cstring>               /* for string handling */
    #include <unordered_map>         /* for storing key-value mappings */
    #include <stack>                 /* for using stack data structure (LIFO) */
    #include <tuple>                 /* for using tuples */
    #include <string>                /* for using string data type */
    #include <cctype>                /* for using character type functions like isdigit() */


    extern FILE *yyin; // Input file for lexical analysis

    // Position tracking variables
    unsigned int yyline = 1;    // Current line number
    unsigned int yycolumn = 1;  // Current column number

    // String position tracking for error reporting
    unsigned int string_line_start = 0;
    unsigned int string_column_start = 0;

    char *fileName; // Name of the file being analyzed

    // Stack for nested comments tracking (line, column)
    std::stack<std::tuple<int, int>> comment_pda;

    // Stack for string state tracking
    std::stack<int> string_pda;

    // Symbol dictionary for output formatting
    std::unordered_map<std::string, std::string> dict;

    bool lexer_debug_mode = false; // Debug mode flag
    char* error_message;           // Error message storage
    
    /**
     * Initializes the dictionary that maps symbols to their textual names
     * Used for debug output formatting
     */
    void initialize_dict() {
        dict["{"] = "lbrace";      // Left brace
        dict["}"] = "rbrace";      // Right brace
        dict["("] = "lpar";        // Left parenthesis
        dict[")"] = "rpar";        // Right parenthesis
        dict[":"] = "colon";       // Colon
        dict[";"] = "semicolon";   // Semicolon
        dict[","] = "comma";       // Comma
        dict["+"] = "plus";        // Addition
        dict["-"] = "minus";       // Subtraction
        dict["*"] = "times";       // Multiplication
        dict["/"] = "div";         // Division
        dict["^"] = "pow";         // Power
        dict["."] = "dot";         // Dot
        dict["="] = "equal";       // Equal
        dict["<="] = "lower-equal"; // Less than or equal
        dict["<"] = "lower";       // Less than
        dict["<-"] = "assign";     // Assignment
    }

    /**
     * Reports a lexical error with position information
     * @param message Error message to display
     * @param line Line number where the error occurred
     * @param column Column number where the error occurred
     */
    void reportLexicalError(std::string message, unsigned int line, unsigned int column) {
        std::cerr << fileName << ":" << line << ":" << column << ": lexical error : " << message<< std::endl;
        exit(1); // Exit the program with an error code
    }
    
    std::string string_buffer; // Buffer to accumulate string content

%}
/* Regular expression definitions */

/* Basic character classes */
DIGIT [0-9]
LOWERCASE_LETTER [a-z]
UPPERCASE_LETTER [A-Z]
LETTER {LOWERCASE_LETTER}|{UPPERCASE_LETTER}

/* Number formats */
INTEGER_LITERAL_DIGITS {DIGIT}+
INTEGER_LITERAL_HEX 0x{HEX_DIGIT}+
BIN_DIGIT [0|1]
HEX_DIGIT ({DIGIT}|[A-F]|[a-f])

/* Whitespace characters */
SPACE [ ]
TAB [\t]
LF [\n]
FF [\f]
CR [\r]
WHITESPACE [ \t\f\r]

/* Operators and symbols */
LBRACE "{"
RBRACE "}"
LPAR "("
RPAR ")"
COLON ":"
SEMICOLON ";"
COMMA ","
PLUS "+"
MINUS "-"
TIMES "*"
DIV "/"
POW "^"
DOT "."
EQUAL "="
LOWER "<"
LOWER_EQUAL "<="
ASSIGN "<-"
GREATER ">"

/* Comment markers */
OPEN_COMMENT "(*"
CLOSE_COMMENT "*)"

/* String markers */
OPEN_STRING \"
CLOSE_STRING \"

/* Operator pattern - matches any of the defined operators */
OPERATORS {LBRACE}|{RBRACE}|{LPAR}|{RPAR}|{COLON}|{SEMICOLON}|{COMMA}|{PLUS}|{MINUS}|{TIMES}|{DIV}|{POW}|{DOT}|{EQUAL}|{LOWER_EQUAL}|{ASSIGN}|{LOWER} 

/* VSOP keywords */
KEYWORDS class|bool|and|do|else|if|in|int32|isnull|let|new|not|self|string|then|unit|while|extends|true|false

/* Identifier patterns */
OBJECT_IDENTIFIER {LOWERCASE_LETTER}({LETTER}|{DIGIT}|"_")*
TYPE_IDENTIFIER {UPPERCASE_LETTER}({LETTER}|{DIGIT}|"_")*

/* Error pattern for malformed hex literals */
INTEGER_LITERAL_HEX_ERROR {DIGIT}+({OBJECT_IDENTIFIER}|{TYPE_IDENTIFIER})

/* Single-line comment pattern */
SINGLE_COMMENT "//"[^\n]*

/* Start conditions for multi-line comments and strings */
%x COMMENT   
%x LEX_STRING 

%%


{LF}	                    { yyline++; yycolumn = 1; }
{WHITESPACE}+               { yycolumn += yyleng; }
{SINGLE_COMMENT}            { yycolumn = 1;}


{KEYWORDS} {
    /* Convert to lowercase for consistent comparison */
    std::string token_str(yytext);
    for (char &c : token_str) {
        c = std::tolower(c);
    }

    if (lexer_debug_mode)
        std::cout<<yyline<<","<<yycolumn<<","<<token_str.c_str()<<std::endl;
    yycolumn += yyleng;

    /* Return the appropriate token for each keyword */
    if (!strcmp(token_str.c_str(), "class"))  return CLASS;
    if (!strcmp(token_str.c_str(), "extends")) return EXTENDS;
    if (!strcmp(token_str.c_str(),  "if"))  return IF;
    if (!strcmp(token_str.c_str(),  "then")) return THEN;
    if (!strcmp(token_str.c_str(),  "else")) return ELSE;
    if (!strcmp(token_str.c_str(),  "while")) return WHILE;
    if (!strcmp(token_str.c_str(),  "do")) return DO;
    if (!strcmp(token_str.c_str(),  "let")) return LET;
    if (!strcmp(token_str.c_str(),  "in")) return IN;
    if (!strcmp(token_str.c_str(),  "new")) return NEW;
    if (!strcmp(token_str.c_str(),  "not")) return NOT;
    if (!strcmp(token_str.c_str(),  "true")) return TRUE_TYPE;
    if (!strcmp(token_str.c_str(),  "false")) return FALSE_TYPE;
    if (!strcmp(token_str.c_str(),  "self")) return SELF;
    if (!strcmp(token_str.c_str(),  "unit")) return UNIT;
    if (!strcmp(token_str.c_str(),  "bool")) return BOOL;
    if (!strcmp(token_str.c_str(),  "int32")) return INT32; 
    if (!strcmp(token_str.c_str(),  "and")) return AND;
    if (!strcmp(token_str.c_str(),  "isnull")) return ISNULL;
    if (!strcmp(token_str.c_str(),  "string")) return STRING;
}

{OBJECT_IDENTIFIER} { 
    if(lexer_debug_mode)
        std::cout<<yyline<<","<<yycolumn<<",object-identifier,"<<yytext<<std::endl;
    
    yylval.loc.str = strdup(yytext); 
    yylval.loc.line = yyline;
    yylval.loc.column = yycolumn;

    yycolumn += yyleng;
    return OBJECT_IDENTIFIER;
    }
{TYPE_IDENTIFIER} {
    if(lexer_debug_mode)
        std::cout<<yyline<<","<<yycolumn<<",type-identifier,"<<yytext<<std::endl;
    yylval.str = strdup(yytext);
    yylval.loc.line = yyline;
    yylval.loc.column = yycolumn;

    yycolumn += yyleng;
    return TYPE_IDENTIFIER;
    }

{INTEGER_LITERAL_DIGITS} {
    if(lexer_debug_mode)
        std::cout<<yyline<<","<<yycolumn<<",integer-literal,"<<std::stoi(yytext)<<std::endl;
    
    yylval.num = std::stoi(yytext);
    // yylval.loc.line = yyline;
    // yylval.loc.column = yycolumn;

    yycolumn += yyleng;
    return NUMBER;
}

{INTEGER_LITERAL_HEX} {   
    if(lexer_debug_mode)    
        std::cout<<yyline<<","<<yycolumn<<",integer-literal,"<<std::stoi(yytext, nullptr, 16)<<std::endl;
    
    yylval.num = std::stoi(yytext, nullptr, 16);
    // yylval.loc.line = yyline;
    // yylval.loc.column = yycolumn;

    yycolumn += yyleng;
    return NUMBER;
}

{INTEGER_LITERAL_HEX_ERROR} {
    yylval.error_location.line_error = yyline;
    yylval.error_location.column_error = yycolumn;
    error_message = strdup("Integral Literal Hexa Error !");
    if (!lexer_debug_mode)
        return ERROR;
    reportLexicalError(yytext+std::string("Integral Literal Hexa Error !"), yyline, yycolumn);
}


{OPERATORS} {
    if(lexer_debug_mode)
        std::cout<<yyline<<","<<yycolumn<<","<<dict[yytext]<<std::endl;
    yycolumn += yyleng;
    
    /* send the right token to the parser */
    if (strcmp(yytext, "{") == 0) return LBRACE;
    else if (strcmp(yytext, "}") == 0) return RBRACE;
    else if (strcmp(yytext, "(") == 0) return LPAR;
    else if (strcmp(yytext, ")") == 0) return RPAR;
    else if (strcmp(yytext, ":") == 0) return COLON;
    else if (strcmp(yytext, ";") == 0) return SEMICOLON;
    else if (strcmp(yytext, ",") == 0) return COMMA;
    else if (strcmp(yytext, "<-") == 0) return ASSIGN;
    else if (strcmp(yytext, ".") == 0) return DOT;
    else if (strcmp(yytext, "+") == 0) return PLUS;
    else if (strcmp(yytext, "-") == 0) return MINUS;
    else if (strcmp(yytext, "*") == 0) return TIMES;
    else if (strcmp(yytext, "/") == 0) return DIV;
    else if (strcmp(yytext, "^") == 0) return POW;
    else if (strcmp(yytext, "=") == 0) return EQUAL;
    else if (strcmp(yytext, "<") == 0) return LOWER;
    else if (strcmp(yytext, "<=") == 0) return LOWER_EQUAL;
}


{OPEN_STRING} {
    string_line_start = yyline;
    string_column_start = yycolumn;
    if(lexer_debug_mode)
        std::cout << yyline << "," << yycolumn << ",string-literal,\"";
    yycolumn += yyleng;
    string_buffer.clear();
    BEGIN(LEX_STRING);
}

<LEX_STRING>\\\"                  { 
    if(lexer_debug_mode) std::cout << "\\x22"; 
    string_buffer += "\\x22"; 
    yycolumn += yyleng; 
}
<LEX_STRING>{WHITESPACE}+         { 
    if(lexer_debug_mode) std::cout << " "; 
    string_buffer += " "; 
    yycolumn += yyleng; 
} 
<LEX_STRING>"\\b"                 { 
    if(lexer_debug_mode) std::cout << "\\x08"; 
    yycolumn += yyleng; 
    string_buffer += "\\x08"; 
}
<LEX_STRING>"\\t"                 { 
    if(lexer_debug_mode) std::cout << "\\x09"; 
    string_buffer += "\\x09"; 
    yycolumn += yyleng; 
}
<LEX_STRING>\\n                   { 
    if(lexer_debug_mode) std::cout << "\\x0a"; 
    string_buffer += "\\x0a"; 
    yycolumn += yyleng; 
}
<LEX_STRING>\\r                   { 
    if(lexer_debug_mode) std::cout << "\\x0d"; 
    string_buffer += "\\x0d"; 
    yycolumn += yyleng; 
}
<LEX_STRING>"\\\\"                { 
    if(lexer_debug_mode) std::cout << "\\x5c"; 
    string_buffer += "\\x5c"; 
    yycolumn += yyleng; 
}
<LEX_STRING>\\\n{WHITESPACE}+ {
    if(lexer_debug_mode) std::cout << "";
    string_buffer += "";
    yycolumn = yyleng-1;
    yyline += 1;
}
<LEX_STRING>\\\n                  {	yycolumn = yyleng-1; yyline += 1; }
<LEX_STRING>[\n] {
    yylval.error_location.line_error = yyline;
    yylval.error_location.column_error = yycolumn;
    error_message = strdup("character '\\n' is illegal in this context !");
    if (!lexer_debug_mode)
        return ERROR;
    reportLexicalError("character '\\n' is illegal in this context.", yyline, yycolumn);
}

<LEX_STRING>\\x{HEX_DIGIT}{2}     { 
    if(lexer_debug_mode) std::cout << yytext; 
    string_buffer += yytext; 
    yycolumn += yyleng; 
}
<LEX_STRING><<EOF>> {
    yylval.error_location.line_error = string_line_start;
    yylval.error_location.column_error = string_column_start;
    error_message = strdup("Unterminated string.");
    if (!lexer_debug_mode)
        return ERROR;
    reportLexicalError("Unterminated string.", string_line_start, string_column_start);
}
<LEX_STRING>{CLOSE_STRING} {
    if(lexer_debug_mode) {
        std::cout << "\"" << std::endl;
        // std::cout << "Envoi du token STR avec valeur : " << string_buffer << std::endl;
    yylval.str = strdup(string_buffer.c_str());
    // yylval.loc.line = yyline;
    // yylval.loc.column = yycolumn;
    }

    yycolumn += yyleng;
    BEGIN(INITIAL);
    return STR;
}
<LEX_STRING>\\.                  { 
    yylval.error_location.line_error = yyline;
    yylval.error_location.column_error = yycolumn;
    error_message = strdup("");
    if (!lexer_debug_mode)
        return ERROR;
    reportLexicalError("", yyline, yycolumn);
    }
<LEX_STRING>.                    { 
    if(lexer_debug_mode) std::cout << yytext; 
    string_buffer += yytext; 
    yycolumn += yyleng; 
}

{CLOSE_COMMENT} {
    yylval.error_location.line_error = yyline;
    yylval.error_location.column_error = yycolumn;
    error_message = strdup("Unexpected closing comment");
    if (!lexer_debug_mode)
        return ERROR;
    reportLexicalError("Unexpected closing comment", yyline, yycolumn);
}

{OPEN_COMMENT}	{
	// open comment detected -> we need to save the (line, column) and go to <COMMENT>
	comment_pda.push(std::make_tuple(yyline, yycolumn));
	yycolumn += yyleng;
	BEGIN(COMMENT);
}
<COMMENT>{
    {OPEN_COMMENT} {
		// open comment detected -> we need to save the (line, column)
        comment_pda.push(std::make_tuple(yyline, yycolumn));
        yycolumn += yyleng;
    }

    {CLOSE_COMMENT} {
        if (comment_pda.empty()) {
			// if a close comment found but any comment was opened !! to check with ayoub
            yylval.error_location.line_error = yyline;
            yylval.error_location.column_error = yycolumn;
            error_message = strdup("Unexpected closing comment");
            if (!lexer_debug_mode)
                return ERROR;
            reportLexicalError("Unexpected closing comment", yyline, yycolumn);
        }
		// if a close comment found we pop in the stack and we continue
        comment_pda.pop();
        yycolumn += yyleng;
        if (comment_pda.empty()) BEGIN(INITIAL);
    }

    <<EOF>> {
		// If EOF is reached and all comments are not closed => lexical error
        if (!comment_pda.empty()){
            int error_line = std::get<0>(comment_pda.top());
            int error_col = std::get<1>(comment_pda.top());

            yylval.error_location.line_error = error_line;
            yylval.error_location.column_error = error_col;
            error_message = strdup("Unterminated Comment");
            if (!lexer_debug_mode)
                return ERROR;
            reportLexicalError("Unterminated Comment", error_line, error_col);
        }
    }

    {LF} { yyline++; yycolumn = 1; }
    [^*] { yycolumn += yyleng; }
    "*"+[^)] { yycolumn += yyleng; }
}

. { 
    yylval.error_location.line_error = yyline;
    yylval.error_location.column_error = yycolumn;
    error_message = strdup("Unknown character");
    if (!lexer_debug_mode)
        return ERROR;
    reportLexicalError(yytext + std::string("Unknown character "), yyline, yycolumn);
    }
%%

int yywrap() {
    return 1;
}


