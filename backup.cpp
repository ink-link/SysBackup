#include "backup.hpp"
#include <fstream>
#include <iostream>
#include <cassert>
#include <filesystem>


namespace fs = std::filesystem;

auto get_file_time(const std::string& path) {
    return fs::last_write_time(path);
}

// ==============================================================================
// FUNÇÃO DE LEITURA DE PARÂMETROS
// ==============================================================================

/**
 * @brief Le o arquivo de parametros para determinar quais arquivos devem ser processados.
 * @details Esta funcao verifica a existencia do arquivo de parametros.
 * @param nome_arquivo_parm Nome e caminho do arquivo de parametros (ex: "Backup.parm").
 * @param arquivos_para_processar Vetor que recebera a lista de arquivos.
 * @return ResultadoBackup Codigo de sucesso ou erro (ERRO_ARQUIVO_PARAMETROS_AUSENTE).
 * @pre O nome do arquivo de parametros nao deve ser uma string vazia.
 * @post Se o retorno for ERRO_ARQUIVO_PARAMETROS_AUSENTE, o arquivo nao existe.
 */
ResultadoBackup le_arquivo_parametros(const std::string& nome_arquivo_parm, 
                                     std::vector<std::string>& arquivos_para_processar) {
    // Assertiva de entrada
    assert(!nome_arquivo_parm.empty() && "O nome do arquivo de parametros nao pode ser vazio.");
                               
    // Implementacao para o Caso de Decisao 1 (ERRO: Backup.parm ausente)
    if (!fs::exists(nome_arquivo_parm)) {
        // Assertiva de saida: O arquivo deve estar ausente se o erro e lancado.
        assert(!fs::exists(nome_arquivo_parm));
        return ERRO_ARQUIVO_PARAMETROS_AUSENTE;
    }
    
    // Lógica para o TESTE DE SUCESSO
    std::ifstream arquivo_parm(nome_arquivo_parm);
    std::string linha;

    if (arquivo_parm.is_open()) {
        while (std::getline(arquivo_parm, linha)) {
            // Remove espaços em branco do início e fim da linha (boas praticas)
            // Para simplicidade, apenas adiciona a linha lida
            if (!linha.empty()) {
                arquivos_para_processar.push_back(linha);
            }
        }
        arquivo_parm.close();
        
        // Assertiva de Saida: Se retornou SUCESSO, o vetor deve ter sido preenchido
        assert(!arquivos_para_processar.empty() && "Vetor deve ser preenchido se o arquivo existe.");
        
        return SUCESSO;
    }
    // Este caso so ocorreria se o arquivo existisse mas nao pudesse ser aberto 
    // (erro de permissao)
    return ERRO_GERAL;
}

ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino, Operacao operacao) {
    // Assertiva de entrada minima
    assert(!origem.empty() && "A string de origem nao pode ser vazia.");
    assert(!destino.empty() && "A string de destino nao pode ser vazia.");

    // Suprime o warning 'unused parameter'
    (void)operacao; 
    
    // ==============================================================================
    // A. LOGICA DE BACKUP (HD -> PD) - OPERACAO: BACKUP (Casos 2, 3, 4, 5)
    // ==============================================================================
    if (operacao == BACKUP) {

        bool origem_existe = fs::exists(origem);
        bool destino_existe = fs::exists(destino);

        // Caso 1 e 6 (Backup): ORIGEM NAO EXISTE.
        if (!origem_existe) {
            // CASO DE DECISÃO 7: HD ausente (F), PD existe (V) -> ACAO: IGNORAR (Faz Nada)
            if (destino_existe) {
                // Assertiva de Saída: Garante que o arquivo no destino nao foi alterado.
                assert(fs::exists(destino) && "Arquivo de destino deveria existir.");
                return IGNORAR;
            }
            // CASO DE DECISÃO 6: HD ausente (F), PD ausente (F) -> ACAO: IGNORAR (Faz Nada, caso nao listado, mas faz sentido)
            else {
                return IGNORAR; 
            }
        }

        // CASO DE DECISAO 2: HD existe (sim), PD nao existe (nao) -> ACAO: COPIAR
        if (!destino_existe) {
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
        if (destino_existe) {
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
                else if (tempo_destino == tempo_origem) {
                    return IGNORAR; 
                }
                
                // CASO DE DECISÃO 5: PD > HD -> ACAO: ERRO
                else { // tempo_destino > tempo_origem
                    return ERRO_ARQUIVO_DESTINO_MAIS_NOVO;
                }

            } catch (const fs::filesystem_error& e) {
                std::cerr << "Erro de comparacao/copia (Caso 3): " << e.what() << std::endl;
                return ERRO_GERAL;
            }
        }
        // CASO DE ERRO 1: Se o HD nao existe E o PD tambem nao (Caso 6)
        if (!origem_existe && !destino_existe) {
            // Se o arquivo foi removido de ambos os lados, nada precisa ser feito.
            return IGNORAR; // A tabela nao lista este caso, mas faz mais sentido IGNORAR
        }
        
        // Ultimo caso de erro: HD existe e PD nao (ja coberto no Caso 2)
        
        // Casos de ERRO FUNDAMENTAL (A Tabela nao lista, mas e um erro de operacao)
        if (!origem_existe && !destino_existe) {
            return ERRO_ARQUIVO_ORIGEM_NAO_EXISTE; // Nao pode copiar o que nao existe
        }
    }

    // ==============================================================================
    // B. LOGICA DE RESTAURACAO (PD -> HD) - OPERACAO: RESTAURACAO (Casos 8, 9, 10, 11)
    // ==============================================================================
    if (operacao == RESTAURACAO) {

        bool origem_existe = fs::exists(origem); // PD
        bool destino_existe = fs::exists(destino); // HD

        (void)origem_existe;

        // Logica para Casos de ERRO (12, 13)
        if (!origem_existe) {
            // CASO 13: PD e HD ausentes OU CASO 12: PD ausente, HD presente
            // Em ambos, a ORIGEM nao existe, o que e um erro critico para a Restauracao.
            // A Tabela exige ERRO para o Caso 12 e o Caso 13
            assert((!destino_existe || get_file_time(destino) == get_file_time(destino)) && "Integridade do destino perdida."); 
            return ERRO_ARQUIVO_ORIGEM_NAO_EXISTE;
        }

        // CASO DE DECISÃO 11: PD existe, HD NAO existe -> ACAO: COPIAR (Restauração Simples)
        if (!destino_existe) {
            try {
                fs::copy(origem, destino, fs::copy_options::overwrite_existing);
                assert(fs::exists(destino) && "Arquivo de destino (HD) nao foi criado na restauracao simples.");
                return SUCESSO;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Erro de copia (Caso 11): " << e.what() << std::endl;
                return ERRO_GERAL;
            }
        }

        // Logica para Restauraçao (PD -> HD):
        if (destino_existe) {
            try {
                auto tempo_origem = get_file_time(origem); // PD
                auto tempo_destino = get_file_time(destino); // HD

                // CASO DE DECISÃO 8: PD < HD -> ACAO: ERRO (PD mais antigo que HD)
                if (tempo_origem < tempo_destino) {
                    // O arquivo de origem (PD) é mais antigo, o destino (HD) é mais novo.
                    assert(fs::exists(destino) && "HD nao deve ter sido removido.");
                    assert(get_file_time(destino) == tempo_destino && "Data do HD nao deve ser alterada.");
                    
                    return ERRO_ARQUIVO_ORIGEM_MAIS_ANTIGO;
                }

                // CASO DE DECISÃO 9: HD existe, PD existe, PD > HD -> ACAO: COPIAR (Restauracao)
                else if (tempo_origem > tempo_destino) {
                    fs::copy(origem, destino, fs::copy_options::overwrite_existing);

                    // Assertiva de saida
                    assert(get_file_time(destino) >= tempo_origem && "A data do HD nao foi atualizada na restauracao.");
                    
                    return SUCESSO;
                }

                // CASO DE DECISÃO 10: PD == HD -> ACAO: IGNORAR
                else if (tempo_origem == tempo_destino) {
                    return IGNORAR;
                }
                
                // Se PD e mais antigo ou igual, a logica sera tratada nos proximos testes (10, 11, 12).
                
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Erro de comparacao/copia (Caso 9 Restaura): " << e.what() << std::endl;
                return ERRO_GERAL;
            }
        }
    }

    return IGNORAR; 
}
