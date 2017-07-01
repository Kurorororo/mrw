SRC_DIR=./src
INC_DIR=./include
TEST_DIR=./test
BIN_DIR=./bin

INCS = -I$(INC_DIR) -I/usr/local/include/

UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  CXX=g++ -Wall
endif
ifeq ($(UNAME),Darwin)
  CXX=clang++ -Wall
endif

release:
	$(CXX) -std=c++14 -O3 $(INCS) $(SRC_DIR)/run_mrw.cc $(SRC_DIR)/mrw.cc \
  $(SRC_DIR)/random_walk.cc $(SRC_DIR)/ff.cc $(SRC_DIR)/graphplan.cc \
  $(SRC_DIR)/data.cc $(SRC_DIR)/trie.cc $(SRC_DIR)/parser.cc \
  -o $(BIN_DIR)/release

debug:
	$(CXX) -std=c++14 -g -pg $(INCS) $(SRC_DIR)/run_mrw.cc $(SRC_DIR)/mrw.cc \
  $(SRC_DIR)/random_walk.cc $(SRC_DIR)/ff.cc $(SRC_DIR)/graphplan.cc \
  $(SRC_DIR)/data.cc $(SRC_DIR)/trie.cc $(SRC_DIR)/parser.cc \
  -o $(BIN_DIR)/debug

test_mrw:
	$(CXX) -std=c++14 $(INCS) $(TEST_DIR)/test_mrw.cc $(SRC_DIR)/mrw.cc \
  $(SRC_DIR)/random_walk.cc $(SRC_DIR)/ff.cc $(SRC_DIR)/graphplan.cc \
  $(SRC_DIR)/data.cc $(SRC_DIR)/trie.cc $(SRC_DIR)/parser.cc \
  -o $(BIN_DIR)/test

additive:
	$(CXX) -std=c++14 -O3 $(INCS) $(SRC_DIR)/run_mrw.cc \
  $(SRC_DIR)/mrw_additive.cc $(SRC_DIR)/additive.cc $(SRC_DIR)/data.cc \
  $(SRC_DIR)/random_walk.cc $(SRC_DIR)/trie.cc $(SRC_DIR)/parser.cc \
  -o $(BIN_DIR)/additive

graphplan:
	$(CXX) -std=c++14 -O3 $(INCS) $(SRC_DIR)/run_graphplan.cc \
  $(SRC_DIR)/graphplan.cc $(SRC_DIR)/data.cc $(SRC_DIR)/parser.cc \
  -o $(BIN_DIR)/graphplan

test_data:
	$(CXX) -std=c++14 $(INCS) $(TEST_DIR)/test_data.cc $(SRC_DIR)/data.cc \
  -o $(BIN_DIR)/test_data

test_parser:
	$(CXX) -std=c++14 $(INCS) $(TEST_DIR)/test_parser.cc $(SRC_DIR)/parser.cc \
  $(SRC_DIR)/data.cc -o $(BIN_DIR)/test_parser

test_trie:
	$(CXX) -std=c++14 $(INCS) $(TEST_DIR)/test_trie.cc $(SRC_DIR)/trie.cc \
  $(SRC_DIR)/data.cc -o $(BIN_DIR)/test_trie

test_graphplan:
	$(CXX) -std=c++14 $(INCS) $(TEST_DIR)/test_graphplan.cc \
  $(SRC_DIR)/graphplan.cc $(SRC_DIR)/data.cc $(SRC_DIR)/parser.cc \
  -o $(BIN_DIR)/test_graphplan

clean:
	rm -f $(BIN_DIR)/*
