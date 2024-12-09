CCX = g++
CCXFLAGS = -std=c++17 -Wall

PRODUCER_SRC = Producer.cpp
CONSUMER_SRC = Consumer.cpp

PRODUCER_BIN = producer
CONSUMER_BIN = consumer

all: $(PRODUCER_BIN) $(CONSUMER_BIN)

$(PRODUCER_BIN): $(PRODUCER_SRC)
	$(CCX) $(CCXFLAGS) $(PRODUCER_SRC) -o $(PRODUCER_BIN)

$(CONSUMER_BIN): $(CONSUMER_SRC)
	$(CCX) $(CCXFLAGS) $(CONSUMER_SRC) -o $(CONSUMER_BIN)

clean:
	rm -f $(PRODUCER_BIN) $(CONSUMER_BIN)

run_producer: $(PRODUCER_BIN)
	./$(PRODUCER_BIN) $(args)

run_consumer: $(CONSUMER_BIN)
	./$(CONSUMER_BIN) $(args)

.PHONY: all clean run_producer run_consumer