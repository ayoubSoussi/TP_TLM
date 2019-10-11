#include <iostream>

using namespace std;

template<class T>
T max(T array[], int length)
{
   T vmax = array[0];
   for (int i = 1; i < length; i++)
      if (array[i] > vmax)
         vmax = array[i];
   return vmax;
}


int main(void)
{
   int    ia[] = {0, 12, -3, 25, 77, -54, 13};
   double fa[] = {1.6666666666e3, 1.32, 3.1416, 1.414, 6e-19, 0.1};

   cout << "max int: " << max(ia, sizeof(ia)/sizeof(*ia)) << endl;
   cout << "max flt: " << max(fa, sizeof(fa)/sizeof(*fa)) << endl;

   return 0;
}
