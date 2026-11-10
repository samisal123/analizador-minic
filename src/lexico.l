%{
/*
 * Analizador léxico de MiniC (especificación Flex).
 * Convierte el archivo fuente en un buffer de tokens con su posición
 * y registra los identificadores en la tabla de símbolos.
 */
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

static int linea = 1, columna = 1;     /* posición del siguiente carácter */
static int tok_linea, tok_columna;     /* posición del lexema reconocido */
static int com_linea, com_columna;     /* inicio del comentario de bloque abierto */
static int n_errores = 0;
static int sustituido;                /* el lexema tenía error y se entregó como token válido */

static void error_lexico(int lin, int col, const char *descripcion, const char *lexema);

/* Antes de cada acción: guarda la posición del lexema y avanza línea/columna.
   Los bytes de continuación UTF-8 no cuentan como columna. */
#define YY_USER_ACTION                                              \
    tok_linea = linea; tok_columna = columna;                       \
    for (int k = 0; k < yyleng; k++) {                              \
        if (yytext[k] == '\n') { linea++; columna = 1; }            \
        else if ((yytext[k] & 0xC0) != 0x80) columna++;             \
    }
%}

%option noyywrap nounput noinput 8bit
%x COMENTARIO

DIGITO      [0-9]
LETRA       [A-Za-z_]
UTF8        [\xC2-\xF4][\x80-\xBF]+
ID          {LETRA}({LETRA}|{DIGITO})*
NUM         {DIGITO}+("."{DIGITO}+)?
NUM_MAL     {DIGITO}+("."{DIGITO}*)+
ID_MAL      {NUM}{LETRA}({LETRA}|{DIGITO})*
PALABRA     ({LETRA}|{DIGITO}|{UTF8})
ID_UTF8     {LETRA}{PALABRA}*{UTF8}{PALABRA}*|{UTF8}+{LETRA}{PALABRA}*

%%

[ \t\r\n]+              { /* espacios en blanco: se descartan */ }
"//".*                  { /* comentario de línea */ }
"/*"                    { com_linea = tok_linea; com_columna = tok_columna; BEGIN(COMENTARIO); }
<COMENTARIO>"*/"        { BEGIN(INITIAL); }
<COMENTARIO>.|\n        { /* contenido del comentario de bloque */ }
<COMENTARIO><<EOF>>     {
                            error_lexico(com_linea, com_columna, "comentario de bloque sin cerrar", NULL);
                            BEGIN(INITIAL);
                            return FIN;
                        }

"int"                   { return INT; }
"float"                 { return FLOAT; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }

{ID}                    { return ID; }
{NUM}                   { return NUMBER; }

    /* Lexemas mal formados: se reportan y se entregan con la clase que se intentaba
       escribir para que el parser no genere errores en cascada. */
{NUM_MAL}               { error_lexico(tok_linea, tok_columna, "número mal formado", yytext); sustituido = 1; return NUMBER; }
{ID_MAL}                { error_lexico(tok_linea, tok_columna, "identificador mal formado", yytext); sustituido = 1; return ID; }
{ID_UTF8}               { error_lexico(tok_linea, tok_columna, "identificador con caracteres no válidos", yytext); sustituido = 1; return ID; }

"<="                    { return LESS_EQUAL; }
">="                    { return GREATER_EQUAL; }
"=="                    { return EQUAL; }
"!="                    { return NOT_EQUAL; }
"<"                     { return LESS; }
">"                     { return GREATER; }
"="                     { return ASSIGN; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return TIMES; }
"/"                     { return DIVIDE; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
";"                     { return SEMICOLON; }
","                     { return COMMA; }

{UTF8}|.                { error_lexico(tok_linea, tok_columna, "carácter no válido", yytext); }

%%

static const char *nombres[] = {
    "FIN", "INT", "FLOAT", "IF", "ELSE", "WHILE", "ID", "NUMBER",
    "ASSIGN", "PLUS", "MINUS", "TIMES", "DIVIDE",
    "LESS", "GREATER", "LESS_EQUAL", "GREATER_EQUAL", "EQUAL", "NOT_EQUAL",
    "LPAREN", "RPAREN", "LBRACE", "RBRACE", "SEMICOLON", "COMMA"
};

const char *nombre_token(TipoToken t) { return nombres[t]; }

int errores_lexicos(void) { return n_errores; }

static void error_lexico(int lin, int col, const char *descripcion, const char *lexema)
{
    n_errores++;
    if (lexema)
        printf("Error léxico en línea %d, columna %d: %s '%s'.\n", lin, col, descripcion, lexema);
    else
        printf("Error léxico en línea %d, columna %d: %s.\n", lin, col, descripcion);
}

static void *crecer(void *p, int *capacidad, size_t tam)
{
    *capacidad = *capacidad ? *capacidad * 2 : 64;
    p = realloc(p, *capacidad * tam);
    if (!p) { perror("analizador"); exit(2); }
    return p;
}

/* Tabla de símbolos: arreglo con búsqueda lineal, suficiente para programas pequeños. */
typedef struct { char *nombre; int linea; int apariciones; } Simbolo;
static Simbolo *simbolos;
static int n_simbolos, cap_simbolos;

static void registrar_simbolo(const char *nombre, int lin)
{
    for (int k = 0; k < n_simbolos; k++)
        if (strcmp(simbolos[k].nombre, nombre) == 0) { simbolos[k].apariciones++; return; }
    if (n_simbolos == cap_simbolos) simbolos = crecer(simbolos, &cap_simbolos, sizeof *simbolos);
    simbolos[n_simbolos++] = (Simbolo){ strdup(nombre), lin, 1 };
}

void imprimir_tabla_simbolos(void)
{
    printf("\nTabla de símbolos\n");
    printf("IDENTIFICADOR         LÍNEA  APARICIONES\n");
    for (int k = 0; k < n_simbolos; k++)
        printf("%-20s  %5d  %11d\n", simbolos[k].nombre, simbolos[k].linea, simbolos[k].apariciones);
}

/* Lee todo el archivo y deja los tokens en un buffer que después consume el parser. */
int analisis_lexico(FILE *entrada, Token **salida)
{
    Token *v = NULL;
    int n = 0, cap = 0;
    TipoToken t;

    yyin = entrada;
    do {
        sustituido = 0;
        t = yylex();
        if (n == cap) v = crecer(v, &cap, sizeof *v);
        if (t == FIN)
            v[n] = (Token){ FIN, strdup("$"), linea, columna };
        else
            v[n] = (Token){ t, strdup(yytext), tok_linea, tok_columna };
        if (t == ID && !sustituido) registrar_simbolo(yytext, tok_linea);
        n++;
    } while (t != FIN);

    *salida = v;
    return n;
}
