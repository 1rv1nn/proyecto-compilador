// ast.c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ast.h"

ASTStmt *ast_root = NULL;

static char *dupstr(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

// ------------- Expresiones -------------

ASTExpr *ast_make_var(const char *name, MeowType t) {
    ASTExpr *e = (ASTExpr *)calloc(1, sizeof(ASTExpr));
    e->kind = AST_EXPR_VAR;
    e->type = t;
    e->u.var = dupstr(name);
    return e;
}

ASTExpr *ast_make_int(int value) {
    ASTExpr *e = (ASTExpr *)calloc(1, sizeof(ASTExpr));
    e->kind = AST_EXPR_INT;
    e->type = TYPE_INT;
    e->u.ival = value;
    return e;
}

ASTExpr *ast_make_bool(int value) {
    ASTExpr *e = (ASTExpr *)calloc(1, sizeof(ASTExpr));
    e->kind = AST_EXPR_BOOL;
    e->type = TYPE_BOOL;
    e->u.ival = value ? 1 : 0;
    return e;
}

ASTExpr *ast_make_binop(ASTBinOp op, ASTExpr *left, ASTExpr *right, MeowType t) {
    ASTExpr *e = (ASTExpr *)calloc(1, sizeof(ASTExpr));
    e->kind = AST_EXPR_BINOP;
    e->type = t;
    e->u.bin.op = op;
    e->u.bin.left = left;
    e->u.bin.right = right;
    return e;
}

// ------------- Sentencias -------------

ASTStmt *ast_make_stmt(ASTStmtKind kind) {
    ASTStmt *s = (ASTStmt *)calloc(1, sizeof(ASTStmt));
    s->kind = kind;
    s->next = NULL;
    return s;
}

ASTStmt *ast_make_decl(MeowType t, const char *name, ASTExpr *init) {
    ASTStmt *s = ast_make_stmt(AST_STMT_DECL);
    s->u.decl.type = t;
    s->u.decl.name = dupstr(name);
    s->u.decl.init = init;
    return s;
}

ASTStmt *ast_make_assign(const char *name, ASTExpr *expr) {
    ASTStmt *s = ast_make_stmt(AST_STMT_ASSIGN);
    s->u.assign.name = dupstr(name);
    s->u.assign.expr = expr;
    return s;
}

ASTStmt *ast_make_while(ASTExpr *cond, ASTStmt *body) {
    ASTStmt *s = ast_make_stmt(AST_STMT_WHILE);
    s->u.while_stmt.cond = cond;
    s->u.while_stmt.body = body;
    return s;
}

ASTStmt *ast_make_if(ASTExpr *cond, ASTStmt *then_branch) {
    ASTStmt *s = ast_make_stmt(AST_STMT_IF);
    s->u.if_stmt.cond = cond;
    s->u.if_stmt.then_branch = then_branch;
    return s;
}

ASTStmt *ast_make_pixel(ASTExpr *x, ASTExpr *y, ASTExpr *color) {
    ASTStmt *s = ast_make_stmt(AST_STMT_PIXEL);
    s->u.pixel.x = x;
    s->u.pixel.y = y;
    s->u.pixel.color = color;
    return s;
}

ASTStmt *ast_make_key(int key_code, const char *dest_name) {
    ASTStmt *s = ast_make_stmt(AST_STMT_KEY);
    s->u.key.key_code = key_code;
    s->u.key.dest_name = dupstr(dest_name);
    return s;
}

ASTStmt *ast_make_input(const char *dest_name) {
    ASTStmt *s = ast_make_stmt(AST_STMT_INPUT);
    s->u.input.dest_name = dupstr(dest_name);
    return s;
}

ASTStmt *ast_make_print(ASTExpr *expr) {
    ASTStmt *s = ast_make_stmt(AST_STMT_PRINT);
    s->u.print.expr = expr;
    return s;
}

ASTStmt *ast_make_block(ASTStmt *stmts) {
    ASTStmt *s = ast_make_stmt(AST_STMT_BLOCK);
    s->u.block.stmts = stmts;
    return s;
}

ASTStmt *ast_append_stmt(ASTStmt *list, ASTStmt *stmt) {
    if (!stmt) return list;
    if (!list) return stmt;
    ASTStmt *p = list;
    while (p->next) p = p->next;
    p->next = stmt;
    return list;
}
