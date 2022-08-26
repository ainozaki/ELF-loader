CXXFLAGS=-g -Wall -Wextra -I./src/include -pthread -DNDEBUG -fsanitize=address
LDFLAGS= -fsanitize=address
SRC = \
	src/main.cc

OBJ=$(SRC:.cc=.o)
DEP=$(SRC:.cc=.d)

TARGET = elfloader

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

-include $(DEP)

run:
	./$(TARGET) misc/a.out

format: $(SRC) $(HEADER) $(MAIN)
	find ./ -type f -name "*.cc" -or -name "*.h" | xargs clang-format -i

clean:
	rm -f $(OBJ) $(DEP) $(TARGET)

.PHONY: all run format clean
.SECONDARY: $(OBJ)
