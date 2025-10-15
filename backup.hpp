#ifndef BACKUP_HPP
#define BACKUP_HPP

#include <string>
#include <filesystem>
#include <cassert>

enum ResultadoBackup {
    SUCESSO = 0,
    IGNORAR = 1,
    ERRO_GERAL = -1,
    ERRO_ARQUIVO_ORIGEM_NAO_EXISTE = -2
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
ResultadoBackup faz_backup_arquivo(const std::string& origem, const std::string& destino);

#endif
