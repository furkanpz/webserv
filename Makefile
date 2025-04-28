NAME = webserv

SRCSF = ./srcs/

SRCS = $(SRCSF)main.cpp $(SRCSF)WebServer.cpp $(SRCSF)Utils.cpp $(SRCSF)Response.cpp $(SRCSF)Clients.cpp $(SRCSF)config.cpp

CC = c++
RM = rm -rf

TESTERFILE = tester 

CFLAGS = -Wall -Wextra -Werror -std=c++98
OBJS = $(SRCS:.cpp=.o)

%.o: %.cpp
		$(CC) $(CFLAGS) -I./inc -c $< -o $@

$(NAME): $(OBJS)
		$(CC) $(CFLAGS) -o $(NAME) $(OBJS)


all: $(NAME)
clean: 
	$(RM) $(OBJS)
fclean: clean
		$(RM) $(NAME)
		$(RM) $(TESTERFILE)
re: fclean all


.PHONY: all clean fclean re