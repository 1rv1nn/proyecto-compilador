# Proyecto Final de Compiladores

Este repositorio contiene un compilador didáctico llamado `meowc` (lenguaje "Meow").
El proyecto usa Flex (lexer) y Bison (parser) junto con código en C para la semántica y la tabla de símbolos.

**Resumen rápido:**
- Compilar: `make`
- Ejecutar: `./meowc <archivo.meow>`

**Estructura principal**
- `parser.y` — Gramática Bison (parser)
- `scanner.l` — Reglas Flex (lexer)
- `main.c` — Programa principal y manejo de errores
- `symtab.c`/`symtab.h` — Tabla de símbolos
- `types.c`/`types.h` — Comprobación y operaciones de tipos
- `Makefile` — Reglas de compilación
- `type_check.meow`, `test.meow` — ejemplos/tests

**Herramientas necesarias**
Instala las herramientas listadas a continuación en una máquina Linux. Las instrucciones también sirven para entornos en contenedores o WSL.

- `gcc` (paquete `build-essential`) — Compilar código C
- `make` — Sistema de construcción
- `flex` — Generador de lexer
- `bison` — Generador de parser
- `git` — (opcional) control de versiones
- `valgrind` (opcional) — detectar fugas de memoria
- `gdb` (opcional) — depuración

Instalación (Debian/Ubuntu):
```bash
sudo apt update
sudo apt install -y build-essential flex bison make git valgrind gdb
```


**Compilación**
1. Generar `parser.c`/`parser.h` desde `parser.y` y `scanner.c` desde `scanner.l`, y compilar todo:
```bash
make
```
Esto ejecuta `bison` y `flex` (según el `Makefile`) y compila los objetos para producir el binario `meowc`.

2. Limpiar artefactos generados:
```bash
make clean
```

**Ejecución**
```bash
./meowc ruta/al/archivo.meow
```

- Para ver la traza del parser de Bison (útil para debugging):
```bash
MEOW_DEBUG=1 ./meowc ruta/al/archivo.meow
```

- Si editas `parser.y` o `scanner.l` y quieres regenerar manualmente:
```bash
bison -d -v parser.y -o parser.c
flex -o scanner.c scanner.l
gcc -Wall -g -c parser.c scanner.c main.c symtab.c types.c
gcc -o meowc *.o
```

**Instrucciones para tests y ejemplos**
- El repositorio incluye `type_check.meow` y `test.meow` como casos de ejemplo. Ejecuta:
```bash
./meowc type_check.meow
```

**Problemas conocidos y notas**
- Ambigüedad de tokens: debido a cómo funciona Flex (leftmost-longest), secuencias como `meow meow meow` pueden tokenizarse como `T_INT` + `T_DECLARACION` (por ejemplo) y dar lugar a un error sintáctico si la gramática no espera esa secuencia. Si ves "syntax error cerca de 'meow'", revisa que las declaraciones en los tests tengan la forma que el lexer y la gramática esperan (ej.: `meow meow entero = 10;` o usar la forma sin prefijo según la gramática).
- Para evitar ambigüedades puedes cambiar los literales de palabra clave en `scanner.l` (p. ej. usar `meow_int`) o mejorar la lógica del lexer para detectar contextos.

**Depuración y desarrollo**
- Para depurar la tabla de símbolos, activa `printf` de debug en `symtab.c` o exporta `MEOW_DEBUG` para activar la trazabilidad del parser.
- Usa `valgrind` para detectar fugas de memoria:
```bash
valgrind --leak-check=full ./meowc type_check.meow
```

**Compilar para generar el archivo `.txt`**

1. Compila el compilador `meowc` (solo es necesario cuando cambias código fuente):
2. Ejecuta el compilador sobre un archivo `meowc` y redirige la salida a un `.txt`
```bash
   make
./meowc examples/opcion_c_marquee.meow > opcion_c_marquee.txt

```
3.El compilador te pedirá en consola `stderr`el mensaje del letrero:
Ingresa el mensaje del letrero (solo 0-9 . $):

Escribe el mensaje (por ejemplo 27.9$) y presiona Enter.

El programa FIS-25 generado se imprime en stdout y queda guardado en el `.txt` que especificaste en la redirección.