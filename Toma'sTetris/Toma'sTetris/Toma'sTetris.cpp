#include "Toma'sTetris.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "Shapes.h"

#define ID_TIMER  1
#define TIME_INTERVAL 1000

;LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
#define BOARD_WIDTH 180
#define BOARD_HEIGHT 400
#define LONG_SLEEP 150

#define COLS 15
#define ROWS 30
#define EXTENDED_COLS 23
#define EXTENDED_ROWS 34

#define BOARD_LEFT 4
#define BOARD_RIGHT 18
#define BOARD_TOP 0
#define BOARD_BOTTOM 29

#define FIGURES_NUMBER 18

static int shape[4][4];
static int score = 0;

static int shape_row = 0;
static int shape_col = EXTENDED_COLS / 2 - 2;

static int** gBoard;
static int shape_num;

static int lattices_top = 40;
static int lattices_left = 20;
static int width = BOARD_WIDTH / COLS;
static int height = (BOARD_HEIGHT - lattices_top) / ROWS;

static HBRUSH white_brush = CreateSolidBrush(RGB(38, 50, 56));
static HBRUSH blue_brush = CreateSolidBrush(RGB(3, 169, 244));
static HPEN hPen = CreatePen(PS_SOLID, 1, RGB(38, 50, 56));
static HBRUSH turquoise_brush = CreateSolidBrush(RGB(24, 255, 255));
static bool gIsPause = false;

void InitGame(HWND);
void InitData();

void TypeInstruction(HWND);

void RandShape();

void AddScore();

void UpdateShapeRect(HWND hwnd);
void UpdateAllBoard(HWND hwnd);

void FallToGround();
void MoveDown(HWND hwnd);
void RePaintBoard(HDC hdc);
void PaintCell(HDC hdc, int x, int y, int color);
void ClearFullLine();

void RotateShape(HWND hwnd);
void MoveHori(HWND hwnd, int direction);
void RotateMatrix();
void ReRotateMatrix();
bool IsLegel();

void RespondKey(HWND hwnd, WPARAM wParam);

void PauseGame(HWND hwnd);
void WakeGame(HWND hwnd);

