#include "catch_amalgamated.hpp"
#include "backup.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <chrono>

namespace fs = std::filesystem;

void setup_test_env(const std::string& test_name) {
    fs::remove_all(test_name + "_origem");
    fs::remove_all(test_name + "_destino");
    fs::create_directories(test_name + "_origem");
    fs::create_directories(test_name + "_destino");
}

void create_file(const std::string& path, const std::string& content) {
    std::ofstream ofs(path);
    assert(ofs.is_open());
    ofs << content;
    ofs.close();
}

void setup_test_env(const std::string& test_name);

void set_file_time(const std::string& path, const fs::file_time_type& new_time) {
    fs::last_write_time(path, new_time);
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
    
    create_file(origem_path, conteudo);

    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, BACKUP);
    
    REQUIRE(resultado == SUCESSO);
    REQUIRE(fs::exists(destino_path) == true);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo);
}

// ==============================================================================
// TESTE 2: ATUALIZACAO DE BACKUP (CASO DE DECISÃO 3: PD < HD)
// ==============================================================================

TEST_CASE("Caso de Decisao 3: Atualiza Backup (PD mais antigo que HD)", "[backup][data]") {
    const std::string test_name = "test_case_3";
    setup_test_env(test_name);

    const std::string origem_dir = test_name + "_origem";
    const std::string destino_dir = test_name + "_destino";
    const std::string arquivo_nome = "atualizar.txt";

    const std::string origem_path = origem_dir + "/" + arquivo_nome;
    const std::string destino_path = destino_dir + "/" + arquivo_nome;
    
    create_file(origem_path, "Novo Conteudo");
    
    create_file(destino_path, "Conteudo Antigo");

    auto tempo_antigo = fs::file_time_type::clock::now() - std::chrono::hours(24);
    set_file_time(destino_path, tempo_antigo);
    
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, BACKUP);
    
    REQUIRE(resultado == SUCESSO);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == "Novo Conteudo");
}

// ==============================================================================
// TESTE 4: IGNORAR ATUALIZACAO (CASO DE DECISÃO 4: Datas Iguais)
// ==============================================================================

TEST_CASE("Caso de Decisao 4: Ignora Backup (Datas Iguais)", "[backup][ignorar][data]") {
    const std::string test_name = "test_case_4_equal";
    setup_test_env(test_name); // Assume-se que esta funcao limpa e cria os dirs

    const std::string origem_dir = test_name + "_origem";
    const std::string destino_dir = test_name + "_destino";
    const std::string arquivo_nome = "ignorar_igual.txt";

    const std::string origem_path = origem_dir + "/" + arquivo_nome;
    const std::string destino_path = destino_dir + "/" + arquivo_nome;
    const std::string conteudo_original = "Conteudo inalterado";
    
    create_file(origem_path, conteudo_original);
    create_file(destino_path, conteudo_original);
    
    auto tempo_atual = fs::file_time_type::clock::now();
    set_file_time(origem_path, tempo_atual);
    set_file_time(destino_path, tempo_atual); // Define a mesma data
    
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, BACKUP);
    
    REQUIRE(resultado == IGNORAR);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo_original);
}

// ==============================================================================
// TESTE 5: ERRO POR DATA (CASO DE DECISÃO 5: PD > HD)
// ==============================================================================

TEST_CASE("Caso de Decisao 5: ERRO - Destino e mais novo que Origem", "[backup][erro][data]") {
    const std::string test_name = "test_case_5_error_newer_pd";
    setup_test_env(test_name); 

    const std::string origem_dir = test_name + "_origem";
    const std::string destino_dir = test_name + "_destino";
    const std::string arquivo_nome = "erro_novo.txt";

    const std::string origem_path = origem_dir + "/" + arquivo_nome;
    const std::string destino_path = destino_dir + "/" + arquivo_nome;
    
    create_file(destino_path, "Versao mais recente no Pen-drive");
    create_file(origem_path, "Versao Antiga no HD");
    
    auto tempo_antigo = fs::file_time_type::clock::now() - std::chrono::hours(48);
    set_file_time(origem_path, tempo_antigo);
    
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, BACKUP);
    
    REQUIRE(resultado == ERRO_ARQUIVO_DESTINO_MAIS_NOVO);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == "Versao mais recente no Pen-drive"); 
}

// ==============================================================================
// TESTE 6: RESTAURACAO (CASO DE DECISÃO 9: HD mais antigo que PD)
// ==============================================================================

TEST_CASE("Caso de Decisao 9: Restaura (PD -> HD) quando HD e o mais antigo", "[restauracao][data]") {
    const std::string test_name = "test_case_9_restore_update";
    setup_test_env(test_name); 

    const std::string origem_pd = test_name + "_destino";
    const std::string destino_hd = test_name + "_origem"; // Usamos 'origem' para simular o HD
    const std::string arquivo_nome = "restaurar_novo.txt";

    const std::string origem_path = origem_pd + "/" + arquivo_nome;
    const std::string destino_path = destino_hd + "/" + arquivo_nome;
    const std::string conteudo_novo = "Versao mais recente no Pen-drive";
    
    create_file(destino_path, "Conteudo Antigo no HD");
    
    create_file(origem_path, conteudo_novo);
    
    auto tempo_antigo = fs::file_time_type::clock::now() - std::chrono::hours(48);
    set_file_time(destino_path, tempo_antigo);
    
    // A funcao deve ser chamada com: origem=PD, destino=HD, Operacao=RESTAURACAO
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, RESTAURACAO);
    
    REQUIRE(resultado == SUCESSO);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo_novo);
}
