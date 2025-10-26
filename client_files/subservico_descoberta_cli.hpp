#pragma once
#include "client_context.hpp"

/**
 * @brief Descoberta do Servidor. Precisa estar na mesma LAN.
 * Executa a lógica de descoberta de broadcast/timeout/receive
 * para encontrar o servidor na rede.
 *
 * @param ctx O contexto do cliente (será ATUALIZADO com o end. do
 * servidor).
 * @param port A porta na qual enviar o broadcast (passado como parametro via stdin).
 * @return true em caso de sucesso, false se a descoberta falhar.
 */
bool runDiscovery(ClientContext& ctx, int port);