// Define interface para o subserviço de descoberta
#pragma once

#include "server_context.hpp"
#include "common.hpp"
#include <netinet/in.h>

/**
 * @brief Lida com um pacote de DESCOBERTA recebido.
 *
 * Registra o novo cliente na tabela (se for novo) e envia
 * uma resposta de DISCOVERY_ACK.
 *
 * @param ctx  O contexto compartilhado do servidor.
 * @param cli_addr O endereço do cliente que enviou a descoberta.
 * @param pkt O pacote de descoberta recebido (para obter o seqn, se houver).
 */
void handleDiscovery(ServerContext& ctx, struct sockaddr_in& cli_addr, const packet& pkt);
