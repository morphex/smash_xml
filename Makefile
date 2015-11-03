all:
	gcc makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	gcc -std=c90 -Wall -Os decode_xml.c -g -o decode_xml -I.
alltest:
	gcc makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	gcc -std=c90 -Wall -Os decode_xml.c -g -o decode_xml -I. -DTEST
debug:
	gcc makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	gcc -DDEBUG  -Wall -Wno-format-extra-args -ggdb3 -std=c90 decode_xml.c -o decode_xml_debug -I. -DTEST
#	gcc -DDEBUG -Wall -ggdb3 -std=c90 message.c -o message -I.
efence-build:
	gcc makeheaders.c -o makeheaders
	./makeheaders decode_xml.c
	gcc -lefence -DDEBUG -Wall -ggdb3 -std=c90 decode_xml.c tests.c -o decode_xml -I. -DTEST
#	gcc -lefence -DDEBUG -Wall -ggdb3 -std=c90 message.c -o message -I.
clean:
	rm -f *.exe
	rm -f *.exe.stackdump
	rm -f *~
	rm -f ./decode_xml
crash:
	gcc crash_gcc_float.c -o crash_gcc_float
	gcc crash_gcc_string.c -o crash_gcc_float
assembler:
	gcc -fno-asynchronous-unwind-tables -S call_overhead.c
	gcc -fno-asynchronous-unwind-tables -S call_overhead_reference.c
test: alltest
	./decode_xml
valgrind: test
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=100 --track-fds=yes ./decode_xml > valgrind_out.log 2>&1
	@echo
	@echo "Program/Valgrind output in valgrind_out.log"
efence: efence-build
	#./decode_xml
