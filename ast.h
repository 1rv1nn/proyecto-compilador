// ast.h
#ifndef AST_H
#define AST_H

#include "types.h"

// ====================
//  Expresiones
// ====================

typedef enum {
    AST_EXPR_VAR,
    AST_EXPR_INT,
    AST_EXPR_BOOL,
    AST_EXPR_BINOP
} ASTExprKind;

typedef enum {
    AST_BINOP_ADD,
    AST_BINOP_SUB
} ASTBinOp;

typedef struct ASTExpr {
    ASTExprKind kind;
    MeowType type;      // TYPE_INT, TYPE_BOOL, etc.
    union {
        char *var;      // para AST_EXPR_VAR
        int ival;       // para AST_EXPR_INT / AST_EXPR_BOOL (0/1)
        struct {
            ASTBinOp op;
            struct ASTExpr *left;
            struct ASTExpr *right;
        } bin;
    } u;
} ASTExpr;

// ====================
//  Sentencias
// ====================

typedef enum {
    AST_STMT_DECL,      // declaración (con posible inicialización)
    AST_STMT_ASSIGN,    // asignación simple id = expr;
    AST_STMT_WHILE,
    AST_STMT_IF,
    AST_STMT_PIXEL,
    AST_STMT_KEY,
    AST_STMT_INPUT,
    AST_STMT_PRINT,
    AST_STMT_BLOCK      // { listaDeSentencias }
} ASTStmtKind;

typedef struct ASTStmt {
    ASTStmtKind kind;
    struct ASTStmt *next;   // para listas de sentencias
    union {
        struct {            // DECL
            MeowType type;
            char *name;
            ASTExpr *init;  // puede ser NULL
        } decl;

        struct {            // ASSIGN
            char *name;
            ASTExpr *expr;
        } assign;

        struct {            // WHILE
            ASTExpr *cond;          // bool
            struct ASTStmt *body;   // lista
        } while_stmt;

        struct {            // IF sin else
            ASTExpr *cond;
            struct ASTStmt *then_branch;
        } if_stmt;

        struct {            // miau_pixel(x, y, c)
            ASTExpr *x;
            ASTExpr *y;
            ASTExpr *color;
        } pixel;

        struct {            // miau_key( keyCode , destId )
            int key_code;
            char *dest_name;
        } key;

        struct {            // miau_input(destId)
            char *dest_name;
        } input;

        struct {            // miau_print(expr)
            ASTExpr *expr;
        } print;

        struct {            // bloque { ... }
            struct ASTStmt *stmts;
        } block;
    } u;
} ASTStmt;

// Raíz del programa
extern ASTStmt *ast_root;

// Constructores básicos
ASTExpr *ast_make_var(const char *name, MeowType t);
ASTExpr *ast_make_int(int value);
ASTExpr *ast_make_bool(int value); // 0 o 1
ASTExpr *ast_make_binop(ASTBinOp op, ASTExpr *left, ASTExpr *right, MeowType t);

ASTStmt *ast_make_decl(MeowType t, const char *name, ASTExpr *init);
ASTStmt *ast_make_assign(const char *name, ASTExpr *expr);
ASTStmt *ast_make_while(ASTExpr *cond, ASTStmt *body);
ASTStmt *ast_make_if(ASTExpr *cond, ASTStmt *then_branch);
ASTStmt *ast_make_pixel(ASTExpr *x, ASTExpr *y, ASTExpr *color);
ASTStmt *ast_make_key(int key_code, const char *dest_name);
ASTStmt *ast_make_input(const char *dest_name);
ASTStmt *ast_make_print(ASTExpr *expr);
ASTStmt *ast_make_block(ASTStmt *stmts);

// Utilidad para concatenar listas de sentencias
ASTStmt *ast_append_stmt(ASTStmt *list, ASTStmt *stmt);

#endif // AST_H
