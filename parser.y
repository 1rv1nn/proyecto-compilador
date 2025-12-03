%{
#include "types.h"
#include "symtab.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Externos del scanner */
extern int   yylex();
extern int   yylineno;
extern char *yytext;
extern FILE *yyin;

/* Prototipo de la función de error; implementación en main.c */
void yyerror(const char *s);
%}

/* ----- Sección visible en los headers generados ----- */
%code requires {
#include "types.h"
#include "symtab.h"
#include "ast.h"

/* Raíz del AST declarada en ast.c */
extern ASTStmt *ast_root;

void yyerror(const char *s);
}

/* Para debug con MEOW_DEBUG */
%debug

/* ---------------- UNIÓN PARA VALORES SEMÁNTICOS ---------------- */
%union {
    int       ival;
    float     fval;
    char     *sval;
    MeowType  type;     /* tipo de expresiones / declaraciones */
    ASTExpr  *expr;
    ASTStmt  *stmt;
}

/* ---------------- TOKENS ---------------- */

/* Tokens sin valor */
%token T_SEMICOLON T_ASSIGN
%token T_PLUS T_MINUS T_MULT T_DIV
%token T_LPAREN T_RPAREN
%token T_LBRACKET T_RBRACKET
%token T_DOT T_COMMA
%token T_LBRACE T_RBRACE

/* Tokens con valor (van en la unión) */
%token <ival>  T_LITERAL_INT
%token <fval>  T_LITERAL_FLOAT
%token <sval>  T_LITERAL_STRING
%token <sval>  T_ID

/* Palabras clave */
%token T_DECLARACION
%token T_IF T_ELSE T_WHILE T_FOR T_FUNCTION T_RETURN
%token T_MIAU_PIXEL T_MIAU_KEY T_MIAU_INPUT T_MIAU_PRINT
%token T_TRUE T_FALSE

/* Tipos del lenguaje */
%token T_INT T_FLOAT T_BOOL T_STRING

/* ---------------- TIPOS DE NO TERMINALES ---------------- */

%type <stmt> Programa ListaSentencias Sentencia
%type <stmt> IfStmt WhileStmt ForStmt

/* no usamos todavía AST real aquí, pero dejamos el tipo listo */
%type <type> Declaracion Asignacion
%type <type> Tipo Expresion Termino Factor Base Optinit
%type <ival> OptArray
%type <type> MiauPixel MiauKey MiauInput MiauPrint

%%  /* ============ GRAMÁTICA ============ */

/* ==================== PROGRAMA ==================== */
/* Versión A con AST mínimo: solo marcamos ast_root como no-NULL
   para que main pueda llamar a codegen_fis25(ast_root).          */

Programa
    : ListaSentencias
      {
          /* AST mínimo: por ahora solo usamos un sentinel distinto de NULL.
             Más adelante, cuando tengas nodos reales, aquí se asigna la raíz
             verdadera del AST (por ejemplo, una lista de sentencias).        */
          ast_root = (ASTStmt *)1;
          $$ = NULL;
      }
    ;

/* Lista de sentencias secuenciales */
ListaSentencias
    : /* vacío */                  { $$ = NULL; }
    | ListaSentencias Sentencia    { $$ = NULL; }
    ;

/* Cada Sentencia termina en ';' excepto if / while / for / bloques */
Sentencia
    : Declaracion T_SEMICOLON                 { $$ = NULL; }
    | Asignacion  T_SEMICOLON                 { $$ = NULL; }
    | MiauPixel   T_SEMICOLON                 { $$ = NULL; }
    | MiauKey     T_SEMICOLON                 { $$ = NULL; }
    | MiauInput   T_SEMICOLON                 { $$ = NULL; }
    | MiauPrint   T_SEMICOLON                 { $$ = NULL; }
    | IfStmt                                   { $$ = NULL; }
    | WhileStmt                                { $$ = NULL; }
    | ForStmt                                  { $$ = NULL; }
    | T_LBRACE ListaSentencias T_RBRACE        { $$ = NULL; }
    ;

/* ==================== CONTROL DE FLUJO ==================== */

