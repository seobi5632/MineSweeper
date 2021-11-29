#include <stdio.h>
#include <ctime>
#include <string>
#include <Windows.h>

void gotoxy(int y, int x) // Ŀ�� ��ġ�� ��ǥ�� ������
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hideCursor(bool hide)		//Ŀ�� ����
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cci;
	cci.dwSize = 25;
	cci.bVisible = !hide;
	SetConsoleCursorInfo(hStdout, &cci);
}

void prepareWindow(int x, int y)		//�ܼ�â ũ�� ����
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions
	MoveWindow(console, r.left, r.top, x, y, TRUE); // 800 width, 100 height

	HWND scroll = GetConsoleWindow();
	ShowScrollBar(scroll, SB_BOTH, FALSE);

	hideCursor(true);
}
//���콺, ȭ�� ���� �ڵ�

enum State		//�� ����
{
	BASE, MINE, FLAG
};

class Map		//����ã�⿡�� �ʰ���
{
private:

public:
	char mapArray[400];		//�ִ�ġ 20 X 20 �ʱ��� �����ϱ⿡ 400���� ����
 	int width, height;

	void setsize(int h, int w)		//ũ�� ����
	{
		height = h;
		width = w;
	}
	void reset(int x, int y)	//�迭 �ʱ�ȭ
	{
		width = x;
		height = y;
		int total = width * height;
		memset(mapArray, BASE, total);
	}

	void CountMine()		//�̸� ���ڰ����� �ľ��ѵ� hMap �� ����.
	{
		char digit[] = { ' ','1','2','3','4','5','6' };		//ǥ���� �͵� �ʱ�ȭ

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)				//2��° for�������� ������ ���� ex) 1,1 2,4 
			{		
				if (mapArray[j + i * width] != MINE)	//�����ϰ�� �ٲ�� �ȵǱ� ������ �� ������ �ɾ��
				{
					int count = 0;
					for (int k = i - 1; k <= i + 1; k++)		//�ֺ��� Ȯ���� �뵵 y��ǥ
					{
						for (int t = j - 1; t <= j + 1; t++)		//�ֺ��� Ȯ���� �뵵 x��ǥ
						{
							if (k < 0 || t < 0 || k == height || t == width) continue; //���������� ������� �������� �Ѿ
							else if (mapArray[t + k * width] == MINE)		//������ ��� ������ �ø�
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
			while (mapArray[mines = rand() % total - 1]); // �ߺ� �˻�
			mapArray[mines] = MINE;
			num++;

		} while (num < mineNum);
	}

	void Draw(char map[])
	{
		gotoxy(0, 0);		//Ŀ���� (0,0)���� �̵����� ���� �ٽ� �׸�
		hideCursor(true);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if (map[j + i * width] == BASE)
				{
					printf("��");
				}
				else if (map[j + i * width] == MINE)
				{
					printf("��");
				}
				else if (map[j + i * width] == FLAG)
				{
					printf("��");
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

class GameManager		//�ʿ��� �ٸ� �ڵ��
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

	// ���콺 ���� �ڵ�
	INPUT_RECORD irInBuf;
	HANDLE hStdin;
	DWORD cNumRead, fdwMode;
	DWORD fdwSaveOldMode;
	//

	Map vMap, hMap;

	void Custom(int x, int y, int m)		//�Է°��� ���� ���̵�, âũ�� �� �޶���
	{
		h = x;
		w = y;
		mineNum = m;

		prepareWindow(x * 20 + 100, y * 20 + 200);
	}

	void init()		//���� ���۽� �ʿ��� �͵��� �ʱ�ȭ �� �ҷ���
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

	void UI()		//�ð��� ã�ƾ��ϴ� ������ ������ ǥ��
	{
		gotoxy(h + 2, 0);
		timer = clock() / (CLOCKS_PER_SEC); //�״�� ���� ������ �޶� �̻��ϰ� ���̱⿡ SEC������ ����
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

	//���� �ʰ��Ұ�� �����߻��ϱ⶧���� �ٽ� ���Է��� ����
		
		Custom(w, h, mineNum);

		system("cls");		//�Է°��� ���� �Ǿ �ѹ� ������ ������
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

	void openCell(int x, int y)		//��Ŭ���� ���ڰ� �´��� �ƴ��� ���ο����� �������� ������ ����
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

	void flagCell(int x, int y)		//��Ŭ���� ����� ���������
	{
		if (vMap.mapArray[x + y * w] != FLAG && vMap.mapArray[x + y * w] == BASE && flag > 0)
		{
			vMap.mapArray[x + y * w] = FLAG;
			if (hMap.mapArray[x + y * w] == MINE) hMap.mapArray[x + y * w] = vMap.mapArray[x + y * w]; // ��߽����� �������� �´��� ǥ�⸦ ����
			flag--;
		}
		else if (vMap.mapArray[x + y * w] == FLAG && vMap.mapArray[x + y * w] != BASE)
		{
			vMap.mapArray[x + y * w] = BASE;
			if (hMap.mapArray[x + y * w] == FLAG) hMap.mapArray[x + y * w] = MINE; // ��� ǥ�� �Ѱ��� �ٽ� Ǯ������ ������� ����ǥ��
			flag++;
		}
	}

	void DigMine(int x, int y)
	{
		if (hMap.mapArray[x + y * w] == ' ')					//�����¸��� ��ĭ�ϰ�� ����Լ��� ��� ȣ��
		{
			vMap.mapArray[x + y * w] = hMap.mapArray[x + y * w];
		}
		else										//�����¸��� ���� Ȥ�� �׿��� ���ϰ�� �Լ��� ����
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

		//�ܼ�â���� ���콺�� ������ ���ִ� �ڵ�
		hStdin = GetStdHandle(STD_INPUT_HANDLE);
		GetConsoleMode(hStdin, &fdwSaveOldMode);
		fdwMode = ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
		SetConsoleMode(hStdin, fdwMode);

		ReadConsoleInput(hStdin, &irInBuf, 1, &cNumRead);
		MOUSE_EVENT_RECORD mer = irInBuf.Event.MouseEvent;

		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)		//��Ŭ����
		{
			x = mer.dwMousePosition.X;
			y = mer.dwMousePosition.Y;

			if (x < w*2 && y < h)		//�ʹ����� �ʰ��Ұ�� Ŭ���� �ȵǵ��� ������ �ɾ���
			{
				x = x / 2;				//���ڴ� 2ĭ���� �޾Ƶ��̱� ������ ������
				click = true;
				openCell(x, y);
			}
		}

		else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)		//��Ŭ����
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

	int checkWin()		//BASE , FLAG �� ������ ���� �װ��� �Ѱ� �̱��� �ƴ��� �Ǵ�
	{
		int x;
		int count = 0;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				if (vMap.mapArray[j + i * w] == BASE)	// BASE�� ������ ��� ��
				{
					count++;
				}
			}
		}
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				if (hMap.mapArray[j + i * w] == FLAG)	//FLAG������ ��(��¥ ���ڸ� FLAGǥ������ ���� �����ϱ⿡ hMap)
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