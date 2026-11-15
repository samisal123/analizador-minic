# Analizador léxico y sintáctico para MiniC

Proyecto de Compiladores, Facultad de Ingeniería, UNAM (grupo 02, semestre 2027-1).

Lee un programa escrito en MiniC (un subconjunto de C), lo divide en tokens con un analizador léxico hecho en Flex y verifica su estructura con un analizador sintáctico descendente recursivo LL(1). Reporta cada error con línea y columna, y sigue analizando para encontrar los demás.

## Requisitos

gcc, flex y make. En Linux o WSL (Windows):

```sh
sudo apt install build-essential flex
```

## Uso

```sh
make                                              # compila
./analizador pruebas/validos/01_ejemplo_enunciado.txt   # analiza un archivo
make pruebas                                      # corre todas las pruebas
```

Para cada archivo, el programa muestra:

- los errores léxicos, si hay;
- la tabla de tokens (línea, columna, token y lexema) y la secuencia de tokens por línea, en el formato del enunciado (`INT ID SEMICOLON`);
- la tabla de símbolos;
- los errores sintácticos, si hay;
- si el programa es válido o no.

Ejemplo de error, con el caso de la asignación:

```text
Error sintáctico en línea 1, columna 12: Se esperaba ')' antes de ';'.
```

## Pruebas

`make pruebas` corre los 6 programas válidos y los 8 inválidos, indica si cada uno se aceptó o se rechazó como debía y guarda la salida completa de cada caso en `pruebas/resultados/`.

```text
OK     01_ejemplo_enunciado                         aceptado
...
OK     04_sintactico_parentesis                     rechazado (0 léxicos, 3 sintácticos)
...
Todas las pruebas pasaron. Salidas completas en pruebas/resultados/
```

Los inválidos cubren caracteres fuera del alfabeto, números e identificadores mal formados, comentarios sin cerrar, paréntesis y punto y coma faltantes, errores en `if` y `while`, declaraciones incompletas y una mezcla de errores léxicos y sintácticos.

## Estructura

```text
src/lexico.l       analizador léxico (Flex)
src/sintactico.c   analizador sintáctico descendente recursivo
src/main.c         lectura del archivo y reporte de resultados
src/tokens.h       tokens compartidos por ambos analizadores
pruebas/           casos válidos, inválidos y sus resultados
docs/              documento técnico (PDF)
```

El lenguaje, la gramática, los conjuntos FIRST y FOLLOW, la tabla LL(1) y el manejo de errores están en [`docs/documento_tecnico.pdf`](docs/documento_tecnico.pdf).

## Integrantes

- Andrés Basilio Illescas
- Samuel Santiago Cruz Macedo
- Jesús Jafet Fernández Hernandez
