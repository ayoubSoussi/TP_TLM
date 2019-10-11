#ifndef RENDEZVOUS_IFS
#define RENDEZVOUS_IFS

#include <systemc>
using namespace sc_core;

template<typename T>
class rendezvous_in_if : virtual public sc_interface
{
public:
	virtual T get() = 0;
};

template<typename T>
class rendezvous_out_if : virtual public sc_interface
{
public:
	virtual void put(const T & val) = 0;
};

#endif
