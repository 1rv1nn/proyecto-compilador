// types.c

#include "types.h"
#include <stdio.h>

const char* MeowTypeToString(MeowType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        case TYPE_ERROR: return "ERROR";
        case TYPE_VOID: return "void";
        case TYPE_ARRAY: return "array";
        default: return "UNKNOWN";
    }
}

MeowType check_arithmetic_type(MeowType t1, MeowType t2) {
    if (t1 == TYPE_ERROR || t2 == TYPE_ERROR) {
        return TYPE_ERROR;
    }
    
    // Solo permitimos int o float para operaciones aritméticas (sin string/bool)
    if ((t1 != TYPE_INT && t1 != TYPE_FLOAT) || (t2 != TYPE_INT && t2 != TYPE_FLOAT)) {
        fprintf(stderr, "Error semántico: Operación aritmética inválida. Tipos incompatibles: %s y %s.\n", 
                MeowTypeToString(t1), MeowTypeToString(t2));
        return TYPE_ERROR;
    }

    // Promoción de tipo: si hay float, el resultado es float
    if (t1 == TYPE_FLOAT || t2 == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    
    // Si ambos son int, el resultado es int
    return TYPE_INT;
}