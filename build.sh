clear
bison -d src/scope.y -o src/output/scope.tab.c
flex -o src/output/scope.yy.c src/scope.l
gcc -g src/*.c src/**/*.c -o scope -lm