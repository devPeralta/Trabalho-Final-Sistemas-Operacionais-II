// subservico_processamento.h
#pragma once

#include "server_context.hpp"
#include "common.hpp"
#include <netinet/in.h>

// Argumentos para a thread de processamento
// É preciso fazer uma copia do pacote q recebemos na main, uma vez q o buffer será reutilizado
// para as requisições que chegarem.
struct ProcessingArgs {
    ServerContext* ctx;
    struct sockaddr_in cli_addr;
    packet pkt;
    
};

/**
 * @brief Função principal da thread de processamento de requisição.
 *
 * Esta função será executada por uma nova thread para cada requisição
 * recebida
 *
 * @param args Um ponteiro para uma struct ProcessingArgs.
 * @return void*
 */
void* processRequest(void* args);