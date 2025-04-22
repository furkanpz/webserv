NAME = webserv

SRCS = main.cpp WebServer.cpp Utils.cpp Response.cpp Clients.cpp config.cpp

CC = c++
RM = rm -rf

TESTERFILE = tester 

CFLAGS = -std=c++98 -g # -Wall -Wextra -Werror -std=c++98
OBJS = $(SRCS:.cpp=.o)

%.o: %.cpp
		$(CC) $(CFLAGS) -I./inc -c $< -o $@

$(NAME): $(OBJS)
		$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(TESTERFILE): tester.cpp 
		$(CC) ./tester.cpp -o tester

all: $(NAME)
clean: 
	$(RM) $(OBJS)
fclean: clean
		$(RM) $(NAME)
		$(RM) $(TESTERFILE)
		$(RM) ./uploads/*
re: fclean all



tester: $(TESTERFILE)

.PHONY: all clean fclean re tester