#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <pthread.h> 
#include <iomanip>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "common.hpp"
#include "utils.hpp"
#include "server_context.hpp"
#include "subservico_descoberta.hpp"
#include "subservico_processamento.hpp"
#include "subservico_interface.hpp"



int main(int argc, char *argv[]) {
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
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    // cria socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Erro ao criar o socket." << std::endl;
        return 1;
    }

    // habilita broadcast no socket UDP criado
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        std::cerr << "Erro ao habilitar broadcast." << std::endl;
        close(sockfd);
        return 1;
    }

    // configura endereço IP e porta do servidor
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    std::cout << "Endereço do servidor configurado: " << inet_ntoa(serv_addr.sin_addr) << ":" << ntohs(serv_addr.sin_port) << std::endl;

    // faz a associação do socket com o endereço ip e porta criados.
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Erro ao fazer bind." << std::endl;
        close(sockfd);
        return 1;
    }

   
    
    //  cria o contexto compartilhado por servidor e cliente
    ServerContext context;
    context.sockfd = sockfd; 

    // imprime mensagem de inicialização de acordo com especificação
    std::cout << getCurrentTimestamp() 
              << " num_transactions " << context.bank_status.num_transactions
              << " total_transferred " << context.bank_status.total_transferred
              << " total_balance " << context.bank_status.total_balance 
              << std::endl;

    // cria a thread de interface (somente leitura). 
    pthread_t interface_thread;
    if (pthread_create(&interface_thread, NULL, runInterface, &context) != 0) {
        std::cerr << "Erro ao criar a thread de interface." << std::endl;
        return 1;
    }
   


    std::cout << "Servidor escutando na porta " << port << std::endl;

    // loop principal do servidor
    while (true) {
        packet pkt; 
        cli_len = sizeof(cli_addr);

        ssize_t n = recvfrom(sockfd, &pkt, sizeof(pkt), 0, 
                             (struct sockaddr *)&cli_addr, &cli_len);
        
        if (n < 0) {
            std::cerr << "Erro no recvfrom." << std::endl;
            continue;
        }

        
        pkt.type = ntohs(pkt.type);
        pkt.seqn = ntohl(pkt.seqn);

        // Encaminha para o módulo de serviço correto
        if (pkt.type == PACKET_DISCOVERY) {
            // Lida com a Descoberta na thread principal
            handleDiscovery(context, cli_addr, pkt);

        } else if (pkt.type == PACKET_REQUEST) {
            // Lança uma thread para Processamento 
            
            ProcessingArgs* args = new ProcessingArgs;
            args->ctx = &context;
            args->cli_addr = cli_addr;

            // Copia o pacote para a struct que a thread que será lançada irá receber
            args->pkt = pkt; 
            args->pkt.data.req.dest_addr = ntohl(pkt.data.req.dest_addr);
            args->pkt.data.req.value = ntohl(pkt.data.req.value);


            pthread_t processing_thread;
            if (pthread_create(&processing_thread, NULL, processRequest, args) != 0) {
                std::cerr << "Erro ao criar thread de processamento." << std::endl;
                delete args; // Limpa a memória se a thread falhar
            } else {
                pthread_detach(processing_thread); // Deixa a thread rodar
            }

        } else {
            // Pacote diferente dos que estão mapeados
            std::cerr << "Pacote de tipo desconhecido recebido: " << pkt.type << std::endl;
        }
    }

    close(sockfd);
    return 0;
}