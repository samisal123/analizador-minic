# Analizador léxico y sintáctico para MiniC

Analizador de programas escritos en MiniC, un lenguaje simplificado con sintaxis semejante a C. El analizador léxico, generado con Flex, convierte el archivo fuente en una secuencia de tokens. El analizador sintáctico, descendente recursivo LL(1), consume esos tokens y verifica que formen instrucciones válidas según la gramática del lenguaje. Los dos reportan los errores con línea y columna, y el análisis continúa después de cada error para mostrarlos todos en una sola ejecución.

Proyecto de la asignatura Compiladores, Facultad de Ingeniería, UNAM. Grupo 02, semestre 2027-1.

## Requisitos

| Herramienta | Versión probada | Uso |
|---|---|---|
| gcc | 13.3 | Compilador de C (cualquier compilador C99 funciona) |
| Flex | 2.6.4 | Genera el analizador léxico a partir de `src/lexico.l` |
| make | 4.3 | Automatiza la compilación y las pruebas (opcional) |

El proyecto no tiene otras dependencias. Instalación de las herramientas:

```sh
# Debian / Ubuntu / WSL
sudo apt install build-essential flex

# Fedora
sudo dnf install gcc make flex

# macOS (Flex viene con las Command Line Tools)
xcode-select --install
```

En Windows se recomienda WSL. También funciona con MSYS2 (`pacman -S gcc make flex`).

## Compilación

```sh
make
```

El resultado es el ejecutable `analizador` en la raíz del proyecto. Sin `make`, los pasos equivalentes son:

```sh
mkdir -p build
flex -o build/lex.yy.c src/lexico.l
gcc -Wall -Wextra -O2 -Isrc -o analizador build/lex.yy.c src/sintactico.c src/main.c
```

`make clean` borra los archivos generados.

## Uso

```sh
./analizador <archivo_fuente>
```

La salida tiene tres secciones:

1. Análisis léxico: errores léxicos encontrados, tokens reconocidos (línea, columna, clase y lexema) y tabla de símbolos.
2. Análisis sintáctico: errores sintácticos, o `Sin errores sintácticos.`
3. Resultado: indica si el programa es léxica y sintácticamente válido.

| Código de salida | Significado |
|---|---|
| 0 | Programa válido |
| 1 | Hay errores léxicos o sintácticos |
| 2 | Error de uso o no se pudo abrir el archivo |

Ejemplo con un programa válido (salida recortada):

```text
$ ./analizador pruebas/validos/01_ejemplo_enunciado.txt
Archivo: pruebas/validos/01_ejemplo_enunciado.txt

== Análisis léxico ==

Tokens reconocidos
LÍNEA  COL  TOKEN           LEXEMA
    1    1  INT             int
    1    5  ID              x
    1    6  SEMICOLON       ;
...
   10    1  RBRACE          }
   11    1  FIN             $

Tabla de símbolos
IDENTIFICADOR         LÍNEA  APARICIONES
x                         1            7
y                         2            4

== Análisis sintáctico ==
Sin errores sintácticos.

== Resultado ==
El programa es léxica y sintácticamente válido.
```

Ejemplo con errores:

```text
$ ./analizador pruebas/invalidos/08_mixto_lexico_y_sintactico.txt
...
== Análisis léxico ==
Error léxico en línea 4, columna 8: carácter no válido '@'.
Error léxico en línea 8, columna 5: número mal formado '3.1.4'.
...
== Análisis sintáctico ==
Error sintáctico en línea 5, columna 11: Se esperaba ')' antes de '{'.
Error sintáctico en línea 8, columna 13: Se esperaba identificador, número, '(' o '-' antes de ';'.
Error sintáctico en línea 9, columna 1: Se esperaba una sentencia o fin de archivo antes de '}'.

== Resultado ==
El programa NO es válido (errores léxicos: 2, errores sintácticos: 3).
```

## Pruebas

```sh
make pruebas
```

