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

// Funcao auxiliar para criar um arquivo de parametros com conteudo de teste
void create_param_file(const std::string& path, const std::vector<std::string>& files) {
    std::ofstream ofs(path);
    assert(ofs.is_open());
    for (const auto& file : files) {
        ofs << file << "\n";
    }
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
// TESTE 3: IGNORAR ATUALIZACAO (CASO DE DECISÃO 4: Datas Iguais)
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
// TESTE 4: ERRO POR DATA (CASO DE DECISÃO 5: PD > HD)
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
// TESTE 5: RESTAURACAO (CASO DE DECISÃO 9: HD mais antigo que PD)
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

// ==============================================================================
// TESTE 6: ERRO NA RESTAURACAO (CASO DE DECISÃO 8: HD mais novo que PD)
// ==============================================================================

TEST_CASE("Caso de Decisao 8: ERRO (PD mais antigo que HD) na Restauracao", "[restauracao][erro][data]") {
    const std::string test_name = "test_case_8_restore_error";
    setup_test_env(test_name); 
    const std::string origem_pd = test_name + "_destino";
    const std::string destino_hd = test_name + "_origem"; 
    const std::string arquivo_nome = "restaurar_antigo.txt";

    const std::string origem_path = origem_pd + "/" + arquivo_nome;
    const std::string destino_path = destino_hd + "/" + arquivo_nome;
    const std::string conteudo_hd_novo = "Versao mais recente no HD";
    
    create_file(origem_path, "Conteudo Antigo no PD");
    
    create_file(destino_path, conteudo_hd_novo);
    
    // Define a data de modificacao da origem (PD) para ser INTENCIONALMENTE mais antiga que o HD (Destino)
    auto tempo_antigo = fs::file_time_type::clock::now() - std::chrono::hours(48);
    set_file_time(origem_path, tempo_antigo);
    
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, RESTAURACAO);
    
    REQUIRE(resultado == ERRO_ARQUIVO_ORIGEM_MAIS_ANTIGO);
    
    // Garante que o conteudo do HD NAO FOI SOBRESCRITO
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo_hd_novo); 
}

// ==============================================================================
// TESTE 7: IGNORAR RESTAURACAO (CASO DE DECISÃO 10: Datas Iguais)
// ==============================================================================

TEST_CASE("Caso de Decisao 10: Ignora Restauracao (PD == HD)", "[restauracao][ignorar]") {
    const std::string test_name = "test_case_10_ignore";
    setup_test_env(test_name); 

    // A origem da copia eh o Pen-drive, e o destino eh o HD.
    const std::string origem_pd = test_name + "_destino";
    const std::string destino_hd = test_name + "_origem";
    const std::string arquivo_nome = "ignorar_restauracao.txt";

    const std::string origem_path = origem_pd + "/" + arquivo_nome;
    const std::string destino_path = destino_hd + "/" + arquivo_nome;
    const std::string conteudo = "Conteudo identico";
    
    create_file(destino_path, conteudo);
    create_file(origem_path, conteudo);
    
    auto tempo_atual = fs::file_time_type::clock::now();
    set_file_time(destino_path, tempo_atual);
    set_file_time(origem_path, tempo_atual); 
    
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, RESTAURACAO);
    
    REQUIRE(resultado == IGNORAR);
    
    // Confirma que o conteudo NAO foi alterado
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo); 
}

// ==============================================================================
// TESTE 8: RESTAURACAO SIMPLES (CASO DE DECISÃO 11: HD nao existe, PD existe)
// ==============================================================================

TEST_CASE("Caso de Decisao 11: Restaura (PD -> HD) quando HD nao existe", "[restauracao][simples]") {
    const std::string test_name = "test_case_11_simple_restore";
    setup_test_env(test_name); 

    // A origem da COPIA eh o Pen-drive, e o destino eh o HD.
    const std::string origem_pd = test_name + "_destino";
    const std::string destino_hd = test_name + "_origem";
    const std::string arquivo_nome = "restaurar_novo.txt";

    const std::string origem_path = origem_pd + "/" + arquivo_nome;
    const std::string destino_path = destino_hd + "/" + arquivo_nome;
    const std::string conteudo = "Conteudo do Pen-drive a ser restaurado.";
    
    create_file(origem_path, conteudo);
    fs::remove(destino_path); 

    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, RESTAURACAO);
    
    REQUIRE(resultado == SUCESSO);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo); // Confirma que o HD foi criado e atualizado
}

