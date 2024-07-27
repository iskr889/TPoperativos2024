#!/bin/bash

# Parámetros
ip_memoria=$1

# Verificar que los parámetros estén presentes
if [ -z "$ip_memoria" ]; then
  echo "Error: faltan parámetros."
  echo "Uso: $0 <ip_memoria>"
  exit 1
fi

# PLANIFICACION
cat <<EOT > planificacion_cpu.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=32
ALGORITMO_TLB=FIFO
EOT

# DEADLOCK
cat <<EOT > deadlock_cpu.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=0
ALGORITMO_TLB=FIFO
EOT

# MEMORIA FIFO
cat <<EOT > memoria_cpu_fifo.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=4
ALGORITMO_TLB=FIFO
EOT

# MEMORIA LRU
cat <<EOT > memoria_cpu_lru.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=4
ALGORITMO_TLB=LRU
EOT

# IO
cat <<EOT > io_cpu.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=0
ALGORITMO_TLB=FIFO
EOT

# FS
cat <<EOT > fs_cpu.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=0
ALGORITMO_TLB=FIFO
EOT

# Salvations
cat <<EOT > salvations_cpu.config
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PUERTO_ESCUCHA_DISPATCH=51512
PUERTO_ESCUCHA_INTERRUPT=51513
CANTIDAD_ENTRADAS_TLB=16
ALGORITMO_TLB=LRU
EOT


{
    echo 'alias planificacion_cpu="cp -f planificacion_cpu.config cpu.config"'
    echo 'alias deadlock_cpu="cp -f deadlock_cpu.config cpu.config"'
    echo 'alias memoria_cpu_fifo="cp -f memoria_cpu_fifo.config cpu.config"'
    echo 'alias memoria_cpu_lru="cp -f memoria_cpu_lru.config cpu.config"'
    echo 'alias io_cpu="cp -f io_cpu.config cpu.config"'
    echo 'alias fs_cpu="cp -f fs_cpu.config cpu.config"'
    echo 'alias salvations_cpu="cp -f salvations_cpu.config cpu.config"'
} >> ~/.bashrc

echo 'Los alias se han añadido a ~/.bashrc. Por favor, recarga el archivo con "source ~/.bashrc" para que los cambios tengan efecto.'