bool JudgeLose();
void LoseGame(HWND hwnd);
void ExitGame(HWND hwnd);
DWORD ReadRecord();
void WriteRecord(DWORD);
int WorkWithRecords(int);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("SUPER DUPER TETRIS");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, TEXT("SUPER DUPER TETRIS"),
		WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		BOARD_WIDTH + 300, BOARD_HEIGHT + 100,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	TypeInstruction(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static HDC hdcBuffer;
	static HBITMAP hBitMap;
	static PAINTSTRUCT ps;

	switch (message)
	{
	case WM_CREATE:
		SetTimer(hwnd, ID_TIMER, TIME_INTERVAL, TimerProc);
		InitGame(hwnd);
		TypeInstruction(hwnd);
		return 0;
	case WM_SIZE:
		TypeInstruction(hwnd);
		return 0;
	case WM_KEYDOWN:
		RespondKey(hwnd, wParam);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (score == 300) {
			KillTimer(hwnd, ID_TIMER);
			SetTimer(hwnd, ID_TIMER, TIME_INTERVAL / 10, TimerProc);
		}
		if (score == 1000) {
			KillTimer(hwnd, ID_TIMER);
			SetTimer(hwnd, ID_TIMER, TIME_INTERVAL / 100, TimerProc);
		}
		RePaintBoard(hdc);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

VOID CALLBACK TimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime)
{
	MoveDown(hwnd);
}

void InitGame(HWND hwnd)
{
	gBoard = new int* [EXTENDED_ROWS];
	for (int i = 0; i < EXTENDED_ROWS; i++)
	{
		gBoard[i] = new int[EXTENDED_COLS];
	}
	srand(time(0));
	InitData();
	UpdateAllBoard(hwnd);
}

void InitData() {
	for (int i = 0; i < EXTENDED_ROWS; i++)
	{
		for (int j = 0; j < EXTENDED_COLS; j++)
		{
			gBoard[i][j] = 0;
		}
	}

	for (int i = 0; i < EXTENDED_ROWS; i++)
	{
		for (int j = 0; j < BOARD_LEFT; j++)
		{
			gBoard[i][j] = 1;
		}
	}

	for (int i = 0; i < EXTENDED_ROWS; i++)
	{
		for (int j = BOARD_RIGHT + 1; j < EXTENDED_COLS; j++)
		{
			gBoard[i][j] = 1;
		}
	}

	for (int i = BOARD_BOTTOM + 1; i < EXTENDED_ROWS; i++)
	{
		for (int j = 0; j < EXTENDED_COLS; j++)
		{
			gBoard[i][j] = 1;
		}
	}

	gIsPause = false;
	score = 0;
	RandShape();
	return;
}

void TypeInstruction(HWND hwnd)
{
	TEXTMETRIC  tm;
	int cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth;
	HDC hdc = GetDC(hwnd);
	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
	cyChar = tm.tmHeight + tm.tmExternalLeading;
	int startX = 180;
	int startY = 40;
	TCHAR Instruction[100];

	wsprintf(Instruction, TEXT("INSTRUCTION: "));
	TextOut(hdc, startX + 40, startY, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("↑       CHANGE SHAPE"));
	TextOut(hdc, startX + 40, startY + cyChar * 3, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("←      MOVE LEFT"));
	TextOut(hdc, startX + 40, startY + cyChar * 5, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("→      MOVE RIGHT"));
	TextOut(hdc, startX + 40, startY + cyChar * 7, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("↓       MOVE DOWN"));
	TextOut(hdc, startX + 40, startY + cyChar * 9, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("SHIFT    PAUSE "));
	TextOut(hdc, startX + 40, startY + cyChar * 11, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("ESC       EXIT "));
	TextOut(hdc, startX + 40, startY + cyChar * 13, Instruction, lstrlen(Instruction));

	DWORD dwRecord = ReadRecord();
	wsprintf(Instruction, TEXT("YOUR CURRENT RECORD: %d"), dwRecord);
	TextOut(hdc, startX + 40, startY + cyChar * 18, Instruction, lstrlen(Instruction));

	ReleaseDC(hwnd, hdc);
}

void RandShape()
{
	shape_num = rand() % FIGURES_NUMBER;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			shape[i][j] = shapes[shape_num][i][j];
		}
	}
}

void UpdateAllBoard(HWND hwnd)
{
	static RECT rect;
	rect.left = lattices_left;
	rect.right = lattices_left + COLS * width + width;
	rect.top = lattices_top - 30;
	rect.bottom = lattices_top + ROWS * height;
	InvalidateRect(hwnd, &rect, false);
}

void UpdateShapeRect(HWND hwnd)
{
	static RECT rect;
	rect.left = lattices_left;
	rect.right = lattices_left + COLS * width + width;
	rect.top = lattices_top + (shape_row - 1) * height;
	rect.bottom = lattices_top + (shape_row + 4) * height;
	InvalidateRect(hwnd, &rect, false);
}

void RePaintBoard(HDC hdc)
{
	SetBkColor(hdc, RGB(255, 255, 255));
	SelectObject(hdc, hPen);
	TCHAR score_str[50];
	wsprintf(score_str, TEXT("Score: %5d "), score);
	TextOut(hdc, 20, 15, score_str, lstrlen(score_str));
	for (int i = BOARD_TOP; i <= BOARD_BOTTOM; i++)
	{
		for (int j = BOARD_LEFT; j <= BOARD_RIGHT; j++)
		{
			PaintCell(hdc, i, j, gBoard[i][j]);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (shape[i][j] == 1)
				PaintCell(hdc, shape_row + i, shape_col + j, shape[i][j]);
		}
	}
}

