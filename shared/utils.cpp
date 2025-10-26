#include "utils.hpp"

std::string getCurrentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();


}

std::string ip_to_string(uint32_t ip_host_order) {
    struct in_addr ip_addr;
    
    // Converte o IP de host byte order (como o CPU armazena)
    // para network byte order (como a rede envia)
    ip_addr.s_addr = htonl(ip_host_order); 
    
    // inet_ntoa converte a estrutura de rede (in_addr) para string
    return std::string(inet_ntoa(ip_addr));
}

/**
 * @brief Implementação de ip_string_to_long
 */
uint32_t ip_string_to_long(const std::string& ip_str) {
    struct in_addr addr;
    // inet_aton é mais seguro que inet_addr, mas inet_addr é mais simples
    // inet_addr converte string para network byte order
    if (inet_aton(ip_str.c_str(), &addr) == 0) {
        // Endereço inválido
        return 0;
    }
    // Convertemos de network order para host order para armazenamento
    return ntohl(addr.s_addr);
}