El script `pruebas/ejecutar_pruebas.sh` corre el analizador sobre cada archivo de `pruebas/validos` (deben aceptarse) y de `pruebas/invalidos` (deben rechazarse) y muestra los errores reportados en cada caso inválido. Termina con código 0 solo si todos los casos se comportan como se espera.

| Archivo | Qué prueba |
|---|---|
| `validos/01_ejemplo_enunciado.txt` | Programa de ejemplo de la asignación |
| `validos/02_declaraciones_y_comentarios.txt` | Declaraciones simples y múltiples, comentarios de línea y de bloque |
| `validos/03_expresiones_aritmeticas.txt` | Precedencia, asociatividad, paréntesis anidados, menos unario, decimales |
| `validos/04_condicionales.txt` | `if`, `if-else`, cadena `else if`, `if` anidados, bloque vacío |
| `validos/05_ciclos_anidados.txt` | `while` anidados con condicionales en el cuerpo |
| `validos/06_programa_completo.txt` | MCD por restas sucesivas y factorial |
| `invalidos/01_lexico_caracteres_invalidos.txt` | Caracteres fuera del alfabeto (`@`, `#`, `$`, `ñ`) |
| `invalidos/02_lexico_literales_mal_formados.txt` | `3.14.15`, `2.`, `2radio` |
| `invalidos/03_lexico_comentario_sin_cerrar.txt` | Comentario de bloque sin `*/` |
| `invalidos/04_sintactico_parentesis.txt` | Paréntesis sin cerrar, de más y vacíos |
| `invalidos/05_sintactico_punto_y_coma.txt` | Falta de `;` en declaración y asignación, operando faltante |
| `invalidos/06_sintactico_estructuras_control.txt` | `if` sin paréntesis, condición incompleta, `else` sin `if`, bloque sin cerrar |
| `invalidos/07_sintactico_declaraciones.txt` | Declaraciones sin identificador, con número o sin coma |
| `invalidos/08_mixto_lexico_y_sintactico.txt` | Errores léxicos y sintácticos en el mismo archivo |

## Estructura

```text
src/tokens.h        Clases léxicas y estructura Token compartidas por ambos analizadores
src/lexico.l        Especificación Flex: expresiones regulares, comentarios, tabla de símbolos y errores léxicos
src/sintactico.c    Analizador descendente recursivo: una función por no terminal y recuperación en modo pánico
src/main.c          Lectura del archivo, ejecución de ambas fases y reporte de resultados
pruebas/            Programas válidos, inválidos y script de ejecución
docs/               Documento técnico (PDF y fuente LaTeX)
```

## Lenguaje MiniC

| Elemento | Definición |
|---|---|
| Palabras reservadas | `int` `float` `if` `else` `while` |
| Identificadores | Letra o `_`, seguida de letras, dígitos o `_`. Distingue mayúsculas de minúsculas |
| Constantes numéricas | Enteras (`42`) y decimales con parte fraccionaria (`3.14`) |
| Operadores | `=` `+` `-` `*` `/` `<` `>` `<=` `>=` `==` `!=` |
| Delimitadores | `;` `,` `(` `)` `{` `}` |
| Comentarios | De línea (`// ...`) y de bloque (`/* ... */`) |

Construcciones: declaraciones (`int a, b;`), asignaciones con expresiones aritméticas, `if` con `else` y `else if` opcionales, y `while`. Los cuerpos de `if`, `else` y `while` siempre van entre llaves; con eso la gramática no tiene la ambigüedad del `else` colgante y es LL(1).

La gramática completa en BNF, los conjuntos FIRST y FOLLOW, la tabla LL(1) y la descripción del manejo de errores están en [`docs/documento_tecnico.pdf`](docs/documento_tecnico.pdf).

## Integrantes

- Andrés Basilio Illescas
- Samuel Santiago Cruz Macedo
- Jesús Jafet Fernández Hernandez
