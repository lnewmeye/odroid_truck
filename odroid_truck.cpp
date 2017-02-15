// Odroid Vision Based Truck Autopilot
// Authors: James Swift, Luke Newmeyer

#include <iostream>
#include "truck.h"

using std::cout;
using std::endl;

int main()
{
	Truck truck;
	
	truck.connect_truck();
	
	return 0;
}
