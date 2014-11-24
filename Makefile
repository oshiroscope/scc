all: cogen	

cogen: main.o cogen.o environment.o list.o parse.o print.o syntree.o tokenizer.o util.o
	gcc -o $@  main.o cogen.o environment.o list.o parse.o print.o syntree.o tokenizer.o util.o

clean:
	rm -rf cogen *.o
