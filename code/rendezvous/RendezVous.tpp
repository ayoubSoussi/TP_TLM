#include <string>
using namespace std;

template<typename T>
RendezVous<T>::RendezVous(const char *name) : sc_prim_channel(name)
{
	shared_value = 0;
	get_ok = false;
	put_ok = false;
	reader = 0;
	writer = 0;
}

template<typename T>
T RendezVous<T>::get()
{
	T retour;

	if (!put_ok)
	{
#ifdef VERBOSE
		cout << "  get() : Attente de put()..." << endl;
#endif
		wait(put_event);
	}

	put_ok = false;

	retour = shared_value;

	get_ok = true;
#ifdef VERBOSE
	cout << "  get() : Notification..." << endl;
#endif
	get_event.notify();

	return retour;
}

template<typename T>
void RendezVous<T>::put(const T & val)
{
	shared_value = val;

	put_ok = true;
#ifdef VERBOSE
	cout << "  put() : Notification..." << endl;
#endif
	put_event.notify();

	if (!get_ok)
	{
#ifdef VERBOSE
		cout << "  put() : Attente de get()..." << endl;
#endif
		wait(get_event);
	}

	get_ok = false;
}

template <typename T>
void RendezVous<T>::register_port(sc_port_base & port, const char* if_typename)
{
	string nm(if_typename);

	if (nm == typeid(rendezvous_in_if<T>).name() )
	{
		// only one reader can be connected
		if (reader != 0)
		{
			SC_REPORT_ERROR(0, "Plus de un processus connecte en lecture sur un rendezvous.");
		}

		reader = &port;
	}
	else
	{
		// only one writer can be connected
		if (writer != 0)
		{
			SC_REPORT_ERROR(0, "Plus de un processus connecte en ecriture sur un rendezvous.");

		}

		writer = &port;
	}
}
