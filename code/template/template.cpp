/********************************************************************
 * Copyright (C) 2012 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

#include <iostream>

using namespace std;

template<typename T1, typename T2>
   struct simple_pair {
      T1 first;
      T2 second;
      void print () {
         cout << "first = "  << first  << endl
              << "second = " << second << endl;
      }
   };

int main(void)
{
   simple_pair<int, const char *> p;
   p.first = 42;
   p.second = "some string";
   p.print();

   simple_pair<int, int> p2;
   p2.first = 1;
   p2.second = 2;
   p2.print();

   return 0;
}
