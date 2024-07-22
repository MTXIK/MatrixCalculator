#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H

#include <windows.h>
#include <gdiplus.h>
#include <string>

extern HINSTANCE hInst;
extern ULONG_PTR gdiplusToken;
extern Gdiplus::GdiplusStartupInput gdiplusStartupInput;

void CopyToClipboard(HWND hwnd);
void CreateControls(HWND hwnd);
void UpdateControlPositions(HWND hwnd);
void ShowPage(HWND hwnd, int page);
void UpdateResultStatic(HWND hwnd, int buttonId, const std::wstring& result, const std::wstring& type);
void ShowInputDialog(HWND hwnd, int buttonId);
Gdiplus::Bitmap* LoadImageFromResource(HMODULE hMod, const wchar_t* resid, const wchar_t* restype);
void SetNewFont(HWND hwndDlg);

#endif // UTILITY_FUNCTIONS_H
