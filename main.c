// main.c
// Implementación de `main` y `yyerror`.
#include <stdio.h>
#include <stdlib.h>
#include "symtab.h"
#include "types.h"

// Símbolos provistos por Flex/Bison
extern FILE *yyin;
extern int yylineno;
extern char *yytext;
extern int yyparse(void);

/* Bison debug flag (declared when parser is generated with %debug) */
extern int yydebug;

void yyerror(const char *s) {
	fprintf(stderr, "Error sintáctico en línea %d: %s cerca de '%s'\n", yylineno, s, yytext);
}

int main(int argc, char **argv) {
	if (argc > 1) {
		yyin = fopen(argv[1], "r");
		if (!yyin) {
			perror(argv[1]);
			return 1;
		}
	}
	printf("Iniciando compilador Meow...\n");
	if (getenv("MEOW_DEBUG") != NULL) {
		yydebug = 1;
		fprintf(stderr, "MEOW_DEBUG enabled: parser debug on\n");
	}
	yyparse();
	printf("Análisis completado. Limpiando tabla de símbolos...\n");
	cleanup_symtab();
	return 0;
}