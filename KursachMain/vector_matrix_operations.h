#ifndef VECTOR_MATRIX_OPERATIONS_H
#define VECTOR_MATRIX_OPERATIONS_H

#include <vector>
#include <string>

using Matrix = std::vector<std::vector<double>>;
using Vector = std::vector<double>;

Vector stringToVector(const std::wstring& str, int size);
std::wstring vectorToString(const Vector& vec);
Vector multiplyVectorByScalar(const Vector& vec, double scalar);
Vector powerVector(const Vector& vec, int power);
double vectorLength(const Vector& vec);
Vector addVectors(const Vector& a, const Vector& b);
Vector subtractVectors(const Vector& a, const Vector& b);
double dotProduct(const Vector& a, const Vector& b);
Vector crossProduct(const Vector& a, const Vector& b);
double mixedProduct(const Vector& a, const Vector& b, const Vector& c);
bool checkVectorDimensions(const Vector& a, const Vector& b);

Matrix stringToMatrix(const std::wstring& str, int rows, int cols);
std::wstring matrixToString(const Matrix& matrix);
Matrix multiplyMatrixByScalar(const Matrix& matrix, double scalar);
Matrix transpose(const Matrix& matrix);
Matrix multiplyMatrices(const Matrix& A, const Matrix& B);
Matrix powerMatrix(const Matrix& matrix, int power);
double determinant(const Matrix& matrix);
Matrix getMinor(const Matrix& matrix, int row, int col);
Matrix adjugate(const Matrix& matrix);
Matrix inverse(const Matrix& matrix);
int calculateRank(Matrix matrix);
Matrix addMatrices(const Matrix& a, const Matrix& b);
Matrix subtractMatrices(const Matrix& a, const Matrix& b);
Matrix multiplyMatricesDiff(const Matrix& a, const Matrix& b);
bool isSquareMatrix(const Matrix& matrix);
bool checkMatrixDimensions(const Matrix& a, const Matrix& b);

#endif // VECTOR_MATRIX_OPERATIONS_H
