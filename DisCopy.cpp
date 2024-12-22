// DisCap.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "DisCopy.h"
#include <string>
#include "stb_iamge_write.h"


#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


struct Color
{
	unsigned char rgb[3] = {};
	bool inited = false;

	void init(unsigned char* init)
	{
		for (size_t i = 0; i < 3; i++)
		{
			rgb[i] = init[i];
		}
		inited = true;
	}

	bool operator!=(const Color& rhs) const
	{
		if (inited != rhs.inited)
			return false;

		if (!inited)
			return true;

		for (size_t i = 0; i < 3; i++)
		{
			if (rgb[i] != rhs.rgb[i])
				return false;
		}
	
		return true;
	}

	bool operator==(const Color& rhs) const
	{
		return !operator!=(rhs);
	}

};

// ==============
// FILEDS
HWND window;
HWND startNumberInput;
HWND xInput;
HWND yInput;
HWND savePath;
HWND statusLabel;


// ==============
// Vars
bool isRunning = false;
int startNumber = 0;

/// <===================UTILS================>
std::wstring getText(HWND hwnd) {
	size_t size = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
	if (!size) return L"";
	auto text = std::wstring(size, '\0');
	SendMessage(hwnd, WM_GETTEXT, text.size() + 1, reinterpret_cast<LPARAM>(text.c_str()));
	return text;
}

int getInt(HWND hwnd)
{
	auto text = getText(hwnd);

	std::string normal(text.begin(), text.end());
	return std::atoi(normal.data());
}

void setText(HWND hwnd, const std::wstring& text)
{
	SendMessage(hwnd, WM_SETTEXT, text.size() + 1, reinterpret_cast<LPARAM>(text.c_str()));
}

void setInt(HWND hwnd, int value)
{
	setText(hwnd, std::to_wstring(value));
}


// =========================
// Save a bitmap to a file
Color SaveBitmapToFile(HDC hdcMemory, HBITMAP hBitmapMemory, const std::wstring& fullPath, Color skipColor)
{
	// Get the bitmap info
	BITMAP bitmap;
	GetObject(hBitmapMemory, sizeof(BITMAP), &bitmap);

	// Create a bitmap info header
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// Create a bitmap info
	BITMAPINFO biInfo;
	biInfo.bmiHeader = bi;

	// Create a file
	HANDLE hFile = CreateFile(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Write the bitmap info header
	DWORD dwBytesWritten = 0;
	WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);

	// Write the bitmap data
	BYTE* pBitmapData = new BYTE[bitmap.bmWidth * bitmap.bmHeight * 3];
	GetDIBits(hdcMemory, hBitmapMemory, 0, bitmap.bmHeight, pBitmapData, &biInfo, DIB_RGB_COLORS);


	Color curMiddle;
	curMiddle.init(pBitmapData + (bitmap.bmWidth * bitmap.bmHeight / 4) * 3);

	if (skipColor != curMiddle)
		WriteFile(hFile, pBitmapData, bitmap.bmWidth * bitmap.bmHeight * 3, &dwBytesWritten, NULL);
	// Clean up
	delete[] pBitmapData;
	CloseHandle(hFile);

	return curMiddle;
}

struct Image
{
	int width;
	int height;
	int channels;
	std:uniqe_ptr<char> data;
	void save(std::string path)
	{
		stbi_write_png(path.data(), width, height, channels, data.get(), width * channels);
	}
};
// ==========
// Save a screenshot to a file
Color takeScreenshot(HWND hWnd, const std::wstring& fullFilePath, Color skipColor)
{
	// Get the screen size
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Create a compatible DC
	HDC hdcScreen = GetDC(NULL);
	HDC hdcCompatible = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
	HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcCompatible, hBitmap);

	// Copy the screen to the compatible DC
	BitBlt(hdcCompatible, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

	// Get the bitmap info
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	// Create a bitmap info header
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// Create a bitmap info
	BITMAPINFO biInfo;
	biInfo.bmiHeader = bi;

	BYTE* pBitmapData = new BYTE[bitmap.bmWidth * bitmap.bmHeight * 3];
	GetDIBits(hdcCompatible, hBitmap, 0, bitmap.bmHeight, pBitmapData, &biInfo, DIB_RGB_COLORS);

	delete[] pBitmapData;


	// Create a file
	//HANDLE hFile = CreateFile(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	//Color curMiddle = SaveBitmapToFile(hdcCompatible, hBitmap, fullFilePath, skipColor);

	//CloseHandle(hFile);

	// Clean up
	SelectObject(hdcCompatible, hBitmapOld);
	DeleteDC(hdcCompatible);
	DeleteObject(hBitmap);
	ReleaseDC(NULL, hdcScreen);

	return curMiddle;
}



