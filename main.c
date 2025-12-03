#include <stdio.h>
#include <string.h>
#include "symtab.h"
#include "types.h"
#include "ast.h"
#include "codegen_fis25.h"

extern FILE *yyin;
extern int   yylineno;
extern char *yytext;
extern int   yyparse(void);
extern int   yydebug;
extern ASTStmt *ast_root;

void yyerror(const char *s) {
    fprintf(stderr, "Error sintáctico en línea %d: %s cerca de '%s'\n",
            yylineno, s, yytext);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo.meow>\n", argv[0]);
        return 1;
    }

    /* 1) Leer el mensaje para el letrero desde stdin */
    char raw_msg[256];
    char msg[64];

    fprintf(stderr, "Ingresa el mensaje del letrero (solo 0-9 . $): ");
    fflush(stderr);

    if (!fgets(raw_msg, sizeof(raw_msg), stdin)) {
        /* Error leyendo stdin: usar valor por defecto */
        fprintf(stderr, "Error leyendo stdin. Usando mensaje por defecto '0'.\n");
        strcpy(msg, "0");
    } else {
        /* 2) Limpiar salto de línea y filtrar solo 0-9 . $ */
        size_t len = strlen(raw_msg);
        size_t j = 0;

        for (size_t i = 0; i < len && j < sizeof(msg) - 1; ++i) {
            char c = raw_msg[i];
            if (c == '\n' || c == '\r')
                continue;

            if ((c >= '0' && c <= '9') || c == '.' || c == '$') {
                msg[j++] = c;
            } else {
                fprintf(stderr,
                        "Advertencia: ignorando caracter no permitido '%c'\n", c);
            }
        }
        msg[j] = '\0';

        if (j == 0) {
            fprintf(stderr,
                    "Mensaje vacío tras filtrado. Usando mensaje por defecto '0'.\n");
            strcpy(msg, "0");
        }
    }

    /* 3) Abrir el archivo fuente Meow */
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror(argv[1]);
        return 1;
    }

    if (getenv("MEOW_DEBUG") != NULL) {
        yydebug = 1;
        fprintf(stderr, "MEOW_DEBUG enabled: parser debug ON\n");
    }

    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        fprintf(stderr, "Compilación fallida: errores sintácticos.\n");
        cleanup_symtab();
        return 2;
    }

    if (!ast_root) {
        fprintf(stderr,
                "Error: el parser terminó sin construir el AST (ast_root == NULL).\n");
        cleanup_symtab();
        return 3;
    }

    /* 4) Generar código FIS-25 usando el mensaje filtrado */
    codegen_fis25(ast_root, msg);

    cleanup_symtab();
    return 0;
}