/* IF (con y sin else) */
IfStmt
    : T_IF T_LPAREN Expresion T_RPAREN Sentencia
      {
          if ($3 != TYPE_BOOL && $3 != TYPE_ERROR) {
              fprintf(stderr,
                      "Error semántico: la condición de if debe ser bool.\n");
          }
          $$ = NULL;
      }
    | T_IF T_LPAREN Expresion T_RPAREN Sentencia T_ELSE Sentencia
      {
          if ($3 != TYPE_BOOL && $3 != TYPE_ERROR) {
              fprintf(stderr,
                      "Error semántico: la condición de if debe ser bool.\n");
          }
          $$ = NULL;
      }
    ;

/* WHILE (cuerpo = una Sentencia) */
WhileStmt
    : T_WHILE T_LPAREN Expresion T_RPAREN Sentencia
      {
          if ($3 != TYPE_BOOL && $3 != TYPE_ERROR) {
              fprintf(stderr,
                      "Error semántico: la condición de while debe ser bool.\n");
          }
          $$ = NULL;
      }
    ;

/* FOR sencillo: for (asign; expr; asign) Sentencia */
ForStmt
    : T_FOR T_LPAREN Asignacion T_SEMICOLON
                     Expresion  T_SEMICOLON
                     Asignacion T_RPAREN
                     Sentencia
      {
          if ($5 != TYPE_BOOL && $5 != TYPE_ERROR) {
              fprintf(stderr,
                      "Error semántico: la condición de for debe ser bool.\n");
          }
          $$ = NULL;
      }
    ;

/* ==================== DECLARACIONES ==================== */
/* Acepta:
 *   meow meow x = 10;
 *   meow meow meow f;
 *   meowmeow bandera = meowt;
 *   meow meow nums[5];
 */

Declaracion
    : T_DECLARACION Tipo T_ID OptArray Optinit
      {
          MeowType declared_type = $2;
          char *id_name = $3;
          SymbolEntry *entry = NULL;

          if ($4 > 0) {
              entry = insert_array_symbol(id_name, declared_type, $4);
          } else {
              entry = insert_symbol(id_name, declared_type);
          }

          if (entry != NULL && $5 != TYPE_VOID) {
              MeowType init_type = $5;
              if ($4 > 0) {
                  fprintf(stderr,
                          "Error semántico: Inicialización directa de arreglo '%s' no soportada.\n",
                          id_name);
              } else {
                  if (declared_type != init_type) {
                      if (!(declared_type == TYPE_FLOAT && init_type == TYPE_INT)) {
                          fprintf(stderr,
                                  "Error semántico: Inicialización incompatible de '%s'. Esperado: %s, Recibido: %s.\n",
                                  id_name,
                                  MeowTypeToString(declared_type),
                                  MeowTypeToString(init_type));
                      } else {
                          printf("DEBUG: Promoción implícita INT -> FLOAT en inicialización de '%s'.\n",
                                 id_name);
                      }
                  }
              }
          }
          $$ = declared_type;
      }
    | Tipo T_ID OptArray Optinit
      {
          MeowType declared_type = $1;
          char *id_name = $2;
          SymbolEntry *entry = NULL;

          if ($3 > 0) {
              entry = insert_array_symbol(id_name, declared_type, $3);
          } else {
              entry = insert_symbol(id_name, declared_type);
          }

          if (entry != NULL && $4 != TYPE_VOID) {
              MeowType init_type = $4;
              if ($3 > 0) {
                  fprintf(stderr,
                          "Error semántico: Inicialización directa de arreglo '%s' no soportada.\n",
                          id_name);
              } else {
                  if (declared_type != init_type) {
                      if (!(declared_type == TYPE_FLOAT && init_type == TYPE_INT)) {
                          fprintf(stderr,
                                  "Error semántico: Inicialización incompatible de '%s'. Esperado: %s, Recibido: %s.\n",
                                  id_name,
                                  MeowTypeToString(declared_type),
                                  MeowTypeToString(init_type));
                      } else {
                          printf("DEBUG: Promoción implícita INT -> FLOAT en inicialización de '%s'.\n",
                                 id_name);
                      }
                  }
              }
          }
          $$ = declared_type;
      }
    ;

