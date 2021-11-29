#include <stdio.h>
#include <ctime>
#include <string>
#include <Windows.h>

void gotoxy(int y, int x) // 커서 위치를 좌표로 보낸다
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor(bool hide)		//커서 숨김
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cci;
	cci.dwSize = 25;
	cci.bVisible = !hide;
	SetConsoleCursorInfo(hStdout, &cci);
}

void prepareWindow(int x, int y)		//콘솔창 크기 조절
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions
	MoveWindow(console, r.left, r.top, x, y, TRUE); // 800 width, 100 height

	HWND scroll = GetConsoleWindow();
	ShowScrollBar(scroll, SB_BOTH, FALSE);

	hideCursor(true);
}
//마우스, 화면 관련 코드

enum State		//맵 상태
{
	BASE, MINE, FLAG
};

class Map		//지뢰찾기에서 맵관련
{
private:

public:
	char mapArray[400];		//최대치 20 X 20 맵까지 가능하기에 400으로 지정
 	int width, height;

	void setsize(int h, int w)		//크기 설정
	{
		height = h;
		width = w;
	}
	void reset(int x, int y)	//배열 초기화
	{
		width = x;
		height = y;
		int total = width * height;
		memset(mapArray, BASE, total);
	}

	void CountMine()		//미리 지뢰갯수를 파악한뒤 hMap 에 저장.
	{
		char digit[] = { ' ','1','2','3','4','5','6' };		//표기할 것들 초기화

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)				//2번째 for문까지는 각셀을 말함 ex) 1,1 2,4 
			{		
				if (mapArray[j + i * width] != MINE)	//지뢰일경우 바뀌면 안되기 때문에 이 조건을 걸어둠
				{
					int count = 0;
					for (int k = i - 1; k <= i + 1; k++)		//주변셀 확인할 용도 y좌표
					{
						for (int t = j - 1; t <= j + 1; t++)		//주변셀 확인할 용도 x좌표
						{
							if (k < 0 || t < 0 || k == height || t == width) continue; //범위밖으로 나갈경우 다음으로 넘어감
							else if (mapArray[t + k * width] == MINE)		//지뢰일 경우 갯수를 올림
							{
								count++;
							}
						}
					}
					mapArray[j + i * width] = digit[count];
				}
			}
		}
	}

	void BuriedMines(int mineNum)
	{
		int total = width * height;
		int mines;
		int num = 0;
		do {
			while (mapArray[mines = rand() % total - 1]); // 중복 검사
			mapArray[mines] = MINE;
			num++;

		} while (num < mineNum);
	}

	void Draw(char map[])
	{
		gotoxy(0, 0);		//커서를 (0,0)으로 이동시켜 위에 다시 그림
		hideCursor(true);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if (map[j + i * width] == BASE)
				{
					printf("■");
				}
				else if (map[j + i * width] == MINE)
				{
					printf("◈");
				}
				else if (map[j + i * width] == FLAG)
				{
					printf("¶");
				}
				else
				{
					printf("%2c", map[j + i * width]);
				}
			}
			printf("\n");
		}
	}
};

class GameManager		//맵외의 다른 코드들
{
public:

	bool isGameover = false;
	bool click = false;
	bool exit = false;
	bool rstart = false;

	int w;
	int h;
	int mineNum;

	int timer;
	int flag;

	// 마우스 관련 코드
	INPUT_RECORD irInBuf;
	HANDLE hStdin;
	DWORD cNumRead, fdwMode;
	DWORD fdwSaveOldMode;
	//

	Map vMap, hMap;

	void Custom(int x, int y, int m)		//입력값에 따라 난이도, 창크기 등 달라짐
	{
		h = x;
		w = y;
		mineNum = m;

		prepareWindow(x * 20 + 100, y * 20 + 200);
	}

	void init()		//게임 시작시 필요한 것들을 초기화 및 불러옴
	{
		srand(time(NULL));
		flag = mineNum;
		hMap.setsize(w, h);
		vMap.setsize(w, h);

		hMap.reset(w, h);
		vMap.reset(w, h);

		hMap.BuriedMines(mineNum);
		hMap.CountMine();
	}

	void UI()		//시간과 찾아야하는 지뢰의 갯수를 표시
	{
		gotoxy(h + 2, 0);
		timer = clock() / (CLOCKS_PER_SEC); //그대로 쓰면 단위가 달라 이상하게 보이기에 SEC단위로 나눔
		if (timer > 60)
		{
			int min = timer / 60;
			int sec = timer % 60;
			if (min < 10) {
				printf("TIMER : 0%d :", min);
			}
			else {
				printf("TIMER : %d :", min);
			}
			if (sec < 10) {
				printf("0%d    ", sec);
			}
			else {
				printf("%d    ", sec);
			}
		}
		else {
			if (timer < 10) {
				printf("TIMER : 00 : 0%d", timer);
			}
			else {
				printf("TIMER : 00 : %d", timer);
			}
		}
		gotoxy(h + 3, 0);
		printf("FIND MINE : %d", mineNum);
	}

