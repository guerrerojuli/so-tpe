# Kernel de Sistema Operativo - Grupo 25

## Integrantes
- Agustín Egea (65385), Lorenzo Méndez (65147), Julián Guerrero (65170)

## Descripción del Proyecto

Este proyecto implementa un kernel monolítico de 64 bits con gestión de memoria, scheduling de procesos con prioridades, sincronización mediante semáforos y comunicación entre procesos mediante pipes.

## Instrucciones de Compilación y Ejecución

### Requisitos Previos

- Docker con la imagen del curso:
```bash
docker pull agodio/itba-so-multi-platform:3.0
```

### Compilación

El proyecto soporta dos gestores de memoria intercambiables:

**Para compilar con el gestor de memoria First-Fit (por defecto):**
```bash
make clean
make all
```

**Para compilar con el gestor de memoria Buddy System:**
```bash
make clean
make buddy
```

### Ejecución

**Para ejecutar en QEMU:**
```bash
./run.sh
```

**Para ejecutar con debugging habilitado:**
```bash
./run.sh gdb
```

## Instrucciones de Replicación

### Comandos Disponibles

#### Comandos Básicos

| Comando | Descripción | Parámetros | Ejemplo |
|---------|-------------|------------|---------|
| `help` | Muestra la lista de comandos disponibles | Ninguno | `help` |
| `clear` | Limpia la pantalla | Ninguno | `clear` |
| `mem` | Muestra el estado de la memoria (total, ocupada, libre) | Ninguno | `mem` |

#### Gestión de Procesos

| Comando | Descripción | Parámetros | Ejemplo |
|---------|-------------|------------|---------|
| `ps` | Lista todos los procesos con sus propiedades (PID, prioridad, estado) | Ninguno | `ps` |
| `loop` | Imprime su PID con un mensaje cada N segundos | `<segundos>` | `loop 3` |
| `kill` | Termina un proceso dado su PID | `<pid>` | `kill 5` |
| `nice` | Cambia la prioridad de un proceso | `<pid> <prioridad>` | `nice 5 2` |
| `block` | Bloquea o desbloquea un proceso | `<pid>` | `block 5` |

#### Comandos de IPC y Filtros

| Comando | Descripción | Parámetros | Ejemplo |
|---------|-------------|------------|---------|
| `cat` | Imprime stdin tal como lo recibe | Ninguno | `cat` |
| `wc` | Cuenta el número de líneas en la entrada | Ninguno | `wc` |
| `filter` | Filtra las vocales de la entrada | Ninguno | `filter` |
| `mvar` | Implementa el problema de múltiples lectores/escritores | `<num_escritores> <num_lectores>` | `mvar 2 3` |

#### Tests del Sistema

| Test | Descripción | Parámetros | Ejemplo |
|------|-------------|------------|---------|
| `test_mm` | Test del gestor de memoria | `<memoria_max_bytes>` | `test_mm 1048576` |
| `test_processes` | Test de creación y gestión de procesos | `<max_procesos>` | `test_processes 10` |
| `test_synchro` | Test de sincronización con semáforos | `<num_procesos>` | `test_synchro 5` |
| `test_no_synchro` | Test sin sincronización (demuestra race conditions) | `<num_procesos>` | `test_no_synchro 5` |

### Caracteres Especiales

#### Operador de Pipe (`|`)
Conecta la salida de un comando con la entrada de otro.

**Sintaxis:**
```bash
comando1 | comando2
```

**Ejemplos:**
```bash
cat | wc        # Cuenta las líneas ingresadas por teclado
cat | filter    # Filtra las vocales del texto ingresado
```

**Nota:** Solo se soporta el encadenamiento de 2 comandos (no se permite `cmd1 | cmd2 | cmd3`).

#### Operador de Background (`&`)
Ejecuta un comando en segundo plano, devolviendo inmediatamente el control al shell.

**Sintaxis:**
```bash
comando &
```

**Ejemplos:**
```bash
loop 5 &                    # Ejecuta loop en background
test_processes 10 &         # Ejecuta el test en background
```

