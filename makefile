run: gbj_qap.cpp
	g++  gbj_qap.cpp -o gbj
do: run
	./gbj ${arg}
clean:
	rm -f *o gbj