void click()
{
	int x = getInt(xInput);
	int y = getInt(yInput);
	//SetCursorPos(x, y);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}


Color missingColor;
void pipeline()
{
	auto folder = getText(savePath);
	if (folder.back() != '\\' && folder.back() != '/')
		folder += '\\';

	int value = getInt(startNumberInput);
	std::wstring filename = folder + std::to_wstring(value) + L".png";

	Color currentColor = SaveScreenshot(window, filename, missingColor);

	// Skip if there is no image loaded yet
	if (currentColor == missingColor)
		return;

	_tprintf(_T("Move to the next iamge\n"));
	click();
	// Set startNumberInput to the next value
	setInt(startNumberInput, value + 1);


	// Get the missing color
	if (!missingColor.inited)
	{
		Sleep(300);
		missingColor = SaveScreenshot(window, folder, missingColor);
	}
}

void checkHotKeys(const MSG& msg)
{
	if (msg.message == WM_HOTKEY)
	{
		_tprintf(_T("WM_HOTKEY received\n"));
		switch (msg.wParam)
		{
		case 1:
			isRunning = true;
			break;
		case 2:
			isRunning = false;
			break;
		}
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Разместите код здесь.

	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_DISCOPY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DISCOPY));

	MSG msg;

	// Цикл основного сообщения:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (isRunning)
		{
			pipeline();
		}
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			checkHotKeys(msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DISCOPY));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DISCOPY);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Create two number input boxes and one text box

	int y = 10;
	HWND xInputLabel = CreateWindowW(L"STATIC", L"X:", WS_CHILD | WS_VISIBLE,
		10, y, 100, 20, hWnd, nullptr, hInstance, NULL);
	xInput = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
		120, y, 100, 20, hWnd, NULL, hInstance, NULL);

	y += 30;
	HWND yInputLabel = CreateWindowW(L"STATIC", L"Y:", WS_CHILD | WS_VISIBLE,
		10, y, 100, 20, hWnd, NULL, hInstance, NULL);
	yInput = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
		120, y, 100, 20, hWnd, nullptr, hInstance, NULL);

	y += 30;
	HWND hwndTextBoxLabel = CreateWindowW(L"STATIC", L"Save folder:", WS_CHILD | WS_VISIBLE,
		10, y, 100, 20, hWnd, NULL, hInstance, NULL);
	savePath = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		120, y, 200, 20, hWnd, NULL, hInstance, NULL);


	y += 30;
	HWND startNumberLabel = CreateWindowW(L"STATIC", L"Start int x:", WS_CHILD | WS_VISIBLE,
		10, y, 100, 20, hWnd, NULL, hInstance, NULL);
	startNumberInput = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
		120, y, 100, 20, hWnd, NULL, hInstance, NULL);


	y += 30;
	HWND label = CreateWindowW(L"STATIC", L"Press F5 to start and F8 to finish", WS_CHILD | WS_VISIBLE | WS_BORDER,
		5, y, 300, 20, hWnd, NULL, hInstance, NULL);

	y += 30;
	statusLabel = CreateWindowW(L"STATIC", L"Waiting to start...", WS_CHILD | WS_VISIBLE | WS_BORDER,
		5, y, 300, 20, hWnd, NULL, hInstance, NULL);

	//CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 70, 100, 80, 25, hWnd, (HMENU)IDC_BUTTON, hInstance, NULL);


	setInt(xInput, 400);
	setInt(yInput, 50);
	setInt(startNumberInput, 1);
	setText(savePath, L"D:\\Edu\\Test");

	// Register hotkey for F7
	RegisterHotKey(hWnd, 1, 0, VK_F5);
	RegisterHotKey(hWnd, 2, 0, VK_F8);
	return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Разобрать выбор в меню:
		switch (wmId)
		{
		case IDC_BUTTON:
			/*
			IDC_BUTTON is id given to button
			This id is specified while creating window. If you are using
			CreateWindow then it will be as follows:

			CreateWindow("BUTTON", 0, WS_CHILD|WS_VISIBLE, 70, 70, 80, 25, g_hWnd, (HMENU)IDC_BUTTON, hInst, 0);

			and in resource.rc file or in the beginning of code. Do "#define IDC_BUTTON                  3456"
			3456 is just a reference you can give any other no as well. Prefer using bigger number so that they can not conflict with existing ones.
			*/

			//do whatever you want to do here when button is pressed
			break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Добавьте сюда любой код прорисовки, использующий HDC...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
