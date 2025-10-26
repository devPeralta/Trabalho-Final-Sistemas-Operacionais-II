// subservico_descoberta.cpp

#include "subservico_descoberta.hpp"
#include "common.hpp"
#include "server_context.hpp"
#include "utils.hpp"

#include <iostream>
#include <string>
#include <cstring>      
#include <arpa/inet.h>  
#include <sys/socket.h> 

// Define saldo inicial para novos clientes
const uint32_t INITIAL_BALANCE = 100;

 // Essa função é chamada pela thread principal do server.cpp
 // verifica se o cliente já é conhecido, se não for, registra na tabela de clientes
 // com saldo inicia e atualiza o saldo total do banco
void handleDiscovery(ServerContext& ctx, struct sockaddr_in& cli_addr, const packet& pkt) {
    
    std::string client_ip = inet_ntoa(cli_addr.sin_addr);

    bool is_new_client = false;  

    {
        // Esse lock protege o acesso à tabela de clientes, tornando a operação atomica
        std::lock_guard<std::mutex> lock(ctx.client_table_mutex);

        // verificia se o cliente já existe em client_table
        if (ctx.client_table.find(client_ip) == ctx.client_table.end()) {
            
            is_new_client = true;

            ClientInfo new_client(INITIAL_BALANCE, 0, cli_addr);
            ctx.client_table[client_ip] = new_client;

            // atualiza saldo no banco usando um mutex unico para escrita, mas ainda permite a leitura por outras threads
            {
                std::unique_lock<std::shared_mutex> bank_lock(ctx.bank_status_mutex);

                ctx.bank_status.total_balance += INITIAL_BALANCE;
                ctx.bank_status_updated = true;
            }
            
            // Notifica a thread de interface que houve uma atualizacao no banco
            ctx.bank_status_cv.notify_one();
        }
    } // fim do lock de client_table

    if (is_new_client) {
         std::cout << "[" << getCurrentTimestamp() << "] Novo cliente registrado: " 
                   << client_ip << ". Saldo inicial: " << INITIAL_BALANCE << std::endl;
    }

    // prepara pacote com ack e envia para o cliente
    
    packet ack_pkt;
    std::memset(&ack_pkt, 0, sizeof(ack_pkt));

    ack_pkt.type = htons(PACKET_DISCOVERY_ACK);
    ack_pkt.seqn = htonl(0); //ack de descoberta n precisa de seqn

    // Usamos cli_addr, que contém o IP e a porta do cliente
    sendto(ctx.sockfd, &ack_pkt, sizeof(ack_pkt), 0,
           (struct sockaddr*)&cli_addr, sizeof(cli_addr));
}

