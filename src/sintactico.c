/*
 * Analizador sintáctico descendente recursivo de MiniC.
 * Cada no terminal de la gramática LL(1) es una función y la producción
 * se elige con un solo token de anticipación (lookahead).
 * Gramática, conjuntos FIRST/FOLLOW y tabla LL(1): docs/documento_tecnico.pdf.
 */
#include <stdio.h>
#include "tokens.h"

static Token *tokens;      /* buffer generado por el analizador léxico */
static int i;              /* posición del lookahead en el buffer */
static Token *lookahead;
static int errores;
static int panico;         /* 1 desde que se detecta un error hasta sincronizar */

/* Texto de cada terminal para los mensajes de error. */
static const char *esperado[] = {
    [FIN] = "fin de archivo", [INT] = "'int'", [FLOAT] = "'float'", [IF] = "'if'",
    [ELSE] = "'else'", [WHILE] = "'while'", [ID] = "identificador", [NUMBER] = "número",
    [ASSIGN] = "'='", [PLUS] = "'+'", [MINUS] = "'-'", [TIMES] = "'*'", [DIVIDE] = "'/'",
    [LESS] = "'<'", [GREATER] = "'>'", [LESS_EQUAL] = "'<='", [GREATER_EQUAL] = "'>='",
    [EQUAL] = "'=='", [NOT_EQUAL] = "'!='", [LPAREN] = "'('", [RPAREN] = "')'",
    [LBRACE] = "'{'", [RBRACE] = "'}'", [SEMICOLON] = "';'", [COMMA] = "','"
};

static void programa(void);
static void lista_sentencias(void);
static void sentencia(void);
static void declaracion(void);
static void tipo(void);
static void lista_id(void);
static void inicializacion(void);
static void asignacion(void);
static void sent_if(void);
static void parte_else(void);
static void resto_else(void);
static void sent_while(void);
static void bloque(void);
static void condicion(void);
static void op_rel(void);
static void E(void);
static void E_Prime(void);
static void T(void);
static void T_Prime(void);
static void F(void);

static void avanzar(void)
{
    if (lookahead->tipo != FIN) lookahead = &tokens[++i];
}

/* Reporta el error en la posición del lookahead y activa el modo pánico.
   Mientras dura el pánico no se reportan más errores, para evitar cascadas.
   Las funciones recursivas regresan de inmediato en pánico para no ciclar sin consumir. */
static void error(const char *que)
{
    if (panico) return;
    panico = 1;
    errores++;
    printf("Error sintáctico en línea %d, columna %d: Se esperaba %s antes de ",
           lookahead->linea, lookahead->columna, que);
    if (lookahead->tipo == FIN) printf("fin de archivo.\n");
    else printf("'%s'.\n", lookahead->lexema);
}

static void match(TipoToken t)
{
    if (panico) return;
    if (lookahead->tipo == t) avanzar();
    else error(esperado[t]);
}

/*
 * Modo pánico: descarta tokens hasta el conjunto de sincronización { ';', '}', $ }.
 * Si en el camino se abre un bloque, se descarta completo, de modo que un error en el
 * encabezado de un if/while no deja llaves sueltas. Si la sentencia era un if, también
 * se descarta su parte else.
 */
static void sincronizar(int era_if)
{
    int nivel = 0;
    while (lookahead->tipo != FIN) {
        TipoToken t = lookahead->tipo;
        if (t == RBRACE && nivel == 0) break;      /* lo consume el bloque que contiene al error */
        avanzar();
        if (t == LBRACE) nivel++;
        else if (t == RBRACE && --nivel == 0 && !(era_if && lookahead->tipo == ELSE))
            break;
        else if (t == SEMICOLON && nivel == 0)
            break;
    }
    panico = 0;
}

/* programa → lista_sent $ */
static void programa(void)
{
    for (;;) {
        lista_sentencias();
        if (lookahead->tipo == FIN) return;
        error("una sentencia o fin de archivo");   /* '}' sin '{' que lo abra */
        avanzar();
        panico = 0;
    }
}

/* lista_sent → sentencia lista_sent | ε        FOLLOW(lista_sent) = { '}', $ } */
static void lista_sentencias(void)
{
    if (panico || lookahead->tipo == RBRACE || lookahead->tipo == FIN) return;
    int era_if = lookahead->tipo == IF;
    sentencia();
    if (panico) sincronizar(era_if);
    lista_sentencias();
}

