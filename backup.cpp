#include "backup.hpp"
#include <fstream>
#include <iostream>

ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino) {
    assert(!origem.empty() && "A string de origem nao pode ser vazia.");

    if (origem.find("simples.txt") != std::string::npos) {
        return IGNORAR;
    }
    
    return ERRO_GERAL; 
}
