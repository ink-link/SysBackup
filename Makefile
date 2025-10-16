# ==============================================================================
# VARIAVEIS DE CONFIGURACAO
# ==============================================================================
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -pedantic
TEST_EXECUTABLE = testa_backup
TEST_CPP = testa_backup.cpp
SRC_CPP = backup.cpp
HEADER = backup.hpp
CATCH_SRC = catch_amalgamated.cpp
CATCH_HEADER = catch_amalgamated.hpp

OBJS = $(SRC_CPP:.cpp=.o) $(TEST_CPP:.cpp=.o) $(CATCH_SRC:.cpp=.o)

.PHONY: all compile test cpplint cppcheck gcov debug valgrind doc clean

# ==============================================================================
# REGRAS PRINCIPAIS
# ==============================================================================

# Meta-regra: executa a compilacao e os testes.
all: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# Regra para linkar os arquivos objeto e criar o executavel final
$(TEST_EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TEST_EXECUTABLE)

# Regra para compilar apenas, sem executar
compile: $(TEST_EXECUTABLE)

# Regra para executar os testes
test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

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
gcov: clean
	# 1. Compila com as flags de cobertura
	$(CC) $(CFLAGS) -fprofile-arcs -ftest-coverage $(SRC_CPP) $(TEST_CPP) $(CATCH_SRC) -o $(TEST_EXECUTABLE)
	# 2. Executa o programa para gerar os arquivos .gcda
	./$(TEST_EXECUTABLE)
	# 3. Roda o gcov nos arquivos de interesse (exclui a biblioteca de teste)
	gcov $(SRC_CPP) $(TEST_CPP)

# 4. GDB: Debugging
debug: clean
	# Compila com a flag de debug -g
	$(CC) $(CFLAGS) -g $(SRC_CPP) $(TEST_CPP) $(CATCH_SRC) -o $(TEST_EXECUTABLE)
	gdb $(TEST_EXECUTABLE)

# 5. VALGRIND: Verifica vazamento de memoria
valgrind: $(TEST_EXECUTABLE)
	# exec eh usado para resolver erros de thread em algumas VMs
	exec valgrind --leak-check=yes --log-file=valgrind.rpt ./$(TEST_EXECUTABLE)

# 6. DOXYGEN: Geracao da documentacao
doc:
	doxygen

# ==============================================================================
# REGRA DE LIMPEZA
# ==============================================================================

clean:
	rm -f *.o *.exe *.gc* $(TEST_EXECUTABLE) a.out valgrind.rpt html/* latex/* rm -rf test_case_*