// subservico_interface.h
#pragma once

#include "server_context.hpp"

/**
 * @brief Função principal da thread de interface
 *
 * Esta thread fica bloqueada esperando por atualizações no BankStatus
 *  e as imprime na tela.
 *
 * @param args Um ponteiro para o ServerContext.
 * @return void*
 */
void* runInterface(void* args);