// Define o contexto (ctx) compartilhado do servidor, incluindo a Tabela de Clientes,
// o Status do Banco, e os mecanismos de sincronização necessários.

#pragma once

#include <map>          
#include <string>
#include <mutex>        
#include <shared_mutex> 
#include <condition_variable> 
#include <netinet/in.h> 

// Estrutura para os dados de um cliente 
struct ClientInfo {
    uint32_t last_req; // Último ID recebido (last_req) 
    uint32_t balance;  // Saldo atual (balance) 
    struct sockaddr_in address; // Endereço completo (IP + Porta) do cliente

    ClientInfo(uint32_t b = 0, uint32_t l = 0, struct sockaddr_in addr = {})
        : last_req(l), balance(b), address(addr) {}
};

// Estrutura para os dados do banco
struct BankStatus {
    uint64_t num_transactions;  // Qtd. transações 
    uint64_t total_transferred; // Valor total transferido 
    uint64_t total_balance;     // Saldo total do banco 

    BankStatus() : num_transactions(0), total_transferred(0), total_balance(0) {}
};

// O Contexto armazena todos os dados compartilhados entre as threads do servidor
// e seus mecanismos de sincronização
struct ServerContext {
    
    // Mapeia um IP (como string) para os dados do cliente
    std::map<std::string, ClientInfo> client_table;
    
    // Mutex para a Tabela de Clientes.
    std::mutex client_table_mutex;

    // Status do Banco 
    BankStatus bank_status;

    // Mutex Leitor/Escritor para o Status do Banco
    // o suberserviço de interface irá apenas ler do banco 
    // o subserviço de processamento e descoberta irão ler/escrever
    std::shared_mutex bank_status_mutex;

    // variavel de condiçao para notificar a interface de que houve alteracao no status do banco
    std::condition_variable_any bank_status_cv;

    // Flag p variavel de condicao
    bool bank_status_updated;

    // O socket do servidor
    int sockfd;

    ServerContext() : bank_status_updated(false), sockfd(-1) {}
};