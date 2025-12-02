%{
#include "types.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Externos de Flex
extern int yylex();
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

// Prototipo de la función de error; implementación en `main.c`.
void yyerror(const char *s);
%}

%code requires {
#include "types.h"
#include "symtab.h"
}

%debug

// Definición de la UNIÓN para pasar valores y tipos entre reglas
%union {
    int ival;
    float fval;
    char *sval;
    MeowType type; // CLAVE: para pasar el tipo de la expresión o declaración
}

// Tokens terminales
%token T_SEMICOLON T_ASSIGN
%token T_PLUS T_MINUS T_MULT T_DIV T_LPAREN T_RPAREN
%token T_LBRACKET T_RBRACKET T_DOT
%token <ival> T_LITERAL_INT
%token <fval> T_LITERAL_FLOAT
%token <sval> T_LITERAL_STRING T_ID

// Tokens de Palabras Clave
%token T_DECLARACION T_IF T_ELSE T_WHILE T_FOR T_FUNCTION T_RETURN
%token T_MIAU_PIXEL T_MIAU_KEY T_MIAU_INPUT T_MIAU_PRINT T_COMMA
%token T_TRUE T_FALSE

// Tokens que representan los Tipos (usados en la Declaración)
%token T_INT T_FLOAT T_BOOL T_STRING

// Tipos de los no-terminales
%type <type> Tipo Expresion Termino Factor Base Optinit Declaracion Asignacion
%type <ival> OptArray
%type <type> MiauPixel MiauKey MiauInput MiauPrint

%%

Programa: ListaSentencias ;

ListaSentencias: Sentencia ListaSentencias 
               | /* epsilon */
               ; 

Sentencia: Declaracion T_SEMICOLON
    | Asignacion  T_SEMICOLON
    | MiauPixel   T_SEMICOLON
    | MiauKey     T_SEMICOLON
    | MiauInput   T_SEMICOLON
    | MiauPrint   T_SEMICOLON
    ;

// DECLARACIÓN 
// Aceptar dos formas: con prefijo `T_DECLARACION` ("meow") o directamente con el tipo
Declaracion: T_DECLARACION Tipo T_ID OptArray Optinit
{
    MeowType declared_type = $2;
    char *id_name = $3;
    SymbolEntry* entry = NULL;
    if ($4 > 0) {
        entry = insert_array_symbol(id_name, declared_type, $4);
    } else {
        entry = insert_symbol(id_name, declared_type);
    }

    if (entry != NULL && $5 != TYPE_VOID) {
        MeowType init_type = $5;
        if ($4 > 0) {
            fprintf(stderr, "Error semántico: Inicialización directa de arreglos no soportada para '%s'.\n", id_name);
        } else {
        if (declared_type != init_type) {
            if (!(declared_type == TYPE_FLOAT && init_type == TYPE_INT)) {
                fprintf(stderr, "Error semántico: Tipo incompatible en inicialización de '%s'. Esperado: %s, Recibido: %s.\n",
                        id_name, MeowTypeToString(declared_type), MeowTypeToString(init_type));
            } else {
                printf("DEBUG: Promoción implícita INT -> FLOAT en inicialización de '%s'.\n", id_name);
            }
        }
        }
    }
}
|
Tipo T_ID OptArray Optinit
{
    MeowType declared_type = $1;
    char *id_name = $2;
    SymbolEntry* entry = NULL;
    if ($3 > 0) {
        entry = insert_array_symbol(id_name, declared_type, $3);
    } else {
        entry = insert_symbol(id_name, declared_type);
    }

    if (entry != NULL && $4 != TYPE_VOID) {
        MeowType init_type = $4;
        if ($3 > 0) {
            fprintf(stderr, "Error semántico: Inicialización directa de arreglos no soportada para '%s'.\n", id_name);
        } else {
        if (declared_type != init_type) {
            if (!(declared_type == TYPE_FLOAT && init_type == TYPE_INT)) {
                fprintf(stderr, "Error semántico: Tipo incompatible en inicialización de '%s'. Esperado: %s, Recibido: %s.\n",
                        id_name, MeowTypeToString(declared_type), MeowTypeToString(init_type));
            } else {
                printf("DEBUG: Promoción implícita INT -> FLOAT en inicialización de '%s'.\n", id_name);
            }
        }
        }
    }
}
;

