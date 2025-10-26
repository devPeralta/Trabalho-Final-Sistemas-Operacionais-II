// subservico_interface.cpp

#include "subservico_interface.hpp"
#include "server_context.hpp"
#include "utils.hpp" 

#include <iostream>
#include <mutex> 

// Essa funcao essencialmente adquire o lock de leitura do status do banco e
// aguarda por uma sinalizacao de atualizacao. Quando acordada, ela imprime o status
void* runInterface(void* args) {
    ServerContext* ctx = static_cast<ServerContext*>(args);

    while (true) {
        
        std::unique_lock<std::shared_mutex> lock(ctx->bank_status_mutex);
        ctx->bank_status_cv.wait(lock, [ctx]{ 
            return ctx->bank_status_updated; 
        });

        //thread foi acordada, entao o status do banco foi atualizado
        // imprime o status atualizado
        std::cout << getCurrentTimestamp() 
                  << " Status do banco atualizado:"
                  << " num_transactions " << ctx->bank_status.num_transactions
                  << " total_transferred " << ctx->bank_status.total_transferred
                  << " total_balance " << ctx->bank_status.total_balance
                  << std::endl;

        //Resetar a flag
        ctx->bank_status_updated = false;

    } 

    return nullptr;
}