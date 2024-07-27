#!/bin/bash

# Parámetros
ip_memoria=$1
ip_cpu=$2

# Verificar que los parámetros estén presentes
if [ -z "$ip_memoria" ] || [ -z "$ip_cpu" ]; then
  echo "Error: faltan parámetros."
  echo "Uso: $0 <ip_memoria> <ip_cpu>"
  exit 1
fi

# PLANIFICACION FIFO
cat <<EOT > planificacion_kernel_FIFO.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=FIFO
QUANTUM=2750
RECURSOS=[RECURSO]
INSTANCIAS_RECURSOS=[1]
GRADO_MULTIPROGRAMACION=10
EOT

# PLANIFICACION RR
cat <<EOT > planificacion_kernel_RR.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=RR
QUANTUM=2750
RECURSOS=[RECURSO]
INSTANCIAS_RECURSOS=[1]
GRADO_MULTIPROGRAMACION=10
EOT

# PLANIFICACION VRR
cat <<EOT > planificacion_kernel_VRR.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=VRR
QUANTUM=2750
RECURSOS=[RECURSO]
INSTANCIAS_RECURSOS=[1]
GRADO_MULTIPROGRAMACION=10
EOT

# DEADLOCK
cat <<EOT > deadlock_kernel.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=FIFO
QUANTUM=1500
RECURSOS=[RA, RB, RC, RD]
INSTANCIAS_RECURSOS=[1, 1, 1, 1]
GRADO_MULTIPROGRAMACION=10
EOT

# MEMORIA
cat <<EOT > memoria_kernel.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=FIFO
QUANTUM=2000
RECURSOS=[RECURSO]
INSTANCIAS_RECURSOS=[1]
GRADO_MULTIPROGRAMACION=10
EOT

# IO
cat <<EOT > io_kernel.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=RR
QUANTUM=750
RECURSOS=[REC1]
INSTANCIAS_RECURSOS=[1]
GRADO_MULTIPROGRAMACION=10
EOT

# FS
cat <<EOT > fs_kernel.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=RR
QUANTUM=5000
RECURSOS=[REC1]
INSTANCIAS_RECURSOS=[1]
GRADO_MULTIPROGRAMACION=10
EOT

# Salvations
cat <<EOT > salvations_kernel.config
PUERTO_ESCUCHA=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
IP_CPU=${ip_cpu}
PUERTO_CPU_DISPATCH=51512
PUERTO_CPU_INTERRUPT=51513
ALGORITMO_PLANIFICACION=VRR
QUANTUM=500
RECURSOS=[RA, RB, RC, RD]
INSTANCIAS_RECURSOS=[1,1,1,1]
GRADO_MULTIPROGRAMACION=3
EOT


{
    echo 'alias planificacion_kernel_FIFO="cp -f planificacion_kernel_FIFO.config kernel.config"'
    echo 'alias planificacion_kernel_RR="cp -f planificacion_kernel_RR.config kernel.config"'
    echo 'alias planificacion_kernel_VRR="cp -f planificacion_kernel_VRR.config kernel.config"'
    echo 'alias deadlock_kernel="cp -f deadlock_kernel.config kernel.config"'
    echo 'alias memoria_kernel="cp -f memoria_kernel.config kernel.config"'
    echo 'alias io_kernel="cp -f io_kernel.config kernel.config"'
    echo 'alias fs_kernel="cp -f fs_kernel.config kernel.config"'
    echo 'alias salvations_kernel="cp -f salvations_kernel.config kernel.config"'
} >> ~/.bashrc

echo 'Los alias se han añadido a ~/.bashrc. Por favor, recarga el archivo con "source ~/.bashrc" para que los cambios tengan efecto.'