// symtab.c

#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicialización de la cabeza de la tabla
SymbolEntry *symbol_table_head = NULL;

SymbolEntry* lookup_symbol(const char *name) {
    SymbolEntry *current = symbol_table_head;
    while (current != NULL) {
        if (strcmp(current->id_name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; // No encontrado
}

SymbolEntry* insert_symbol(const char *name, MeowType type) {
    if (lookup_symbol(name) != NULL) {
        fprintf(stderr, "Error semántico: Redefinición de variable '%s'.\n", name);
        return NULL;
    }

    SymbolEntry *new_entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (new_entry == NULL) {
        perror("Error de memoria al asignar SymbolEntry");
        exit(EXIT_FAILURE);
    }
    
    new_entry->id_name = strdup(name); // Duplicar la cadena para evitar problemas de punteros
    new_entry->id_type = type;
    new_entry->is_array = 0;
    new_entry->element_type = TYPE_ERROR;
    new_entry->array_length = -1;
    
    // Insertar al inicio de la lista
    new_entry->next = symbol_table_head;
    symbol_table_head = new_entry;
    
    fprintf(stderr, "DEBUG: Símbolo '%s' (%s) insertado en la tabla.\n",
        name, MeowTypeToString(type));

    return new_entry;
}

SymbolEntry* insert_array_symbol(const char *name, MeowType elem_type, int length) {
    if (lookup_symbol(name) != NULL) {
        fprintf(stderr, "Error semántico: Redefinición de variable '%s'.\n", name);
        return NULL;
    }

    SymbolEntry *new_entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (new_entry == NULL) {
        perror("Error de memoria al asignar SymbolEntry");
        exit(EXIT_FAILURE);
    }
    new_entry->id_name = strdup(name);
    new_entry->id_type = TYPE_ARRAY;
    new_entry->is_array = 1;
    new_entry->element_type = elem_type;
    new_entry->array_length = length > 0 ? length : -1;
    new_entry->next = symbol_table_head;
    symbol_table_head = new_entry;

    
    return new_entry;
}

MeowType get_symbol_type(const char *name) {
    SymbolEntry *entry = lookup_symbol(name);
    if (entry != NULL) {
        return entry->id_type;
    }
    // Si no se encuentra, es un error semántico de uso de variable no declarada
    fprintf(stderr, "Error semántico: Variable no declarada '%s'.\n", name);
    return TYPE_ERROR;
}

MeowType get_symbol_element_type(const char *name) {
    SymbolEntry *entry = lookup_symbol(name);
    if (entry != NULL && entry->is_array) {
        return entry->element_type;
    }
    fprintf(stderr, "Error semántico: '%s' no es un arreglo o no existe.\n", name);
    return TYPE_ERROR;
}

int get_symbol_array_length(const char *name) {
    SymbolEntry *entry = lookup_symbol(name);
    if (entry != NULL && entry->is_array) {
        return entry->array_length;
    }
    return -1;
}

void cleanup_symtab() {
    SymbolEntry *current = symbol_table_head;
    SymbolEntry *next;
    while (current != NULL) {
        next = current->next;
        free(current->id_name);
        free(current);
        current = next;
    }
    symbol_table_head = NULL;
}