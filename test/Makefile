
tests: test-encoding
	./test-encoding

test-encoding: test-encoding.cpp ../clair.cpp
	g++ -DDEBUG=0 -I.. test-encoding.cpp -o test-encoding

clean:
	rm -f test-encoding
