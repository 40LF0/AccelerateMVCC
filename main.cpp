// main.cpp : Defines the entry point for the application.
//
#include <iostream>
#include "include/accelerateMVCC.h"

using namespace std;

void trxManagerTest();

int main()
{
	cout << "Hello CMake." << endl;

	trxManagerTest();


	return 0;
}

void trxManagerTest() {
	acmvcc::TrxManager trxManager = acmvcc::TrxManager(1);

	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
	cout << trxManager.startTrx() << endl;
}
