#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <gdiplus.h>
#include <cmath>
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include "resource.h"
#include "main_buttons.h"
#include "vector_matrix_operations.h"
#include "dialog_procedures.h"
#include "utility_functions.h"

HINSTANCE hInst;
bool showCopyFirst = false;
bool showCopySecond = false;
bool showMatrixButtonsFirst = false;
bool showMatrixButtonsSecond = false;
bool showVectorButtonsFirst = false;
bool showVectorButtonsSecond = false;

Matrix globalMatrixFirst;
Matrix globalMatrixSecond;
Vector globalVectorFirst;
Vector globalVectorSecond;
int globalRowsFirst = 0;
int globalColsFirst = 0;
int globalRowsSecond = 0;
int globalColsSecond = 0;

bool isFirstVector = false;
bool isSecondVector = false;

HFONT hNewFont = NULL;

ULONG_PTR gdiplusToken;
Gdiplus::GdiplusStartupInput gdiplusStartupInput;
Gdiplus::Image* pImage = nullptr;

Gdiplus::Image* LoadImageFromResource(HINSTANCE hInstance, LPCWSTR resourceName) 
{
    HRSRC hResource = FindResource(hInstance, resourceName, RT_BITMAP);
    if (!hResource) return nullptr;

    HGLOBAL hMem = LoadResource(hInstance, hResource);
    if (!hMem) return nullptr;

    void* pData = LockResource(hMem);
    DWORD size = SizeofResource(hInstance, hResource);
    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, size);
    void* pBuffer = GlobalLock(hBuffer);
    CopyMemory(pBuffer, pData, size);
    IStream* pStream = nullptr;
    CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);
    Gdiplus::Image* pImage = Gdiplus::Image::FromStream(pStream);
    pStream->Release();

    return pImage;
}

void SetNewFont(HWND hwndDlg) {
    if (hNewFont == NULL) {
        hNewFont = CreateFont(
            24, // Высота шрифта (отрицательное значение указывает на высоту в пикселях)
            0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
    }

    // Устанавливаем новый шрифт для всех элементов управления
    HWND hChild = GetWindow(hwndDlg, GW_CHILD);
    while (hChild) {
        SendMessage(hChild, WM_SETFONT, (WPARAM)hNewFont, TRUE);
        hChild = GetNextWindow(hChild, GW_HWNDNEXT);
    }
}

// Точка входа в приложение
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    hInst = hInstance;

    // Инициализация GDI+
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Зарегистрируйте класс окна
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    RegisterClassW(&wc);

    // Создайте окно
    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Векторно-Матричные преобразования",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // Загрузить изображение
    pImage = new Gdiplus::Image(L"logo.bmp");
    if (pImage->GetLastStatus() != Gdiplus::Ok) 
    {
        MessageBox(hwnd, L"Не удалось загрузить изображение!", L"Error", MB_ICONERROR);
        delete pImage;
        pImage = nullptr;
    }

    ShowWindow(hwnd, nCmdShow);

    // Цикл сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Очистка GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);


    
    return 0;
}


Vector stringToVector(const std::wstring& str, int size) 
{
    Vector vec(size);
    std::wistringstream wiss(str);
    for (int i = 0; i < size; ++i) 
    {
        if (!(wiss >> vec[i])) 
        {
            throw std::runtime_error("Неправильный формат вектора");
        }
    }
    return vec;
}

std::wstring vectorToString(const Vector& vec) 
{
    std::wostringstream woss;
    for (size_t i = 0; i < vec.size(); ++i) 
    {
        woss << vec[i];
        if (i < vec.size() - 1) 
        {
            woss << L" "; // Пространство между элементами
        }
    }
    return woss.str();
}

Vector multiplyVectorByScalar(const Vector& vec, double scalar) 
{
    Vector result = vec;
    for (auto& elem : result) 
    {
        elem *= scalar;
    }
    return result;
}

Vector powerVector(const Vector& vec, int power) 
{
    Vector result = vec;
    for (auto& elem : result) 
    {
        elem = std::pow(elem, power);
    }
    return result;
}

double vectorLength(const Vector& vec) 
{
    double sum = 0.0;
    for (double elem : vec) 
    {
        sum += elem * elem;
    }
    return std::sqrt(sum);
}

Vector addVectors(const Vector& a, const Vector& b) 
{
    if (a.size() != b.size()) 
    {
        throw std::invalid_argument("Векторы должны быть одного размера, чтобы их сложить");
    }
    Vector result(a.size());
    for (size_t i = 0; i < a.size(); ++i) 
    {
        result[i] = a[i] + b[i];
    }
    return result;
}

Vector subtractVectors(const Vector& a, const Vector& b) 
{
    if (a.size() != b.size()) 
    {
        throw std::invalid_argument("Векторы должны быть одного размера, чтобы их вычесть");
    }
    Vector result(a.size());
    for (size_t i = 0; i < a.size(); ++i) 
    {
        result[i] = a[i] - b[i];
    }
    return result;
}

double dotProduct(const Vector& a, const Vector& b) 
{
    if (a.size() != b.size()) 
    {
        throw std::invalid_argument("Векторы должны быть одного размера, чтобы найти их скалярное произведение");
    }
    double result = 0;
    for (size_t i = 0; i < a.size(); ++i) 
    {
        result += a[i] * b[i];
    }
    return result;
}

Vector crossProduct(const Vector& a, const Vector& b) 
{
    if (a.size() != 3 || b.size() != 3) 
    {
        throw std::invalid_argument("Векторное произведение определено только для трехмерных векторов");
    }
    Vector result(3);
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
    return result;
}

double mixedProduct(const Vector& a, const Vector& b, const Vector& c) 
{
    if (a.size() != 3 || b.size() != 3 || c.size() != 3) 
    {
        throw std::invalid_argument("Mixed product is defined only for 3D vectors");
    }
    Vector cross = crossProduct(a, b);
    return dotProduct(cross, c);
}

bool checkVectorDimensions(const Vector& a, const Vector& b) 
{
    return a.size() == b.size();
}


Matrix transpose(const Matrix& matrix) 
{
    if (matrix.empty()) return Matrix();
    int rows = matrix.size();
    int cols = matrix[0].size();
    Matrix transposed(cols, std::vector<double>(rows));
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            transposed[j][i] = matrix[i][j];
        }
    }
    return transposed;
}

std::wstring matrixToString(const Matrix& matrix) 
{
    std::wostringstream woss;
    for (const auto& row : matrix) 
    {
        for (size_t j = 0; j < row.size(); ++j) 
        {
            woss << row[j];
            if (j < row.size() - 1) 
            {
                woss << L" "; // Пространство между элементами
            }
        }
        woss << L"\n"; // Новая строка
    }
    return woss.str();
}

Matrix stringToMatrix(const std::wstring& str, int rows, int cols) 
{
    Matrix matrix(rows, std::vector<double>(cols));
    std::wistringstream wiss(str);
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            if (!(wiss >> matrix[i][j])) 
            {
                throw std::runtime_error("Недопустимый формат матрицы");
            }
        }
    }
    return matrix;
}

Matrix multiplyMatrixByScalar(const Matrix& matrix, double scalar) 
{
    Matrix result = matrix;
    for (auto& row : result) 
    {
        for (auto& elem : row) 
        {
            elem *= scalar;
        }
    }
    return result;
}

