#include <stdio.h>
#include "tokens.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <programa_fuente>\n", argv[0]);
        return 2;
    }
    FILE *fuente = fopen(argv[1], "r");
    if (!fuente) {
        perror(argv[1]);
        return 2;
    }

    printf("Archivo: %s\n\n== Análisis léxico ==\n", argv[1]);
    Token *tokens;
    int n = analisis_lexico(fuente, &tokens);
    fclose(fuente);

    printf("\nTokens reconocidos\n");
    printf("LÍNEA  COL  TOKEN           LEXEMA\n");
    for (int i = 0; i < n; i++)
        printf("%5d  %3d  %-14s  %s\n", tokens[i].linea, tokens[i].columna,
               nombre_token(tokens[i].tipo), tokens[i].lexema);
    imprimir_tabla_simbolos();

    return errores_lexicos() ? 1 : 0;
}
