# Nombre del ejecutable. Deseable en mayusculas
TARGET = RUN
# Bibliotecas incluidas
LIBS = -lm -pthread
# Compilador utilizado, por ejemplo icc, pgcc, gcc
CC = gcc

# Compilacion por defecto
default: $(TARGET)
all: default

# Incluye los archivos .o y .c que estan en el directorio actual
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

# Incluye los archivos.h que estan en el directorio actual
HEADERS = $(vildcard *.h) **/*.h

# compila auntomaticamente solo archivos fuente que se han Modificado
# $< es el primer prerrequisito, generalmente el archivofuente
# $@ nombre del archivo que se esta generando, archivo objeto
%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@

# Preserva archivos intermedios
.PRECIOUS: $(TARGET) $(OBJECTS)

# Enlaza objetos y crea el ejecutable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $@

# Borra archivos .o
clean:
	-rm -f *.o core

# Borra archivos .o y el ejecutable
cleanall: clean 
	-rm -f $(TARGET)
