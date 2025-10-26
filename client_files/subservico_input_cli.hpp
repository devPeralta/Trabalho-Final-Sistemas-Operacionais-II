// cliente/subservico_input_cli.hpp
#pragma once

/**
 * @brief Thread Leitor de Input do Cliente
 * - Fica em loop, lendo comandos do stdin (formato "IP_DESTINO VALOR")
 * - Envia requisições de transação ao servidor
 * - Gerencia o envio e retransmissão por timeout.
 *
 * @param args Ponteiro para o ClientContext.
 * @return void*
 */

void* runInputReader(void* args);