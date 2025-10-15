#include "backup.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <filesystem>


namespace fs = std::filesystem;

ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino) {
    // CASO DE DECISAO 2: HD existe (sim), PD nao existe (nao) -> ACAO: COPIAR
    if (!fs::exists(destino)) {
        try {
            // fs::copy ira criar o arquivo e sobrescrever se ja existir (o que nao e o caso aqui)
            fs::copy(origem, destino, fs::copy_options::overwrite_existing);
            
            // Assertiva de saida: Verifica se o arquivo foi criado (requisito do teste)
            assert(fs::exists(destino) && "O arquivo de destino nao foi criado.");
            
            return SUCESSO;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Erro de copia: " << e.what() << std::endl;
            return ERRO_GERAL;
        }
    }
    
    return IGNORAR; 
}
