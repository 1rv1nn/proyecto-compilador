// codegen_fis25.c
#include <stdio.h>
#include <string.h>

#include "codegen_fis25.h"
#include "ast.h"

/*
 * Generador de código FIS-25 para la Opción C: Letrero Dinámico (Marquee).
 *
 * - El mensaje ya viene filtrado desde main.c (solo 0–9, '.', '$').
 * - Se dibuja una “barra” de píxeles: un píxel por carácter,
 *   separado por 1 columna, para visualizar el desplazamiento.
 * - Control manual: el texto SOLO se mueve cuando el usuario
 *   presiona A o D (un paso por pulsación).
 *
 * Mapeo de teclas en el simulador:
 *   0=Up, 1=Down, 2=Left, 3=Right, 4=W, 5=S, 6=A, 7=D
 * Aquí usamos:
 *   KEY 6 -> A  (mover a la izquierda)
 *   KEY 7 -> D  (mover a la derecha)
 */

void codegen_fis25(ASTStmt *root, const char *msg)
{
    (void)root;  /* por ahora no usamos el AST */

    if (!msg || msg[0] == '\0') {
        msg = "0";
    }

    /* Un píxel por carácter, separado por 1 columna */
    const int CHAR_SPACING = 2;   /* “ancho lógico” por carácter (1 pixel + 1 espacio) */
    size_t len = strlen(msg);
    int total_width = (int)len * CHAR_SPACING;

    int min_x = -total_width;     /* que pueda salir por completo a la izquierda */
    int max_x = 63;               /* pantalla 0..63 */

    /* ================= CABECERA ================= */
    printf("// Opción C: Letrero Dinámico (Marquee)\n");
    printf("// Mensaje: \"%s\"\n", msg);
    printf("// Cada carácter ocupa 1 píxel + 1 espacio -> ancho total = %d\n",
           total_width);
    printf("// Controles: A = izquierda (KEY 6), D = derecha (KEY 7)\n\n");

    /* ================= VARIABLES ================= */
    printf("VAR x            // offset horizontal del letrero\n");
    printf("VAR y            // fila en la pantalla\n");
    printf("VAR color        // color del letrero (1 = encendido)\n");

    printf("VAR left_now     // estado actual tecla A (KEY 6)\n");
    printf("VAR right_now    // estado actual tecla D (KEY 7)\n");
    printf("VAR left_prev    // estado previo tecla A\n");
    printf("VAR right_prev   // estado previo tecla D\n");

    printf("VAR ONE          // constante 1\n");
    printf("VAR STEP         // paso en columnas por movimiento\n");
    printf("VAR MIN_X        // límite izquierdo\n");
    printf("VAR MAX_X        // límite derecho\n");
    printf("VAR TMP          // temporal\n");
    printf("VAR COND         // resultado de comparaciones\n");
    printf("VAR CLR_X        // para limpiar la fila\n");
    printf("VAR IDX          // índice del carácter\n");
    printf("VAR PIX_X        // columna actual de pixel a dibujar\n");
    printf("VAR TEXT_LEN     // longitud del mensaje\n\n");

    /* ================= INICIALIZACIÓN ================= */
    printf("// Posición inicial del letrero\n");
    printf("ASSIGN 20 x\n");
    printf("ASSIGN 30 y\n");
    printf("ASSIGN 1 color\n");

    printf("// Estados iniciales del teclado\n");
    printf("ASSIGN 0 left_now\n");
    printf("ASSIGN 0 right_now\n");
    printf("ASSIGN 0 left_prev\n");
    printf("ASSIGN 0 right_prev\n");

    printf("ASSIGN 1 ONE\n");
    printf("ASSIGN 1 STEP\n");          /* 1 columna por pulsación (cambio pequeño y visible) */
    printf("ASSIGN %d MIN_X\n", min_x);
    printf("ASSIGN %d MAX_X\n", max_x);
    printf("ASSIGN %zu TEXT_LEN\n", len);
    printf("\n");

    /* ================= BUCLE PRINCIPAL ================= */
    printf("LABEL MAIN_LOOP\n");

    /* ---- Limpiar la fila antes de dibujar ---- */
    printf("    ASSIGN 0 CLR_X\n");
    printf("LABEL CLEAR_ROW\n");
    printf("    PIXEL CLR_X y 0\n");        /* apaga (CLR_X, y)             */
    printf("    ADD CLR_X ONE CLR_X\n");
    printf("    LT CLR_X 64 COND\n");
    printf("    IF COND GOTO CLEAR_ROW\n\n");

    /* ---- Dibujar el mensaje como una tira de píxeles ---- */
    printf("    ASSIGN 0 IDX\n");
    printf("LABEL DRAW_LOOP\n");
    printf("    LT IDX TEXT_LEN COND\n");
    printf("    IF COND GOTO DRAW_STEP\n");
    printf("    GOTO AFTER_DRAW\n\n");

    printf("LABEL DRAW_STEP\n");
    printf("    MUL IDX %d TMP\n", CHAR_SPACING); /* TMP = IDX * CHAR_SPACING      */
    printf("    ADD x TMP PIX_X\n");              /* PIX_X = x + IDX*CHAR_SPACING  */
    printf("    PIXEL PIX_X y color\n");          /* enciende píxel                */
    printf("    ADD IDX ONE IDX\n");
    printf("    GOTO DRAW_LOOP\n\n");

    printf("LABEL AFTER_DRAW\n\n");

    /* ---- Leer teclas A (6) y D (7) ---- */
    printf("    KEY 6 left_now    // A -> izquierda\n");
    printf("    KEY 7 right_now   // D -> derecha\n\n");

    /* ========= DETECCIÓN DE FLANCOS (0 -> 1) ========= */
    /* Queremos UN paso por pulsación, no por iteración. */

    /* Flanco de PRESIONAR A: left_prev == 0 && left_now == 1 */
    printf("    EQ left_prev 0 TMP\n");
    printf("    IF TMP GOTO A_PREV_ZERO\n");
    printf("    GOTO CHECK_RIGHT\n\n");

    printf("LABEL A_PREV_ZERO\n");
    printf("    EQ left_now 1 TMP\n");
    printf("    IF TMP GOTO MOVE_LEFT\n");
    printf("    GOTO CHECK_RIGHT\n\n");

    /* Flanco de PRESIONAR D: right_prev == 0 && right_now == 1 */
    printf("LABEL CHECK_RIGHT\n");
    printf("    EQ right_prev 0 TMP\n");
    printf("    IF TMP GOTO D_PREV_ZERO\n");
    printf("    GOTO END_KEYS\n\n");

    printf("LABEL D_PREV_ZERO\n");
    printf("    EQ right_now 1 TMP\n");
    printf("    IF TMP GOTO MOVE_RIGHT\n");
    printf("    GOTO END_KEYS\n\n");

    /* ================= MOVIMIENTO IZQUIERDA ================= */
    printf("LABEL MOVE_LEFT\n");
    printf("    SUB x STEP x\n");           /* x = x - STEP                      */
    printf("    LT x MIN_X COND\n");
    printf("    IF COND GOTO CLAMP_LEFT\n");
    printf("    GOTO END_KEYS\n\n");

    printf("LABEL CLAMP_LEFT\n");
    printf("    ASSIGN MIN_X x\n");
    printf("    GOTO END_KEYS\n\n");

    /* ================= MOVIMIENTO DERECHA ================= */
    printf("LABEL MOVE_RIGHT\n");
    printf("    ADD x STEP x\n");           /* x = x + STEP                      */
    printf("    GT x MAX_X COND\n");
    printf("    IF COND GOTO CLAMP_RIGHT\n");
    printf("    GOTO END_KEYS\n\n");

    printf("LABEL CLAMP_RIGHT\n");
    printf("    ASSIGN MAX_X x\n");
    printf("    GOTO END_KEYS\n\n");

    /* ========= ACTUALIZAR ESTADO PREVIO Y REPETIR ========= */
    printf("LABEL END_KEYS\n");
    printf("    ASSIGN left_now  left_prev\n");
    printf("    ASSIGN right_now right_prev\n");
    printf("    GOTO MAIN_LOOP\n\n");
}
