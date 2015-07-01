buildrun:
	rm -rf bin
	mkdir bin
	gcc -g src/main.c src/utility.c -lglut -lGL -lGLU -lGLEW  -lm -Linclude -Linclude/x64 -Wl,-rpath,./include/:./include/x64/ -lbass -o bin/fin -Wall
	./bin/fin
build:
	rm -rf bin
	mkdir bin
	gcc -g src/main.c src/utility.c -lglut -lGL -lGLU -lGLEW  -lm -Linclude -Linclude/x64 -Wl,-rpath,./include/:./include/x64/ -lbass -o bin/fin -Wall
run:
	./bin/fin
clean:
	rm -rf bin
