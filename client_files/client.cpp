#include <iostream>
#include <string>

//Libs para manipulação de tempo
#include <chrono>         
#include <cstring>
#include <cstdlib>
// Libs de rede

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// libs de threads,sincronização e etc
#include <mutex>          
#include <condition_variable> 
#include <unistd.h>       
#include <sstream>        
#include <pthread.h>     

// headers files do projeto
#include "common.hpp"     
#include "utils.hpp" 
#include "client_context.hpp"  
#include "subservico_descoberta_cli.hpp"  
#include "subservico_network_cli.hpp"
#include "subservico_input_cli.hpp" 

// timeout para espera de ACK (conforme definido na especificação)
const int TIMEOUT_MS = 10;


int main(int argc, char *argv[]) {
    // Validação da linha de comando 
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <porta_udp>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port <= 0) {
        std::cerr << "Porta inválida." << std::endl;
        return 1;
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Erro ao criar o socket." << std::endl;
        return 1;
    }

    // Habilita o broadcast no socket UDP do cliente
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        std::cerr << "Erro ao habilitar broadcast." << std::endl;
        close(sockfd);
        return 1;
    }

    // Inicializa o contexto do cliente
    ClientContext context;
    context.sockfd = sockfd;
    context.current_seqn = 1; // Requisições começam em 1, conforme especificação 
    context.is_waiting_for_ack = false;

    // Começa a descoberta do servidor 
    std::cout << "Iniciando descoberta do servidor na porta " << port << std::endl;
    if (!runDiscovery(context, port)) {
        std::cerr << "Servidor não encontrado. Encerrando." << std::endl;
        close(sockfd);
        return 1;
    }

    // Inicia as threads
    pthread_t network_thread_id;
    pthread_t input_thread_id;

    if (pthread_create(&network_thread_id, NULL, runNetworkReceiver, &context) != 0) {
        std::cerr << "Erro ao criar a thread de rede (saída)." << std::endl;
        return 1;
    }
    
    if (pthread_create(&input_thread_id, NULL, runInputReader, &context) != 0) {
        std::cerr << "Erro ao criar a thread de input (entrada)." << std::endl;
        return 1;
    }

    //  Aguardar o fim
    // A input_thread termina quando o usuario digitar ctrl+c ou ctrl+d
    pthread_join(input_thread_id, NULL);

    shutdown(sockfd, SHUT_RDWR);
    pthread_join(network_thread_id, NULL); 

    close(sockfd);
    std::cout << "Cliente encerrado." << std::endl;
    return 0;
}


