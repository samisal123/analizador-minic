#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>

/* Clases léxicas de MiniC. FIN representa el símbolo $ (fin de entrada). */
typedef enum {
    FIN = 0,
    INT, FLOAT, IF, ELSE, WHILE,
    ID, NUMBER,
    ASSIGN, PLUS, MINUS, TIMES, DIVIDE,
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, EQUAL, NOT_EQUAL,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA
} TipoToken;

typedef struct {
    TipoToken tipo;
    char *lexema;
    int linea;
    int columna;
} Token;

/* lexico.l */
int analisis_lexico(FILE *entrada, Token **tokens);  /* devuelve el número de tokens, incluido FIN */
int errores_lexicos(void);
const char *nombre_token(TipoToken t);
void imprimir_tabla_simbolos(void);

/* sintactico.c */
int analisis_sintactico(Token *tokens);              /* devuelve el número de errores sintácticos */

#endif
