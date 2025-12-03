// codegen_fis25.c
#include <stdio.h>
#include <string.h>

#include "codegen_fis25.h"
#include "ast.h"

/*
 * Generador de código FIS-25 para la Opción C: Letrero Dinámico (Marquee).
 *
 * Por ahora ignoramos el AST (root) y generamos un programa FIS-25 fijo
 * que cumple:
 *   - El mensaje se lee por stdin (ya lo hace main.c) y aquí sólo lo
 *     usamos para comentarios y para calcular el ancho total.
 *   - El texto se desplaza MANUALMENTE: cada evento de teclado (flanco)
 *     mueve el letrero exactamente 1 columna a la izquierda o derecha.
 *   - Restricción de caracteres (0–9, '.', '$') ya la aplica main.c.
 */
void codegen_fis25(ASTStmt *root, const char *msg)
{
    (void)root; /* AST aún no se usa */

    if (!msg) {
        msg = "0";
    }

    /* Cada carácter ocupa 4 columnas (bitmap 3x5 + 1 espacio) */
    const int CHAR_WIDTH = 4;
    size_t len = strlen(msg);
    if (len == 0) {
        len = 1;
    }
    int total_width = (int)len * CHAR_WIDTH;

    /* Límites de desplazamiento: dejamos que salga un poco por la izquierda */
    int min_x = -total_width;
    int max_x = 63;  /* pantalla 64x64: última columna visible */

    /* ---------------- CABECERA / COMENTARIOS ---------------- */
    printf("// Opción C: Letrero Dinámico (Marquee)\n");
    printf("// Mensaje: \"%s\"\n", msg);
    printf("// Cada carácter ocupa %d columnas -> ancho total = %d\n",
           CHAR_WIDTH, total_width);
    printf("// Controles: A = izquierda (KEY 6), D = derecha (KEY 7)\n");
    printf("\n");

    /* ---------------- DECLARACIÓN DE VARIABLES ---------------- */
    printf("VAR x\n");
    printf("VAR y\n");
    printf("VAR color\n");

    printf("VAR left_now\n");
    printf("VAR right_now\n");
    printf("VAR left_prev\n");
    printf("VAR right_prev\n");

    printf("VAR ONE\n");
    printf("VAR MIN_X\n");
    printf("VAR MAX_X\n");
    printf("VAR TMP\n");
    printf("VAR COND\n");
    printf("\n");

    /* ---------------- INICIALIZACIÓN ---------------- */
    printf("// Posición inicial del texto\n");
    printf("ASSIGN 0 x\n");        // inicio en columna 0
    printf("ASSIGN 30 y\n");       // fila central aproximada
    printf("ASSIGN 3 color\n");    // color arbitrario válido

    printf("ASSIGN 0 left_now\n");
    printf("ASSIGN 0 right_now\n");
    printf("ASSIGN 0 left_prev\n");
    printf("ASSIGN 0 right_prev\n");

    printf("ASSIGN 1 ONE\n");
    printf("ASSIGN %d MIN_X\n", min_x);
    printf("ASSIGN %d MAX_X\n", max_x);
    printf("\n");

    /* ---------------- BUCLE PRINCIPAL ---------------- */
    printf("LABEL MAIN_LOOP\n");
    printf("    // Aquí debería dibujarse el texto completo a partir de x,y\n");
    printf("    // Por ahora, como placeholder, dibujamos un solo píxel.\n");
    printf("    PIXEL x y color\n");
    printf("\n");

    /* Leer teclas: A y D mapeadas a KEY 6 y KEY 7 según el manual */
    printf("    // Leer estado actual de teclas (A y D)\n");
    printf("    KEY 6 left_now    // A -> izquierda\n");
    printf("    KEY 7 right_now   // D -> derecha\n");
    printf("\n");

    /* ----------- DETECCIÓN DE FLANCO IZQUIERDA ----------- */
    printf("    // Flanco: left_now == 1 && left_prev == 0 ?\n");
    printf("    EQ left_now 1 TMP\n");
    printf("    IF TMP GOTO LEFT_MAYBE\n");
    printf("    GOTO CHECK_RIGHT\n");
    printf("\n");

    printf("LABEL LEFT_MAYBE\n");
    printf("    EQ left_prev 0 TMP\n");
    printf("    IF TMP GOTO MOVE_LEFT\n");
    printf("    GOTO CHECK_RIGHT\n");
    printf("\n");

    /* ----------- DETECCIÓN DE FLANCO DERECHA ----------- */
    printf("LABEL CHECK_RIGHT\n");
    printf("    // Flanco: right_now == 1 && right_prev == 0 ?\n");
    printf("    EQ right_now 1 TMP\n");
    printf("    IF TMP GOTO RIGHT_MAYBE\n");
    printf("    GOTO END_KEYS\n");
    printf("\n");

    printf("LABEL RIGHT_MAYBE\n");
    printf("    EQ right_prev 0 TMP\n");
    printf("    IF TMP GOTO MOVE_RIGHT\n");
    printf("    GOTO END_KEYS\n");
    printf("\n");

    /* ----------- MOVIMIENTO IZQUIERDA + CLAMP ----------- */
    printf("LABEL MOVE_LEFT\n");
    printf("    SUB x ONE x          // x = x - 1\n");
    printf("    // Clamp: si x < MIN_X => x = MIN_X\n");
    printf("    LT x MIN_X COND\n");
    printf("    IF COND GOTO CLAMP_LEFT\n");
    printf("    GOTO END_KEYS\n");
    printf("\n");

    printf("LABEL CLAMP_LEFT\n");
    printf("    ASSIGN MIN_X x\n");
    printf("    GOTO END_KEYS\n");
    printf("\n");

    /* ----------- MOVIMIENTO DERECHA + CLAMP ----------- */
    printf("LABEL MOVE_RIGHT\n");
    printf("    ADD x ONE x          // x = x + 1\n");
    printf("    // Clamp: si x > MAX_X => x = MAX_X\n");
    printf("    GT x MAX_X COND\n");
    printf("    IF COND GOTO CLAMP_RIGHT\n");
    printf("    GOTO END_KEYS\n");
    printf("\n");

    printf("LABEL CLAMP_RIGHT\n");
    printf("    ASSIGN MAX_X x\n");
    printf("    GOTO END_KEYS\n");
    printf("\n");

    /* ----------- ACTUALIZAR ESTADO PREVIO Y REPETIR ----------- */
    printf("LABEL END_KEYS\n");
    printf("    ASSIGN left_now left_prev\n");
    printf("    ASSIGN right_now right_prev\n");
    printf("    GOTO MAIN_LOOP\n");
    printf("\n");
}
