#!/bin/bash

path_instrucciones=$1

# Verificar que los parámetros estén presentes
if [ -z "$path_instrucciones" ]; then
  echo "Error: faltan parámetros."
  echo "Uso: $0 <path_instrucciones>"
  exit 1
fi

# PLANIFICACION
cat <<EOT > planificacion_memoria.config
PUERTO_ESCUCHA=51511
TAM_MEMORIA=1024
TAM_PAGINA=32
PATH_INSTRUCCIONES=${path_instrucciones}
RETARDO_RESPUESTA=1000
EOT

# DEADLOCK
cat <<EOT > deadlock_memoria.config
PUERTO_ESCUCHA=51511
TAM_MEMORIA=1024
TAM_PAGINA=32
PATH_INSTRUCCIONES=${path_instrucciones}
RETARDO_RESPUESTA=1000
EOT

# MEMORIA
cat <<EOT > memoria_memoria.config
PUERTO_ESCUCHA=51511
TAM_MEMORIA=1024
TAM_PAGINA=32
PATH_INSTRUCCIONES=${path_instrucciones}
RETARDO_RESPUESTA=1000
EOT

# IO
cat <<EOT > io_memoria.config
PUERTO_ESCUCHA=51511
TAM_MEMORIA=1024
TAM_PAGINA=16
PATH_INSTRUCCIONES=${path_instrucciones}
RETARDO_RESPUESTA=100
EOT

# FS
cat <<EOT > fs_memoria.config
PUERTO_ESCUCHA=51511
TAM_MEMORIA=1024
TAM_PAGINA=16
PATH_INSTRUCCIONES=${path_instrucciones}
RETARDO_RESPUESTA=100
EOT

# Salvations
cat <<EOT > salvations_memoria.config
PUERTO_ESCUCHA=51511
TAM_MEMORIA=4096
TAM_PAGINA=32
PATH_INSTRUCCIONES=${path_instrucciones}
RETARDO_RESPUESTA=100
EOT

{
    echo 'alias planificacion_memoria="cp -f planificacion_memoria.config memoria.config"'
    echo 'alias deadlock_memoria="cp -f deadlock_memoria.config memoria.config"'
    echo 'alias memoria_memoria="cp -f memoria_memoria.config memoria.config"'
    echo 'alias io_memoria="cp -f io_memoria.config memoria.config"'
    echo 'alias fs_memoria="cp -f fs_memoria.config memoria.config"'
    echo 'alias salvations_memoria="cp salvations_memoria.config memoria.config"'
} >> ~/.bashrc

echo 'Los alias se han añadido a ~/.bashrc. Por favor, recarga el archivo con "source ~/.bashrc" para que los cambios tengan efecto.'