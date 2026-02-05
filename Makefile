NAME		= Gomoku
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 -O2

RAYLIB_INC	= -I/opt/homebrew/include
RAYLIB_LIB	= -L/opt/homebrew/lib -lraylib \
			  -framework CoreVideo -framework IOKit -framework Cocoa \
			  -framework GLUT -framework OpenGL

SRC_DIR		= src
INC_DIR		= include
OBJ_DIR		= obj

SRC			= $(wildcard $(SRC_DIR)/*.cpp)
OBJ			= $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(RAYLIB_LIB) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(RAYLIB_INC) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
