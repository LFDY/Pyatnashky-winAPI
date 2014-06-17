/*
��������� �������� � �������������� WINAPI.
@copyright S.Aksyonchikov
@author Svyatoslav Aksyonchikov
@e-mail axesvt@gmail.com
*/
#include <windows.h>
#include <tchar.h>		//for both ANSI and UNICODE strings
#include <windowsx.h>
#include <vector>
#include <ctime>
#include "resource.h"	//����������� ����� ��������
#include "player.h"

#pragma warning(disable:4996)	//���� �� ��������� �������� ��� ������ �� ��������

static TCHAR appTitle[] = _T("PyatnashkiWindow");	//����� ���������
HINSTANCE hInst;

HWND button[15];

Player currentPlayer;

int globalCount;		//������� �����

int source[16];
int origin[16];

int buttonPosition[15];	

struct{
	int x;
	int y;
	int pos;
}freeSpace;		//���������� � ������� ������ ������


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);		//���������� ��������� �� ��������� ����
BOOL CALLBACK PropertiesProc(HWND, UINT, WPARAM, LPARAM);	//���������� ��������� �� ����������� ���� "���������"

BOOL Register(HINSTANCE hInstance);							//�-�� ����������� ������ ����
HWND Create(HINSTANCE hInstance, int nCmdShow);				//�-�� �������� ������� ���� �� ������ ����� ���������� ������

void but_create(HWND hwnd, RECT rect);						//�-�� �������� ������

TCHAR* getTable();											//�-�� �������� ������-������� ��������
void finishGame();											//�-��, ������������� ��� �������� ���������� ����

BOOL checkComplete(){										//�������� �� ��������� ����
	for (int i = 0; i < 16; ++i){
		if (source[i] != origin[i]) return FALSE;			//��������� �������� ������������ 
	}														//������ � ����������
	return TRUE;
}

void swapArr(int s1, int s2){								//������ ������� ��-��� ��������� ������
	int tmp = source[s1];
	source[s1] = source[s2];
	source[s2] = tmp;
}

