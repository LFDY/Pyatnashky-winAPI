/*
Программа пятнашки с использованием WINAPI.
@copyright S.Aksyonchikov
@author Svyatoslav Aksyonchikov
@e-mail axesvt@gmail.com
*/
#include <windows.h>
#include <tchar.h>		//for both ANSI and UNICODE strings
#include <windowsx.h>
#include <vector>
#include <ctime>
#include "resource.h"	//подключение файла ресурсов
#include "player.h"

#pragma warning(disable:4996)	//чтоб не показывал ворнинги при работе со строками

static TCHAR appTitle[] = _T("PyatnashkiWindow");	//текст заголовка
HINSTANCE hInst;

HWND button[15];

Player currentPlayer;

int globalCount;		//счётчик ходов

int source[16];
int origin[16];

int buttonPosition[15];	

struct{
	int x;
	int y;
	int pos;
}freeSpace;		//координаты и позиция пустой клетки


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);		//обработчик сообщений от основного окна
BOOL CALLBACK PropertiesProc(HWND, UINT, WPARAM, LPARAM);	//обработчик сообщений от диалогового окна "Параметры"

BOOL Register(HINSTANCE hInstance);							//Ф-ия регистрации класса окна
HWND Create(HINSTANCE hInstance, int nCmdShow);				//Ф-ия создания объекта окна на основе ранее созданного класса

void but_create(HWND hwnd, RECT rect);						//Ф-ия создания кнопок

TCHAR* getTable();											//Ф-ия создания строки-таблицы рекордов
void finishGame();											//Ф-ия, выполняющаяся при успешном завершении игры

BOOL checkComplete(){										//Проверка на окончание игры
	for (int i = 0; i < 16; ++i){
		if (source[i] != origin[i]) return FALSE;			//Сравнение текущего расположения 
	}														//кнопок с оригиналом
	return TRUE;
}

void swapArr(int s1, int s2){								//Замена местами эл-тов положения кнопок
	int tmp = source[s1];
	source[s1] = source[s2];
	source[s2] = tmp;
}

