#include "MatrixUtility.h"




void MatrixUtility::outputBoolMatrix(bool ** matrix, int row, int col, std::ostream * resultFile)
{
	(*resultFile) << "    ";
	for (int i = 0; i < row; i++) {
		(*resultFile) << i << " ";
	}
	(*resultFile) << endl;
	for (int i = 0; i < row; i++) {
		(*resultFile) << "(" << i << ") ";
		for (int j = 0; j < col; j++) {
			(*resultFile) << matrix[i][j] << " ";
		}
		(*resultFile) << endl;
	}
}

void MatrixUtility::outputFloatMatrix(float ** matrix, int row, int col, std::ostream * resultFile)
{
	(*resultFile) << "    ";
	for (int i = 0; i < row; i++) {
		(*resultFile) << i << " ";
	}
	(*resultFile) << endl;
	for (int i = 0; i < row; i++) {
		(*resultFile) << "(" << i << ") ";
		for (int j = 0; j < col; j++) {
			(*resultFile) << matrix[i][j] << " ";
		}
		(*resultFile) << endl;
	}
}

void MatrixUtility::outputIntMatrix(int ** matrix, int row, int col, std::ostream * resultFile)
{
	(*resultFile) << "    ";
	for (int i = 0; i < row; i++) {
		(*resultFile) << i << " ";
	}
	(*resultFile) << endl;
	for (int i = 0; i < row; i++) {
		(*resultFile) << "(" << i << ") ";
		for (int j = 0; j < col; j++) {
			(*resultFile) << matrix[i][j] << " ";
		}
		(*resultFile) << endl;
	}
}