Matrix multiplyMatrices(const Matrix& A, const Matrix& B) 
{
    int rows = A.size();
    int cols = B[0].size();
    int inner = B.size();
    Matrix result(rows, std::vector<double>(cols, 0.0));

    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            for (int k = 0; k < inner; ++k) 
            {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

Matrix powerMatrix(const Matrix& matrix, int power) 
{
    if (matrix.size() != matrix[0].size()) 
    {
        throw std::runtime_error("Матрица должна быть квадратной, чтобы возвести ее в степень.");
    }

    Matrix result = matrix;
    Matrix temp = matrix;

    for (int p = 1; p < power; ++p) 
    {
        result = multiplyMatrices(result, temp);
    }

    return result;
}

double determinant(const Matrix& matrix) 
{
    int n = matrix.size();
    if (n == 1) return matrix[0][0];

    double det = 0.0;
    int sign = 1;
    for (int i = 0; i < n; ++i) 
    {
        Matrix subMatrix(n - 1, std::vector<double>(n - 1));
        for (int j = 1; j < n; ++j) 
        {
            int subCol = 0;
            for (int k = 0; k < n; ++k) 
            {
                if (k == i) continue;
                subMatrix[j - 1][subCol] = matrix[j][k];
                ++subCol;
            }
        }
        det += sign * matrix[0][i] * determinant(subMatrix);
        sign = -sign;
    }
    return det;
}

Matrix getMinor(const Matrix& matrix, int row, int col) 
{
    Matrix minorMatrix(matrix.size() - 1, std::vector<double>(matrix.size() - 1));
    for (int i = 0, mi = 0; i < matrix.size(); ++i) 
    {
        if (i == row) continue;
        for (int j = 0, mj = 0; j < matrix.size(); ++j) 
        {
            if (j == col) continue;
            minorMatrix[mi][mj] = matrix[i][j];
            ++mj;
        }
        ++mi;
    }
    return minorMatrix;
}

Matrix adjugate(const Matrix& matrix) 
{
    int size = matrix.size();
    Matrix adj(size, std::vector<double>(size));

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) 
        {
            adj[j][i] = determinant(getMinor(matrix, i, j));
            if ((i + j) % 2 == 1) adj[j][i] = -adj[j][i];
        }
    }
    return adj;
}

Matrix inverse(const Matrix& matrix) 
{
    double det = determinant(matrix);
    if (det == 0) throw std::runtime_error("Матрица является единичной и не может иметь обратной.");

    Matrix adj = adjugate(matrix);
    int size = matrix.size();
    Matrix inv(size, std::vector<double>(size));

    for (int i = 0; i < size; ++i) 
    {
        for (int j = 0; j < size; ++j) 
        {
            inv[i][j] = adj[i][j] / det;
        }
    }
    return inv;
}

int calculateRank(Matrix matrix) {
    int m = matrix.size();
    int n = matrix[0].size();
    int rank = 0;

    for (int row = 0; row < m; ++row) {
        if (matrix[row][rank] != 0) {
            for (int col = 0; col < m; ++col) {
                if (col != row) {
                    double mult = matrix[col][rank] / matrix[row][rank];
                    for (int i = rank; i < n; ++i) {
                        matrix[col][i] -= mult * matrix[row][i];
                    }
                }
            }
            rank++;
        }
        if (rank == n) {
            return rank;
        }
    }

    return rank;
}

Matrix addMatrices(const Matrix& a, const Matrix& b) 
{
    if (a.size() != b.size() || a[0].size() != b[0].size()) 
    {
        throw std::invalid_argument("Матрицы должны иметь одинаковые размеры, чтобы их можно было сложить");
    }
    int rows = a.size();
    int cols = a[0].size();
    Matrix result(rows, std::vector<double>(cols, 0));
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            result[i][j] = a[i][j] + b[i][j];
        }
    }
    return result;
}

Matrix subtractMatrices(const Matrix& a, const Matrix& b) 
{
    if (a.size() != b.size() || a[0].size() != b[0].size()) 
    {
        throw std::invalid_argument("Для вычитания матрицы должны иметь одинаковые размеры");
    }
    int rows = a.size();
    int cols = a[0].size();
    Matrix result(rows, std::vector<double>(cols, 0));
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            result[i][j] = a[i][j] - b[i][j];
        }
    }
    return result;
}

