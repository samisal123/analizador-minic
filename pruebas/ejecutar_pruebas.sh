#!/bin/sh
# Ejecuta el analizador sobre todos los casos de prueba.
# Los programas de pruebas/validos deben aceptarse (código de salida 0)
# y los de pruebas/invalidos deben rechazarse (código de salida 1).

cd "$(dirname "$0")/.." || exit 2
[ -x ./analizador ] || { echo "Primero compila con: make"; exit 2; }

fallas=0

echo "== Programas válidos =="
for f in pruebas/validos/*.txt; do
    if ./analizador "$f" > /dev/null; then
        echo "  ACEPTADO   $f"
    else
        echo "  FALLA      $f (debía aceptarse)"
        fallas=$((fallas + 1))
    fi
done

echo
echo "== Programas inválidos =="
for f in pruebas/invalidos/*.txt; do
    salida=$(./analizador "$f")
    if [ $? -eq 1 ]; then
        echo "  RECHAZADO  $f"
    else
        echo "  FALLA      $f (debía rechazarse)"
        fallas=$((fallas + 1))
    fi
    echo "$salida" | grep '^Error' | sed 's/^/             /'
done

echo
if [ $fallas -eq 0 ]; then
    echo "Todas las pruebas pasaron."
else
    echo "Pruebas fallidas: $fallas"
fi
exit $fallas
