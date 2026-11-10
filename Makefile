CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Isrc
BUILD   = build

analizador: $(BUILD)/lex.yy.c src/main.c src/tokens.h
	$(CC) $(CFLAGS) -o $@ $(BUILD)/lex.yy.c src/main.c

$(BUILD)/lex.yy.c: src/lexico.l src/tokens.h
	mkdir -p $(BUILD)
	flex -o $@ src/lexico.l

pruebas: analizador
	sh pruebas/ejecutar_pruebas.sh

clean:
	rm -rf $(BUILD) analizador

.PHONY: pruebas clean