// ==============================================================================
// TESTE 9: ERRO (CASO DE DECISÃO 12: PD nao existe na Restauracao)
// ==============================================================================

TEST_CASE("Caso de Decisao 12: ERRO (PD inexistente) na Restauracao", "[restauracao][erro]") {
    const std::string test_name = "test_case_12_pd_missing";
    setup_test_env(test_name); 

    // A origem da COPIA eh o Pen-drive, e o destino eh o HD.
    const std::string origem_pd = test_name + "_destino";
    const std::string destino_hd = test_name + "_origem"; 
    const std::string arquivo_nome = "arquivo_perdido.txt";

    const std::string origem_path = origem_pd + "/" + arquivo_nome;
    const std::string destino_path = destino_hd + "/" + arquivo_nome;
    const std::string conteudo_hd_original = "Conteudo que deve ser mantido no HD";
    
    create_file(destino_path, conteudo_hd_original);
    
    fs::remove(origem_path); 

    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, RESTAURACAO);
    
    REQUIRE(resultado == ERRO_ARQUIVO_ORIGEM_NAO_EXISTE);
    
    // Garante que o conteudo do HD NAO FOI ALTERADO (mantem a integridade)
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo_hd_original); 
}

// ==============================================================================
// TESTE 10: ERRO (CASO DE DECISÃO 13: Arquivo ausente em ambos)
// ==============================================================================

TEST_CASE("Caso de Decisao 13: ERRO (Arquivo ausente em HD e PD) na Restauracao", "[restauracao][erro][final]") {
    const std::string test_name = "test_case_13_missing_both";
    setup_test_env(test_name); 

    // A origem da COPIA eh o Pen-drive, e o destino eh o HD.
    const std::string origem_pd = test_name + "_destino";
    const std::string destino_hd = test_name + "_origem"; 
    const std::string arquivo_nome = "arquivo_desaparecido.txt";

    const std::string origem_path = origem_pd + "/" + arquivo_nome;
    const std::string destino_path = destino_hd + "/" + arquivo_nome;
    
    
    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, RESTAURACAO);
    
    REQUIRE(resultado == ERRO_ARQUIVO_ORIGEM_NAO_EXISTE);
    
    // Garante que o arquivo NAO foi criado no HD.
    REQUIRE(fs::exists(destino_path) == false); 
}

// ==============================================================================
// TESTE 11: IGNORAR (CASO DE DECISÃO 7: HD nao existe, PD existe)
// ==============================================================================

TEST_CASE("Caso de Decisao 7: IGNORAR (HD ausente, PD presente)", "[backup][ignorar][final]") {
    const std::string test_name = "test_case_7_ignore_delete";
    setup_test_env(test_name); 

    const std::string origem_dir = test_name + "_origem";
    const std::string destino_dir = test_name + "_destino";
    const std::string arquivo_nome = "arquivo_a_ignorar.txt";

    const std::string origem_path = origem_dir + "/" + arquivo_nome;
    const std::string destino_path = destino_dir + "/" + arquivo_nome;
    const std::string conteudo_original = "Conteudo que deve permanecer";
    
    create_file(destino_path, conteudo_original);
    
    fs::remove(origem_path); 
    // Garante que a pasta de origem existe (para evitar erro de FS)
    fs::create_directories(origem_dir); 

    ResultadoBackup resultado = faz_backup_arquivo(origem_path, destino_path, BACKUP);
    
    REQUIRE(resultado == IGNORAR);
    REQUIRE(fs::exists(destino_path) == true);
    
    std::ifstream ifs(destino_path);
    std::string linha;
    std::getline(ifs, linha);
    REQUIRE(linha == conteudo_original);
}

// ==============================================================================
// TESTE 14: ERRO - ARQUIVO DE PARAMETROS AUSENTE (CASO DE DECISÃO 1)
// ==============================================================================