Matrix multiplyMatricesDiff(const Matrix& a, const Matrix& b) 
{
    if (a[0].size() != b.size()) 
    {
        throw std::invalid_argument("Матрицы нельзя перемножать");
    }
    int rows = a.size();
    int cols = b[0].size();
    int common_dim = b.size();
    Matrix result(rows, std::vector<double>(cols, 0));
    for (int i = 0; i < rows; ++i) 
    {
        for (int j = 0; j < cols; ++j) 
        {
            for (int k = 0; k < common_dim; ++k) 
            {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

bool isSquareMatrix(const Matrix& matrix) 
{
    if (matrix.empty()) return false;
    int numRows = matrix.size();
    int numCols = matrix[0].size();
    return numRows == numCols;
}

bool checkMatrixDimensions(const Matrix& a, const Matrix& b) 
{
    return a.size() == b.size() && (a.empty() || a[0].size() == b[0].size());
}

// Функция создания кнопок и надписей
void CreateControls(HWND hwnd)
{
    // Создание надписей и кнопок
    HWND hStatic = CreateWindowExW(
        0,
        L"STATIC",
        L"Векторно-Матричный калькулятор",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, 0, 0, // Размеры будут обновлены позже
        hwnd,
        (HMENU)ID_STATIC_LABEL, // Label ID
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    HFONT hFont = CreateFontW(
        32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

    SendMessage(hStatic, WM_SETFONT, WPARAM(hFont), TRUE);

    // Шрифт
    HFONT hFontLarge = CreateFontW(
        24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

    HWND hBtnStart = CreateWindowW(
        L"BUTTON",
        L"Старт",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_BTN_START,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnStart, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnExit = CreateWindowW(
        L"BUTTON",
        L"Выход",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_BTN_EXIT,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnExit, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hStaticCredit = CreateWindowW(
        L"STATIC",
        L"Made by Nikita P.",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_STATIC_CREDIT,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);
    SendMessage(hStaticCredit, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnBack = CreateWindowW(
        L"BUTTON",
        L"Назад",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 100, 30,
        hwnd,
        (HMENU)ID_BTN_BACK,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnBack, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnInputFirst = CreateWindowW(
        L"BUTTON",
        L"Ввод первого элемента",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_BTN_INPUT_FIRST,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnInputFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnInputSecond = CreateWindowW(
        L"BUTTON",
        L"Ввод второго элемента",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_BTN_INPUT_SECOND,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnInputSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hStaticTypeFirst = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_STATIC_TYPE_FIRST,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hStaticTypeFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hStaticTypeSecond = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_STATIC_TYPE_SECOND,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hStaticTypeSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hResultFirst = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_STATIC_RESULT_FIRST,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hResultFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hResultSecond = CreateWindowW(
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0,
        hwnd,
        (HMENU)ID_STATIC_RESULT_SECOND,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hResultSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnCopyFirst = CreateWindowW(
        L"BUTTON",
        L"Копи",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE,
        0, 0, 50, 30,
        hwnd,
        (HMENU)ID_BTN_COPY_FIRST,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnCopyFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnCopySecond = CreateWindowW(
        L"BUTTON",
        L"Копи",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE,
        0, 0, 50, 30,
        hwnd,
        (HMENU)ID_BTN_COPY_SECOND,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnCopySecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    // Create buttons for matrix and vector operations
    HWND hBtnDetFirst = CreateWindowW(L"BUTTON", L"Определитель", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DETERMINANT_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnDetFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnInvFirst = CreateWindowW(L"BUTTON", L"Обратная", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_INVERSE_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnInvFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnTransFirst = CreateWindowW(L"BUTTON", L"Транспонировать", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRANSPOSE_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnTransFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnRankFirst = CreateWindowW(L"BUTTON", L"Ранг", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_RANK_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnRankFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMultFirst = CreateWindowW(L"BUTTON", L"Умножить на число", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MULTIPLY_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMultFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnPowerFirst = CreateWindowW(L"BUTTON", L"Возвести в степень", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_POWER_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnPowerFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnDetSecond = CreateWindowW(L"BUTTON", L"Определитель", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_DETERMINANT_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnDetSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnInvSecond = CreateWindowW(L"BUTTON", L"Обратная", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_INVERSE_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnInvSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnTransSecond = CreateWindowW(L"BUTTON", L"Транспонировать", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRANSPOSE_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnTransSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnRankSecond = CreateWindowW(L"BUTTON", L"Ранг", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_RANK_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnRankSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMultSecond = CreateWindowW(L"BUTTON", L"Умножить на число", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MULTIPLY_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMultSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnPowerSecond = CreateWindowW(L"BUTTON", L"Возвести в степень", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_POWER_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnPowerSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnLengthFirst = CreateWindowW(L"BUTTON", L"Длина", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LENGTH_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnLengthFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMultVectorFirst = CreateWindowW(L"BUTTON", L"Умножить на число", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MULTIPLY_VECTOR_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMultVectorFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnPowerVectorFirst = CreateWindowW(L"BUTTON", L"Возвести в степень", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_POWER_VECTOR_FIRST, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnPowerVectorFirst, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnLengthSecond = CreateWindowW(L"BUTTON", L"Длина", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_LENGTH_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnLengthSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMultVectorSecond = CreateWindowW(L"BUTTON", L"Умножить на число", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MULTIPLY_VECTOR_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMultVectorSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnPowerVectorSecond = CreateWindowW(L"BUTTON", L"Возвести в степень", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_POWER_VECTOR_SECOND, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnPowerVectorSecond, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMatrixMultiply = CreateWindowW(L"BUTTON", L"Умножение", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MATRIX_MULTIPLY, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMatrixMultiply, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMatrixAdd = CreateWindowW(L"BUTTON", L"Сложение", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MATRIX_ADD, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMatrixAdd, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMatrixSubtract = CreateWindowW(L"BUTTON", L"Вычитание", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MATRIX_SUBTRACT, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMatrixSubtract, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnVectorAdd = CreateWindowW(L"BUTTON", L"Сложение", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_VECTOR_ADD, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnVectorAdd, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnVectorSubtract = CreateWindowW(L"BUTTON", L"Вычитание", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_VECTOR_SUBTRACT, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnVectorSubtract, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnVectorDot = CreateWindowW(L"BUTTON", L"Скалярное произведение", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_VECTOR_DOT, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnVectorDot, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnVectorCross = CreateWindowW(L"BUTTON", L"Векторное произведение", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_VECTOR_CROSS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnVectorCross, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnVectorMixed = CreateWindowW(L"BUTTON", L"Смешанное произведение", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_VECTOR_MIXED, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnVectorMixed, WM_SETFONT, WPARAM(hFontLarge), TRUE);

    HWND hBtnMatrixVectorMultiply = CreateWindowW(L"BUTTON", L"Умножение матрицы на вектор", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_MATRIX_VECTOR_MULTIPLY, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    SendMessage(hBtnMatrixVectorMultiply, WM_SETFONT, WPARAM(hFontLarge), TRUE);
}

// Функция обновления позиций кнопок управления
void UpdateControlPositions(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);

    int btnWidth = 240;
    int btnHeight = 30;
    int spacing = 10;
    int labelWidth = 300;
    int labelHeight = 65;

    int startX = (rect.right - btnWidth) / 2;
    int startY = (rect.bottom - (2 * btnHeight + spacing)) / 2;

    HWND hStatic = GetDlgItem(hwnd, ID_STATIC_LABEL);
    SetWindowPos(hStatic, NULL, (rect.right - labelWidth) / 2, startY - 2 * btnHeight - 20, labelWidth, labelHeight, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hwnd, ID_BTN_START), NULL, startX, startY, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_EXIT), NULL, startX, startY + btnHeight + spacing, btnWidth, btnHeight, SWP_NOZORDER);

    HWND hCredit = GetDlgItem(hwnd, ID_STATIC_CREDIT);
    SetWindowPos(hCredit, NULL, startX, startY + 2 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hwnd, ID_BTN_BACK), NULL, 10, 10, 100, 30, SWP_NOZORDER);

    int margin = rect.right / 4 - btnWidth / 2;
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_INPUT_FIRST), NULL, margin, (rect.bottom - btnHeight) / 5, btnWidth + 20, btnHeight + 30, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_INPUT_SECOND), NULL, rect.right - btnWidth - margin, (rect.bottom - btnHeight) / 5, btnWidth + 20, btnHeight + 30, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hwnd, ID_BTN_COPY_FIRST), NULL, margin + btnWidth + 30, (rect.bottom - btnHeight) / 5, 70, btnHeight + 30, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_COPY_SECOND), NULL, rect.right - margin + btnWidth / 8, (rect.bottom - btnHeight) / 5, 70, btnHeight + 30, SWP_NOZORDER);

    int resultStartY = (rect.bottom - btnHeight) / 5 + 2 * (btnHeight + spacing);
    SetWindowPos(GetDlgItem(hwnd, ID_STATIC_TYPE_FIRST), NULL, margin, resultStartY - 20, rect.right - 2 * margin, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_STATIC_TYPE_SECOND), NULL, rect.right - btnWidth - margin, resultStartY - 20, rect.right - 2 * margin, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_STATIC_RESULT_FIRST), NULL, margin, resultStartY + 2, rect.right - margin, 6 * btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_STATIC_RESULT_SECOND), NULL, rect.right - btnWidth - margin, resultStartY, rect.right - margin, 6 * btnHeight, SWP_NOZORDER);

    int matrixButtonYFirst = resultStartY + 6 * btnHeight + spacing;
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_DETERMINANT_FIRST), NULL, margin, matrixButtonYFirst, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_INVERSE_FIRST), NULL, margin, matrixButtonYFirst + btnHeight + spacing, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_FIRST), NULL, margin, matrixButtonYFirst + 2 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_RANK_FIRST), NULL, margin, matrixButtonYFirst + 3 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MULTIPLY_FIRST), NULL, margin, matrixButtonYFirst + 4 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_POWER_FIRST), NULL, margin, matrixButtonYFirst + 5 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);

    int matrixButtonYSecond = resultStartY + 6 * btnHeight + spacing;
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_DETERMINANT_SECOND), NULL, rect.right - btnWidth - margin, matrixButtonYSecond, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_INVERSE_SECOND), NULL, rect.right - btnWidth - margin, matrixButtonYSecond + btnHeight + spacing, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_SECOND), NULL, rect.right - btnWidth - margin, matrixButtonYSecond + 2 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_RANK_SECOND), NULL, rect.right - btnWidth - margin, matrixButtonYSecond + 3 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MULTIPLY_SECOND), NULL, rect.right - btnWidth - margin, matrixButtonYSecond + 4 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_POWER_SECOND), NULL, rect.right - btnWidth - margin, matrixButtonYSecond + 5 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);

    int vectorButtonYFirst = resultStartY + 6 * btnHeight + spacing;
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_LENGTH_FIRST), NULL, margin, vectorButtonYFirst, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_FIRST), NULL, margin, vectorButtonYFirst + btnHeight + spacing, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_FIRST), NULL, margin, vectorButtonYFirst + 2 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);

    int vectorButtonYSecond = resultStartY + 6 * btnHeight + spacing;
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_LENGTH_SECOND), NULL, rect.right - btnWidth - margin, vectorButtonYSecond, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_SECOND), NULL, rect.right - btnWidth - margin, vectorButtonYSecond + btnHeight + spacing, btnWidth, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_SECOND), NULL, rect.right - btnWidth - margin, vectorButtonYSecond + 2 * (btnHeight + spacing), btnWidth, btnHeight, SWP_NOZORDER);

    int operationButtonY = resultStartY + 6 * btnHeight + spacing;
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MATRIX_MULTIPLY), NULL, (rect.right - btnWidth) / 2, operationButtonY, btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MATRIX_ADD), NULL, (rect.right - btnWidth) / 2, operationButtonY + btnHeight + spacing, btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MATRIX_SUBTRACT), NULL, (rect.right - btnWidth) / 2, operationButtonY + 2 * (btnHeight + spacing), btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_VECTOR_ADD), NULL, (rect.right - btnWidth) / 2, operationButtonY, btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_VECTOR_SUBTRACT), NULL, (rect.right - btnWidth) / 2, operationButtonY + btnHeight + spacing, btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_VECTOR_DOT), NULL, (rect.right - btnWidth) / 2, operationButtonY + 2 * (btnHeight + spacing), btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_VECTOR_CROSS), NULL, (rect.right - btnWidth) / 2, operationButtonY + 3 * (btnHeight + spacing), btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_VECTOR_MIXED), NULL, (rect.right - btnWidth) / 2, operationButtonY + 4 * (btnHeight + spacing), btnWidth + 50, btnHeight, SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hwnd, ID_BTN_MATRIX_VECTOR_MULTIPLY), NULL, (rect.right - btnWidth) / 2 - 15, operationButtonY, btnWidth + 90, btnHeight, SWP_NOZORDER);
}

