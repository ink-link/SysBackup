#ifndef BACKUP_HPP
#define BACKUP_HPP

#include <string>
#include <filesystem>
#include <cassert>
#include <fstream>

enum ResultadoBackup {
    SUCESSO = 0,
    IGNORAR = 1,
    ERRO_GERAL = -1,
    ERRO_ARQUIVO_ORIGEM_NAO_EXISTE = -2,
    ERRO_ARQUIVO_DESTINO_MAIS_NOVO = -3 
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

#endif
