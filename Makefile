CC = clang++
CFLAGS = -c -Wall
LFLAGS = -Wall

DBG_TYPE = g

HANDLER_SOURCE_DIR = src

EXERC_FILE_DIR = bin
HANDLER_TEST_DIR = test/handler_unit_test

DFH = data_file_handler
TMH = table_meta_handler
TH = table_handler
DMH = database_meta_handler
DH = database_handler
AP = action_processor
TRH = table_row_handler
HD = hot_data
CD = cold_data

all: $(EXERC_FILE_DIR)/$(DFH).o $(EXERC_FILE_DIR)/$(TMH).o $(EXERC_FILE_DIR)/$(TH).o $(EXERC_FILE_DIR)/$(DMH).o $(EXERC_FILE_DIR)/$(DH).o $(EXERC_FILE_DIR)/$(TRH).o $(EXERC_FILE_DIR)/$(AP).o $(EXERC_FILE_DIR)/$(HD).o $(EXERC_FILE_DIR)/$(CD).o

# clean

clean:
	rm $(EXERC_FILE_DIR)/*

clean_DFH:
	rm $(EXERC_FILE_DIR)/$(DFH)_unit_test $(EXERC_FILE_DIR)/$(DFH)_unit_test.o $(EXERC_FILE_DIR)/$(DFH).o

clean_TMH:
	rm $(EXERC_FILE_DIR)/$(TMH)_unit_test $(EXERC_FILE_DIR)/$(TMH)_unit_test.o $(EXERC_FILE_DIR)/$(TMH).o

clean_TH:
	rm $(EXERC_FILE_DIR)/$(TH)_unit_test $(EXERC_FILE_DIR)/$(TH)_unit_test.o $(EXERC_FILE_DIR)/$(TH).o

clean_DMH:
	rm $(EXERC_FILE_DIR)/$(DMH)_unit_test $(EXERC_FILE_DIR)/$(DMH)_unit_test.o $(EXERC_FILE_DIR)/$(DMH).o

clean_DH:
	rm $(EXERC_FILE_DIR)/$(DH)_unit_test $(EXERC_FILE_DIR)/$(DH)_unit_test.o $(EXERC_FILE_DIR)/$(DH).o

clean_AP:
	rm $(EXERC_FILE_DIR)/$(AP)_unit_test $(EXERC_FILE_DIR)/$(AP)_unit_test.o $(EXERC_FILE_DIR)/$(AP).o


# DFH单元测试，依赖于DFH, JSON
$(DFH)_unit_test: $(EXERC_FILE_DIR)/$(DFH)_unit_test.o $(EXERC_FILE_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(EXERC_FILE_DIR)/$@

$(EXERC_FILE_DIR)/$(DFH)_unit_test.o: $(HANDLER_TEST_DIR)/$(DFH)/$(DFH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(DFH).o: $(HANDLER_SOURCE_DIR)/$(DFH)/$(DFH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17


# TMH单元测试，依赖于DFH、TMH、JSON、DMH
$(TMH)_unit_test: $(EXERC_FILE_DIR)/$(TMH)_unit_test.o $(EXERC_FILE_DIR)/$(TMH).o $(EXERC_FILE_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(EXERC_FILE_DIR)/$@

$(EXERC_FILE_DIR)/$(TMH)_unit_test.o: $(HANDLER_TEST_DIR)/$(TMH)/$(TMH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(TMH).o: $(HANDLER_SOURCE_DIR)/$(TMH)/$(TMH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17


# TH单元测试，依赖于所有Handler
$(TH)_unit_test: $(EXERC_FILE_DIR)/$(TH)_unit_test.o $(EXERC_FILE_DIR)/$(AP).o $(EXERC_FILE_DIR)/$(DH).o  $(EXERC_FILE_DIR)/$(TH).o $(EXERC_FILE_DIR)/$(HD).o $(EXERC_FILE_DIR)/$(CD).o $(EXERC_FILE_DIR)/$(TRH).o $(EXERC_FILE_DIR)/$(TMH).o $(EXERC_FILE_DIR)/$(DMH).o $(EXERC_FILE_DIR)/$(DFH).o 
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(EXERC_FILE_DIR)/$@

$(EXERC_FILE_DIR)/$(TH)_unit_test.o: $(HANDLER_TEST_DIR)/$(TH)/$(TH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(TH).o: $(HANDLER_SOURCE_DIR)/$(TH)/$(TH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17


# database_meta_handler单元测试，依赖于：data_file_handler、database_meta_handler、json
$(DMH)_unit_test: $(EXERC_FILE_DIR)/$(DMH)_unit_test.o $(EXERC_FILE_DIR)/$(DMH).o $(EXERC_FILE_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(EXERC_FILE_DIR)/$@

$(EXERC_FILE_DIR)/$(DMH)_unit_test.o: $(HANDLER_TEST_DIR)/$(DMH)/$(DMH)_unit_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(DMH).o: $(HANDLER_SOURCE_DIR)/$(DMH)/$(DMH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

# database_handler单元测试，不能用CFLAG
# $(EXERC_FILE_DIR)/$(DH)_unit_test.o: $(HANDLER_TEST_DIR)/$(DH)/$(DH)_unit_test.cc
# 	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(DH).o: $(HANDLER_SOURCE_DIR)/$(DH)/$(DH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17


# table_row
# $(EXERC_FILE_DIR)/$(TRH)_unit_test.o: $(HANDLER_TEST_DIR)/$(TRH)/$(TRH)_unit_test.cc
# 	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(TRH).o: $(HANDLER_SOURCE_DIR)/$(TRH)/$(TRH).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

# action_processor单元测试
# $(EXERC_FILE_DIR)/$(AP)_unit_test.o: $(HANDLER_TEST_DIR)/$(AP)/$(AP)_unit_test.cc
# 	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

$(EXERC_FILE_DIR)/$(AP).o: $(HANDLER_SOURCE_DIR)/$(AP)/$(AP).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

# 热数据
$(EXERC_FILE_DIR)/$(HD).o: $(HANDLER_SOURCE_DIR)/cold_hot_data/$(HD).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

# 冷数据
$(EXERC_FILE_DIR)/$(CD).o: $(HANDLER_SOURCE_DIR)/cold_hot_data/$(CD).cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

# 总测试
handler_test: $(EXERC_FILE_DIR)/handler_test.o $(EXERC_FILE_DIR)/$(TRH).o $(EXERC_FILE_DIR)/$(AP).o $(EXERC_FILE_DIR)/$(DH).o $(EXERC_FILE_DIR)/$(DMH).o $(EXERC_FILE_DIR)/$(TH).o $(EXERC_FILE_DIR)/$(TMH).o $(EXERC_FILE_DIR)/$(DFH).o
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o $(EXERC_FILE_DIR)/$@ -std=c++17

$(EXERC_FILE_DIR)/handler_test.o: test/handler_unit_test/test_all/handler_test.cc
	$(CC) $(CFLAGS) $^ -$(DBG_TYPE) -o $@ -I ./ -std=c++17

# proto_test
proto_test : test/test_proto/proto_test.cc
	$(CC) $(LFLAGS) $^ -$(DBG_TYPE) -o test/test_proto/$@ -std=c++17 -lprotobuf -lpthread 