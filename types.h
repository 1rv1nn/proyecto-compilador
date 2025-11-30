// types.h

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

// [cite: 18]
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_ERROR,  // Para manejo de errores
    TYPE_VOID    // Para inicialización opcional o funciones sin retorno
    , TYPE_ARRAY  // Tipo genérico para arrays (detalles en la tabla de símbolos)
} MeowType;

/**
 * @brief Convierte un MeowType a una cadena legible.
 * * @param type El tipo a convertir.
 * @return const char* El nombre del tipo.
 */
const char* MeowTypeToString(MeowType type);

/**
 * @brief Aplica las reglas de promoción de tipo para operadores aritméticos.
 * * @param t1 Tipo del operando izquierdo.
 * @param t2 Tipo del operando derecho.
 * @return MeowType El tipo resultante de la operación.
 */
MeowType check_arithmetic_type(MeowType t1, MeowType t2);

#endif // TYPES_H