### Atajos de Teclado

| Atajo | Acción | Descripción |
|-------|--------|-------------|
| `Ctrl+C` | Interrumpir ejecución | Termina el proceso en foreground actual |
| `Ctrl+D` | Enviar EOF | Envía señal de fin de archivo al proceso en foreground |

## Ejemplos de Uso

### Ejemplo 1: Gestión de Procesos
```bash
# Crear procesos en background
loop 2 &
loop 3 &
loop 5 &

# Ver los procesos en ejecución
ps

# Cambiar la prioridad de un proceso
nice 3 4

# Bloquear un proceso
block 2

# Desbloquear el proceso
block 2

# Terminar un proceso
kill 3
```

### Ejemplo 2: Uso de Pipes
```bash
# Contar líneas de entrada
cat | wc
Hola
Mundo
Test
[Ctrl+D]
# Output: 3

# Filtrar vocales
cat | filter
Hello World
[Ctrl+D]
# Output: Hll Wrld
```

### Ejemplo 3: Test de Memoria
```bash
# Ejecutar test con 2MB de memoria máxima
test_mm 2097152

# Ver el estado de la memoria después del test
mem
```

### Ejemplo 4: Test de Sincronización
```bash
# Test con semáforos (debe resultar en 0)
test_synchro 4 1000

# Test sin semáforos (demuestra race conditions)
test_no_synchro 4 1000
```

### Ejemplo 5: MVar (Múltiples Lectores/Escritores)
```bash
# 2 escritores y 3 lectores
mvar 2 3

# Observar el comportamiento: los escritores escriben 'A' y 'B',
# los lectores consumen los valores alternadamente
```

## Características Implementadas

- ✅ **Gestión de Memoria**: Dos gestores intercambiables (First-Fit y Buddy System)
- ✅ **Procesos y Scheduling**: Round Robin con prioridades, scheduling preemptivo
- ✅ **Sincronización**: Semáforos sin busy-waiting
- ✅ **IPC**: Pipes unidireccionales con bloqueo
- ✅ **Shell**: Soporte para pipes (`|`) y procesos en background (`&`)
- ✅ **Interrupciones de Teclado**: Ctrl+C para interrumpir, Ctrl+D para EOF

## Limitaciones Conocidas

- Los pipes solo soportan el encadenamiento de 2 comandos
- La función de sleep (utilizada en el comando loop) hace busy waiting, utilizando _hlt() que lo hace más eficiente en términos de ejecuciones por segundo, pero sin embargo al correr un loop en background la shell comienza a funcionar más lento debido a esto. Lo óptimo sería tener una estructura separada para los procesos que están haciendo sleep, y consultarlos periódicamente si ya pasó el suficiente tiempo para levantarlos. No llegamos con el tiempo a implementar esto.
- Nuestra shell no imprime con distintos colores, por lo tanto en el comando MVAR utilizamos el id como identificador.

## Notas de Implementación

### Gestores de Memoria
- **First-Fit**: Lista libre circular con nodo centinela, asigna bloques de tamaño variable en unidades alineadas y coalescea automáticamente bloques adyacentes al liberar
- **Buddy System**: Bloques de tamaño potencia de 2, división y coalescencia automática

### Scheduler
- Implementa Round Robin con 5 niveles de prioridad (0-4)
- Mayor prioridad = más tiempo de CPU

### Semáforos
- Implementados usando instrucciones atómicas (`XCHG`)
- Los procesos bloqueados no consumen CPU
- Semáforos nombrados accesibles por identificador

### Pipes
- Buffer circular
- Operaciones de lectura/escritura con bloqueo
- Integrados con la tabla de descriptores de archivo

## Referencias

- Material del curso de Sistemas Operativos - ITBA
- OSDev Wiki: https://wiki.osdev.org/

## Uso de IA

- Claude Code CLI y Cursor Desktop fueron utilizados durante el desarrollo del TP, mayoritariamente para planning y diseño de implementaciones, y en menor medida para generación de código.