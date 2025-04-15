# **Proyecto de Sincronización: Simulación de Tráfico con Semáforos Inteligentes**  

## **📌 Descripción del Proyecto**  
Simulación de un sistema de tráfico vehicular donde autos **emisores (productores)** y **receptores (consumidores)** intercambian señales acústicas bajo un esquema **Lectores-Escritores** y **Productor-Consumidor**, utilizando:  
- **Semáforos POSIX** para sincronización.  
- **Mutex** para evitar condiciones de carrera.  
- **Buffers circulares** para gestión eficiente de señales.  

**Contexto**: Proyecto académico para *Sistemas Operativos* (Universidad de Carabobo).  

   


## **🎯 Objetivos**  
### **1. Simular interacciones entre:**  
   - **Autos emisores (productores)**: Generan señales de corneta  
   - **Autos receptores (consumidores)**: Procesan señales (límite: 3 por emisor)  
### **2. Implementar mecanismos para:**  
   - Evitar **condiciones de carrera** (`pthread_mutex_t`)  
   - Prevenir **interbloqueos** con semáforos contadores  
   - Gestionar **buffers acotados** (estructura circular)  
### **3. Generar reportes diarios de:**    
   - Multas procesadas  
   - Señales justificadas/injustificadas  
   - Amonestaciones por conductor  
   
## **🧠 ¿Por qué usamos pthreads y semáforos POSIX en este proyecto?**

### **1. Necesidad de Concurrencia**  
El problema requiere simular **múltiples vehículos operando simultáneamente** (emisores/receptores). Los **hilos POSIX (pthreads)** permiten:  
- Modelar cada auto como un hilo independiente  
- Ejecutar operaciones concurrentes (envío/recepción de señales)  
- Maximizar el uso de CPU en sistemas multicore  

## **2.Sincronización de Recursos Compartidos**  
El **buffer de señales** es un recurso crítico compartido entre hilos. Los **semáforos POSIX** resuelven:             
### Control de capacidad del buffer
Los semáforos (`sem_t`) garantizan que ningún auto receptor reciba más de 3 señales del mismo emisor, evitando desbordamientos en el buffer circular. Cuando el buffer está lleno, los productores se bloquean automáticamente (`sem_wait`).

### Exclusión mutua
Aunque el proyecto usa `mutex` para las secciones críticas, los semáforos POSIX complementan:
- Coordinan el acceso entre múltiples hilos a buffers compartidos
- Gestionan dependencias complejas (ej: un consumidor esperando a múltiples productores)

### Evitan interbloqueos (deadlocks)
La implementación con `sem_wait`/`sem_post` asegura que:
- Los productores no saturan el sistema
- Los consumidores siempre liberan recursos

### Sincronización entre adyacentes
Cada auto tiene sus propios semáforos, permitiendo que:
- Señales de múltiples emisores se procesen sin colisiones
- Se mantenga el orden FIFO (primero en entrar, primero en salir) en cada buffer

### Eficiencia en concurrencia
- Permiten bloqueo selectivo: solo los hilos afectados esperan

## **🚀 Instalación y Ejecución**  

### **Requisitos**  
- Compilador de C (gcc).  
- Biblioteca `pthread`.  
- SO Linux (recomendado).  
- GNU Make (para usar el Makefile).  

### **Compilación**  
```bash
make  # Compila el proyecto usando el Makefile
```
## 📊 Resultados
- Registro diario de:
  - Multas procesadas
  - Señales justificadas/injustificadas
  - Amonestaciones por conductor

## 👥 Autores
- **Edgar Gutiérrez** (CI: 28505513)
- **Juan Suárez** (CI: 28083693)
- **Yosismar Arcia** (CI: 27894120

## **📜 Licencia**

Este proyecto se distribuye bajo la licencia **MIT** - un permiso amplio para uso académico y modificaciones.

```text
MIT License

Copyright (c) 2025 Universidad de Carabobo
