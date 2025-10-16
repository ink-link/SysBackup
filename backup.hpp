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

/**
 * @brief
 * * @param origem
 * @param destino
 * @return.
 * * @pre
 * @pre
 * @post
 */
ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino, Operacao operacao);

ResultadoBackup le_arquivo_parametros(const std::string& nome_arquivo_parm, 
                                     std::vector<std::string>& arquivos_para_processar);
#endif
