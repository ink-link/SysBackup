#ifndef BACKUP_HPP
#define BACKUP_HPP

#include <string>
#include <filesystem>
#include <cassert>
#include <fstream>
#include <vector>

enum ResultadoBackup {
    SUCESSO = 0,
    IGNORAR = 1,
    ERRO_GERAL = -1,
    ERRO_ARQUIVO_ORIGEM_NAO_EXISTE = -2,
    ERRO_ARQUIVO_DESTINO_MAIS_NOVO = -3,
    ERRO_ARQUIVO_ORIGEM_MAIS_ANTIGO = -4,
    ERRO_ARQUIVO_PARAMETROS_AUSENTE = -5 
};

enum Operacao {
    BACKUP = 0,
    RESTAURACAO = 1
};

// ==============================================================================
// DECLARAÇÕES DE FUNÇÕES DE MÓDULO
// ==============================================================================

// Funções de arquivo (deve ser mantida)
auto get_file_time(const std::string& path); 
ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino, Operacao operacao);
ResultadoBackup le_arquivo_parametros(const std::string& nome_arquivo_parm, 
                                     std::vector<std::string>& arquivos_para_processar);


/**
 * @brief Orquestra o processo de backup/restauracao lendo o arquivo de parametros e
 * chamando faz_backup_arquivo para cada arquivo listado.
 * @param nome_arquivo_parm Nome e caminho do arquivo de parametros (ex: "Backup.parm").
 * @param caminho_origem_base Caminho base de onde os arquivos listados serao copiados.
 * @param caminho_destino_base Caminho base para onde os arquivos serao copiados.
 * @param operacao Define se e BACKUP ou RESTAURACAO.
 * @return ResultadoBackup SUCESSO se todos os arquivos foram processados, ou o primeiro erro.
 * @pre Nao deve haver strings vazias para os caminhos.
 * @post Se SUCESSO, todos os arquivos foram processados sem erros criticos.
 */
ResultadoBackup executa_backup_restauracao(const std::string& nome_arquivo_parm,
                                            const std::string& caminho_origem_base,
                                            const std::string& caminho_destino_base,
                                            Operacao operacao);

ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino, Operacao operacao);

ResultadoBackup le_arquivo_parametros(const std::string& nome_arquivo_parm, 
                                     std::vector<std::string>& arquivos_para_processar);
#endif
