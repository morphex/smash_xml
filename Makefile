CC = gcc
COMPILER_OPTIONS = -std=iso9899:1990 -pedantic -fms-extensions decode_xml.c -g -o decode_xml -I. -Wall -Wuninitialized -Wfatal-errors
COMPILER_OPTIONS_ASSEMBLER = -std=iso9899:1990 -pedantic -fms-extensions decode_xml.c -g -S -Wa,-ahl=test.s -I. -Wall -Wuninitialized -fverbose-asm
#CC = tcc
#COMPILER_OPTIONS = -o decode_xml -I. decode_xml.c

all:
	$(CC) makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	$(CC) $(COMPILER_OPTIONS)
assembler:
	$(CC) makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	$(CC) $(COMPILER_OPTIONS_ASSEMBLER)
alltest:
	$(CC) makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	$(CC) $(COMPILER_OPTIONS) -DTEST
debug:
	$(CC) makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	$(CC) $(COMPILER_OPTIONS) -DDEBUG -I. -DTEST
#	$(CC) -DDEBUG -Wall -ggdb3 -ansi message.c -o message -I.
#efence-build:
#	$(CC) makeheaders.c -o makeheaders
#	./makeheaders decode_xml.c
#	$(CC) -lefence -DDEBUG -Wall -ggdb3 -ansi -fms-extensions decode_xml.c tests.c -o decode_xml -I. -DTEST
##	$(CC) -lefence -DDEBUG -Wall -ggdb3 -ansi message.c -o message -I.
clean:
	rm -f *.exe
	rm -f *.exe.stackdump
	rm -f *~
	rm -f ./decode_xml
	rm -f ./decode_xml_debug
test: alltest
	./decode_xml
valgrind: test
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=100 --track-fds=yes ./decode_xml > valgrind_out.log 2>&1
	@echo
	@echo "Program/Valgrind output in valgrind_out.log"
#efence: efence-build
#	#./decode_xml
