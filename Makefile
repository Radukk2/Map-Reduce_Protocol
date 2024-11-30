build:
		g++ -g -std=c++17 tema1.cpp -o tema1 -Wall -Wextra -Werror -lpthread


clean:
		rm -f tema1
		rm ?.txt
