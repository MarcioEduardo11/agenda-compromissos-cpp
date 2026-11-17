CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

agenda: src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

# Compila com sanitizers (erros de memoria e vazamentos) e roda o roteiro de teste.
teste: src/main.cpp
	$(CXX) $(CXXFLAGS) -g -fsanitize=address,undefined -o agenda_teste src/main.cpp
	./agenda_teste < tests/roteiro_completo.txt > /dev/null

clean:
	rm -f agenda agenda_teste

.PHONY: clean teste
