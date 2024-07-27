#!/bin/bash

# Parámetros
ip_kernel=$1
ip_memoria=$2
path_dials=$3

# Verificar que los parámetros estén presentes
if [ -z "$ip_kernel" ] || [ -z "$ip_memoria" ] || [ -z "$path_dials" ]; then
  echo "Error: faltan parámetros."
  echo "Uso: $0 <ip_kernel> <ip_memoria> <path_dials>"
  exit 1
fi

# PLANIFICACION
cat <<EOT > slp1_planificacion.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=50
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

# DEADLOCK
cat <<EOT > espera_deadlock.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=500
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

# MEMORIA
cat <<EOT > io_gen_sleep_memoria.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

# IO
cat <<EOT > generica_io.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > teclado_io.config
TIPO_INTERFAZ=STDIN
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > monitor_io.config
TIPO_INTERFAZ=STDOUT
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

# FS
cat <<EOT > fs_io.config
TIPO_INTERFAZ=DIALFS
TIEMPO_UNIDAD_TRABAJO=2000
BLOCK_SIZE=16
BLOCK_COUNT=32
RETRASO_COMPACTACION=7500
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > teclado_io.config
TIPO_INTERFAZ=STDIN
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > monitor_io.config
TIPO_INTERFAZ=STDOUT
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

# salvations
cat <<EOT > teclado_salvations.config
TIPO_INTERFAZ=STDIN
TIEMPO_UNIDAD_TRABAJO=50
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > monitor_salvations.config
TIPO_INTERFAZ=STDOUT
TIEMPO_UNIDAD_TRABAJO=50
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > generica_salvations.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=250
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > slp1_salvations.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=50
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

cat <<EOT > espera_salvations.config
TIPO_INTERFAZ=GENERICA
TIEMPO_UNIDAD_TRABAJO=500
IP_KERNEL=${ip_kernel}
PUERTO_KERNEL=51514
IP_MEMORIA=${ip_memoria}
PUERTO_MEMORIA=51511
PATH_BASE_DIALFS=${path_dials}
EOT

# Añadir alias al archivo ~/.bashrc
{
    echo '# Alias para manejar archivos de configuración por sección - inicio'
    echo 'alias planificacion_es="cp -f slp1_planificacion.config SLP1.config"'
    echo 'alias deadlock_es="cp -f espera_deadlock.config ESPERA.config"'
    echo 'alias memoria_es="cp -f io_gen_sleep_memoria.config IO_GEN_SLEEP.config"'
    echo 'alias io_es="cp -f generica_io.config GENERICA.config; cp -f teclado_io.config TECLADO.config; cp -f monitor_io.config MONITOR.config"'
    echo 'alias fs_es="cp -f fs_io.config FS.config; cp -f teclado_io.config TECLADO.config; cp -f monitor_io.config MONITOR.config"'
    echo 'alias salvations_es="cp -f teclado_salvations.config TECLADO.config; cp -f monitor_salvations.config MONITOR.config; cp -f generica_salvations.config GENERICA.config; cp -f slp1_salvations.config SLP1.config; cp -f espera_salvations.config ESPERA.config"'
    echo '# Alias para manejar archivos de configuración por sección - fin'
} >> ~/.bashrc



# Informa al usuario que debe recargar su archivo ~/.bashrc
echo 'Los alias se han añadido a ~/.bashrc. Por favor, recarga el archivo con "source ~/.bashrc" para que los cambios tengan efecto.'
