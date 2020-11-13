CC = g++
CFLAGS = -c -Wall
LFLAGS = -Wall

SOURCE_DIR = handler
TEST_DIR = test/executable_file

DFH = data_file_handler
TMH = table_meta_handler
TH = table_handler
DMH = database_meta_handler

# clean

clean_data_file_handler:
	rm $(TEST_DIR)/$(DFH)_unit_test $(TEST_DIR)/$(DFH)_unit_test.o $(TEST_DIR)/$(DFH).o

clean_table_meta_handler:
	rm $(TEST_DIR)/$(TMH)_unit_test $(TEST_DIR)/$(TMH)_unit_test.o $(TEST_DIR)/$(TMH).o

clean:
	rm $(TEST_DIR)/$(TH)_unit_test $(TEST_DIR)/$(TH)_unit_test.o $(TEST_DIR)/$(TH).o

clean_all:
	rm $(TEST_DIR)/*
	

# data_file_handler单元测试
# 依赖于data_file_handler
$(DFH)_unit_test: $(TEST_DIR)/$(DFH)_unit_test.o $(TEST_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -g -o $(TEST_DIR)/$@

$(TEST_DIR)/$(DFH)_unit_test.o: $(SOURCE_DIR)/$(DFH)/$(DFH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./

$(TEST_DIR)/$(DFH).o: $(SOURCE_DIR)/$(DFH)/$(DFH).cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./


# table_meta_handler单元测试
# 依赖于data_file_handler、table_meta_handler、json
$(TMH)_unit_test: $(TEST_DIR)/$(TMH)_unit_test.o $(TEST_DIR)/$(TMH).o $(TEST_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -g -o $(TEST_DIR)/$@

$(TEST_DIR)/$(TMH)_unit_test.o: $(SOURCE_DIR)/$(TMH)/$(TMH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(TMH).o: $(SOURCE_DIR)/$(TMH)/$(TMH).cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./ -std=c++11


# table_handler单元测试
# 依赖于：table_handler、data_file_handler、table_meta_handler、json
$(TH)_unit_test: $(TEST_DIR)/$(TH)_unit_test.o $(TEST_DIR)/$(TH).o $(TEST_DIR)/$(DFH).o $(TEST_DIR)/$(TMH).o
	$(CC) $(LFLAGS) $^ -g -o $(TEST_DIR)/$@

$(TEST_DIR)/$(TH)_unit_test.o: $(SOURCE_DIR)/$(TH)/$(TH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(TH).o: $(SOURCE_DIR)/$(TH)/$(TH).cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./ -std=c++11

# database_meta_handler单元测试
# 依赖于：data_file_handler、database_meta_handler、json
$(DMH)_unit_test: $(TEST_DIR)/$(DMH)_unit_test.o $(TEST_DIR)/$(DMH).o $(TEST_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -g -o $(TEST_DIR)/$@

$(TEST_DIR)/$(DMH)_unit_test.o: $(SOURCE_DIR)/$(DMH)/$(DMH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(DMH).o: $(SOURCE_DIR)/$(DMH)/$(DMH).cc
	$(CC) $(CFLAGS) $^ -g -o $@ -I ./ -std=c++11