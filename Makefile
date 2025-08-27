NAME = ircserv

SRC	=   sources/main.cpp \
		sources/core/Channel.cpp \
		sources/core/Server.cpp \
		sources/core/Client.cpp \
		sources/registration/NickCommand.cpp \
		sources/registration/PassCommand.cpp \
		sources/registration/UserCommand.cpp \
		sources/utils/utils.cpp \
		sources/commands/InviteCommand.cpp \
		sources/commands/JoinCommand.cpp \
		sources/commands/KickCommand.cpp \
		sources/commands/PartCommand.cpp \
		sources/commands/PrivmsgCommand.cpp \
		sources/commands/TopicCommand.cpp 

INC =   -I ./includes \
		-I ./includes/core \
        -I ./includes/utils \
		-I ./includes/commands
       
   
OBJS = $(SRC:%.cpp=%.o)

CPP = c++
CPP_FLAGS = -Wall -Wextra -Werror -std=c++98 

all: $(NAME)

$(NAME):		$(OBJS)
	@$(CPP) $(CPP_FLAGS) $(INC) $(OBJS) -o $(NAME)
	@echo "\n✨ ircserv is ready.\n"

%.o: %.cpp
	@$(CPP) $(CPP_FLAGS) -c -o $@ $<
	
clean:
	@rm -f $(OBJS)
	@echo "\n💧 Clean done \n"

fclean: clean
	@rm -f $(NAME)
	
re: fclean all

.PHONY: all clean fclean re