void PaintCell(HDC hdc, int x, int y, int color)
{
	if (x < BOARD_TOP || x > BOARD_BOTTOM ||
		y < BOARD_LEFT || y > BOARD_RIGHT)
	{
		return;
	}

	x -= BOARD_TOP;
	y -= BOARD_LEFT;

	int _left = lattices_left + y * width;
	int _right = lattices_left + y * width + width;
	int _top = lattices_top + x * height;
	int _bottom = lattices_top + x * height + height;

	MoveToEx(hdc, _left, _top, NULL);
	LineTo(hdc, _right, _top);
	MoveToEx(hdc, _left, _top, NULL);
	LineTo(hdc, _left, _bottom);
	MoveToEx(hdc, _left, _bottom, NULL);
	LineTo(hdc, _right, _bottom);
	MoveToEx(hdc, _right, _top, NULL);
	LineTo(hdc, _right, _bottom);

	if (color == 0)
	{
		SelectObject(hdc, white_brush);
	}
	else if (color == 1)
	{
		SelectObject(hdc, turquoise_brush);
	}
	else if (color == 2)
	{
		SelectObject(hdc, blue_brush);
	}

	Rectangle(hdc, _left, _top, _right, _bottom);
}

void RespondKey(HWND hwnd, WPARAM wParam)
{

	if (wParam == VK_ESCAPE)
	{
		ExitGame(hwnd);
		return;
	}

	if (wParam == VK_SHIFT)
	{
		gIsPause = !gIsPause;
		if (gIsPause == true)
		{
			PauseGame(hwnd);
			return;
		}
		else
		{
			WakeGame(hwnd);
			return;
		}
	}

	if (!gIsPause)
	{
		if (wParam == VK_UP)
		{
			RotateShape(hwnd);
			return;
		}
		if (wParam == VK_DOWN)
		{
			MoveDown(hwnd);
			return;
		}
		if (wParam == VK_LEFT)
		{
			MoveHori(hwnd, 0);
			return;
		}
		if (wParam == VK_RIGHT)
		{
			MoveHori(hwnd, 1);
			return;
		}
	}
}

void PauseGame(HWND hwnd)
{
	KillTimer(hwnd, ID_TIMER);
}

void WakeGame(HWND hwnd)
{
	SetTimer(hwnd, ID_TIMER, TIME_INTERVAL, TimerProc);
}

void ExitGame(HWND hwnd)
{

	SendMessage(hwnd, WM_KEYDOWN, VK_SPACE, 0);

	int exit = MessageBox(NULL, TEXT("Do you want to exit?"), TEXT("EXIT"), MB_YESNO);

	if (exit == IDYES)
	{
		SendMessage(hwnd, WM_DESTROY, NULL, 0);
	}

	else if (exit == IDNO)
	{
		return;
	}
}

void RotateShape(HWND hwnd)
{
	RotateMatrix();
	if (!IsLegel())
	{
		ReRotateMatrix();
	}
	UpdateShapeRect(hwnd);
	return;
}

bool IsLegel()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (shape[i][j] == 1 && (gBoard[shape_row + i][shape_col + j] == 1 || gBoard[shape_row + i][shape_col + j] == 2))
			{
				return false;
			}
		}
	}
	return true;
}

void RotateMatrix()
{

	int(*a)[4] = shape;
	int s = 0;
	for (int n = 4; n >= 1; n -= 2)
	{
		for (int i = 0; i < n - 1; i++)
		{
			int t = a[s + i][s];
			a[s + i][s] = a[s][s + n - i - 1];
			a[s][s + n - i - 1] = a[s + n - i - 1][s + n - 1];
			a[s + n - i - 1][s + n - 1] = a[s + n - 1][s + i];
			a[s + n - 1][s + i] = t;
		}
		s++;
	}
}

void ReRotateMatrix()
{
	int(*a)[4] = shape;
	int s = 0;
	for (int n = 4; n >= 1; n -= 2)
	{
		for (int i = 0; i < n - 1; i++)
		{
			int t = a[s + i][s];
			a[s + i][s] = a[s + n - 1][s + i];
			a[s + n - 1][s + i] = a[s + n - i - 1][s + n - 1];
			a[s + n - i - 1][s + n - 1] = a[s][s + n - i - 1];
			a[s][s + n - i - 1] = t;
		}
		s++;
	}
}

