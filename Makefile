#CXXFLAGS=-g -Wall -Wextra -I./src/include -pthread -DNDEBUG -fsanitize=address
#LDFLAGS= -fsanitize=address
CXXFLAGS=-g -Wall -Wextra -I./src/include -pthread -DNDEBUG
LDFLAGS=
OBJ = \
	src/main.o \
	src/jump.o \
	src/print.o
DEP=$(OBJ:.o=.d)

TARGET = elfloader

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.o: %.s
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEP)

run:
	./$(TARGET) misc/a.out

format: $(SRC) $(HEADER) $(MAIN)
	find ./ -type f -name "*.cc" -or -name "*.h" | xargs clang-format -i

clean:
	rm -f $(OBJ) $(DEP) $(TARGET)

.PHONY: all run format clean
.SECONDARY: $(OBJ)
