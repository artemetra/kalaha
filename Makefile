default:
	mkdir -p result	
	clang -Wall -Wextra main.c -o result/game

clean:
	rm -rf result
