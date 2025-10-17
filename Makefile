# ==============================================================================
# VARIAVEIS DE CONFIGURACAO
# ==============================================================================
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -pedantic
GCOV_FLAGS = -fprofile-arcs -ftest-coverage

# --- Arquivos de Teste ---
TEST_EXECUTABLE = testa_backup
TEST_CPP = testa_backup.cpp
SRC_CPP = backup.cpp
HEADER = backup.hpp
CATCH_SRC = catch_amalgamated.cpp
CATCH_HEADER = catch_amalgamated.hpp
OBJS_TEST = $(SRC_CPP:.cpp=.o) $(TEST_CPP:.cpp=.o) $(CATCH_SRC:.cpp=.o)

# --- Arquivos da Aplicação Final ---
FINAL_EXECUTABLE = backup_app
MAIN_CPP = main.cpp
OBJS_APP = $(MAIN_CPP:.cpp=.o) $(SRC_CPP:.cpp=.o)

# --- Diretórios ---
REPORTS_DIR = reports

OBJS = $(SRC_CPP:.cpp=.o) $(TEST_CPP:.cpp=.o) $(CATCH_SRC:.cpp=.o)


.PHONY: all compile test cpplint cppcheck gcov debug valgrind doc clean app

# ==============================================================================
# REGRAS PRINCIPAIS
# ==============================================================================

# Meta-regra: executa a compilacao e os testes.
all: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# Regra para linkar os arquivos objeto e criar o executavel final
$(TEST_EXECUTABLE): $(OBJS_TEST)
	$(CC) $(CFLAGS) $(OBJS_TEST) -o $(TEST_EXECUTABLE)

# Regra para compilar apenas, sem executar
compile: $(TEST_EXECUTABLE)

# Regra para executar os testes
test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# Regra para compilar a aplicacao final  
app: $(FINAL_EXECUTABLE)

$(FINAL_EXECUTABLE): $(OBJS_APP)
	$(CC) $(CFLAGS) $(OBJS_APP) -o $(FINAL_EXECUTABLE)

# ==============================================================================
# REGRAS DE COMPILACAO DOS ARQUIVOS OBJETO
# ==============================================================================

# Regra para compilar o modulo principal (backup.o)
$(SRC_CPP:.cpp=.o): $(SRC_CPP) $(HEADER)
	$(CC) $(CFLAGS) -c $<

# Regra para compilar o modulo de testes (testa_backup.o)
$(TEST_CPP:.cpp=.o): $(TEST_CPP) $(HEADER) $(CATCH_HEADER)
	$(CC) $(CFLAGS) -c $<

# Regra para compilar o arquivo de implementacao do Catch2
$(CATCH_SRC:.cpp=.o): $(CATCH_SRC) $(CATCH_HEADER)
	$(CC) $(CFLAGS) -c $<

# ==============================================================================
# FERRAMENTAS DE QUALIDADE (ANALISE)
# ==============================================================================

# 1. CPP LINT: Verifica o estilo do codigo
cpplint:
	cpplint --filter=-build/include_subdir --exclude=$(CATCH_HEADER) --exclude=$(CATCH_SRC) *.cpp *.hpp

# 2. CPP CHECK: Verifica erros estaticos e avisos
cppcheck:
	cppcheck --enable=warning --std=c++17 .

# 3. GCOV: Verifica a cobertura do codigo (80% exigido)
gcov:
	make clean
	@REPORTS_DIR="reports"; mkdir -p $$REPORTS_DIR
	# 1. Compila CADA ARQUIVO SEPARADAMENTE com flags de cobertura (.gcno)
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -c $(SRC_CPP)
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -c $(TEST_CPP)
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage -c $(CATCH_SRC)
	# 2. Linka os objetos - ADICIONA AS FLAGS DE COBERTURA AQUI
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage $(OBJS) -o $(TEST_EXECUTABLE)
	# 3. Executa o programa para gerar os arquivos .gcda
	./$(TEST_EXECUTABLE)
	# 4. Roda o gcov APENAS nos arquivos do projeto (backup.cpp e testa_backup.cpp)
	gcov $(SRC_CPP) $(TEST_CPP)
	@mv $(SRC_CPP).gcov $(TEST_CPP).gcov $$REPORTS_DIR || true
	make clean

# 4. GDB: Debugging
debug:
	# Compila com a flag de debug -g
	$(CC) $(CFLAGS) -g $(SRC_CPP) $(TEST_CPP) $(CATCH_SRC) -o $(TEST_EXECUTABLE)
	gdb $(TEST_EXECUTABLE)

# 5. VALGRIND: Verifica vazamento de memoria
valgrind: $(TEST_EXECUTABLE)
	@mkdir -p $(REPORTS_DIR)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=$(REPORTS_DIR)/valgrind.rpt ./$(TEST_EXECUTABLE)

# 6. DOXYGEN: Geracao da documentacao
doc:
	doxygen

# ==============================================================================
# REGRA DE LIMPEZA
# ==============================================================================

clean:
	rm -f *.o *.exe .gc *.gcda *.gcno *.gcov $(TEST_EXECUTABLE) $(FINAL_EXECUTABLE) a.out
	rm -rf reports
	rm -rf test_case_*