// Функция переключения видимости страницы
void ShowPage(HWND hwnd, int page)
{
    ShowWindow(GetDlgItem(hwnd, ID_STATIC_LABEL), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_START), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_EXIT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_BACK), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_STATIC_CREDIT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_INPUT_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_INPUT_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_STATIC_TYPE_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_STATIC_TYPE_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_STATIC_RESULT_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_STATIC_RESULT_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_COPY_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_COPY_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_FIRST), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_SECOND), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_MULTIPLY), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_ADD), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_SUBTRACT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_ADD), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_SUBTRACT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_DOT), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_CROSS), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_MIXED), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_VECTOR_MULTIPLY), SW_HIDE);

    if (page == 0) 
    {
        ShowWindow(GetDlgItem(hwnd, ID_STATIC_LABEL), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_START), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_EXIT), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_STATIC_CREDIT), SW_SHOW); 
    }
    else {
        ShowWindow(GetDlgItem(hwnd, ID_BTN_BACK), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_INPUT_FIRST), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_INPUT_SECOND), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_STATIC_TYPE_FIRST), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_STATIC_TYPE_SECOND), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_STATIC_RESULT_FIRST), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_STATIC_RESULT_SECOND), SW_SHOW);
        if (showCopyFirst) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_COPY_FIRST), SW_SHOW);
        }
        if (showCopySecond) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_COPY_SECOND), SW_SHOW);
        }
        if (showMatrixButtonsFirst) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_FIRST), SW_SHOW);
        }
        if (showMatrixButtonsSecond) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_SECOND), SW_SHOW);
        }
        if (showVectorButtonsFirst) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_FIRST), SW_SHOW);
        }
        if (showVectorButtonsSecond) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_SECOND), SW_SHOW);
        }
        if (showMatrixButtonsFirst && showMatrixButtonsSecond) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_MULTIPLY), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_ADD), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_SUBTRACT), SW_SHOW);
        }
        if (showVectorButtonsFirst && showVectorButtonsSecond) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_ADD), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_SUBTRACT), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_DOT), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_CROSS), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_MIXED), SW_SHOW);
        }
        if ((showMatrixButtonsFirst && showVectorButtonsSecond) || (showVectorButtonsFirst && showMatrixButtonsSecond)) 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_VECTOR_MULTIPLY), SW_SHOW);
        }
    }
}

// Функция обновления страницы при вводе элементов
void UpdateResultStatic(HWND hwnd, int buttonId, const std::wstring& result, const std::wstring& type)
{
    if (buttonId == ID_BTN_INPUT_FIRST) 
    {
        ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_FIRST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_FIRST), SW_HIDE);

        InvalidateRect(hwnd, NULL, TRUE);
        SetWindowText(GetDlgItem(hwnd, ID_STATIC_TYPE_FIRST), type.c_str());
        SetWindowText(GetDlgItem(hwnd, ID_STATIC_RESULT_FIRST), result.c_str());
        ShowWindow(GetDlgItem(hwnd, ID_BTN_COPY_FIRST), SW_SHOW);
        showCopyFirst = true;
        if (type == L"Матрица") 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_FIRST), SW_SHOW);
            showMatrixButtonsFirst = true;
            showVectorButtonsFirst = false;
        }
        else if (type == L"Вектор") 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_FIRST), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_FIRST), SW_SHOW);
            showVectorButtonsFirst = true;
            showMatrixButtonsFirst = false;
        }
    }
    else if (buttonId == ID_BTN_INPUT_SECOND) 
    {
        ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_SECOND), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_SECOND), SW_HIDE);

        InvalidateRect(hwnd, NULL, TRUE);
        SetWindowText(GetDlgItem(hwnd, ID_STATIC_TYPE_SECOND), type.c_str());
        SetWindowText(GetDlgItem(hwnd, ID_STATIC_RESULT_SECOND), result.c_str());
        ShowWindow(GetDlgItem(hwnd, ID_BTN_COPY_SECOND), SW_SHOW);
        showCopySecond = true;
        if (type == L"Матрица") 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_DETERMINANT_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_INVERSE_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_TRANSPOSE_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_RANK_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_SECOND), SW_SHOW);
            showMatrixButtonsSecond = true;
            showVectorButtonsSecond = false;
        }
        else if (type == L"Вектор") 
        {
            ShowWindow(GetDlgItem(hwnd, ID_BTN_LENGTH_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_MULTIPLY_VECTOR_SECOND), SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, ID_BTN_POWER_VECTOR_SECOND), SW_SHOW);
            showVectorButtonsSecond = true;
            showMatrixButtonsSecond = false;
        }
    }

    if (showMatrixButtonsFirst && showMatrixButtonsSecond) 
    {
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_ADD), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_SUBTRACT), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_DOT), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_CROSS), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_MIXED), SW_HIDE);

        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_VECTOR_MULTIPLY), SW_HIDE);

        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_MULTIPLY), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_ADD), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_SUBTRACT), SW_SHOW);
    }
    else if (showVectorButtonsFirst && showVectorButtonsSecond) 
    {
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_MULTIPLY), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_ADD), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_SUBTRACT), SW_HIDE);

        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_VECTOR_MULTIPLY), SW_HIDE);

        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_ADD), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_SUBTRACT), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_DOT), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_CROSS), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_MIXED), SW_SHOW); //no 3rd vector 
    }
    else if ((showMatrixButtonsFirst && showVectorButtonsSecond) || (showVectorButtonsFirst && showMatrixButtonsSecond)) 
    {
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_ADD), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_SUBTRACT), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_DOT), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_CROSS), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_VECTOR_MIXED), SW_HIDE);

        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_MULTIPLY), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_ADD), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_SUBTRACT), SW_HIDE);

        ShowWindow(GetDlgItem(hwnd, ID_BTN_MATRIX_VECTOR_MULTIPLY), SW_HIDE); //do not exist
    }
}