	void Play()
	{
		int count = 0;
		
		system("title MineSweeper");

	inputw:
		printf("WIDTH( MAX : 20) : ");
		scanf("%d", &w);
		if (w > 20) goto inputw;

	inputh:
		printf("HEIGHT( MAX : 20) : ");
		scanf("%d", &h);
		if (h > 20) goto inputh;

	inputm:
		printf("MINES( MAX : %d ) : ", (int)(w*h*0.16));
		scanf("%d", &mineNum);
		if (mineNum > (int)(w*h*0.16)) goto inputm;

	//값을 초과할경우 오류발생하기때문에 다시 재입력을 받음
		
		Custom(w, h, mineNum);

		system("cls");		//입력값이 남게 되어서 한번 완전히 지워줌
		init();
		vMap.Draw(vMap.mapArray);

		while (!isGameover)
		{
			UI();
			checkInput();
			checkWin();
			if (click)
			{
				vMap.Draw(vMap.mapArray);
				click = false;
			}
			if (checkWin() == mineNum)
			{
				hMap.Draw(hMap.mapArray);
				UI();
				gotoxy(h + 4, 0);
				printf("WIN\n");
			}
		}
		hMap.Draw(hMap.mapArray);
		UI();
		gotoxy(h + 4, 0);
		printf("GAME OVER");
	}

	void openCell(int x, int y)		//좌클릭시 지뢰가 맞는지 아닌지 여부에따라 게임진행 방향을 따짐
	{
		if (vMap.mapArray[x + y * w] != FLAG)
		{
			if (hMap.mapArray[x + y * w] == MINE)
			{
				isGameover = true;
			}
			else
			{
				DigMine(x, y);
			}
		}
	}

	void flagCell(int x, int y)		//우클릭시 깃발을 세울수있음
	{
		if (vMap.mapArray[x + y * w] != FLAG && vMap.mapArray[x + y * w] == BASE && flag > 0)
		{
			vMap.mapArray[x + y * w] = FLAG;
			if (hMap.mapArray[x + y * w] == MINE) hMap.mapArray[x + y * w] = vMap.mapArray[x + y * w]; // 깃발심은곳 끝나고나서 맞는지 표기를 위해
			flag--;
		}
		else if (vMap.mapArray[x + y * w] == FLAG && vMap.mapArray[x + y * w] != BASE)
		{
			vMap.mapArray[x + y * w] = BASE;
			if (hMap.mapArray[x + y * w] == FLAG) hMap.mapArray[x + y * w] = MINE; // 깃발 표시 한곳을 다시 풀었을때 원래대로 지뢰표시
			flag++;
		}
	}

	void DigMine(int x, int y)
	{
		if (hMap.mapArray[x + y * w] == ' ')					//가리는맵이 빈칸일경우 재귀함수를 계속 호출
		{
			vMap.mapArray[x + y * w] = hMap.mapArray[x + y * w];
		}
		else										//가리는맵을 숫자 혹은 그외의 것일경우 함수를 끝냄
		{
			vMap.mapArray[x + y * w] = hMap.mapArray[x + y * w];
			return;
		}
		if (vMap.mapArray[(x - 1) + y * w] == BASE && x > 0) DigMine(x - 1, y);
		if (vMap.mapArray[(x + 1) + y * w] == BASE && x < w - 1) DigMine(x + 1, y);
		if (vMap.mapArray[x + (y - 1)* w] == BASE && y > 0) DigMine(x, y - 1);
		if (vMap.mapArray[x + (y + 1)* w] == BASE && y < h - 1) DigMine(x, y + 1);
		if (vMap.mapArray[(x + 1) + (y - 1)* w] == BASE && x < w - 1 && y > 0) DigMine(x + 1, y - 1);
		if (vMap.mapArray[(x - 1) + (y + 1)* w] == BASE && x > 0 && y < h - 1) DigMine(x - 1, y + 1);
		if (vMap.mapArray[(x - 1) + (y - 1)* w] == BASE && x > 0 && y > 0) DigMine(x - 1, y - 1);
		if (vMap.mapArray[(x + 1) + (y + 1)* w] == BASE && x < w - 1 && y < h - 1) DigMine(x + 1, y + 1);
	}

	void checkInput()
	{
		int x, y;

		//콘솔창에서 마우스가 먹히게 해주는 코드
		hStdin = GetStdHandle(STD_INPUT_HANDLE);
		GetConsoleMode(hStdin, &fdwSaveOldMode);
		fdwMode = ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
		SetConsoleMode(hStdin, fdwMode);

		ReadConsoleInput(hStdin, &irInBuf, 1, &cNumRead);
		MOUSE_EVENT_RECORD mer = irInBuf.Event.MouseEvent;

		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)		//좌클릭시
		{
			x = mer.dwMousePosition.X;
			y = mer.dwMousePosition.Y;

			if (x < w*2 && y < h)		//맵범위를 초과할경우 클릭이 안되도록 조건을 걸어줌
			{
				x = x / 2;				//문자는 2칸으로 받아들이기 때문에 나눠줌
				click = true;
				openCell(x, y);
			}
		}

		else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)		//우클릭시
		{
			x = mer.dwMousePosition.X;
			y = mer.dwMousePosition.Y;

			if (x < w*2 && y < h)
			{
				x = x / 2;
				click = true;
				flagCell(x, y);
			}
		}
	}

	int checkWin()		//BASE , FLAG 의 갯수를 합쳐 그값을 넘겨 이긴지 아닌지 판단
	{
		int x;
		int count = 0;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				if (vMap.mapArray[j + i * w] == BASE)	// BASE의 갯수를 계속 셈
				{
					count++;
				}
			}
		}
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				if (hMap.mapArray[j + i * w] == FLAG)	//FLAG갯수를 셈(진짜 지뢰를 FLAG표시했을 때만 세야하기에 hMap)
				{
					count++;
				}
			}
		}
		x = count;
		return x;
	}
};

void main()
{
	GameManager mag;

	mag.Play();
}