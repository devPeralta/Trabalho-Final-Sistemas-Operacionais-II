#pragma once
#include <string>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>  

#include "common.hpp" // Para struct packet

// Struct de contexto para compartilhar o estado entre as threads do lado cliente. 
// O lado do servidor também implementa uma estrutura de contexto análoga

struct ClientContext {
    int sockfd;                       // socket UDP
    struct sockaddr_in serv_addr;     // O endereço do servidor (descoberto)
    std::string serv_ip_str;          // O IP do servidor (recebe como string)
    
    uint32_t current_seqn;            // Próximo ID de requisição a ser enviado
    bool is_waiting_for_ack;          // Flag de controle
    packet last_sent_packet;          // Copia do ultimo pacote para reenviar, caso necessário

    std::mutex mtx;                   // Mutex para proteger todo este estado
    std::condition_variable cv;       // CV para "acordar" a thread de input
};