/* Tipos */
Tipo
    : T_INT      { $$ = TYPE_INT; }
    | T_FLOAT    { $$ = TYPE_FLOAT; }
    | T_BOOL     { $$ = TYPE_BOOL; }
    | T_STRING   { $$ = TYPE_STRING; }
    ;

/* Inicialización opcional:  = Expresion */
Optinit
    : T_ASSIGN Expresion  { $$ = $2; }
    | /* vacío */         { $$ = TYPE_VOID; }
    ;

/* Tamaño opcional de arreglo: [N] */
OptArray
    : T_LBRACKET T_LITERAL_INT T_RBRACKET  { $$ = $2; }
    | /* vacío */                          { $$ = 0; }
    ;

/* ==================== ASIGNACIONES ==================== */

Asignacion
    : T_ID T_ASSIGN Expresion
      {
          char *id_name = $1;
          MeowType lhs_type = get_symbol_type(id_name);
          MeowType rhs_type = $3;

          if (lhs_type != TYPE_ERROR && rhs_type != TYPE_ERROR) {
              if (lhs_type != rhs_type) {
                  if (!(lhs_type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                      fprintf(stderr,
                              "Error semántico: Asignación incompatible en '%s'. Esperado: %s, Recibido: %s.\n",
                              id_name,
                              MeowTypeToString(lhs_type),
                              MeowTypeToString(rhs_type));
                  } else {
                      printf("DEBUG: Promoción implícita INT -> FLOAT en asignación de '%s'.\n",
                             id_name);
                  }
              }
          }
          $$ = rhs_type;
      }
    | T_ID T_LBRACKET Expresion T_RBRACKET T_ASSIGN Expresion
      {
          char *id_name = $1;
          MeowType arr_type  = get_symbol_type(id_name);
          MeowType idx_type  = $3;
          MeowType rhs_type  = $6;
          MeowType elem_type = get_symbol_element_type(id_name);

          if (arr_type != TYPE_ARRAY) {
              fprintf(stderr, "Error semántico: '%s' no es un arreglo.\n", id_name);
          }
          if (idx_type != TYPE_INT && idx_type != TYPE_ERROR) {
              fprintf(stderr, "Error semántico: Índice de arreglo '%s' debe ser int.\n", id_name);
          }

          if (elem_type != TYPE_ERROR && rhs_type != TYPE_ERROR) {
              if (elem_type != rhs_type) {
                  if (!(elem_type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                      fprintf(stderr,
                              "Error semántico: Asignación incompatible en '%s[...]'. Esperado: %s, Recibido: %s.\n",
                              id_name,
                              MeowTypeToString(elem_type),
                              MeowTypeToString(rhs_type));
                  } else {
                      printf("DEBUG: Promoción implícita INT -> FLOAT en '%s[...]'.\n",
                             id_name);
                  }
              }
          }
          $$ = rhs_type;
      }
    ;

/* ==================== EXPRESIONES ==================== */

/* Asignaciones dentro de expresiones: h = i = 10 */
Expresion
    : T_ID T_ASSIGN Expresion
      {
          char *id_name = $1;
          MeowType lhs_type = get_symbol_type(id_name);
          MeowType rhs_type = $3;

          if (lhs_type != TYPE_ERROR && rhs_type != TYPE_ERROR) {
              if (lhs_type != rhs_type) {
                  if (!(lhs_type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                      fprintf(stderr,
                              "Error semántico: Asignación incompatible en '%s'. Esperado: %s, Recibido: %s.\n",
                              id_name,
                              MeowTypeToString(lhs_type),
                              MeowTypeToString(rhs_type));
                  } else {
                      printf("DEBUG: Promoción implícita INT -> FLOAT en asignación de '%s'.\n",
                             id_name);
                  }
              }
          }
          $$ = rhs_type;
      }
    | Expresion T_PLUS  Termino  { $$ = check_arithmetic_type($1, $3); }
    | Expresion T_MINUS Termino  { $$ = check_arithmetic_type($1, $3); }
    | Termino                    { $$ = $1; }
    ;

Termino
    : Termino T_MULT Factor  { $$ = check_arithmetic_type($1, $3); }
    | Termino T_DIV Factor   { $$ = check_arithmetic_type($1, $3); }
    | Factor                 { $$ = $1; }
    ;

Factor
    : Base                   { $$ = $1; }
    ;

Base
    : T_LPAREN Expresion T_RPAREN { $$ = $2; }
    | T_ID
      {
          MeowType t = get_symbol_type($1);
          if (t == TYPE_ARRAY) {
              fprintf(stderr, "Error semántico: Uso de arreglo '%s' sin índice.\n", $1);
              $$ = TYPE_ERROR;
          } else {
              $$ = t;
              if ($$ == TYPE_VOID) {
                  fprintf(stderr,
                          "Error interno: ID '%s' tiene tipo VOID en expresión.\n", $1);
                  $$ = TYPE_ERROR;
              }
          }
      }
    | T_LITERAL_INT         { $$ = TYPE_INT; }
    | T_LITERAL_FLOAT       { $$ = TYPE_FLOAT; }
    | T_LITERAL_STRING      { $$ = TYPE_STRING; }
    | T_TRUE                { $$ = TYPE_BOOL; }
    | T_FALSE               { $$ = TYPE_BOOL; }
    | T_ID T_LBRACKET Expresion T_RBRACKET
      {
          char *id_name = $1;
          MeowType arr_type = get_symbol_type(id_name);
          MeowType idx_type = $3;

          if (arr_type != TYPE_ARRAY) {
              fprintf(stderr, "Error semántico: '%s' no es un arreglo.\n", id_name);
              $$ = TYPE_ERROR;
          } else if (idx_type != TYPE_INT && idx_type != TYPE_ERROR) {
              fprintf(stderr,
                      "Error semántico: Índice de arreglo debe ser int en '%s'.\n",
                      id_name);
              $$ = TYPE_ERROR;
          } else {
              $$ = get_symbol_element_type(id_name);
          }
      }
    | T_ID T_DOT T_ID
      {
          char *id_name = $1;
          char *prop    = $3;

          if (strcmp(prop, "length") == 0) {
              if (get_symbol_type(id_name) == TYPE_ARRAY) {
                  $$ = TYPE_INT;
              } else {
                  fprintf(stderr,
                          "Error semántico: '%s' no es un arreglo y no tiene 'length'.\n",
                          id_name);
                  $$ = TYPE_ERROR;
              }
          } else {
              fprintf(stderr,
                      "Error semántico: Propiedad desconocida '%s' en '%s'.\n",
                      prop, id_name);
              $$ = TYPE_ERROR;
          }
      }
    ;

/* ==================== PRIMITIVAS FIS-25 ==================== */

MiauPixel
    : T_MIAU_PIXEL T_LPAREN Expresion T_COMMA Expresion T_COMMA Expresion T_RPAREN
      {
          if ($3 == TYPE_INT && $5 == TYPE_INT && $7 == TYPE_INT) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr,
                      "Error semántico: miau_pixel espera tres argumentos int (x, y, color).\n");
              $$ = TYPE_ERROR;
          }
      }
    ;

MiauKey
    : T_MIAU_KEY T_LPAREN Expresion T_COMMA T_ID T_RPAREN
      {
          int destType = get_symbol_type($5);
          if ($3 == TYPE_INT && (destType == TYPE_INT || destType == TYPE_BOOL)) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr,
                      "Error semántico: miau_key espera (int, id<int|bool>).\n");
              $$ = TYPE_ERROR;
          }
      }
    ;

MiauInput
    : T_MIAU_INPUT T_LPAREN T_ID T_RPAREN
      {
          int destType = get_symbol_type($3);
          if (destType == TYPE_INT) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr,
                      "Error semántico: miau_input espera un identificador int.\n");
              $$ = TYPE_ERROR;
          }
      }
    ;

MiauPrint
    : T_MIAU_PRINT T_LPAREN Expresion T_RPAREN
      {
          if ($3 == TYPE_INT || $3 == TYPE_FLOAT ||
              $3 == TYPE_BOOL || $3 == TYPE_STRING) {
              $$ = TYPE_VOID;
          } else {
              fprintf(stderr,
                      "Error semántico: miau_print no acepta este tipo de expresión.\n");
              $$ = TYPE_ERROR;
          }
      }
    ;

%%  /* fin de las reglas */
