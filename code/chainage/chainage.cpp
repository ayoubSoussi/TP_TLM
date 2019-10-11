/********************************************************************
 * Copyright (C) 2012 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

#include <iostream>

using namespace std;

class Base {
public:
	Base(const char *name)
		/*
		 * Chainage sur le constructeur d'un champ.
		 * Ici, m_name n'est pas un objet, mais on peut quand
		 * même chainer dessus.
		 */
		: m_name(name) {
		/*
		 * On ne peut pas écrire « m_name = name; » parce que
		 * m_name est const (error: assignment of read-only
		 * member ‘Base::m_name’).
		 */
	}
	void print() {
		cout << "My name is " << m_name << endl;
	}
private:
	const char * const m_name;
};

class Derived : public Base {
public:
	/* Constructeur par défaut : celui qui n'a pas d'argument */
	Derived()
		/* Chainage sur le constructeur de la classe de base */
		: Base("default name") {}

	Derived(const char *name) : Base(name) {}
};

int main() {
	Base b("b");
	b.print();
	Derived d_named("d_named");
	d_named.print();
	Derived d_anonymous; /* Appelle le constructeur par défaut */
	d_anonymous.print();
}