/* sentencia → declaracion | asignacion | sent_if | sent_while */
static void sentencia(void)
{
    switch (lookahead->tipo) {
    case INT: case FLOAT: declaracion(); break;
    case ID:              asignacion(); break;
    case IF:              sent_if(); break;
    case WHILE:           sent_while(); break;
    default:              error("una sentencia (declaración, asignación, if o while)");
    }
}

/* declaracion → tipo id inicializacion lista_id ; */
static void declaracion(void)
{
    tipo();
    match(ID);
    inicializacion();
    lista_id();
    match(SEMICOLON);
}

/* tipo → int | float */
static void tipo(void)
{
    if (lookahead->tipo == INT) match(INT);
    else match(FLOAT);
}

/* lista_id → , id inicializacion lista_id | ε */
static void lista_id(void)
{
    if (panico) return;
    if (lookahead->tipo == COMMA) {
        match(COMMA);
        match(ID);
        inicializacion();
        lista_id();
    }
    /* ε-producción: el ';' lo verifica declaracion() */
}

/* inicializacion → = E | ε */
static void inicializacion(void)
{
    if (panico) return;
    if (lookahead->tipo == ASSIGN) {
        match(ASSIGN);
        E();
    }
}

/* asignacion → id = E ; */
static void asignacion(void)
{
    match(ID);
    match(ASSIGN);
    E();
    match(SEMICOLON);
}

/* sent_if → if ( condicion ) bloque parte_else */
static void sent_if(void)
{
    match(IF);
    match(LPAREN);
    condicion();
    match(RPAREN);
    bloque();
    parte_else();
}

/* parte_else → else resto_else | ε */
static void parte_else(void)
{
    if (panico) return;
    if (lookahead->tipo == ELSE) {
        match(ELSE);
        resto_else();
    }
}

/* resto_else → bloque | sent_if */
static void resto_else(void)
{
    if (lookahead->tipo == IF) sent_if();
    else bloque();
}

/* sent_while → while ( condicion ) bloque */
static void sent_while(void)
{
    match(WHILE);
    match(LPAREN);
    condicion();
    match(RPAREN);
    bloque();
}

/* bloque → { lista_sent } */
static void bloque(void)
{
    match(LBRACE);
    lista_sentencias();
    match(RBRACE);
}

/* condicion → E op_rel E */
static void condicion(void)
{
    E();
    op_rel();
    E();
}

/* op_rel → < | > | <= | >= | == | != */
static void op_rel(void)
{
    switch (lookahead->tipo) {
    case LESS: case GREATER: case LESS_EQUAL: case GREATER_EQUAL: case EQUAL: case NOT_EQUAL:
        match(lookahead->tipo);
        break;
    default:
        error("un operador relacional");
    }
}

/* E → T E' */
static void E(void)
{
    T();
    E_Prime();
}

/* E' → + T E' | - T E' | ε */
static void E_Prime(void)
{
    if (panico) return;
    if (lookahead->tipo == PLUS || lookahead->tipo == MINUS) {
        match(lookahead->tipo);
        T();
        E_Prime();
    }
    /* ε-producción: no hacer nada */
}

/* T → F T' */
static void T(void)
{
    F();
    T_Prime();
}

/* T' → * F T' | / F T' | ε */
static void T_Prime(void)
{
    if (panico) return;
    if (lookahead->tipo == TIMES || lookahead->tipo == DIVIDE) {
        match(lookahead->tipo);
        F();
        T_Prime();
    }
    /* ε-producción: no hacer nada */
}

/* F → ( E ) | id | num | - F */
static void F(void)
{
    if (panico) return;
    switch (lookahead->tipo) {
    case LPAREN: match(LPAREN); E(); match(RPAREN); break;
    case ID:     match(ID); break;
    case NUMBER: match(NUMBER); break;
    case MINUS:  match(MINUS); F(); break;
    default:     error("identificador, número, '(' o '-'");
    }
}

int analisis_sintactico(Token *buffer)
{
    tokens = buffer;
    i = 0;
    lookahead = &tokens[0];
    errores = panico = 0;
    programa();
    return errores;
}
