#include <iostream>
#include "Employee.h"

using namespace std;
using namespace Records;

int main()
{
	cout << "Testing the Employee class." << endl;

	Employee emp;
	emp.setFirstName("Jay");
	emp.setLastName("Li");
	emp.setEmployeeNumber(50);
	emp.setSalary(100000);
	emp.promote();
	emp.promote(1000);
	emp.hire();
	emp.display();

	return 0;
}