//Замена местами кнопок
void swapBut(HWND hwnd, HWND but, int nBut){	

	//Двигать можно только соседние кнопки
	if (buttonPosition[nBut] != freeSpace.pos - 1 && buttonPosition[nBut] != freeSpace.pos + 1 && buttonPosition[nBut] != freeSpace.pos - 4 && buttonPosition[nBut] != freeSpace.pos + 4) return;
	
	++globalCount;

	RECT rect;
	GetClientRect(hwnd, &rect);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	//Замена местами позиции пустой клетки и текущей кнопки
	int tmpPos = buttonPosition[nBut];
	buttonPosition[nBut] = freeSpace.pos;
	freeSpace.pos = tmpPos;

	RECT rt;
	GetWindowRect(but, &rt);
	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rt, 2);

	//Перемещение кнопки
	SetWindowPos(but, HWND_TOP, freeSpace.x, freeSpace.y, width / 4, height / 4, SWP_SHOWWINDOW);

	//Изменение координат пустой клетки
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
	HWND hwnd = CreateWindow((LPCWSTR)appTitle, (LPCWSTR)_T("Пятнашки"), WS_OVERLAPPEDWINDOW,
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
		//Сообщение, посылаемое при создании окна
		case WM_CREATE:
			for (int i = 0; i < 15; ++i){
				origin[i] = i + 1;
			}
			origin[15] = 0;
		break;

		//Сообщение, вызываемое при нажатии клавиш
		case WM_COMMAND:

			switch (wParam){
			//Нажата кнопка меню "новая игра"
			case IDM_New:
				if (button[0]){
					//Если игра уже запущена, уточненить, действительно ли хочет пользователь запустить новую игру
					if (MessageBox(NULL, L"Вы уверены, что хотите начать новую игру? Весь текущий прогресс будет потерян.", L"Новая игра", MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL) == IDNO) break;
				}
				GetClientRect(hwnd, &rect);
				but_create(hwnd, rect);
				break;

			//Нажата кнопка "Параметры"
			case IDM_Properties:{
				//Вызов диалогового окна из файла ресурсов
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_PROP), hwnd, (DLGPROC)PropertiesProc);
				break; }
			//Нажата кнопка "Рекорды"
			case IDM_Records:{
				TCHAR *txt = getTable();
				MessageBox(NULL, txt, _T("Таблица рекордов"), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
				break;
			}
			//Кнопка "Выход"
			case IDM_Exit:
				if (button[0]){
					if (MessageBox(NULL, L"Вы уверены, что хотите выйти из игры? Весь текущий прогресс будет потерян.", L"Выход", MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL) == IDNO) break;
				}
				SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				break;
			//Кнопка "Управление"
			case IDM_Control:
				MessageBox(NULL, L"Управление осуществляется с помощью мыши. Активными являются соседние кнопки с пустой клеткой. При нажатии на кнопку она занимает место пустой клетки.", _T("Управление"), MB_OK | MB_ICONQUESTION | MB_TASKMODAL);
				break;
			//Кнопка "О программе"
			case IDM_About:
				MessageBox(NULL, _T("Игра \"Пятнашки\" v.1.0\n\n\nАвтор: Аксёнчиков С.А."), _T("О программе"), MB_OK | MB_ICONQUESTION | MB_TASKMODAL);
				break;
			
			//Далее идёт обработка вызовов кнопок непосредственно пятнашек
			case 10001:
				swapBut(hwnd, button[0], 0);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10002:
				swapBut(hwnd, button[1], 1);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10003:
				swapBut(hwnd, button[2], 2);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10004:
				swapBut(hwnd, button[3], 3);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10005:
				swapBut(hwnd, button[4], 4);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10006:
				swapBut(hwnd, button[5], 5);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10007:
				swapBut(hwnd, button[6], 6);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10008:
				swapBut(hwnd, button[7], 7);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10009:
				swapBut(hwnd, button[8], 8);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10010:
				swapBut(hwnd, button[9], 9);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10011:
				swapBut(hwnd, button[10], 10);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10012:
				swapBut(hwnd, button[11], 11);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10013:
				swapBut(hwnd, button[12], 12);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10014:
				swapBut(hwnd, button[13], 13);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;

			case 10015:{
				swapBut(hwnd, button[14], 14);
				if (checkComplete()){
					finishGame();
					if (MessageBox(NULL, _T("Поздравляем!\nХотите начать новую игру?"), _T("ПОБЕДА"), MB_YESNO | MB_ICONINFORMATION | MB_TASKMODAL) == IDYES) SendMessage(hwnd, WM_COMMAND, IDM_New, NULL);
					else SendMessage(hwnd, WM_CLOSE, NULL, NULL);
				}
				break;
				}
			}
			break;
		//Задание граничных параметров изменения размеров окна (делаем неизменяемое)
		case WM_GETMINMAXINFO:
			{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = 400;
			lpMMI->ptMinTrackSize.y = 400;

			lpMMI->ptMaxTrackSize.x = 400;
			lpMMI->ptMaxTrackSize.y = 400;
			}
			break;
		//Удаление окна
		case WM_DESTROY:{
			PostQuitMessage(0);
			break;
		}
	//Обработку всех остальных сообщений передаём системе
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void but_create(HWND hwnd, RECT rect){
	//Если уже существовала игра, удаляем с экрана кнопки
	for (int i = 0; i < 15; ++i){
		if (button[i]){
			DestroyWindow(button[i]);
		}
		else break;
	}
	//Задание начальных значений глобальных переменных
	globalCount = 0;
	source[15] = 0;
	srand(unsigned(time(NULL)));
	std::vector<int> v;

	//Тут мы случайным образом распологаем кнопки
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

	//Дальше рисуем кнопки по очереди
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
	//Инициализация диалога
	case WM_INITDIALOG:
		//В текстовом окне пишем имя текущего игрока
		SetDlgItemText(hwnd, IDC_TEXTBOX1, currentPlayer.getName());
		return FALSE;
		break;
	//Обработка нажатий клавиш
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
		//Кнопка обнуления таблицы рекордов
		case IDC_RESTORE:
			if (MessageBox(NULL, L"Вы уверены, что хотите очистить таблицу рекордов?", L"Сброс", MB_YESNO | MB_TASKMODAL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) == IDYES){
				Player::clearRecords();
			}
		break;
		}
		break;
	}
	//Закрытие окна
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return FALSE;
	}
	return FALSE;
}

TCHAR* getTable(){
	//Считываем из файла таблицу рекордов
	Player *arr = Player::start();
	TCHAR *txt = new TCHAR[3000];
	//Записываем их в строку
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
	//Задаём кол-во заработанных очков
	if (globalCount < 500){
		currentPlayer.setRecord((500 - globalCount));
	}
	else currentPlayer.setRecord(1);

	//Далее проверяем, "достоин ли" данный игрок быть записанным в 10-ку лучших
	//Если да, то записываем.
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
	//Записываем значение обратно в файл
	Player::stop(arr, size);
}