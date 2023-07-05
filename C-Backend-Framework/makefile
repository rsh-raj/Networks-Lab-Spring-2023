# a.out: server.c response.o request.o template.o dictionary.o auth.o
# 	gcc server.c response.o request.o dictionary.o template.o auth.o -lpthread

# debug:
# 	gcc -ggdb3 -std=c11 server.c response.o request.o dictionary.o template.o auth.o -lpthread
a.out: main.c CBack.h server.o response.o request.o template.o dictionary.o auth.o
	gcc server.o response.o request.o template.o dictionary.o auth.o main.c -o server -lpthread

server.o: server.c response.h request.h template.h dictionary.h auth.h
	gcc -c server.c 

auth.o : request.h auth.c template.h common.h dictionary.h response.h
	gcc -c auth.c

response.o: response.c response.h common.h
	gcc -c response.c

request.o: request.c request.h common.h
	gcc -c request.c

val:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./a.out 8080

template.o: template.c template.h common.h request.h response.h dictionary.h auth.h
	gcc -c template.c

dictionary.o: dictionary.c dictionary.h
	gcc -c dictionary.c

git:
	git add . && git commit . -m "${msg}" && git push origin

clean:
	rm -f response.o request.o dictionary.o template.o auth.o a.out server server.o