CC = g++
CFLAGS = -c -Wall
LFLAGS = -Wall

DBG_TYPE = g

SOURCE_DIR = handler
TEST_DIR = test/executable_file

DFH = data_file_handler
TMH = table_meta_handler
TH = table_handler
DMH = database_meta_handler
DH = database_handler
AP = action_processor
EH = exception_handler
TRH = table_row_handler

# clean

clean:
	rm $(TEST_DIR)/*

clean_DFH:
	rm $(TEST_DIR)/$(DFH)_unit_test $(TEST_DIR)/$(DFH)_unit_test.o $(TEST_DIR)/$(DFH).o

clean_TMH:
	rm $(TEST_DIR)/$(TMH)_unit_test $(TEST_DIR)/$(TMH)_unit_test.o $(TEST_DIR)/$(TMH).o

clean_TH:
	rm $(TEST_DIR)/$(TH)_unit_test $(TEST_DIR)/$(TH)_unit_test.o $(TEST_DIR)/$(TH).o

clean_DMH:
	rm $(TEST_DIR)/$(DMH)_unit_test $(TEST_DIR)/$(DMH)_unit_test.o $(TEST_DIR)/$(DMH).o

clean_DH:
	rm $(TEST_DIR)/$(DH)_unit_test $(TEST_DIR)/$(DH)_unit_test.o $(TEST_DIR)/$(DH).o

clean_AP:
	rm $(TEST_DIR)/$(AP)_unit_test $(TEST_DIR)/$(AP)_unit_test.o $(TEST_DIR)/$(AP).o


# exception_handler
$(TEST_DIR)/$(EH).o:$(SOURCE_DIR)/$(EH)/$(EH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11


# data_file_handler单元测试，依赖于data_file_handler、exception_handler与json
$(DFH)_unit_test: $(TEST_DIR)/$(DFH)_unit_test.o $(TEST_DIR)/$(DFH).o $(TEST_DIR)/$(EH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(TEST_DIR)/$@

$(TEST_DIR)/$(DFH)_unit_test.o: $(SOURCE_DIR)/$(DFH)/$(DFH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

# data_file_handler
$(TEST_DIR)/$(DFH).o: $(SOURCE_DIR)/$(DFH)/$(DFH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11


# table_meta_handler单元测试，依赖于data_file_handler、table_meta_handler、json、database_meta_handler
$(TMH)_unit_test: $(TEST_DIR)/$(TMH)_unit_test.o $(TEST_DIR)/$(TMH).o $(TEST_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(TEST_DIR)/$@

$(TEST_DIR)/$(TMH)_unit_test.o: $(SOURCE_DIR)/$(TMH)/$(TMH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(TMH).o: $(SOURCE_DIR)/$(TMH)/$(TMH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11


# table_handler单元测试，依赖于：table_handler、data_file_handler、table_meta_handler、database_meta_handler、json
$(TH)_unit_test: $(TEST_DIR)/$(TH)_unit_test.o $(TEST_DIR)/$(TH).o $(TEST_DIR)/$(DFH).o $(TEST_DIR)/$(TMH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(TEST_DIR)/$@

$(TEST_DIR)/$(TH)_unit_test.o: $(SOURCE_DIR)/$(TH)/$(TH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(TH).o: $(SOURCE_DIR)/$(TH)/$(TH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11


# database_meta_handler单元测试，依赖于：data_file_handler、database_meta_handler、json
$(DMH)_unit_test: $(TEST_DIR)/$(DMH)_unit_test.o $(TEST_DIR)/$(DMH).o $(TEST_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(TEST_DIR)/$@

$(TEST_DIR)/$(DMH)_unit_test.o: $(SOURCE_DIR)/$(DMH)/$(DMH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(DMH).o: $(SOURCE_DIR)/$(DMH)/$(DMH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

# database_handler单元测试
# TODO

$(TEST_DIR)/$(DH)_unit_test.o: $(SOURCE_DIR)/$(DH)/$(DH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(DH).o: $(SOURCE_DIR)/$(DH)/$(DH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11


# table_row

$(TEST_DIR)/$(TRH)_unit_test.o: $(SOURCE_DIR)/$(TRH)/$(TRH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(TRH).o: $(SOURCE_DIR)/$(TRH)/$(TRH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

# action_processor单元测试
# TODO

$(TEST_DIR)/$(AP)_unit_test.o: $(SOURCE_DIR)/$(AP)/$(AP)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

$(TEST_DIR)/$(AP).o: $(SOURCE_DIR)/$(AP)/$(AP).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

# test_all
handler_test: $(TEST_DIR)/handler_test.o $(TEST_DIR)/$(TRH).o $(TEST_DIR)/$(AP).o $(TEST_DIR)/$(DH).o $(TEST_DIR)/$(DMH).o $(TEST_DIR)/$(TH).o $(TEST_DIR)/$(TMH).o $(TEST_DIR)/$(DFH).o $(TEST_DIR)/$(EH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(TEST_DIR)/$@ -std=c++11

$(TEST_DIR)/handler_test.o: test/handler_unit_test/test_all/handler_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++11

# proto_test
proto_test : test/test_proto/proto_test.cc
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o test/test_proto/$@ -std=c++11 -lprotobuf -lpthread 