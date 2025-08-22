NAME = ircserv

SRC	=   sources/main.cpp \
		sources/core/Server.cpp \
		sources/core/Client.cpp \
		sources/utils/utils.cpp \
		sources/commands/RegistrationCommands.cpp \
		sources/commands/JoinCommand.cpp \
		sources/commands/PartCommand.cpp \
		sources/commands/PrivmsgCommand.cpp 
#		sources/core/Channel.cpp \

INC =   -I ./includes/core \
        -I ./includes/utils \
		-I ./includes/commands \
#		-I ./includes \
       
   
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