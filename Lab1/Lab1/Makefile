linker: main.o tokenizer.o
	g++ -g tokenizer.o main.o -o linker

tokenizer.o: tokenizer.h tokenizer.cpp

%.o : %.c 
	g++ -c -Wp,-MD,$@.d  -o $@ $<

clean:
	rm -f linker *~
	rm -f *.o