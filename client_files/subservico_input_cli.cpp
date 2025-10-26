// cliente/subservico_input_cli.cpp
#include "subservico_input_cli.hpp"
#include "client_context.hpp"
#include "common.hpp"
#include "utils.hpp" // Para ip_string_to_long()

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>   
#include <cstring>  
#include <sys/socket.h> 
#include <netinet/in.h>

// O timeout para reenvio de requisições, conforme sugerido na especificação
const int REQUEST_TIMEOUT_MS = 10;

//Note q essa funcao somente implementa o envio de requisições e controle de timeout
// a funcao de network que implementa o recebimendo do ack pelo servidor e acorda a thread
//que lançou essa funcao.
void* runInputReader(void* args) {
    ClientContext* ctx = static_cast<ClientContext*>(args);
    
    std::string line;
    // O loop principal le da entrada padrão (std::cin)
    while (std::getline(std::cin, line)) {
        std::string dest_ip_str;
        uint32_t value;

        // Parseia o comando "IP_DESTINO VALOR" 
        std::stringstream parsed_string(line);
        if (!(parsed_string >> dest_ip_str >> value)) {
            // Comando invalido, ignora e pede o próximo
            continue;
        }

        // Converte o IP de string para uint32_t
        uint32_t dest_ip_long = ip_string_to_long(dest_ip_str); // funcao definida em utils.hpp
        if (dest_ip_long == 0) {
            std::cerr << "Endereço IP de destino invalido." << std::endl;
            continue;
        }

        // Seção Crítica
        std::unique_lock<std::mutex> lock(ctx->mtx); // adquire o mutex do ctx compartilhado

        // Zera o conteudo do pacote e preenche os campos
        std::memset(&ctx->last_sent_packet, 0, sizeof(packet));
        ctx->last_sent_packet.type = htons(PACKET_REQUEST);
        ctx->last_sent_packet.seqn = htonl(ctx->current_seqn);
        ctx->last_sent_packet.data.req.dest_addr = htonl(dest_ip_long);
        ctx->last_sent_packet.data.req.value = htonl(value);

        // Define a flag de espera por ACK
        ctx->is_waiting_for_ack = true;

        // Loop de retransmissão por timeout de 10ms 
        // A especificação do trabalho define q nenhuma nova requisição deve ser enviada
        // até receber o ack da requisição anterior
        while (ctx->is_waiting_for_ack) {
            // Envia a requisição para o servidor 
            sendto(ctx->sockfd, &ctx->last_sent_packet, sizeof(packet), 0,
                   (struct sockaddr*)&ctx->serv_addr, sizeof(ctx->serv_addr));

            // Aguarda por um timeout ou uma notificação
            auto timeout = std::chrono::milliseconds(REQUEST_TIMEOUT_MS);
            auto status = ctx->cv.wait_for(lock, timeout);

            if (status == std::cv_status::timeout) {
               // Timeout
               // loop ira reenviar a requisicão
            } else {
                // Recebeu o ack do servidor
                
            }
        }
        //  Fim da Seção Crítica 
    }

    return nullptr;
}