void MoveDown(HWND hwnd)
{
	shape_row++;

	if (!IsLegel())
	{
		shape_row--;
		if (JudgeLose())
		{
			LoseGame(hwnd);
			return;
		}

		FallToGround();
		ClearFullLine();
		UpdateAllBoard(hwnd);
		shape_row = 0;
		shape_col = EXTENDED_COLS / 2 - 2;
		RandShape();
	}
	UpdateShapeRect(hwnd);
}

bool JudgeLose()
{
	if (shape_row == 0)
	{
		return true;
	}
	return false;
}

void LoseGame(HWND hwnd)
{
	SendMessage(hwnd, WM_KEYDOWN, VK_SPACE, 0);
	TCHAR words[100];
	int IsRecord = WorkWithRecords(score);
	if (IsRecord == 1)
	{
		wsprintf(words, TEXT("Congratulations! \nYou set a new record %d. \nDo you want to try again?"), score);

	}
	else
	{
		wsprintf(words, TEXT("You have lost the Game. Total score: %d. \nDo you want to try again?"), score);
	}
	KillTimer(hwnd, ID_TIMER);
	int exit = MessageBox(NULL, words, TEXT("SUPER DUPER TETRIS"), MB_YESNO);

	if (exit == IDYES)
	{
		SendMessage(hwnd, WM_CREATE, NULL, 0);
		return;
	}
	else
	{
		SendMessage(hwnd, WM_DESTROY, NULL, 0);
		return;
	}
}

void FallToGround()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			gBoard[shape_row + i][shape_col + j] = shape[i][j] == 1 ? 2 : gBoard[shape_row + i][shape_col + j];
		}
	}
}

void ClearFullLine()
{
	for (int i = shape_row; i <= shape_row + 3; i++)
	{
		if (i > BOARD_BOTTOM)
		{
			continue;
		}

		bool there_is_blank = false;

		for (int j = BOARD_LEFT; j <= BOARD_RIGHT; j++)
		{
			if (gBoard[i][j] == 0) {
				there_is_blank = true;
				break;
			}
		}

		if (!there_is_blank)
		{
			AddScore();
			for (int r = i; r >= 1; r--) {
				for (int c = BOARD_LEFT; c <= BOARD_RIGHT; c++)
				{
					gBoard[r][c] = gBoard[r - 1][c];
				}
			}
		}
	}
}

void AddScore()
{
	score += 100;
}

void MoveHori(HWND hwnd, int direction)
{
	int temp = shape_col;

	if (direction == 0)
		shape_col--;
	else
		shape_col++;

	if (!IsLegel()) {
		shape_col = temp;
	}

	UpdateShapeRect(hwnd);
	return;
}

const TCHAR szRecordsFileName[] = L"record.dat";

DWORD ReadRecord()
{
	DWORD dwRecord, dwTemp;
	HANDLE hFile = CreateFile(szRecordsFileName, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 1;
	}
	ReadFile(hFile, &dwRecord, sizeof(dwRecord), &dwTemp, NULL);
	if (sizeof(dwRecord) != dwTemp)
	{
		CloseHandle(hFile);
		return 1;
	}
	CloseHandle(hFile);
	return dwRecord;
}

void WriteRecord(DWORD dwRecord)
{
	DWORD dwTemp;
	HANDLE hFile = CreateFile(szRecordsFileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return;
	}
	for (int i = 0; i < 3; i++)
	{
		WriteFile(hFile, &dwRecord, sizeof(dwRecord), &dwTemp, NULL);
	}

	CloseHandle(hFile);
}

int WorkWithRecords(int score) {
	DWORD dwRecord = ReadRecord();
	if (score > dwRecord)
	{
		WriteRecord(score);
		return 1;
	}
	return 0;
}