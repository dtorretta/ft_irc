NAME = ircserv

# Detect OS
UNAME_S := $(shell uname -s)

# Compiler
ifeq ($(UNAME_S), Darwin)
	CPP	=	g++-14
else
	CPP	=	c++
endif

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
		sources/commands/TopicCommand.cpp \
		sources/commands/ModeCommand.cpp \
		sources/commands/QuitCommand.cpp

INC =   -I ./includes \
		-I ./includes/core \
        -I ./includes/utils \
		-I ./includes/commands

OBJ_DIR = obj

OBJS = $(SRC:sources/%.cpp=$(OBJ_DIR)/%.o)

#CPP = c++
CPP_FLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME):		$(OBJS)
	@$(CPP) $(CPP_FLAGS) $(INC) $(OBJS) -o $(NAME)
	@echo "\n✨ IRCserv is ready.\n"

 $(OBJ_DIR):
		@mkdir -p $(OBJ_DIR)
		@mkdir -p $(dir $(OBJS))

$(OBJ_DIR)/%.o: sources/%.cpp | $(OBJ_DIR)
	@echo "Compiling $<"
	@$(CPP) $(CPP_FLAGS) $(INC) -c -o $@ $<

clean:
	@rm -rf $(OBJ_DIR)
	@echo "\n💧 Clean done \n"

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
