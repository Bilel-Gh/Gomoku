NAME		= Gomoku
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -O2

UNAME_S		:= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	SDL_INC	= -I/opt/homebrew/include/SDL2
	SDL_LIB	= -L/opt/homebrew/lib -lSDL2
else
	SDL_INC	= $(shell sdl2-config --cflags)
	SDL_LIB	= $(shell sdl2-config --libs) -lm
endif

SRC_DIR		= src
INC_DIR		= include
OBJ_DIR		= obj

SRC			= $(wildcard $(SRC_DIR)/*.cpp)
OBJ			= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(SDL_LIB) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(SDL_INC) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
