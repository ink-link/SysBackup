#include "backup.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <filesystem>


namespace fs = std::filesystem;

auto get_file_time(const std::string& path) {
    return fs::last_write_time(path);
}

ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino) {
    // Assertiva de entrada minima
    assert(!origem.empty() && "A string de origem nao pode ser vazia.");
    assert(!destino.empty() && "A string de destino nao pode ser vazia.");
    
    // Se a origem nao existe, retorna erro, e os proximos passos sao ignorados.
    if (!fs::exists(origem)) {
        return ERRO_ARQUIVO_ORIGEM_NAO_EXISTE;
    }
    
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
   
    // CASO DE DECISÃO 3: PD existe, PD < HD -> ACAO: COPIAR
    if (fs::exists(destino)) {
        try {
            auto tempo_origem = get_file_time(origem);
            auto tempo_destino = get_file_time(destino);
            
            // Verifica se o arquivo do Pen-drive (destino) e mais antigo que o HD (origem)
            if (tempo_destino < tempo_origem) {
                // Se o PD e mais antigo, copia e sobrescreve (Atualizacao)
                fs::copy(origem, destino, fs::copy_options::overwrite_existing);

                // Assertiva de saida: A data do destino deve ser igual ou superior a origem
                assert(get_file_time(destino) >= tempo_origem && "A data de destino nao foi atualizada.");
                
                return SUCESSO;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Erro de comparacao/copia (Caso 3): " << e.what() << std::endl;
            return ERRO_GERAL;
        }
    }
    return IGNORAR; 
}
