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

    /* Misma secuencia agrupada por línea, en el formato del enunciado. FIN no se muestra. */
    printf("\nSecuencia de tokens por línea\n");
    for (int i = 0; i < n - 1; i++) {
        if (i == 0 || tokens[i].linea != tokens[i - 1].linea)
            printf("%s%5d  ", i ? "\n" : "", tokens[i].linea);
        else
            printf(" ");
        printf("%s", nombre_token(tokens[i].tipo));
    }
    if (n > 1) printf("\n");

    imprimir_tabla_simbolos();

    printf("\n== Análisis sintáctico ==\n");
    int err_sintacticos = analisis_sintactico(tokens);
    int err_lexicos = errores_lexicos();
    if (err_sintacticos == 0) printf("Sin errores sintácticos.\n");

    printf("\n== Resultado ==\n");
    if (err_lexicos == 0 && err_sintacticos == 0) {
        printf("El programa es léxica y sintácticamente válido.\n");
        return 0;
    }
    printf("El programa NO es válido (errores léxicos: %d, errores sintácticos: %d).\n",
           err_lexicos, err_sintacticos);
    return 1;
}
