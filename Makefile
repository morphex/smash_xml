all:
	gcc -std=c90 decode_xml.c tests.c -g -o decode_xml -I.
	gcc -std=c90 message.c -g -o message -I.
clean:
	rm *.exe
