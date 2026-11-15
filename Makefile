CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Isrc
BUILD   = build

all: analizador

analizador: $(BUILD)/lex.yy.c src/sintactico.c src/main.c src/tokens.h
	$(CC) $(CFLAGS) -o $@ $(BUILD)/lex.yy.c src/sintactico.c src/main.c

$(BUILD)/lex.yy.c: src/lexico.l src/tokens.h
	mkdir -p $(BUILD)
	flex -o $@ src/lexico.l

pruebas: analizador
	sh pruebas/ejecutar_pruebas.sh

docs:
	cd docs && latexmk -pdf -interaction=nonstopmode documento_tecnico.tex

clean:
	rm -rf $(BUILD) analizador

.PHONY: all pruebas docs clean
