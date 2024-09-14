/*
GFALBLC
A GFA like Basic for Linux coded in C

By Dr. Eric O. Flores email: <eoftoro@gmail.com>
MIT License

Copyright (c) 2024 drericflores

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define token types as an enum
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_DEF,
    TOKEN_PRINT,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_ENDIF,
    TOKEN_WHILE,
    TOKEN_WEND,
    TOKEN_FOR,
    TOKEN_TO,
    TOKEN_STEP,
    TOKEN_NEXT,
    TOKEN_REPEAT,
    TOKEN_UNTIL,
    TOKEN_GOTO,
    TOKEN_GOSUB,
    TOKEN_RETURN,
    TOKEN_ON,
    TOKEN_ERROR,
    TOKEN_RESUME,
    TOKEN_DATA,
    TOKEN_READ,
    TOKEN_RESTORE,
    TOKEN_DIM,
    TOKEN_POKE,
    TOKEN_PEEK,
    TOKEN_OPEN,
    TOKEN_CLOSE,
    TOKEN_INPUT,
    TOKEN_CLS,
    TOKEN_LOCATE,
    TOKEN_PLOT,
    TOKEN_LINE,
    TOKEN_CIRCLE,
    TOKEN_SELECT,
    TOKEN_CASE,
    TOKEN_ENDSELECT,
    TOKEN_FUNCTION,
    TOKEN_PROCEDURE,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_MID,
    TOKEN_LEN,
    TOKEN_INSTR,
    TOKEN_VAL,
    TOKEN_STR,
    TOKEN_CHR,
    TOKEN_ASC,
    TOKEN_ABS,
    TOKEN_SQR,
    TOKEN_SIN,
    TOKEN_COS,
    TOKEN_TAN,
    TOKEN_ATN,
    TOKEN_EXP,
    TOKEN_LOG,
    TOKEN_RND,
    TOKEN_INT,
    TOKEN_FIX,
    TOKEN_SGN,
    TOKEN_ALLOCATE,
    TOKEN_FREE,
    TOKEN_ADDR,
    TOKEN_REM
} TokenType;

// Define a structure for tokens
typedef struct {
    TokenType type;
    char* value;
} Token;

// Lexer structure
typedef struct {
    char* source_code;
    int position;
    char current_char;
} Lexer;

// Function prototypes
Lexer* create_lexer(char* source_code);
void advance(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);
void skip_whitespace(Lexer* lexer);
void skip_comment(Lexer* lexer);
Token number(Lexer* lexer);
Token identifier_or_keyword(Lexer* lexer);
Token string_literal(Lexer* lexer);
Token create_token(TokenType type, char* value);
char* substring(const char* str, size_t begin, size_t len);

// Create a lexer instance
Lexer* create_lexer(char* source_code) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source_code = source_code;
    lexer->position = 0;
    lexer->current_char = source_code[lexer->position];
    return lexer;
}

// Advance the lexer to the next character
void advance(Lexer* lexer) {
    lexer->position++;
    if (lexer->position < strlen(lexer->source_code)) {
        lexer->current_char = lexer->source_code[lexer->position];
    } else {
        lexer->current_char = '\0';  // End of input
    }
}

// Create a token
Token create_token(TokenType type, char* value) {
    Token token;
    token.type = type;
    token.value = value ? strdup(value) : NULL;
    return token;
}

// Get the next token from the lexer
Token lexer_next_token(Lexer* lexer) {
    while (lexer->current_char != '\0') {
        if (isspace(lexer->current_char)) {
            skip_whitespace(lexer);
            continue;
        }
        if (isdigit(lexer->current_char)) {
            return number(lexer);
        }
        if (isalpha(lexer->current_char)) {
            return identifier_or_keyword(lexer);
        }
        switch (lexer->current_char) {
            case '+': advance(lexer); return create_token(TOKEN_PLUS, "+");
            case '-': advance(lexer); return create_token(TOKEN_MINUS, "-");
            case '*': advance(lexer); return create_token(TOKEN_MUL, "*");
            case '/': advance(lexer); return create_token(TOKEN_DIV, "/");
            case '=': advance(lexer); return create_token(TOKEN_ASSIGN, "=");
            case '(': advance(lexer); return create_token(TOKEN_LPAREN, "(");
            case ')': advance(lexer); return create_token(TOKEN_RPAREN, ")");
            case ',': advance(lexer); return create_token(TOKEN_COMMA, ",");
            case '"': return string_literal(lexer);
            case '\'': skip_comment(lexer); continue;
            default: fprintf(stderr, "Unexpected character: %c\n", lexer->current_char); exit(1);
        }
    }
    return create_token(TOKEN_EOF, NULL);
}

// Skip whitespace characters
void skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        advance(lexer);
    }
}

// Skip comments (starting with ')
void skip_comment(Lexer* lexer) {
    while (lexer->current_char != '\0' && lexer->current_char != '\n') {
        advance(lexer);
    }
}

// Parse a number token (integer literal)
Token number(Lexer* lexer) {
    int start_position = lexer->position;
    while (lexer->current_char != '\0' && isdigit(lexer->current_char)) {
        advance(lexer);
    }
    char* value = substring(lexer->source_code, start_position, lexer->position - start_position);
    return create_token(TOKEN_INT_LITERAL, value);
}

// Parse an identifier or a keyword
Token identifier_or_keyword(Lexer* lexer) {
    int start_position = lexer->position;
    while (lexer->current_char != '\0' && (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        advance(lexer);
    }
    char* value = substring(lexer->source_code, start_position, lexer->position - start_position);

    // Map keywords to token types
    if (strcmp(value, "DEF") == 0) return create_token(TOKEN_DEF, value);
    if (strcmp(value, "PRINT") == 0) return create_token(TOKEN_PRINT, value);
    if (strcmp(value, "IF") == 0) return create_token(TOKEN_IF, value);
    // Add other keywords similarly...

    return create_token(TOKEN_IDENTIFIER, value);  // Otherwise, it's an identifier
}

// Parse a string literal token
Token string_literal(Lexer* lexer) {
    advance(lexer);  // Skip the opening quote
    int start_position = lexer->position;
    while (lexer->current_char != '\0' && lexer->current_char != '"') {
        advance(lexer);
    }
    char* value = substring(lexer->source_code, start_position, lexer->position - start_position);
    advance(lexer);  // Skip the closing quote
    return create_token(TOKEN_STRING_LITERAL, value);
}

// Utility function to extract a substring from a string
char* substring(const char* str, size_t begin, size_t len) {
    char* substr = (char*)malloc(len + 1);
    strncpy(substr, str + begin, len);
    substr[len] = '\0';
    return substr;
}

int main() {
    // Example usage of the lexer
    char source_code[] = "PRINT \"Hello, World!\"";
    Lexer* lexer = create_lexer(source_code);

    Token token;
    do {
        token = lexer_next_token(lexer);
        printf("Token(Type: %d, Value: %s)\n", token.type, token.value ? token.value : "None");
        if (token.value) free(token.value);  // Free the token value if allocated
    } while (token.type != TOKEN_EOF);

    free(lexer);  // Free lexer after use
    return 0;
}

