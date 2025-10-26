// cliente/subservico_network_cli.cpp
#include "subservico_network_cli.hpp"
#include "client_context.hpp"
#include "common.hpp"
#include "utils.hpp" 

#include <iostream>
#include <cstring>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

// Essa funcao define a thread que fica escutando por pacotes do servidor
void* runNetworkReceiver(void* args) {
    ClientContext* ctx = static_cast<ClientContext*>(args);
    
    packet ack_pkt;
    socklen_t serv_len = sizeof(ctx->serv_addr);

    // Loop infinito para receber pacotes
    while (true) {
        ssize_t n = recvfrom(ctx->sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                             (struct sockaddr*)&ctx->serv_addr, &serv_len);

        if (n <= 0) {
            // Erro ou socket fechado 
            break;
        }

        ack_pkt.type = ntohs(ack_pkt.type);

        // Ignora qualquer pacote que nao seja um ACK de requisição
        if (ack_pkt.type != PACKET_REQUEST_ACK) {
            continue;
        }

        //Início da Seção Crítica
        // Protege o acesso ao estado compartilhado (ctx)
        std::lock_guard<std::mutex> lock(ctx->mtx);

        uint32_t ack_seqn = ntohl(ack_pkt.data.ack.seqn);

        // Verifica se é o ACK esperado
        if (ctx->is_waiting_for_ack && ack_seqn == ctx->current_seqn) {
            
            // Processa o ACK.
            uint32_t new_balance = ntohl(ack_pkt.data.ack.new_balance);

            // Recupera os dados da requisição original para o log
            uint32_t dest_ip_long = ntohl(ctx->last_sent_packet.data.req.dest_addr);
            std::string dest_ip_str = ip_to_string(dest_ip_long);
            uint32_t value = ntohl(ctx->last_sent_packet.data.req.value);

            // Imprime a resposta formatada 
            std::cout << getCurrentTimestamp() 
                      << " server " << ctx->serv_ip_str
                      << " id req " << ack_seqn
                      << " dest " << dest_ip_str
                      << " value " << value
                      << " new_balance " << new_balance
                      << std::endl;

            // Atualiza o estado: prepara para a próxima requisição
            ctx->is_waiting_for_ack = false;
            ctx->current_seqn++; // Incrementa o ID para a próxima

            // Acorda a thread de input (definicao em runInputReader), que está em wait_for()
            ctx->cv.notify_one();
        }
        // Se for um ACK duplicado (ack_seqn < current_seqn) ou
        // inesperado, ele é simplesmente ignorado.
        //Fim da Seção Crítica
    }

    return nullptr;
}