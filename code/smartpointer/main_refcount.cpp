/********************************************************************
 * Copyright (C) 2009 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

#include "refcount_ptr.h"

#define PRINT(x) do { cout << #x << " = " << x << endl; } while (false)

using namespace std;

refcount_ptr<int> get_ptr() {
	return refcount_ptr<int>(new int(100));
}

void buggy() {
	throw 42;
}

void calling_buggy () {
	refcount_ptr<int> p(new int(33));
	cout << "Going to call buggy()" << endl;
	buggy();
	cout << "end of calling_buggy" << endl;
}

int main () {
	refcount_ptr<int> p1(new int(42));
	PRINT(*p1);
	REFCOUNT_PTR_DEBUG(p1);
	{
		refcount_ptr<int> p2 = p1;
		refcount_ptr<int> p3(new int(666));
		p3 = p1;
		REFCOUNT_PTR_DEBUG(p1);
		REFCOUNT_PTR_DEBUG(p2);
		REFCOUNT_PTR_DEBUG(p3);
		*p1 = 12;
		PRINT(*p2);
	}
	REFCOUNT_PTR_DEBUG(p1);
	{
		refcount_ptr<int> retval = get_ptr();
		REFCOUNT_PTR_DEBUG(retval);
		PRINT(*retval);
	}
	try {
		calling_buggy();
	} catch (int i) {
		cout << "exception cought" << endl;
	}
	cout << "end of main" << endl;
	return 0;
}
