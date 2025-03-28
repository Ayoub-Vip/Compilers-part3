CXX         = g++
CXXFLAGS    = -std=c++17 -Wall -Wextra -I.

BISONFLAGS  = -d -v
LEXFLAGS    =

EXEC        = vsopc

SRC         = AST.cpp parser.cpp lexer.cpp
OBJ         = $(SRC:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

parser.cpp parser.hpp: parser.y
	bison $(BISONFLAGS) -o parser.cpp parser.y

lexer.cpp: lexer.l parser.hpp
	flex -o lexer.cpp lexer.l

parser.o: parser.cpp parser.hpp AST.hpp
	$(CXX) $(CXXFLAGS) -c parser.cpp -o parser.o

lexer.o: lexer.cpp parser.hpp AST.hpp
	$(CXX) $(CXXFLAGS) -c lexer.cpp -o lexer.o

AST.o: AST.cpp AST.hpp
	$(CXX) $(CXXFLAGS) -c AST.cpp -o AST.o
	
install-tools:
	@echo "nothing to do"

clean:
	rm -f $(EXEC) *.o parser.cpp parser.hpp lexer.cpp parser.output

.PHONY: all clean install-tools