TEST_CASE("Caso de Decisao 1: ERRO - Backup.parm ausente", "[parametros][erro]") {
    std::string test_name = "test_caso_1_parm_ausente";
    setup_test_env(test_name); 

    const std::string nome_arquivo_parm = test_name + "/Backup.parm";
    std::vector<std::string> lista_arquivos;
    
    fs::remove(nome_arquivo_parm); 
    
    ResultadoBackup resultado = le_arquivo_parametros(nome_arquivo_parm, lista_arquivos);
    
    REQUIRE(resultado == ERRO_ARQUIVO_PARAMETROS_AUSENTE);
    REQUIRE(lista_arquivos.empty() == true);
}

// ==============================================================================
// TESTE 15: LEITURA DE PARAMETROS COM SUCESSO
// ==============================================================================

TEST_CASE("Leitura de Backup.parm com sucesso", "[parametros][sucesso]") {
    const std::string param_file = "Backup.parm";
    std::vector<std::string> lista_arquivos_esperada = {
        "caminho/do/arquivo1.txt",
        "pasta/subpasta/arquivo2.cpp",
        "config.json"
    };
    
    create_param_file(param_file, lista_arquivos_esperada);
    
    std::vector<std::string> lista_arquivos_obtida;
    
    ResultadoBackup resultado = le_arquivo_parametros(param_file, lista_arquivos_obtida);
    
    REQUIRE(resultado == SUCESSO);
    REQUIRE(lista_arquivos_obtida.size() == lista_arquivos_esperada.size());
    
    // Verifica se cada item lido corresponde ao item esperado
    for (size_t i = 0; i < lista_arquivos_esperada.size(); ++i) {
        REQUIRE(lista_arquivos_obtida[i] == lista_arquivos_esperada[i]);
    }
    
    fs::remove(param_file);
}

// ==============================================================================
// TESTE 16: ORQUESTRAÇÃO DE BACKUP (SUCESSO DE PONTA A PONTA)
// ==============================================================================

TEST_CASE("Orquestracao: Execucao de Backup com multiplos arquivos", "[orquestracao][backup]") {
    const std::string test_name = "test_case_orquestracao_backup";
    setup_test_env(test_name); 

    const std::string origem_base = test_name + "_origem";
    const std::string destino_base = test_name + "_destino";
    const std::string parm_file = "Backup.parm";
    
    // Caminhos relativos (como estariam no Backup.parm)
    const std::string arq1 = "documentos/doc1.txt";
    const std::string arq2 = "fotos/foto2.jpg";

    // 1. Setup: Cria o arquivo Backup.parm
    std::ofstream ofs_parm(parm_file);
    ofs_parm << arq1 << "\n";
    ofs_parm << arq2 << "\n";
    ofs_parm.close();

    // 2. Setup: Cria os arquivos de ORIGEM e as subpastas
    fs::create_directories(origem_base + "/documentos");
    fs::create_directories(origem_base + "/fotos");
    create_file(origem_base + "/" + arq1, "Conteudo 1");
    create_file(origem_base + "/" + arq2, "Conteudo 2");
    
    // 3. Execucao: Roda o processo de orquestracao completo
    ResultadoBackup resultado = executa_backup_restauracao(
        parm_file, 
        origem_base, 
        destino_base, 
        BACKUP
    );
    
    // 4. Verificacao: Espera-se SUCESSO e que os arquivos existam no destino
    REQUIRE(resultado == SUCESSO);
    REQUIRE(fs::exists(destino_base + "/" + arq1) == true);
    REQUIRE(fs::exists(destino_base + "/" + arq2) == true);
    
    // Limpeza adicional do arquivo parm
    fs::remove(parm_file);
}

// ==============================================================================
// TESTE 16: ERRO NA ORQUESTRAÇÃO (CASO DE DECISÃO 1: Backup.parm ausente)
// ==============================================================================

TEST_CASE("Caso de Decisao 1: Orquestracao Falha por Backup.parm Ausente", "[orquestracao][erro]") {
    const std::string test_name = "test_case_orquestracao_erro_parm";
    setup_test_env(test_name); 

    const std::string origem_base = test_name + "_origem";
    const std::string destino_base = test_name + "_destino";
    const std::string parm_file = "Backup.parm";
    
    fs::remove(parm_file);
    
    // A funcao deve falhar imediatamente ao tentar ler o arquivo.
    ResultadoBackup resultado = executa_backup_restauracao(
        parm_file, 
        origem_base, 
        destino_base, 
        BACKUP
    );
    
    REQUIRE(resultado == ERRO_ARQUIVO_PARAMETROS_AUSENTE);
}
