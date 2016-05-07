CXX = g++
CXXFLAGS = -g -Wall

SRC_DIR = src/
INCLUDE_DIR = include/
OBJ_DIR = obj/
DEPS_DIR = deps/
EXMPL_DIR = example/

SRC_MODULES = $(wildcard $(SRC_DIR)*.cpp)
OBJ_MODULES = $(addprefix $(OBJ_DIR), $(notdir $(SRC_MODULES:.cpp=.o)))
DEPS_MODULES = $(addprefix $(DEPS_DIR), $(notdir $(SRC_MODULES:.cpp=.d)))

CXXFLAGS += -I $(INCLUDE_DIR)

src_to_obj = $(addprefix $(OBJ_DIR), $(notdir $(1:.cpp=.o)))

$(DEPS_DIR)%.d: $(SRC_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -E -MM -MT $(call src_to_obj, $<) -MT $@ -MF $@ $<

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

smtp: $(EXMPL_DIR)example.cpp $(OBJ_MODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRC_MODULES)
	mkdir -p $(OBJ_DIR)
	mkdir -p $(DEPS_DIR)
	echo "-include $(DEPS_MODULES)" > $@

.PHONY: clean

clean:
	rm -f smtp
	rm -rf $(OBJ_DIR)
	rm -rf $(DEPS_DIR)
	rm -f deps.mk
