// main.cpp : Defines the entry point for the application.
//
#include <iostream>
#include "include/accelerateMVCC.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	acmvcc::TrxManager trxManager = acmvcc::TrxManager(1);

	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;

	return 0;
}
