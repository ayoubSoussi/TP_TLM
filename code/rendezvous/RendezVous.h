#ifndef RENDEZVOUS_H
#define RENDEZVOUS_H

#include <systemc>
#include "rendezvous_ifs.h"

template<typename T>
class RendezVous : public sc_prim_channel,
		   virtual public rendezvous_in_if<T>,
		   virtual public rendezvous_out_if<T>
{
public:
	RendezVous(const char *name);

	virtual T get();
	virtual void put(const T & val);
	
	virtual void register_port(sc_port_base & port, const char* if_typename);
	
private:
	T	      shared_value;
	bool	      get_ok, put_ok;
	sc_event      get_event, put_event;
	
	sc_port_base  *reader, *writer;
};

#include "RendezVous.tpp"

#endif
