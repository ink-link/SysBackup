all: testa_velha

testa_velha: velha.o testa_velha.o catch_amalgamated.o
	g++ -std=c++14 -Wall velha.o testa_velha.o catch_amalgamated.o -o testa_velha
	./testa_velha

compile: testa_velha

velha.o : velha.cpp velha.hpp
	g++ -std=c++14 -Wall -c velha.cpp
    
testa_velha.o: testa_velha.cpp velha.hpp catch_amalgamated.hpp
	g++ -std=c++14 -Wall -c testa_velha.cpp

catch_amalgamated.o: catch_amalgamated.cpp
	g++ -std=c++14 -Wall -c catch_amalgamated.cpp
    
test: testa_velha   
	./testa_velha
    
cpplint:
	cpplint --filter=-build/include_subdir --exclude=catch_amalgamated.hpp  --exclude=catch_amalgamated.cpp *.*
    
gcov: velha.o testa_velha.o catch_amalgamated.o
	g++ -std=c++14 -Wall -fprofile-arcs -ftest-coverage velha.o testa_velha.o catch_amalgamated.o -o testa_velha
	./testa_velha
	gcov *.cpp  
     
debug: velha.o testa_velha.o catch_amalgamated.o
	g++ -std=c++14 -Wall -g velha.o testa_velha.o catch_amalgamated.o -o testa_velha
	gdb testa_velha
    
    
cppcheck:
	cppcheck  --enable=warning .

valgrind: testa_velha
	exec valgrind --leak-check=yes --log-file=valgrind.rpt testa_velha


clean:
	rm -rf *.o *.exe *.gc* testa_velha