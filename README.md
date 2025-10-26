# INF01151 - Trabalho Prático - Etapa 1
## Sistema de Pagamentos Distribuído (Estilo PIX)

Este projeto é a implementação de um serviço distribuído de transferência de valores entre clientes, simulando um sistema de pagamento similar ao PIX, para a disciplina de Sistemas Operacionais II.

O sistema é desenvolvido em C++ e utiliza comunicação via UDP para a troca de mensagens. A concorrência no servidor e no cliente é gerenciada com Pthreads, e a sincronização de dados é feita com as primitivas da biblioteca padrão do C++ (`std::mutex`, `std::shared_mutex`, `std::condition_variable_any`).

## Pré-requisitos para execução

Este projeto foi desenvolvido para ambientes Linux e requer as seguintes ferramentas para compilação e execução:

- `g++` (com suporte a C++17)
- `make`

## Estrutura de Arquivos

O projeto está organizado em três diretórios principais, com um `Makefile` na raiz para orquestrar a compilação de ambos os executáveis.

A projeto conta com os seguinte diretórios a partir da raíz:
- `server_files/`
    - Contém todos os arquivos `.hpp` e `.cpp` do servidor.
- `client_files/`
    - Contém todos os arquivos `.hpp` e `.cpp` do cliente.
- `shared/`
    - Contém `.hpp` e `.cpp` de funcões que são úteis a ambas aplicações, bem como a definição da estrutura dos pacotes usados na comunição cliente/servidor.
    
## Como Compilar

O `Makefile` na raiz do projeto está configurado para compilar os executáveis `servidor` e `cliente`, colocando-os no diretório raiz.

**Para compilar ambos (servidor e cliente):**

make all

Para compilar ambos (servidor e cliente):
make all (só make também funciona)

para compilar somente o servidor:

make servidor

para compilar somente o cliente:

make cliente

Para limpar todos os executaveis e .o gerados do processo de compilação:

make clean

Como Executar e Usar
Para testar o sistema, será necessário pelo menos dois terminais (um para o servidor e um para o cliente). Para testar a transferência entre clientes, precisará de três terminais.

Iniciar o servidor

No diretório raiz, digite: 

./servidor 4000

Iniciar o cliente no terminal 1 (ip exemplo: 192.168.0.10)

./cliente 4000 

Iniciar o cliente no terminal 2 (ip exemplo: 192.168.0.20)

./cliente 4000

Exemplo de transferência:

No terminal do cliente 1, digite:
192.168.1.20 50  ** transfere R$50 do cliente 1 para o cliente 2 **

Para encerrar o processo dos clientes ou do servidor, basta ctrl+c ou ctrl+d.






