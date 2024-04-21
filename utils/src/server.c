
#include "server.h"

int handshake_con_servidor(int socket_servidor) {
	int32_t handshake = 1;
	int32_t result;

	if(send(socket_servidor, &handshake, sizeof(int32_t), 0) < 0)
		return ERROR;
	if(recv(socket_servidor, &result, sizeof(int32_t), MSG_WAITALL))
		return ERROR;

	return result; // Retorna 0 si el handshake es correcto
}