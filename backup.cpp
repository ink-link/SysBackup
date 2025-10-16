#include "backup.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <filesystem>


namespace fs = std::filesystem;

auto get_file_time(const std::string& path) {
    return fs::last_write_time(path);
}

/**
 * @brief Implementa a logica de decisao e copia para um unico arquivo.
 * * @details Esta funcao avalia as condicoes de existencia e data entre origem (HD) e destino (Pen-drive)
 * para decidir se o arquivo deve ser copiado, atualizado ou ignorado.
 * * @param origem Caminho completo do arquivo de origem (HD).
 * @param destino Caminho completo para o arquivo de destino (Pen-drive).
 * @return ResultadoBackup Codigo de sucesso ou erro (0, 1, -1, -2, etc.).
 * * @pre A string 'origem' e 'destino' nao devem ser vazias.
 * @pre O caminho da origem e destino devem ser sintaticamente validos.
 * * @post Se a funcao retornar SUCESSO, o arquivo deve existir em 'destino' e ter o mesmo
 * conteudo e a data de modificacao deve ser maior ou igual a data de origem.
 */

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
            
            // CASO DE DECISÃO 4: PD == HD -> ACAO: IGNORAR (Datas Iguais)
            if (tempo_destino == tempo_origem) {
                // Retorno explicito para o caso onde as datas sao identicas (Coluna 4)
                // Nao e necessaria copia.
                return IGNORAR; 
            }

        } catch (const fs::filesystem_error& e) {
            std::cerr << "Erro de comparacao/copia (Caso 3): " << e.what() << std::endl;
            return ERRO_GERAL;
        }
    }
    return IGNORAR; 
}
