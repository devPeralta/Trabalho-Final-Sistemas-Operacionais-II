// cliente/subservico_descoberta_cli.cpp
#include "subservico_descoberta_cli.hpp"
#include "common.hpp" 
#include "utils.hpp"  

#include <iostream>
#include <cstring>      
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  
#include <unistd.h>     
#include <cerrno>      

// Define o timeout de descoberta (1seg)
const int DISCOVERY_TIMEOUT_SEC = 1;

bool runDiscovery(ClientContext& ctx, int port) {

    struct timeval tv;
    tv.tv_sec = DISCOVERY_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(ctx.sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "Erro ao configurar o timeout de descoberta." << std::endl;
        return false;
    }

    
    packet disc_pkt; // pacote de descoberta
    std::memset(&disc_pkt, 0, sizeof(disc_pkt));
    disc_pkt.type = htons(PACKET_DISCOVERY);
    disc_pkt.seqn = htonl(0); 


    //  "255.255.255.255" para broadcast
    struct sockaddr_in broadcast_addr;
    std::memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    // Converte a string "255.255.255.255" para o formato de rede
    if (inet_pton(AF_INET, "255.255.255.255", &broadcast_addr.sin_addr) <= 0) {
        std::cerr << "Erro ao configurar o endereco de broadcast." << std::endl;
        return false;
    }

    packet ack_pkt;
    struct sockaddr_in temp_serv_addr;
    socklen_t serv_len = sizeof(temp_serv_addr);

    //  Loop de Descoberta (Enviar broadcast, esperar resposta)
    while (true) {
        // Envia o pacote de descoberta em broadcast
        if (sendto(ctx.sockfd, &disc_pkt, sizeof(disc_pkt), 0,
                   (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
            std::cerr << "Erro ao enviar broadcast de descoberta." << std::endl;
            return false;
        }

        // Aguarda a resposta do servidor, com timeout (1 segundo).  
        ssize_t n = recvfrom(ctx.sockfd, &ack_pkt, sizeof(ack_pkt), 0,
                             (struct sockaddr*)&temp_serv_addr, &serv_len);

        if (n < 0) {
            // Verifica se o erro foi um timeout
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Timeout na descoberta, tentando novamente" << std::endl;
                continue; // Reenvia pacote de descoberta
            } else {
                std::cerr << "Erro no recvfrom durante a descoberta." << std::endl;
                return false;
            }
        }

        
        ack_pkt.type = ntohs(ack_pkt.type);

        if (ack_pkt.type == PACKET_DISCOVERY_ACK) {
            // Recebemos o ACK de descoberta do servidor
            
            ctx.serv_addr = temp_serv_addr; // salva o end. do servidor no ctx
            ctx.serv_ip_str = inet_ntoa(ctx.serv_addr.sin_addr);

            std::cout << getCurrentTimestamp() << " server_addr " << ctx.serv_ip_str << std::endl;
            break; 
        }
        
    }
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(ctx.sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    return true; 
}