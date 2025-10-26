#pragma once

// Bibliotecas padrão
#include <cstdint> 
#include <string>
// Bibliotecas de rede
#include <netinet/in.h> 
#include <arpa/inet.h>

// Bibliotecas de tempo
#include <ctime>
#include <iomanip>
#include <sstream>

/**
 * @brief Retorna o timestamp atual formatado como "YYYY-MM-DD HH:MM:SS".
 * @return std::string Timestamp formatado.
 */
std::string getCurrentTimestamp();

/**
 * @brief Converte um endereço IP (em host byte order) para uma string.
 * @param ip_host_order O IP como um inteiro (ordem de bytes do host).
 * @return std::string O IP formatado (ex: "192.168.0.1").
 */
std::string ip_to_string(uint32_t ip_host_order);

/**
 * @brief Converte uma string de IP (ex: "192.168.0.1") para um uint32_t
 * em host byte order.
 * @param ip_str A string do IP.
 * @return uint32_t O IP em formato numérico (host byte order).
 */
uint32_t ip_string_to_long(const std::string& ip_str);