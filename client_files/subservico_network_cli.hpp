// cliente/subservico_network_cli.hpp
#pragma once

/**
 * @brief Thread Leitor de recebimento de ACKs do Cliente
 * - Fica em loop, recebendo ACKs do servidor e 
 * - Imprime asrespostas formatadas na tela
 *
 * @param args Ponteiro para o ClientContext.
 * @return void*
 */
void* runNetworkReceiver(void* args);