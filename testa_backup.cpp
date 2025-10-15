#include "catch_amalgamated.hpp"
#include "backup.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

// Funcao auxiliar para criar e limpar o ambiente de teste
void setup_test_env(const std::string& test_name) {
    fs::remove_all(test_name + "_origem");
    fs::remove_all(test_name + "_destino");
    fs::create_directories(test_name + "_origem");
    fs::create_directories(test_name + "_destino");
}

// Funcao auxiliar para criar um arquivo com conteudo
void create_file(const std::string& path, const std::string& content) {
    std::ofstream ofs(path);
    assert(ofs.is_open());
    ofs << content;
    ofs.close();
}

// ==============================================================================
// TESTE 1: CÓPIA SIMPLES (CASO DE DECISÃO 9: HD EXISTE, PD NÃO EXISTE)
// ==============================================================================

TEST_CASE("Caso de Decisao 2: Copia HD para Pen-drive", "[backup][copia]") {
    const std::string test_name = "test_case_2";
    setup_test_env(test_name);

    const std::string origem_dir = test_name + "_origem";
    const std::string destino_dir = test_name + "_destino";
    const std::string arquivo_nome = "simples.txt";

    const std::string origem_path = origem_dir + "/" + arquivo_nome;
    const std::string destino_path = destino_dir + "/" + arquivo_nome;
    const std::string conteudo = "Conteudo para o primeiro backup.";
    
    // 1. Setup: Cria o arquivo de origem
    create_file(origem_path, conteudo);

    // 2. Execucao: Tenta fazer o backup
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path);
    
    REQUIRE(resultado == SUCESSO);
    REQUIRE(fs::exists(destino_path) == true);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo);
}