// subservico_processamento.cpp

#include "subservico_processamento.hpp"
#include "common.hpp"
#include "server_context.hpp"
#include "utils.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h> 




// Funcao de processamento de requisicao. 
// Cada requisição recebida irá lançar uma thread que executa esta função.

 
void* processRequest(void* args) {
    
    ProcessingArgs* req_args = static_cast<ProcessingArgs*>(args);
    
    
    ServerContext& ctx = *req_args->ctx;
    struct sockaddr_in cli_addr = req_args->cli_addr;
    packet pkt = req_args->pkt;
    
    // Libera a memória alocada para os argumentos pela thread principal
    // usa a copia feita localmente
    delete req_args;


    std::string origin_ip_str = inet_ntoa(cli_addr.sin_addr);
    std::string dest_ip_str = ip_to_string(pkt.data.req.dest_addr);

    // Variáveis para a resposta ACK
    uint32_t ack_seqn;
    uint32_t ack_balance;

    bool log_as_duplicate = false;
    bool do_process_transaction = false;

    // Inicio da Sessão Crítica 
    {
        // Mutex p/ escrever de forma exclusiva na tabela de clientes
        std::lock_guard<std::mutex> client_lock(ctx.client_table_mutex);

        // Valida se cliente que fez a requisição existe na tabela
        auto it_origin = ctx.client_table.find(origin_ip_str);
        if (it_origin == ctx.client_table.end()) {
            std::cerr << "[" << getCurrentTimestamp() 
                      << "] Erro: Requisição de cliente desconhecido: " 
                      << origin_ip_str << std::endl;
            return nullptr;
        }

        // Valida se cliente destino da requisição existe na tabela
        auto it_dest = ctx.client_table.find(dest_ip_str);
        bool dest_exists = (it_dest != ctx.client_table.end());

        // carrega valor esperado e recebido de seqn, bem como valor da transferência
        uint32_t expected_seqn = it_origin->second.last_req + 1;
        uint32_t received_seqn = pkt.seqn;
        uint32_t transfer_value = pkt.data.req.value;

        if (received_seqn == expected_seqn) {
            // Número de seqn correto 
            
            // Destino existe e saldo suficiente?
            if (dest_exists && it_origin->second.balance >= transfer_value) {
                // sim. processa a transferencia
        
                do_process_transaction = true;
                
                // Atualiza a tabela de clientes
                it_origin->second.balance -= transfer_value;
                it_dest->second.balance += transfer_value;
                it_origin->second.last_req = received_seqn; // Avança o seqn

            } else {
                // Não. Saldo insuficiente ou destino não existe 
                // A transação não ocorre, mas avançamos o número de
                // sequência para que o cliente não fique preso
                // reenviando uma transação que sempre falhará.
                it_origin->second.last_req = received_seqn;
            }
        
        } else if (received_seqn <= it_origin->second.last_req) {
            // Requisição duplicada 
            log_as_duplicate = true;
        
        } else {
            //  Pacote perdido (received > expected) 
            log_as_duplicate = true; 
        }

        // 
        if (do_process_transaction) {
            // Lock exclusivo p/ atualizar o status do banco
            std::unique_lock<std::shared_mutex> bank_lock(ctx.bank_status_mutex);

            ctx.bank_status.num_transactions++;
            ctx.bank_status.total_transferred += transfer_value;
            
            
            // Imprime o log de sucesso 
            std::cout << getCurrentTimestamp() 
                      << " client " << origin_ip_str
                      << " id req " << received_seqn
                      << " dest " << dest_ip_str
                      << " value " << transfer_value
                      << " num_transactions " << ctx.bank_status.num_transactions
                      << " total_transferred " << ctx.bank_status.total_transferred
                      << " total_balance " << ctx.bank_status.total_balance
                      << std::endl;

            // Notifica a thread de interface
            ctx.bank_status_updated = true;
            bank_lock.unlock(); // Libera o lock antes de notificar
            ctx.bank_status_cv.notify_one();

        } else if (log_as_duplicate) {
            // DUP
            //Nesse caso precisamos de um lock compartilhado (operacao de leitura).
            
            std::shared_lock<std::shared_mutex> bank_lock(ctx.bank_status_mutex);

            // Imprime o log de DUP
            std::cout << getCurrentTimestamp() 
                      << " client " << origin_ip_str
                      << " DUP!! id req " << received_seqn
                      << " dest " << dest_ip_str
                      << " value " << transfer_value
                      << " num_transactions " << ctx.bank_status.num_transactions
                      << " total_transferred " << ctx.bank_status.total_transferred
                      << " total_balance " << ctx.bank_status.total_balance
                      << std::endl;
        }

        // Prepara o ACK
        // O ACK Deve refletir o último estado processado do cliente que está na tabela. 
        
        ack_seqn = it_origin->second.last_req;
        ack_balance = it_origin->second.balance;

    } 

    packet ack_pkt;
    std::memset(&ack_pkt, 0, sizeof(ack_pkt));
    
    ack_pkt.type = htons(PACKET_REQUEST_ACK);
    
    // O ACK contém o seqn que está sendo confirmado e o novo 
    
    ack_pkt.data.ack.seqn = htonl(ack_seqn);
    ack_pkt.data.ack.new_balance = htonl(ack_balance);
    // envia o ACK de volta ao cliente
    sendto(ctx.sockfd, &ack_pkt, sizeof(ack_pkt), 0,
           (struct sockaddr*)&cli_addr, sizeof(cli_addr));

    return nullptr;
}