// TIPOS 
Tipo: T_INT    { $$ = TYPE_INT; }
    | T_FLOAT  { $$ = TYPE_FLOAT; }
    | T_BOOL   { $$ = TYPE_BOOL; }
    | T_STRING { $$ = TYPE_STRING; }
;

// INICIALIZACIÓN OPCIONAL [cite: 83, 84]
Optinit: T_ASSIGN Expresion { $$ = $2; } // El tipo devuelto es el tipo de la Expresión
    | /* epsilon */     { $$ = TYPE_VOID; }
;

OptArray: T_LBRACKET T_LITERAL_INT T_RBRACKET { $$ = $2; }
     | /* epsilon */ { $$ = 0; }
;


// ASIGNACIÓN [cite: 85]
Asignacion: T_ID T_ASSIGN Expresion
{
    char *id_name = $1;
    MeowType lhs_type = get_symbol_type(id_name);
    MeowType rhs_type = $3;

    if (lhs_type != TYPE_ERROR && rhs_type != TYPE_ERROR) {
        // Comprobación de Tipo para la asignación
        if (lhs_type != rhs_type) {
             if (!(lhs_type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                fprintf(stderr, "Error semántico: Asignación incompatible en '%s'. Esperado: %s, Recibido: %s.\n", 
                        id_name, MeowTypeToString(lhs_type), MeowTypeToString(rhs_type));
            } else {
                printf("DEBUG: Promoción implícita INT -> FLOAT en asignación de '%s'.\n", id_name);
            }
        }
    }
    // Propagación de tipo: en una asignación no hay tipo resultante para el código intermedio,
    // pero si para la comprobación. Aquí solo devolvemos el tipo de la derecha (no estrictamente necesario aquí).
    $$ = rhs_type; 
}
;

/* Asignación a un elemento de arreglo */
Asignacion: T_ID T_LBRACKET Expresion T_RBRACKET T_ASSIGN Expresion
{
    char *id_name = $1;
    MeowType arr_type = get_symbol_type(id_name);
    MeowType rhs_type = $6;
    MeowType elem_type = get_symbol_element_type(id_name);

    if (arr_type != TYPE_ERROR && rhs_type != TYPE_ERROR && elem_type != TYPE_ERROR) {
        if (elem_type != rhs_type) {
            if (!(elem_type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                fprintf(stderr, "Error semántico: Asignación incompatible en '%s[...]'. Esperado: %s, Recibido: %s.\n",
                        id_name, MeowTypeToString(elem_type), MeowTypeToString(rhs_type));
            } else {
                printf("DEBUG: Promoción implícita INT -> FLOAT en asignación a elemento de '%s'.\n", id_name);
            }
        }
    }
    $$ = rhs_type;
}
;

// EXPRESIONES - Se enfoca en la propagación y coherción de Tipos
// Añadimos soporte para expresiones de asignación (derecha asociativa) para
// permitir cosas como `h = i = 10` dentro de inicializadores.
Expresion: T_ID T_ASSIGN Expresion
           {
               char *id_name = $1;
               MeowType lhs_type = get_symbol_type(id_name);
               MeowType rhs_type = $3;
               if (lhs_type != TYPE_ERROR && rhs_type != TYPE_ERROR) {
                   if (lhs_type != rhs_type) {
                       if (!(lhs_type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                           fprintf(stderr, "Error semántico: Asignación incompatible en '%s'. Esperado: %s, Recibido: %s.\n",
                                   id_name, MeowTypeToString(lhs_type), MeowTypeToString(rhs_type));
                       } else {
                           printf("DEBUG: Promoción implícita INT -> FLOAT en asignación de '%s'.\n", id_name);
                       }
                   }
               }
               $$ = rhs_type;
           }
         | Expresion T_PLUS Termino { $$ = check_arithmetic_type($1, $3); } // [cite: 86]
         | Expresion T_MINUS Termino { $$ = check_arithmetic_type($1, $3); } // [cite: 86]
         | Termino { $$ = $1; }
         ;

Termino: Termino T_MULT Factor { $$ = check_arithmetic_type($1, $3); } // [cite: 88]
       | Termino T_DIV Factor { $$ = check_arithmetic_type($1, $3); } // [cite: 88]
       | Factor { $$ = $1; }
       ;

Factor: Base { $$ = $1; } 
      // | Base T_POWER Factor { /* Operador de potencia, omitido por simplicidad */ } // [cite: 89]
      ;

Base: T_LPAREN Expresion T_RPAREN { $$ = $2; } // 
    | T_ID { 
        // Recuperar el tipo del ID desde la Tabla de Símbolos
        MeowType t = get_symbol_type($1);
        if (t == TYPE_ARRAY) {
            fprintf(stderr, "Error semántico: Uso de arreglo '%s' sin índice.\n", $1);
            $$ = TYPE_ERROR;
        } else {
            $$ = t;
            if ($$ == TYPE_VOID) {
                // Esto no debería suceder si 'get_symbol_type' funciona correctamente
                fprintf(stderr, "Error interno: ID '%s' tiene tipo VOID en expresión.\n", $1);
                $$ = TYPE_ERROR;
            }
        }
    } // 
    | T_LITERAL_INT { $$ = TYPE_INT; } // 
    | T_LITERAL_FLOAT { $$ = TYPE_FLOAT; } // 
    | T_LITERAL_STRING { $$ = TYPE_STRING; } 
    | T_TRUE { $$ = TYPE_BOOL; }
    | T_FALSE { $$ = TYPE_BOOL; }
    /* Acceso a elemento de arreglo: id[expr] */
    | T_ID T_LBRACKET Expresion T_RBRACKET {
        char *id_name = $1;
        MeowType arr_type = get_symbol_type(id_name);
        MeowType idx_type = $3;
        if (arr_type != TYPE_ARRAY) {
            fprintf(stderr, "Error semántico: '%s' no es un arreglo.\n", id_name);
            $$ = TYPE_ERROR;
        } else if (idx_type != TYPE_INT) {
            fprintf(stderr, "Error semántico: Índice de arreglo debe ser int en '%s'.\n", id_name);
            $$ = TYPE_ERROR;
        } else {
            $$ = get_symbol_element_type(id_name);
        }
    }
    /* Propiedad length: id.length */
    | T_ID T_DOT T_ID {
        char *id_name = $1;
        char *prop = $3;
        if (strcmp(prop, "length") == 0) {
            if (get_symbol_type(id_name) == TYPE_ARRAY) {
                $$ = TYPE_INT;
            } else {
                fprintf(stderr, "Error semántico: '%s' no es un arreglo y no tiene 'length'.\n", id_name);
                $$ = TYPE_ERROR;
            }
        } else {
            fprintf(stderr, "Error semántico: Propiedad desconocida '%s' en '%s'.\n", prop, id_name);
            $$ = TYPE_ERROR;
        }
    }
;


//Primitivas FIS-25

MiauPixel:
      T_MIAU_PIXEL T_LPAREN Expresion T_COMMA Expresion T_COMMA Expresion T_RPAREN
      {
          /* Todos los argumentos deben ser int */
          if ($3 == TYPE_INT && $5 == TYPE_INT && $7 == TYPE_INT) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr, "Error semántico: miau_pixel espera tres argumentos de tipo int (x, y, color).\n");
              $$ = TYPE_ERROR;
          }
      }
    ;

MiauKey:
      T_MIAU_KEY T_LPAREN Expresion T_COMMA T_ID T_RPAREN
      {
          /* Primer argumento debe ser int. El identificador debe ser bool o int */
          int idType = get_symbol_type($5);
          if ($3 == TYPE_INT && (idType == TYPE_INT || idType == TYPE_BOOL)) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr, "Error semántico: miau_key espera (int, id<int|bool>). \n");
              $$ = TYPE_ERROR;
          }
      }
    ;

MiauInput:
      T_MIAU_INPUT T_LPAREN T_ID T_RPAREN
      {
          /* El identificador debe ser de tipo int */
          int idType = get_symbol_type($3);
          if (idType == TYPE_INT) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr, "Error semántico: miau_input espera un identificador int. \n");
              $$ = TYPE_ERROR;
          }
      }
    ;

MiauPrint:
      T_MIAU_PRINT T_LPAREN Expresion T_RPAREN
      {
          /* Acepta int, float, bool o string */
          if ($3 == TYPE_INT || $3 == TYPE_FLOAT || $3 == TYPE_BOOL || $3 == TYPE_STRING) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr, "Error semántico: miau_print no acepta este tipo de expresión. \n");
              $$ = TYPE_ERROR;
          }
      }
    ;

%%