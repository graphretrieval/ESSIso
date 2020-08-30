#pragma once
#ifndef MATRIX_UTILITY
#define MATRIX_UTILITY

#include<iostream>
#include<fstream>

using namespace std;


class MatrixUtility {
public:
	static void outputBoolMatrix(bool ** matrix, int row, int col, std::ostream * resultFile);

	static void outputFloatMatrix(float ** matrix, int row, int col, std::ostream * resultFile);

	static void outputIntMatrix(int ** matrix, int row, int col, std::ostream * resultFile);
};
#endif