NAME = btc

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
DEBUGFLAGS = -g -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer

SRCS = main.cpp BitcoinExchange.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CXXFLAGS += $(DEBUGFLAGS)
debug: fclean $(NAME)
	@echo "Debug build complete with sanitizers enabled"

.PHONY: all clean fclean re debug
