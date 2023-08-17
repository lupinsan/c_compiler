OBJECTS=	./build/cprocess.o ./build/compiler.o ./build/buffer.o ./build/vector.o ./build/lex_process.o ./build/lexer.o ./build/token.o ./build/parser.o  ./build/node.o  ./build/expressionable.o ./build/datatype.o                   
INCLUDES=	-I ./


all:	${OBJECTS}
		gcc		main.c 	${INCLUDES} ${OBJECTS}	 -g -o ./main

./build/compiler.o:	./compiler.c
		gcc  ./compiler.c ${INCLUDES} -o ./build/compiler.o -g -c

./build/cprocess.o:	./cprocess.c
		gcc  ./cprocess.c ${INCLUDES} -o ./build/cprocess.o -g -c

./build/lex_process.o:	./lex_process.c
		gcc  ./lex_process.c ${INCLUDES} -o ./build/lex_process.o -g -c

./build/lexer.o:	./lexer.c
		gcc  ./lexer.c ${INCLUDES} -o ./build/lexer.o -g -c

./build/token.o:	./token.c
		gcc  ./token.c ${INCLUDES} -o ./build/token.o -g -c

./build/parser.o:	./parser.c
		gcc  ./parser.c ${INCLUDES} -o ./build/parser.o -g -c

./build/datatype.o:	./datatype.c
		gcc  ./datatype.c ${INCLUDES} -o ./build/datatype.o -g -c

./build/node.o:	./node.c
		gcc  ./node.c ${INCLUDES} -o ./build/node.o -g -c


./build/expressionable.o:	./expressionable.c
		gcc  ./expressionable.c ${INCLUDES} -o ./build/expressionable.o -g -c


./build/buffer.o: ./helpers/buffer.c
		gcc ./helpers/buffer.c ${INCLUDES} -o ./build/buffer.o -g -c

./build/vector.o: ./helpers/vector.c
		gcc ./helpers/vector.c ${INCLUDES} -o ./build/vector.o -g -c


clean:
		rm 		./main
		rm 		${OBJECTS}

