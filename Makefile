CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -g
LDFLAGS = 
# Se agrega -Wno-unused-result para evitar advertencia común en main.c
BISON_FLAGS = -d -v 

# Archivos fuente del compilador
SOURCES = main.c symtab.c types.c
OBJECTS = $(SOURCES:.c=.o) parser.o scanner.o

# Nombre del ejecutable final
EXECUTABLE = meowc

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

# Regla para generar el parser (parser.c y parser.h)
parser.c parser.h: parser.y
	$(BISON) $(BISON_FLAGS) parser.y -o parser.c

# Regla para generar el scanner (scanner.c)
scanner.c: scanner.l
	$(FLEX) -o scanner.c scanner.l

# Regla genérica para compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(EXECUTABLE) $(OBJECTS) parser.c parser.h scanner.c *.output

.PHONY: all clean