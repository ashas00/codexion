NAME	= codexion

CFLAGS	= -Wall -Wextra -Werror -pthread -g
SRCS	= main.c \
		  parsing/parser.c parsing/parse_coders.c \
		  core/create.c core/logs.c core/dongl.c \
		  coders/coder.c coders/monitor.c \
		  scheduler/fifo.c scheduler/edf.c scheduler/queue.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