//������ ������� ������
void swapBut(HWND hwnd, HWND but, int nBut){	

	//������� ����� ������ �������� ������
	if (buttonPosition[nBut] != freeSpace.pos - 1 && buttonPosition[nBut] != freeSpace.pos + 1 && buttonPosition[nBut] != freeSpace.pos - 4 && buttonPosition[nBut] != freeSpace.pos + 4) return;
	
	++globalCount;

	RECT rect;
	GetClientRect(hwnd, &rect);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	//������ ������� ������� ������ ������ � ������� ������
	int tmpPos = buttonPosition[nBut];
	buttonPosition[nBut] = freeSpace.pos;
	freeSpace.pos = tmpPos;

	RECT rt;
	GetWindowRect(but, &rt);
	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rt, 2);

	//����������� ������
	SetWindowPos(but, HWND_TOP, freeSpace.x, freeSpace.y, width / 4, height / 4, SWP_SHOWWINDOW);

	//��������� ��������� ������ ������
	freeSpace.x = rt.left;
	freeSpace.y = rt.top;

	swapArr(buttonPosition[nBut] - 1, freeSpace.pos - 1);
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE prevInstance,
	LPSTR lpCmdLine,
	int nCmdShow){

	MSG msg;
	hInst = hInstance;

	if (!Register(hInstance)) return FALSE;
	if (!Create(hInstance, nCmdShow)) return FALSE;

	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
HWND Create(HINSTANCE hInstance, int nCmdShow){
	HWND hwnd = CreateWindow((LPCWSTR)appTitle, (LPCWSTR)_T("��������"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		400, 400,
		NULL, NULL, hInstance, NULL);
	
	if (!hwnd){
		return FALSE;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	return hwnd;
}

BOOL Register(HINSTANCE hInstance){
	WNDCLASS WndClass;

	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	WndClass.lpszMenuName = (LPCWSTR)IDR_MENU1;
	WndClass.lpszClassName = (LPCWSTR)appTitle;

	return (RegisterClass(&WndClass) != 0);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	RECT rect;

	switch (message){
		//���������, ���������� ��� �������� ����
		case WM_CREATE:
			for (int i = 0; i < 15; ++i){
				origin[i] = i + 1;
			}
			origin[15] = 0;
		break;

		//���������, ���������� ��� ������� ������
		case WM_COMMAND:

			switch (wParam){
			//������ ������ ���� "����� ����"
			case IDM_New:
				if (button[0]){
					//���� ���� ��� ��������, ����������, ������������� �� ����� ������������ ��������� ����� ����
					if (MessageBox(NULL, L"�� �������, ��� ������ ������ ����� ����? ���� ������� �������� ����� �������.", L"����� ����", MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL) == IDNO) break;
				}
				GetClientRect(hwnd, &rect);
				but_create(hwnd, rect);
				break;

			//������ ������ "���������"
			case IDM_Properties:{
				//����� ����������� ���� �� ����� ��������
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_PROP), hwnd, (DLGPROC)PropertiesProc);
				break; }
			//������ ������ "�������"
			case IDM_Records:{
				TCHAR *txt = getTable();
				MessageBox(NULL, txt, _T("������� ��������"), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
				break;
			}
			//������ "�����"
			case IDM_Exit:
				if (button[0]){
					if (MessageBox(NULL, L"�� �������, ��� ������ ����� �� ����? ���� ������� �������� ����� �������.", L"�����", MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL) == IDNO) break;
				}
				SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				break;
			//������ "����������"
			case IDM_Control:
				MessageBox(NULL, L"���������� �������������� � ������� ����. ��������� �������� �������� ������ � ������ �������. ��� ������� �� ������ ��� �������� ����� ������ ������.", _T("����������"), MB_OK | MB_ICONQUESTION | MB_TASKMODAL);
				break;
			//������ "� ���������"
			case IDM_About:
				MessageBox(NULL, _T("���� \"��������\" v.1.0\n\n\n�����: ��������� �.�."), _T("� ���������"), MB_OK | MB_ICONQUESTION | MB_TASKMODAL);
				break;
			
			//����� ��� ��������� ������� ������ ��������������� ��������
			case 10001:
				swapBut(hwnd, button[0], 0);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10002:
				swapBut(hwnd, button[1], 1);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10003:
				swapBut(hwnd, button[2], 2);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10004:
				swapBut(hwnd, button[3], 3);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10005:
				swapBut(hwnd, button[4], 4);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10006:
				swapBut(hwnd, button[5], 5);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10007:
				swapBut(hwnd, button[6], 6);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10008:
				swapBut(hwnd, button[7], 7);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10009:
				swapBut(hwnd, button[8], 8);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10010:
				swapBut(hwnd, button[9], 9);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10011:
				swapBut(hwnd, button[10], 10);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10012:
				swapBut(hwnd, button[11], 11);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10013:
				swapBut(hwnd, button[12], 12);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10014:
				swapBut(hwnd, button[13], 13);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10015:{
				swapBut(hwnd, button[14], 14);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("�����������!\n������ ������ ����� ����?"), _T("������"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;
				}
			}
			break;
		//������� ��������� ���������� ��������� �������� ���� (������ ������������)
		case WM_GETMINMAXINFO:
			{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = 400;
			lpMMI->ptMinTrackSize.y = 400;

			lpMMI->ptMaxTrackSize.x = 400;
			lpMMI->ptMaxTrackSize.y = 400;
			}
			break;
		//�������� ����
		case WM_DESTROY:{
			PostQuitMessage(0);
			break;
		}
	//��������� ���� ��������� ��������� ������� �������
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void but_create(HWND hwnd, RECT rect){
	//���� ��� ������������ ����, ������� � ������ ������
	for (int i = 0; i < 15; ++i){
		if (button[i]){
			DestroyWindow(button[i]);
		}
		else break;
	}
	//������� ��������� �������� ���������� ����������
	globalCount = 0;
	source[15] = 0;
	srand(unsigned(time(NULL)));
	std::vector<int> v;

	//��� �� ��������� ������� ����������� ������
	for (int i = 1; i < 16; ++i){
		v.push_back(i);
	}
	for (int i = 15; i > 0; --i){
		buttonPosition[i - 1] = i;
		int tmp = rand() % i;
		source[15 - i] = v.at(tmp);
		v.erase(v.begin() + tmp);
	}

	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	freeSpace.x = width / 4 * 3;
	freeSpace.y = height / 4 * 3; 
	freeSpace.pos = 16;

	wchar_t tmp[30];

	//������ ������ ������ �� �������
	//--------------------------------------------------------------------------------

	_itow_s(source[0], tmp, 10);
	button[0] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width/4 * 0, 0, width/4, height/4, hwnd, (HMENU)10001, hInst, NULL);

	_itow_s(source[1], tmp, 10);
	button[1] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 1, 0, width / 4, height / 4, hwnd, (HMENU)10002, hInst, NULL);

	_itow_s(source[2], tmp, 10);
	button[2] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 2, 0, width / 4, height / 4, hwnd, (HMENU)10003, hInst, NULL);

	_itow_s(source[3], tmp, 10);
	button[3] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 3, 0, width / 4, height / 4, hwnd, (HMENU)10004, hInst, NULL);

	_itow_s(source[4], tmp, 10);
	button[4] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 0, height / 4 * 1, width / 4, height / 4, hwnd, (HMENU)10005, hInst, NULL);

	_itow_s(source[5], tmp, 10);
	button[5] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 1, height / 4 * 1, width / 4, height / 4, hwnd, (HMENU)10006, hInst, NULL);

	_itow_s(source[6], tmp, 10);
	button[6] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 2, height / 4 * 1, width / 4, height / 4, hwnd, (HMENU)10007, hInst, NULL);

	_itow_s(source[7], tmp, 10);
	button[7] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 3, height / 4 * 1, width / 4, height / 4, hwnd, (HMENU)10008, hInst, NULL);

	_itow_s(source[8], tmp, 10);
	button[8] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 0, height / 4 * 2, width / 4, height / 4, hwnd, (HMENU)10009, hInst, NULL);

	_itow_s(source[9], tmp, 10);
	button[9] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 1, height / 4 * 2, width / 4, height / 4, hwnd, (HMENU)10010, hInst, NULL);

	_itow_s(source[10], tmp, 10);
	button[10] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 2, height / 4 * 2, width / 4, height / 4, hwnd, (HMENU)10011, hInst, NULL);

	_itow_s(source[11], tmp, 10);
	button[11] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 3, height / 4 * 2, width / 4, height / 4, hwnd, (HMENU)10012, hInst, NULL);

	_itow_s(source[12], tmp, 10);
	button[12] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 0, height / 4 * 3, width / 4, height / 4, hwnd, (HMENU)10013, hInst, NULL);

	_itow_s(source[13], tmp, 10);
	button[13] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 1, height / 4 * 3, width / 4, height / 4, hwnd, (HMENU)10014, hInst, NULL);

	_itow_s(source[14], tmp, 10);
	button[14] = CreateWindow(_T("button"), tmp, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		width / 4 * 2, height / 4 * 3, width / 4, height / 4, hwnd, (HMENU)10015, hInst, NULL);

	//---------------------------------------------------------------------------------------------
}

