cc= g++
ccflags=-std=c++11 -Wall


PRODUCER_BIN = producer
CONSUMER_BIN = consumer

all: $(PRODUCER_BIN) $(CONSUMER_BIN)

$(PRODUCER_BIN): $(producer.cpp)
	$(cc) $(ccflags) $(producer.cpp) -o $(PRODUCER_BIN)

$(CONSUMER_BIN): $(consumer.cpp)
	$(cc) $(ccflags) $(consumer.cpp) -o $(CONSUMER_BIN)

clean:
	rm -f $(PRODUCER_BIN) $(CONSUMER_BIN)

run_producer: $(PRODUCER_BIN)
	./$(PRODUCER_BIN) $(args)

run_consumer: $(CONSUMER_BIN)
	./$(CONSUMER_BIN) $(args)

.PHONY: 