// Функция отображения диалога ввода
void ShowInputDialog(HWND hwnd, int buttonId)
{
    DialogBoxParam(hInst, MAKEINTRESOURCE(ID_DIALOG), hwnd, DialogProc, buttonId);
}

// Функция копирования текста в буфер обмена
void CopyToClipboard(HWND hwnd)
{
    int len = GetWindowTextLength(hwnd);
    if (len > 0)
    {
        HGLOBAL hMem = GlobalAlloc(GHND, (len + 1) * sizeof(TCHAR));
        if (hMem)
        {
            LPTSTR pMem = (LPTSTR)GlobalLock(hMem);
            if (pMem)
            {
                GetWindowText(hwnd, pMem, len + 1);
                GlobalUnlock(hMem);
                OpenClipboard(hwnd);
                EmptyClipboard();
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
        }
    }
}

INT_PTR CALLBACK TransposeResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY:
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_PASTE_FIRST:
            // Paste result into the initiating element
            if (elementId == ID_BTN_TRANSPOSE_FIRST) 
            {
                globalMatrixFirst = stringToMatrix(resultText, globalMatrixFirst.size(), globalMatrixFirst[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Матрица");
            }
            else if (elementId == ID_BTN_TRANSPOSE_SECOND) 
            {
                globalMatrixSecond = stringToMatrix(resultText, globalMatrixSecond.size(), globalMatrixSecond[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Матрица");
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ScalarInputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static double* scalar;

    switch (message) 
    {
    case WM_INITDIALOG:
        scalar = (double*)lParam;
        return (INT_PTR)TRUE;
        SetNewFont(hDlg);

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_OK:
        {
            wchar_t buffer[1024];
            GetDlgItemText(hDlg, ID_EDIT_SCALAR, buffer, 1024);
            *scalar = _wtof(buffer);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case ID_BTN_CANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ScalarResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_PASTE_FIRST:
            // Paste result into the initiating element
            if (elementId == ID_BTN_MULTIPLY_FIRST) 
            {
                globalMatrixFirst = stringToMatrix(resultText, globalMatrixFirst.size(), globalMatrixFirst[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Матрица");
            }
            else if (elementId == ID_BTN_MULTIPLY_SECOND) 
            {
                globalMatrixSecond = stringToMatrix(resultText, globalMatrixSecond.size(), globalMatrixSecond[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Матрица");
            }
            else if (elementId == ID_BTN_MULTIPLY_VECTOR_FIRST) 
            {
                globalVectorFirst = stringToVector(resultText, globalVectorFirst.size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Вектор");
            }
            else if (elementId == ID_BTN_MULTIPLY_VECTOR_SECOND) 
            {
                globalVectorSecond = stringToVector(resultText, globalVectorSecond.size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Вектор");
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK PowerInputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static int* power;

    switch (message) 
    {
    case WM_INITDIALOG:
        power = (int*)lParam;
        return (INT_PTR)TRUE;
        SetNewFont(hDlg);
    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_OK:
        {
            wchar_t buffer[1024];
            GetDlgItemText(hDlg, ID_EDIT_POWER, buffer, 1024);
            *power = _wtoi(buffer);
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case ID_BTN_CANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK PowerResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_POWER_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_POWER:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_PASTE_POWER_FIRST:
            // Paste result into the initiating element
            if (elementId == ID_BTN_POWER_FIRST) 
            {
                globalMatrixFirst = stringToMatrix(resultText, globalMatrixFirst.size(), globalMatrixFirst[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Матрица");
            }
            else if (elementId == ID_BTN_POWER_SECOND) 
            {
                globalMatrixSecond = stringToMatrix(resultText, globalMatrixSecond.size(), globalMatrixSecond[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Матрица");
            }
            else if (elementId == ID_BTN_POWER_VECTOR_FIRST) 
            {
                globalVectorFirst = stringToVector(resultText, globalVectorFirst.size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Вектор");
            }
            else if (elementId == ID_BTN_POWER_VECTOR_SECOND) 
            {
                globalVectorSecond = stringToVector(resultText, globalVectorSecond.size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Вектор");
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK DeterminantResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_DETERMINANT_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_DETERMINANT:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK VectorLengthResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_VECTOR_LENGTH_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_VECTOR_LENGTH:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK InverseResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_INVERSE_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_INVERSE:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_PASTE_INVERSE_FIRST:
            // Paste result into the initiating element
            if (elementId == ID_BTN_INVERSE_FIRST) 
            {
                globalMatrixFirst = stringToMatrix(resultText, globalMatrixFirst.size(), globalMatrixFirst[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Матрица");
            }
            else if (elementId == ID_BTN_INVERSE_SECOND) 
            {
                globalMatrixSecond = stringToMatrix(resultText, globalMatrixSecond.size(), globalMatrixSecond[0].size());
                UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Матрица");
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK RankResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int elementId;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        elementId = params->second;
        SetDlgItemText(hDlg, ID_STATIC_RANK_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_RANK:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK MatrixOpResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int operationType; // 1 for addition, 2 for subtraction, 3 for multiplication

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        operationType = params->second;
        SetDlgItemText(hDlg, ID_STATIC_MATRIX_OP_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_MATRIX_OP:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_PASTE_MATRIX_OP_FIRST:
            // Paste result into the first element
            globalMatrixFirst = stringToMatrix(resultText, globalMatrixFirst.size(), globalMatrixFirst[0].size());
            UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Матрица");
            break;

        case ID_BTN_PASTE_MATRIX_OP_SECOND:
            // Paste result into the second element
            globalMatrixSecond = stringToMatrix(resultText, globalMatrixSecond.size(), globalMatrixSecond[0].size());
            UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Матрица");
            break;

        case ID_BTN_OK:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK VectorOpResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;
    static int operationType; // 1 for addition, 2 for subtraction

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        std::pair<std::wstring*, int>* params = (std::pair<std::wstring*, int>*)lParam;
        resultText = *(params->first);
        operationType = params->second;
        SetDlgItemText(hDlg, ID_STATIC_VECTOR_OP_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_VECTOR_OP:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_PASTE_VECTOR_OP_FIRST:
            // Paste result into the first element
            globalVectorFirst = stringToVector(resultText, globalVectorFirst.size());
            UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_FIRST, resultText, L"Вектор");
            break;

        case ID_BTN_PASTE_VECTOR_OP_SECOND:
            // Paste result into the second element
            globalVectorSecond = stringToVector(resultText, globalVectorSecond.size());
            UpdateResultStatic(GetParent(hDlg), ID_BTN_INPUT_SECOND, resultText, L"Вектор");
            break;

        case ID_BTN_OK_VECTOR_OP:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK VectorProdResultDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    static std::wstring resultText;

    switch (message) 
    {
    case WM_INITDIALOG:
    {
        resultText = *(std::wstring*)lParam;
        SetDlgItemText(hDlg, ID_STATIC_VECTOR_PROD_RESULT, resultText.c_str());
        SetNewFont(hDlg);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_COPY_VECTOR_PROD:
            
            if (OpenClipboard(hDlg)) 
            {
                EmptyClipboard();
                size_t size = (resultText.size() + 1) * sizeof(wchar_t);
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                memcpy(GlobalLock(hMem), resultText.c_str(), size);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
                CloseClipboard();
            }
            break;

        case ID_BTN_OK_VECTOR_PROD:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int currentPage = 0;

    switch (uMsg)
    {
    case WM_CREATE:
        CreateControls(hwnd);
        UpdateControlPositions(hwnd);
        ShowPage(hwnd, currentPage);
        return 0;

    case WM_SIZE:
        UpdateControlPositions(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (INT_PTR)GetStockObject(NULL_BRUSH);
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
        {
        case ID_BTN_START:
            currentPage = 1;
            ShowPage(hwnd, currentPage);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_BTN_EXIT:
            PostQuitMessage(0);
            break;
        case ID_BTN_BACK:
            currentPage = 0;
            ShowPage(hwnd, currentPage);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_BTN_INPUT_FIRST:
        case ID_BTN_INPUT_SECOND:
            ShowInputDialog(hwnd, LOWORD(wParam));
            break;
        case ID_BTN_TRANSPOSE_FIRST:
            if (!isFirstVector && !globalMatrixFirst.empty()) 
            {
                Matrix transposed = transpose(globalMatrixFirst);
                std::wstring result = matrixToString(transposed);
                std::pair<std::wstring*, int> params = { &result, ID_BTN_TRANSPOSE_FIRST };
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_RESULT), hwnd, TransposeResultDlgProc, (LPARAM)&params);
            }
            break;
        case ID_BTN_TRANSPOSE_SECOND:
            if (!isSecondVector && !globalMatrixSecond.empty()) 
            {
                Matrix transposed = transpose(globalMatrixSecond);
                std::wstring result = matrixToString(transposed);
                std::pair<std::wstring*, int> params = { &result, ID_BTN_TRANSPOSE_SECOND };
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_RESULT), hwnd, TransposeResultDlgProc, (LPARAM)&params);
            }
            break;
        case ID_BTN_MULTIPLY_VECTOR_FIRST:
        case ID_BTN_MULTIPLY_VECTOR_SECOND:
        case ID_BTN_MULTIPLY_FIRST:
        case ID_BTN_MULTIPLY_SECOND:
        {
            int buttonId = LOWORD(wParam);
            bool isVector = (buttonId == ID_BTN_MULTIPLY_VECTOR_FIRST || buttonId == ID_BTN_MULTIPLY_VECTOR_SECOND);
            double scalar = 0.0;

            // Show scalar input dialog
            if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_INPUT), hwnd, ScalarInputDlgProc, (LPARAM)&scalar) == ID_BTN_OK) 
            {
                if (buttonId == ID_BTN_MULTIPLY_FIRST || buttonId == ID_BTN_MULTIPLY_VECTOR_FIRST) 
                {
                    if (!isFirstVector && !globalMatrixFirst.empty()) 
                    {
                        Matrix resultMatrix = multiplyMatrixByScalar(globalMatrixFirst, scalar);
                        std::wstring result = matrixToString(resultMatrix);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_RESULT), hwnd, ScalarResultDlgProc, (LPARAM)&params);
                    }
                    else if (isFirstVector) {
                        Vector resultVector = multiplyVectorByScalar(globalVectorFirst, scalar);
                        std::wstring result = vectorToString(resultVector);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_RESULT), hwnd, ScalarResultDlgProc, (LPARAM)&params);
                    }
                }
                else if (buttonId == ID_BTN_MULTIPLY_SECOND || buttonId == ID_BTN_MULTIPLY_VECTOR_SECOND) 
                {
                    if (!isSecondVector && !globalMatrixSecond.empty()) 
                    {
                        Matrix resultMatrix = multiplyMatrixByScalar(globalMatrixSecond, scalar);
                        std::wstring result = matrixToString(resultMatrix);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_RESULT), hwnd, ScalarResultDlgProc, (LPARAM)&params);
                    }
                    else if (isSecondVector) 
                    {
                        Vector resultVector = multiplyVectorByScalar(globalVectorSecond, scalar);
                        std::wstring result = vectorToString(resultVector);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SCALAR_RESULT), hwnd, ScalarResultDlgProc, (LPARAM)&params);
                    }
                }
            }
        }
        break;
        case ID_BTN_POWER_VECTOR_FIRST:
        case ID_BTN_POWER_VECTOR_SECOND:
        case ID_BTN_POWER_FIRST:
        case ID_BTN_POWER_SECOND:
        {
            int buttonId = LOWORD(wParam);
            bool isVector = (buttonId == ID_BTN_POWER_VECTOR_FIRST || buttonId == ID_BTN_POWER_VECTOR_SECOND);
            int power = 0;

            // Show power input dialog
            if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_POWER_INPUT), hwnd, PowerInputDlgProc, (LPARAM)&power) == ID_BTN_OK) 
            {
                if (buttonId == ID_BTN_POWER_FIRST || buttonId == ID_BTN_POWER_VECTOR_FIRST) 
                {
                    if (!isFirstVector && !globalMatrixFirst.empty()) 
                    {
                        if (!isSquareMatrix(globalMatrixFirst)) 
                        {
                            MessageBox(hwnd, L"Матрица должна быть квадратной для возведения в степень.", L"Ошибка", MB_ICONERROR);
                            break;
                        }
                        Matrix resultMatrix = powerMatrix(globalMatrixFirst, power);
                        std::wstring result = matrixToString(resultMatrix);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_POWER_RESULT), hwnd, PowerResultDlgProc, (LPARAM)&params);
                    }
                    else if (isFirstVector) 
                    {
                        Vector resultVector = powerVector(globalVectorFirst, power);
                        std::wstring result = vectorToString(resultVector);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_POWER_RESULT), hwnd, PowerResultDlgProc, (LPARAM)&params);
                    }
                }
                else if (buttonId == ID_BTN_POWER_SECOND || buttonId == ID_BTN_POWER_VECTOR_SECOND) 
                {
                    if (!isSecondVector && !globalMatrixSecond.empty()) 
                    {
                        if (!isSquareMatrix(globalMatrixSecond)) 
                        {
                            MessageBox(hwnd, L"Матрица должна быть квадратной для возведения в степень.", L"Ошибка", MB_ICONERROR);
                            break;
                        }
                        Matrix resultMatrix = powerMatrix(globalMatrixSecond, power);
                        std::wstring result = matrixToString(resultMatrix);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_POWER_RESULT), hwnd, PowerResultDlgProc, (LPARAM)&params);
                    }
                    else if (isSecondVector) 
                    {
                        Vector resultVector = powerVector(globalVectorSecond, power);
                        std::wstring result = vectorToString(resultVector);
                        std::pair<std::wstring*, int> params = { &result, buttonId };
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_POWER_RESULT), hwnd, PowerResultDlgProc, (LPARAM)&params);
                    }
                }
            }
        }
        break;
        case ID_BTN_DETERMINANT_FIRST:
        case ID_BTN_DETERMINANT_SECOND:
        {
            int buttonId = LOWORD(wParam);
            if ((buttonId == ID_BTN_DETERMINANT_FIRST && !isFirstVector && !globalMatrixFirst.empty()) ||
                (buttonId == ID_BTN_DETERMINANT_SECOND && !isSecondVector && !globalMatrixSecond.empty())) 
            {
                const Matrix& matrix = (buttonId == ID_BTN_DETERMINANT_FIRST) ? globalMatrixFirst : globalMatrixSecond;
                if (!isSquareMatrix(matrix)) 
                {
                    MessageBox(hwnd, L"Матрица должна быть квадратной для вычисления определителя.", L"Ошибка", MB_ICONERROR);
                    break;
                }
                try 
                {
                    double det = determinant(matrix);
                    std::wstring result = L"Определитель: " + std::to_wstring(det);
                    std::pair<std::wstring*, int> params = { &result, buttonId };
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DETERMINANT_RESULT), hwnd, DeterminantResultDlgProc, (LPARAM)&params);
                }
                catch (const std::exception& e) 
                {
                    MessageBox(hwnd, L"Произошла ошибка при вычислении определителя.", L"Ошибка", MB_ICONERROR);
                }
            }
        }
        break;
        case ID_BTN_LENGTH_FIRST:
        case ID_BTN_LENGTH_SECOND:
        {
            int buttonId = LOWORD(wParam);
            if ((buttonId == ID_BTN_LENGTH_FIRST && isFirstVector && !globalVectorFirst.empty()) ||
                (buttonId == ID_BTN_LENGTH_SECOND && isSecondVector && !globalVectorSecond.empty())) 
            {
                const Vector& vec = (buttonId == ID_BTN_LENGTH_FIRST) ? globalVectorFirst : globalVectorSecond;
                double length = vectorLength(vec);
                std::wstring result = L"Длина вектора: " + std::to_wstring(length);
                std::pair<std::wstring*, int> params = { &result, buttonId };
                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VECTOR_LENGTH_RESULT), hwnd, VectorLengthResultDlgProc, (LPARAM)&params);
            }
        }
        break;
        case ID_BTN_INVERSE_FIRST:
        case ID_BTN_INVERSE_SECOND:
        {
            int buttonId = LOWORD(wParam);
            if ((buttonId == ID_BTN_INVERSE_FIRST && !isFirstVector && !globalMatrixFirst.empty()) ||
                (buttonId == ID_BTN_INVERSE_SECOND && !isSecondVector && !globalMatrixSecond.empty())) 
            {
                const Matrix& matrix = (buttonId == ID_BTN_INVERSE_FIRST) ? globalMatrixFirst : globalMatrixSecond;
                if (!isSquareMatrix(matrix)) 
                {
                    MessageBox(hwnd, L"Матрица должна быть квадратной, чтобы существовала ее обратная матрица.", L"Error", MB_ICONERROR);
                    break;
                }
                try 
                {
                    Matrix invMatrix = inverse(matrix);
                    std::wstring result = matrixToString(invMatrix);
                    std::pair<std::wstring*, int> params = { &result, buttonId };
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INVERSE_RESULT), hwnd, InverseResultDlgProc, (LPARAM)&params);
                }
                catch (const std::runtime_error& e) 
                {
                    MessageBox(hwnd, L"Матрица является вырожденной (определитель равнен 0), у такой матрицы нет обратной матрицы.", L"Error", MB_ICONERROR);
                }
            }
        }
        break;
        case ID_BTN_RANK_FIRST:
        case ID_BTN_RANK_SECOND:
        {
            int buttonId = LOWORD(wParam);
            if ((buttonId == ID_BTN_RANK_FIRST && !isFirstVector && !globalMatrixFirst.empty()) ||
                (buttonId == ID_BTN_RANK_SECOND && !isSecondVector && !globalMatrixSecond.empty())) 
            {
                const Matrix& matrix = (buttonId == ID_BTN_RANK_FIRST) ? globalMatrixFirst : globalMatrixSecond;
                try 
                {
                    int rank = calculateRank(matrix);
                    std::wstring result = L"Ранг матрицы: " + std::to_wstring(rank);
                    std::pair<std::wstring*, int> params = { &result, buttonId };
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_RANK_RESULT), hwnd, RankResultDlgProc, (LPARAM)&params);
                }
                catch (const std::exception& e) 
                {
                    MessageBox(hwnd, L"Произошла ошибка при вычислении ранга.", L"Ошибка", MB_ICONERROR);
                }
            }
        }
        break;
        case ID_BTN_MATRIX_MULTIPLY:
        case ID_BTN_MATRIX_ADD:
        case ID_BTN_MATRIX_SUBTRACT:
        {
            int buttonId = LOWORD(wParam);
            if (!isFirstVector && !isSecondVector && !globalMatrixFirst.empty() && !globalMatrixSecond.empty()) 
            {
                try 
                {
                    Matrix resultMatrix;
                    if (buttonId == ID_BTN_MATRIX_MULTIPLY) 
                    {
                        resultMatrix = multiplyMatricesDiff(globalMatrixFirst, globalMatrixSecond);
                    }
                    else if (buttonId == ID_BTN_MATRIX_ADD) 
                    {
                        resultMatrix = addMatrices(globalMatrixFirst, globalMatrixSecond);
                    }
                    else if (buttonId == ID_BTN_MATRIX_SUBTRACT) 
                    {
                        resultMatrix = subtractMatrices(globalMatrixFirst, globalMatrixSecond);
                    }
                    std::wstring result = matrixToString(resultMatrix);
                    std::pair<std::wstring*, int> params = { &result, buttonId };
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MATRIX_OP_RESULT), hwnd, MatrixOpResultDlgProc, (LPARAM)&params);
                }
                catch (const std::exception& e) {
                    MessageBox(hwnd, L"Возникла какая-то проблема.", L"Error", MB_ICONERROR);
                }
            }
        }
        break;
        case ID_BTN_VECTOR_ADD:
        case ID_BTN_VECTOR_SUBTRACT:
        {
            int buttonId = LOWORD(wParam);
            if (isFirstVector && isSecondVector && !globalVectorFirst.empty() && !globalVectorSecond.empty()) 
            {
                try 
                {
                    Vector resultVector;
                    if (buttonId == ID_BTN_VECTOR_ADD) 
                    {
                        resultVector = addVectors(globalVectorFirst, globalVectorSecond);
                    }
                    else if (buttonId == ID_BTN_VECTOR_SUBTRACT) 
                    {
                        resultVector = subtractVectors(globalVectorFirst, globalVectorSecond);
                    }
                    std::wstring result = vectorToString(resultVector);
                    std::pair<std::wstring*, int> params = { &result, buttonId };
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VECTOR_OP_RESULT), hwnd, VectorOpResultDlgProc, (LPARAM)&params);
                }
                catch (const std::exception& e) 
                {
                    MessageBox(hwnd, L"Возникла какая-то проблема.", L"Error", MB_ICONERROR);
                }
            }
        }
        break;
        case ID_BTN_VECTOR_DOT:
        case ID_BTN_VECTOR_CROSS:
        case ID_BTN_VECTOR_MIXED:
        {
            int buttonId = LOWORD(wParam);
            if (isFirstVector && isSecondVector && !globalVectorFirst.empty() && !globalVectorSecond.empty())
            {
                try 
                {
                    std::wstring result;
                    if (buttonId == ID_BTN_VECTOR_DOT) 
                    {
                        double dotProd = dotProduct(globalVectorFirst, globalVectorSecond);
                        result = L"Скалярное произведение: " + std::to_wstring(dotProd);
                    }
                    else if (buttonId == ID_BTN_VECTOR_CROSS) 
                    {
                        Vector crossProd = crossProduct(globalVectorFirst, globalVectorSecond);
                        result = L"Векторное произведение: " + vectorToString(crossProd);
                    }
                    else if (buttonId == ID_BTN_VECTOR_MIXED) 
                    {
                        double mixedProd = mixedProduct(globalVectorFirst, globalVectorSecond, globalVectorSecond);
                        result = L"Смешанное произведение: " + std::to_wstring(mixedProd);
                    }
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VECTOR_PROD_RESULT), hwnd, VectorProdResultDlgProc, (LPARAM)&result);
                }
                catch (const std::exception& e) {
                    MessageBox(hwnd, L"Возникла какая-то проблема.", L"Error", MB_ICONERROR);
                }
            }
        }
        break;
        }
        return 0;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pInfo = (MINMAXINFO*)lParam;
        pInfo->ptMinTrackSize.x = 1200;
        pInfo->ptMinTrackSize.y = 800;
        return 0;
    }

    case WM_DESTROY:
        if (pImage) 
        {
            delete pImage;
        }
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);

        if (currentPage == 0 && pImage != nullptr) 
        {
            Gdiplus::Graphics graphics(hdc);
            int imgWidth = pImage->GetWidth() / 2.5;
            int imgHeight = pImage->GetHeight() / 2.5;
            int x = (ps.rcPaint.right - imgWidth) / 2;
            int y = 20;
            graphics.DrawImage(pImage, x, y, imgWidth, imgHeight);
        }

        if (currentPage == 1 && pImage != nullptr) 
        {
            Gdiplus::Graphics graphics(hdc);
            int imgWidth = pImage->GetWidth() / 2.5;
            int imgHeight = pImage->GetHeight() / 2.5;
            int x = (ps.rcPaint.right - imgWidth) / 2 + 30;
            int y = 20;
            graphics.DrawImage(pImage, x, y, imgWidth, imgHeight);
        }

        EndPaint(hwnd, &ps);
    }
    return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static int step = 0;
    static int buttonId;
    static BOOL isVector;
    static HWND hwndParent;
    
    HWND hChild = GetWindow(hDlg, GW_CHILD);

    switch (message) {
    case WM_INITDIALOG:
        step = 0;
        buttonId = lParam;
        hwndParent = GetParent(hDlg);
        CheckRadioButton(hDlg, ID_RADIO_VECTOR, ID_RADIO_MATRIX, ID_RADIO_VECTOR);
        ShowWindow(GetDlgItem(hDlg, ID_EDIT_ROWS), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, ID_EDIT_COLS), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, ID_STATIC_ROWS), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, ID_STATIC_COLS), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, ID_EDIT_INPUT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, ID_STATIC_INPUT), SW_HIDE);
        SetNewFont(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN_NEXT) 
        {
            if (step == 0) 
            {
                isVector = IsDlgButtonChecked(hDlg, ID_RADIO_VECTOR);
                step = 1;
                ShowWindow(GetDlgItem(hDlg, ID_EDIT_ROWS), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, ID_STATIC_ROWS), SW_SHOW);
                if (isVector) 
                {
                    SetDlgItemText(hDlg, ID_STATIC_ROWS, L"Введите размерность вектора:");
                }
                else 
                {
                    ShowWindow(GetDlgItem(hDlg, ID_EDIT_COLS), SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, ID_STATIC_COLS), SW_SHOW);
                    SetDlgItemText(hDlg, ID_STATIC_ROWS, L"Введите количество строк:");
                    SetDlgItemText(hDlg, ID_STATIC_COLS, L"Введите количество столбцов:");
                }
            }
            else if (step == 1) 
            {
                wchar_t buffer[10];
                GetDlgItemText(hDlg, ID_EDIT_ROWS, buffer, 10);
                int rows = _wtoi(buffer);

                if (isVector) 
                {
                    if (buttonId == ID_BTN_INPUT_FIRST) 
                    {
                        globalVectorFirst = Vector(rows);
                        isFirstVector = true;
                    }
                    else if (buttonId == ID_BTN_INPUT_SECOND) 
                    {
                        globalVectorSecond = Vector(rows);
                        isSecondVector = true;
                    }
                }
                else 
                {
                    GetDlgItemText(hDlg, ID_EDIT_COLS, buffer, 10);
                    int cols = _wtoi(buffer);
                    if (buttonId == ID_BTN_INPUT_FIRST) 
                    {
                        globalMatrixFirst = Matrix(rows, std::vector<double>(cols));
                        isFirstVector = false;
                    }
                    else if (buttonId == ID_BTN_INPUT_SECOND) 
                    {
                        globalMatrixSecond = Matrix(rows, std::vector<double>(cols));
                        isSecondVector = false;
                    }
                }

                step = 2;
                ShowWindow(GetDlgItem(hDlg, ID_EDIT_ROWS), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, ID_EDIT_COLS), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, ID_STATIC_ROWS), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, ID_STATIC_COLS), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, ID_EDIT_INPUT), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, ID_STATIC_INPUT), SW_SHOW);

                if (isVector) 
                {
                    SetDlgItemText(hDlg, ID_STATIC_INPUT, L"Введите элементы вектора:");
                }
                else 
                {
                    SetDlgItemText(hDlg, ID_STATIC_INPUT, L"Введите элементы матрицы (построчно):");
                }
            }
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == ID_BTN_OK) 
        {
            if (step != 2) 
            {
                MessageBox(hDlg, L"Пожалуйста, завершите ввод данных.", L"Ошибка", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            wchar_t inputBuffer[1024];
            GetDlgItemText(hDlg, ID_EDIT_INPUT, inputBuffer, 1024);

            try 
            {
                std::wstring result;
                std::wstring type;

                if (isVector) 
                {
                    if (buttonId == ID_BTN_INPUT_FIRST) 
                    {
                        globalVectorFirst = stringToVector(inputBuffer, globalVectorFirst.size());
                        result = vectorToString(globalVectorFirst);
                        type = L"Вектор";
                    }
                    else if (buttonId == ID_BTN_INPUT_SECOND) 
                    {
                        globalVectorSecond = stringToVector(inputBuffer, globalVectorSecond.size());
                        result = vectorToString(globalVectorSecond);
                        type = L"Вектор";
                    }
                }
                else 
                {
                    int rows = (buttonId == ID_BTN_INPUT_FIRST) ? globalMatrixFirst.size() : globalMatrixSecond.size();
                    int cols = (buttonId == ID_BTN_INPUT_FIRST) ? globalMatrixFirst[0].size() : globalMatrixSecond[0].size();
                    if (buttonId == ID_BTN_INPUT_FIRST) 
                    {
                        globalMatrixFirst = stringToMatrix(inputBuffer, rows, cols);
                        result = matrixToString(globalMatrixFirst);
                        type = L"Матрица";
                    }
                    else if (buttonId == ID_BTN_INPUT_SECOND) 
                    {
                        globalMatrixSecond = stringToMatrix(inputBuffer, rows, cols);
                        result = matrixToString(globalMatrixSecond);
                        type = L"Матрица";
                    }
                }

                UpdateResultStatic(hwndParent, buttonId, result, type);
            }
            catch (const std::exception& e) 
            {
                MessageBox(hDlg, L"Ошибка ввода данных", L"Ошибка", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == ID_BTN_CANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}