#include "backup.hpp"
#include <iostream>
#include <string>
#include <cstdlib> // Para EXIT_SUCCESS / EXIT_FAILURE
#include <cstring> // Para strcmp

// ==============================================================================
// FUNÇÃO PRINCIPAL
// ==============================================================================

int main(int argc, char* argv[]) {
    // Verifica o numero minimo de argumentos (./backup_app -b Backup.parm origem destino)
    if (argc != 5) {
        std::cerr << "ERRO: Numero incorreto de argumentos." << std::endl;
        std::cerr << "Uso: " << argv[0] << " <MODO> <ARQUIVO_PARAM> <ORIGEM_BASE> <DESTINO_BASE>" << std::endl;
        std::cerr << "MODO: -b (Backup) ou -r (Restauracao)" << std::endl;
        return EXIT_FAILURE;
    }

    Operacao operacao;
    std::string modo_arg = argv[1];
    
    // Leitura do modo de operacao
    if (modo_arg == "-b") {
        operacao = BACKUP;
        std::cout << "MODO: Backup (HD -> Pen-drive)" << std::endl;
    } else if (modo_arg == "-r") {
        operacao = RESTAURACAO;
        std::cout << "MODO: Restauração (Pen-drive -> HD)" << std::endl;
    } else {
        std::cerr << "ERRO: Modo de operacao invalido. Use -b ou -r." << std::endl;
        return EXIT_FAILURE;
    }

    // Argumentos corrigidos:
    const std::string arquivo_parametros = argv[2]; // Deve ser Backup.parm
    const std::string caminho_origem = argv[3];     // Deve ser hd_source
    const std::string caminho_destino = argv[4];    // Deve ser pen_drive_target

    std::cout << "Arquivo Parametros: " << arquivo_parametros << std::endl;
    std::cout << "Base Origem: " << caminho_origem << std::endl;
    std::cout << "Base Destino: " << caminho_destino << std::endl;
    
    // Chamada da funcao principal
    ResultadoBackup resultado = executa_backup_restauracao(
        arquivo_parametros, 
        caminho_origem, 
        caminho_destino, 
        operacao
    );

    if (resultado == SUCESSO) {
        std::cout << "Operação concluída com SUCESSO." << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cerr << "ERRO FATAL: Operacao falhou com codigo " << resultado_para_string(resultado) << std::endl;
        return EXIT_FAILURE;
    }
}
