// symtab.h

#ifndef SYMTAB_H
#define SYMTAB_H

#include "types.h"
#include <stdlib.h>
#include <string.h>

// Estructura de un nodo en la Tabla de Símbolos
typedef struct SymbolEntry {
    char *id_name;      // Nombre del identificador
    MeowType id_type;   // Tipo del identificador
    // Si es un arreglo, guardamos información adicional
    int is_array;             // 0 = no, 1 = sí
    MeowType element_type;    // tipo del elemento si is_array == 1
    int array_length;         // longitud del arreglo (si conocida, >0)
    struct SymbolEntry *next; 
} SymbolEntry;

// Declaración de la tabla de símbolos (puede ser la cabeza de una lista enlazada)
extern SymbolEntry *symbol_table_head;

/**
 * @brief Busca un símbolo por nombre.
 * * @param name El nombre del identificador.
 * @return SymbolEntry* El puntero a la entrada, o NULL si no se encuentra.
 */
SymbolEntry* lookup_symbol(const char *name);

/**
 * @brief Inserta un nuevo símbolo en la tabla.
 * * @param name Nombre del identificador.
 * @param type Tipo del identificador.
 * @return SymbolEntry* El puntero a la nueva entrada, o NULL si ya existe.
 */
SymbolEntry* insert_symbol(const char *name, MeowType type);

/**
 * @brief Inserta un símbolo que representa un arreglo unidimensional.
 * @param name Nombre del identificador.
 * @param elem_type Tipo de los elementos.
 * @param length Longitud del arreglo (si <= 0, se considera desconocida).
 * @return SymbolEntry* Puntero a la nueva entrada o NULL si ya existe.
 */
SymbolEntry* insert_array_symbol(const char *name, MeowType elem_type, int length);

/**
 * @brief Obtiene el tipo de un identificador.
 * * @param name El nombre del identificador.
 * @return MeowType El tipo del identificador, o TYPE_ERROR si no se encuentra.
 */
MeowType get_symbol_type(const char *name);

/**
 * @brief Obtiene el tipo de elemento de un arreglo.
 * @param name Nombre del identificador.
 * @return MeowType Tipo del elemento si es arreglo, o TYPE_ERROR.
 */
MeowType get_symbol_element_type(const char *name);

/**
 * @brief Obtiene la longitud del arreglo declarado.
 * @param name Nombre del identificador.
 * @return int Longitud (>0) o -1 si desconocida o no es arreglo.
 */
int get_symbol_array_length(const char *name);

/**
 * @brief Inicializa y limpia la memoria de la tabla.
 */
void cleanup_symtab();

#endif // SYMTAB_H