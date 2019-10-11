/********************************************************************
 * Copyright (C) 2013-2014 by Verimag                               *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

#include <iostream>
#include <vector>

using namespace std;

class Drawable {
public:
	virtual void draw() = 0;
};

class Square : public Drawable {
public:
	void draw () {
		cout << "Drawing a square" << endl;
	}
};

class Triangle : public Drawable {
public:
	void draw () {
		cout << "Drawing a triangle" << endl;
	}	
};

int main () {
	vector<Drawable *> canvas;
	canvas.push_back(new Square());
	canvas.push_back(new Triangle());
	canvas.push_back(new Square());
	// canvas.push_back(new Drawable()); // not compilable
	for (auto d : canvas) { // C++11
		d->draw();
	}
}