BOOL CALLBACK PropertiesProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch (msg){
	//������������� �������
	case WM_INITDIALOG:
		//� ��������� ���� ����� ��� �������� ������
		SetDlgItemText(hwnd, IDC_TEXTBOX1, currentPlayer.getName());
		return FALSE;
		break;
	//��������� ������� ������
	case WM_COMMAND:{
		switch (LOWORD(wParam)){
		case IDOK:{
			TCHAR *tmp = new TCHAR[100];
			GetDlgItemText(hwnd, IDC_TEXTBOX1, tmp, 100);
			currentPlayer.setName(tmp);
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
		break; }
		case IDCANCEL:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
		break;
		//������ ��������� ������� ��������
		case IDC_RESTORE:
			if (MessageBox(NULL, L"�� �������, ��� ������ �������� ������� ��������?", L"�����", MB_YESNO | MB_TASKMODAL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDYES){
				Player::clearRecords();
			}
		break;
		}
		break;
	}
	//�������� ����
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return FALSE;
	}
	return FALSE;
}

TCHAR* getTable(){
	//��������� �� ����� ������� ��������
	Player *arr = Player::start();
	TCHAR *txt = new TCHAR[3000];
	//���������� �� � ������
	wcscpy(txt, L"");
	int i = 0;
	while (i < 10 && wcscmp(arr[i].getName(), L"__EOFflag") != 0){
		wchar_t num[10];
		_itow_s(i+1, num, 10);
		wcscat(txt, num);
		wcscat(txt, L". ");
		wcscat(txt, arr[i].getName());
		wcscat(txt, L"\t");
		_itow_s(arr[i].getRecord(), num, 10);
		wcscat(txt, num);
		wcscat(txt, L"\n");
		++i;
	}
	return txt;
}

void finishGame(){
	//����� ���-�� ������������ �����
	if (globalCount < 500){
		currentPlayer.setRecord((500 - globalCount));
	}
	else currentPlayer.setRecord(1);

	//����� ���������, "������� ��" ������ ����� ���� ���������� � 10-�� ������
	//���� ��, �� ����������.
	Player *arr = Player::start();
	int size = 0;
	while (size < 10 && wcscmp(arr[size].getName(), L"__EOFflag") != 0){
		++size;
	}
	for (int i = 0; i < size; ++i){
		if (currentPlayer.getRecord() > arr[i].getRecord()){
			for (int j = size; j > i; --j){
				if (j == 10) continue;
				arr[j] = arr[j - 1];
			}
			arr[i] = currentPlayer;
			break;
		}
		if (i == size - 1 && size < 10) arr[size] = currentPlayer;
	}
	if (size == 0) arr[size] = currentPlayer;
	if (size < 10) ++size;
	//���������� �������� ������� � ����
	Player::stop(arr, size);
}