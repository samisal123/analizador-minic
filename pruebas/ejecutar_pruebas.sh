#!/bin/sh
# Corre el analizador sobre todos los casos de prueba y guarda la salida de cada uno
# en pruebas/resultados/. Los válidos deben aceptarse y los inválidos rechazarse.

cd "$(dirname "$0")/.." || exit 2
[ -x ./analizador ] || make -s || exit 2

mkdir -p pruebas/resultados
rm -f pruebas/resultados/*.txt
fallas=0

revisar() {   # $1 = archivo, $2 = código esperado (0 válido, 1 inválido)
    nombre=$(basename "$1" .txt)
    salida="pruebas/resultados/$(basename "$(dirname "$1")")_$nombre.txt"
    ./analizador "$1" > "$salida"
    codigo=$?
    if [ $codigo -eq "$2" ]; then estado="OK   "; else estado="FALLA"; fallas=$((fallas + 1)); fi
    if [ $codigo -eq 0 ]; then
        detalle="aceptado"
    else
        detalle="rechazado ($(grep -c '^Error léxico' "$salida") léxicos, $(grep -c '^Error sintáctico' "$salida") sintácticos)"
    fi
    printf '%s  %-44s %s\n' "$estado" "$nombre" "$detalle"
}

echo "Programas válidos (deben aceptarse)"
for f in pruebas/validos/*.txt; do revisar "$f" 0; done
echo
echo "Programas inválidos (deben rechazarse)"
for f in pruebas/invalidos/*.txt; do revisar "$f" 1; done
echo
if [ $fallas -eq 0 ]; then
    echo "Todas las pruebas pasaron. Salidas completas en pruebas/resultados/"
else
    echo "Pruebas fallidas: $fallas"
fi
exit $fallas
