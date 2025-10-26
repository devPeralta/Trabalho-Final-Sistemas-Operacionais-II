// common.hpp
// Interface usada pelo servidor e cliente
// define formato e campos dos pacotes 
#pragma once

#include <cstdint> 

// Definindo os tipos de pacote conforme especificação
enum PacketType : uint16_t {
    PACKET_DISCOVERY = 1, // Cliente -> Servidor (Broadcast) 
    PACKET_DISCOVERY_ACK, // Servidor -> Cliente (Unicast)
    PACKET_REQUEST,       // Cliente -> Servidor (Unicast) 
    PACKET_REQUEST_ACK    // Servidor -> Cliente (Unicast) 
};

// Estrutura para uma requisição 
struct requisicao {
    uint32_t dest_addr; // Endereço IP do cliente destino 
    uint32_t value;     // Valor da transferência 
};

// Estrutura para um ACK de REQUISIÇÃO
struct requisicao_ack {
    uint32_t seqn;        // Numero de sequência q está sendo feito o ACK
    uint32_t new_balance; // Novo saldo do cliente origem
};

// Estrutura do Pacote principal 
// Usamos #pragma pack(1) para garantir que o compilador não adicione
// "padding" entre os campos, o que corromperia a comunicação em rede.
#pragma pack(push, 1)
struct packet {
    uint16_t type; // Tipo do pacote (DESC | REQ | DESC_ACK | REQ_ACK) 
    uint32_t seqn; // Número de sequência de uma requisição 

    union { 
        struct requisicao req;     // Usado se type == PACKET_REQUEST 
        struct requisicao_ack ack; // Usado se type == PACKET_REQUEST_ACK
    } data;
};
#pragma pack(pop)