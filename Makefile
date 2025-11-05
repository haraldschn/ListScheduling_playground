NAME=simple_test

$(NAME): $(NAME).cpp
	g++ -o $(NAME) $(NAME).cpp -Wall -Wextra

run: $(NAME)
	./$(NAME)

all: $(NAME)

clean:
	rm -rf $(NAME)
