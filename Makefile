all:
	gcc -std=c90 decode_xml.c tests.c -g -o decode_xml -I.
	gcc -std=c90 message.c -g -o message -I.
debug:
	gcc -DDEBUG -std=c90 decode_xml.c tests.c -g -o decode_xml -I.
	gcc -DDEBUG -std=c90 message.c -g -o message -I.
clean:
	rm *.exe
crash:
	gcc crash_gcc_float.c -o crash_gcc_float
	gcc crash_gcc_string.c -o crash_gcc_float
