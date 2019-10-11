// Programme SystemC _ultra_ minimaliste.

// Tout en un fichier pour plus de simplicité, mais ce n'est pas un
// exemple à suivre pour des vrais projets. Dans la vrai vie,
// un composant = 1 fichier .cpp + 1 fichier .h, au moins.

#include <iostream>
#include <systemc>

using namespace sc_core;
using namespace std;

SC_MODULE(writer) {
	sc_out<bool> out;

	void compute (void) {
		bool val = false;
		while (true) {
			wait(1, SC_NS);
			val = !val;
			cout << name() << ": sending " << val << endl;
			out.write(val);
		}
	}
	SC_CTOR(writer) {
		SC_THREAD(compute);
	}
};

SC_MODULE(receiver) {
	sc_in<bool> in;

	void compute (void) {
		std::cout << name() << ": received " << in.read() << endl;
	}

	SC_CTOR(receiver) {
		SC_METHOD(compute);
		sensitive << in;
	}
};

int sc_main(int argc, char ** argv) {
	writer w("writer");
	receiver r("receiver");
	sc_signal<bool> s;

	w.out.bind(s);
	r.in.bind(s);
	sc_start(100, SC_NS);
	return 0;
}
