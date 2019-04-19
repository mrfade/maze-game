/*
*	Author: Enes Solak
*	Web: https://enessolak.com.tr
*
*/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <conio.h>
#include <windows.h>

using namespace std;

typedef struct
{
	int c;
	int visited;
} cell;

class wallType {
public:
	static const int road = 0;
	static const int wall = 1;
	static const int coin = 2;
	static const int enemy = 3;
	static const int cursor = 9;
};

class wallTypeC {
public:
	static const char road = 32;
	static const char wall = 219;
	static const char coin = 248;
	static const char enemy = 36;
	static const char cursor = 42;
};

class keyType {
public:
	static const int up = 72;
	static const int down = 80;
	static const int left = 75;
	static const int right = 77;
	static const int escape = 27;
	static const int tab = 9;
};


cell maze[100][100];
int mazeSize = 20; // fallback on any input error

int moveHistory[10000][2];
int solution[10000][2];
int moveHistoryIterator = 0;

bool showProcess = false;
bool autoSolverEnabled = false;
int totalCoins = 0;

void generateMaze();
void printMaze();
void solveMaze();
void solveMazeAnimation(int *, int *);
void playGame();
void removeEnemiesOnTheWay();
void setCellVisited(int);
void saveMove(int, int);
void getLastMove(int *, int *);
void textColor(int);
void copyArray(int *, int *, int);
void gameMenu();
string saveSolution(); // returns file name
void updateSolution(string);
cell *m(int, int);

void main() {

	srand(time(NULL));

	playGame();

	system("cls");
	textColor(10); cout << endl << "OK! Good Bye.." << endl;
	Sleep(1000);

	// system("pause");
}

void playGame() {

	system("cls");

	char gp;

	textColor(10); cout << "WELCOME TO MAZE GAME";
	textColor(14); cout << " by Enes Solak" << endl << endl;

	textColor(7); cout << "Enter maze size (10-100): ";
	cin >> mazeSize;

	if (mazeSize > 100) mazeSize = 100;
	if (mazeSize < 10) mazeSize = 10;

	cout << "Do you want to see maze generation process? (Y/n): ";
	gp = _getch();

	if (gp == 'Y' || gp == 'y')
		showProcess = true;
	else showProcess = false;

	generateMaze();

	char c = ' ';
	int curX = 0, curY = 1;
	int lastX = 0, lastY = 0;
	bool hitWall = false;

	string file_name = saveSolution();

	setCellVisited(0);
	totalCoins = 0;
	moveHistoryIterator = 0;

	do {
		system("cls");

		gameMenu();

		if (c == keyType::tab) autoSolverEnabled = true;

		if (autoSolverEnabled) {

			cout << endl << "Do you want to see maze solving process? (Y/n): ";
			gp = _getch();

			if (gp == 'Y' || gp == 'y')
				showProcess = true;
			else showProcess = false;

			system("cls");

			solveMazeAnimation(&curX, &curY);

		}
		else {

			m(curY, curX)->c = wallType::road;

			switch (c)
			{
			case keyType::up:
				curY--;
				break;
			case keyType::down:
				curY++;
				break;
			case keyType::right:
				curX++;
				break;
			case keyType::left:
				curX--;
				break;
			case keyType::escape:
				exit(1);
				break;
			case -32:
				continue;
				break;
			}

			if (curX == mazeSize - 1 && curY == mazeSize - 2) break;

			if (m(curY, curX)->c == wallType::wall || curX < 0) { // hit wall
				curX = lastX;
				curY = lastY;
				hitWall = true;
			}
			else if (m(curY, curX)->c == wallType::coin) // hit coin
				totalCoins++;
			else if (m(curY, curX)->c == wallType::enemy) { // hit enemy
				system("cls");
				textColor(12); cout << endl << "You Lose! Restarting.." << endl; textColor(7);
				remove(file_name.c_str());
				Sleep(1000);
				playGame();
			}

			m(curY, curX)->c = wallType::cursor;

			lastX = curX;
			lastY = curY;

			if (hitWall) {
				textColor(12); cout << endl << "You Hit Wall!" << endl; textColor(7);
				hitWall = false;
			}
			else {
				saveMove(curX, curY);
			}

			cout << endl;
			printMaze();

		}

	} while (c = _getch());

	system("cls");

	textColor(10); cout << endl << "You Win!" << endl; textColor(7);
	textColor(14); cout << endl << wallTypeC::coin; textColor(7); cout << " Total Coins: " << totalCoins << endl;

	if (file_name != "") {
		textColor(10); cout << endl << "Solution file created: "; textColor(7); cout << file_name << endl;
		updateSolution(file_name);
	}
	else {
		textColor(12); cout << endl << "An error occurred while trying to save solution file!" << endl; textColor(7);
	}

	cout << endl << "Do you want to play again? (Y/n): ";
	c = _getch();

	if (c == 'Y' || c == 'y') {
		system("cls");
		textColor(10); cout << endl << "OK! Starting.." << endl; textColor(7);
		Sleep(1000);
		playGame();
	}

}

void generateMaze() {

	moveHistoryIterator = 0;

	setCellVisited(0);

	for (int i = 0; i < mazeSize; i++)
	{
		for (int j = 0; j < mazeSize; j++)
		{
			m(i, j)->c = wallType::wall;
			m(i, j)->visited = 0;
		}
	}

	int curX = 1;
	int curY = 1;
	bool leftBlock, rightBlock, upperBlock, lowerBlock;
	bool initialized = false;
	int random;
	int canMoveCount = 0;
	char randomMoves[4];
	int randomMovesIterator = 0;

	do {

		if (showProcess) {
			textColor(10); cout << "Generating Maze..." << endl << endl; textColor(7);
			m(curY, curX)->c = wallType::cursor;
			printMaze();
			Sleep(50);
			system("cls");
		}

		m(curY, curX)->c = wallType::road;
		m(curY, curX)->visited = 1;

		if (curY > 0 && curY < mazeSize - 1 && curX > 1 && m(curY, curX - 1)->c != wallType::road && m(curY, curX - 1)->visited != 1) {
			leftBlock = true;
			if (curX > 2 && m(curY, curX - 2)->c != wallType::wall)
				leftBlock = false;
			else if (m(curY - 1, curX - 1)->c != wallType::wall || m(curY + 1, curX - 1)->c != wallType::wall)
				leftBlock = false;
		}
		else leftBlock = false;

		if (curY > 0 && curY < mazeSize - 1 && curX < mazeSize - 2 && m(curY, curX + 1)->c != wallType::road && m(curY, curX + 1)->visited != 1) {
			rightBlock = true;
			if (curX < mazeSize - 3 && m(curY, curX + 2)->c != wallType::wall)
				rightBlock = false;
			else if (m(curY - 1, curX + 1)->c != wallType::wall || m(curY + 1, curX + 1)->c != wallType::wall)
				rightBlock = false;
		}
		else rightBlock = false;

		if (curX > 0 && curX < mazeSize - 1 && curY > 1 && m(curY - 1, curX)->c != wallType::road && m(curY - 1, curX)->visited != 1) {
			upperBlock = true;
			if (curY > 2 && m(curY - 2, curX)->c != wallType::wall)
				upperBlock = false;
			else if (m(curY - 1, curX + 1)->c != wallType::wall || m(curY - 1, curX - 1)->c != wallType::wall)
				upperBlock = false;
		}
		else upperBlock = false;

		if (curX > 0 && curX < mazeSize - 1 && curY < mazeSize - 2 && m(curY + 1, curX)->c != wallType::road && m(curY + 1, curX)->visited != 1) {
			lowerBlock = true;
			if (curY < mazeSize - 3 && m(curY + 2, curX)->c != wallType::wall)
				lowerBlock = false;
			else if (m(curY + 1, curX + 1)->c != wallType::wall || m(curY + 1, curX - 1)->c != wallType::wall)
				lowerBlock = false;
		}
		else lowerBlock = false;

		if (leftBlock) {
			randomMoves[randomMovesIterator] = 'l';
			randomMovesIterator++;
			canMoveCount++;
		}
		if (rightBlock) {
			randomMoves[randomMovesIterator] = 'r';
			randomMovesIterator++;
			canMoveCount++;
		}
		if (upperBlock) {
			randomMoves[randomMovesIterator] = 'u';
			randomMovesIterator++;
			canMoveCount++;
		}
		if (lowerBlock) {
			randomMoves[randomMovesIterator] = 'o';
			randomMovesIterator++;
			canMoveCount++;
		}

		if (curX == 1 && curY == 1 && initialized == true) break;

		if (canMoveCount == 0) {

			random = rand() % 11;
			if (random == 1) // place coin
				m(curY, curX)->c = wallType::coin;
			if (random == 2) // place enemy
				m(curY, curX)->c = wallType::enemy;

			getLastMove(&curX, &curY);

			continue;
		}

		saveMove(curX, curY);

		random = rand() % canMoveCount;

		switch (randomMoves[random])
		{
			// right
		case 'r':
			curX++;
			break;
			// left
		case 'l':
			curX--;
			break;
			// lower
		case 'o':
			curY++;
			break;
			// upper
		case 'u':
			curY--;
			break;
		}

		canMoveCount = 0;
		randomMovesIterator = 0;

		initialized = true;

	} while (true);

	(*maze + 1)->c = wallType::cursor;

	(*maze + mazeSize - 2 + (mazeSize - 1) * mazeSize)->c = wallType::road;
	(*maze + mazeSize - 2 + (mazeSize - 2) * mazeSize)->c = wallType::road;

	// avoid no exit path

	int test_x = 2, test_y = 2;

	for (int i = 0; i < mazeSize; i++)
	{
		if ((*maze + mazeSize - test_y + (mazeSize - test_x - 1) * mazeSize)->c == wallType::wall && (*maze + mazeSize - test_y - 1 + (mazeSize - test_x) * mazeSize)->c == wallType::wall) {
			(*maze + mazeSize - test_y + (mazeSize - test_x - 1) * mazeSize)->c = wallType::road;
			test_x++;
		}
		else break;
	}

	solveMaze(); // calculated correct way by computer

	removeEnemiesOnTheWay();

	/* cout << "curx" << curX << endl;
	cout << "cury" << curY << endl; */

}

void saveMove(int x, int y) {
	moveHistory[moveHistoryIterator][0] = x;
	moveHistory[moveHistoryIterator][1] = y;
	moveHistoryIterator++;
}

void getLastMove(int *x, int *y) {
	moveHistoryIterator -= 2;
	*x = moveHistory[moveHistoryIterator][0];
	*y = moveHistory[moveHistoryIterator][1];
	moveHistoryIterator++;
}

void textColor(int c) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void printMaze() {

	for (int i = 0; i < mazeSize; i++)
	{
		for (int j = 0; j < mazeSize; j++)
		{
			textColor(7);
			if (m(i, j)->c == wallType::wall) // wall
				cout << wallTypeC::wall << wallTypeC::wall;
			else if (m(i, j)->c == wallType::coin) {
				textColor(14);
				cout << wallTypeC::coin << " ";
			} // coin
			else if (m(i, j)->c == wallType::enemy) {
				textColor(12);
				cout << wallTypeC::enemy << " ";
			} // enemy
			else if (m(i, j)->c == wallType::cursor) {
				textColor(10);
				cout << wallTypeC::cursor << " ";
			} // cursor
			else
				cout << "  ";

		}
		cout << endl;
	}
}

void solveMaze() {

	moveHistoryIterator = 0;

	setCellVisited(0);

	int curX = 0;
	int curY = 1;

	while (true) {

		bool canMoveLeft = curX > 1 && m(curY, curX - 1)->c != wallType::wall && m(curY, curX - 1)->visited != wallType::wall ? true : false;
		bool canMoveRight = curX < mazeSize - 2 && m(curY, curX + 1)->c != wallType::wall && m(curY, curX + 1)->visited != wallType::wall ? true : false;
		bool canMoveUpper = curY > 1 && m(curY - 1, curX)->c != wallType::wall && m(curY - 1, curX)->visited != wallType::wall ? true : false;
		bool canMoveLower = curY < mazeSize - 2 && m(curY + 1, curX)->c != wallType::wall && m(curY + 1, curX)->visited != wallType::wall ? true : false;

		m(curY, curX)->visited = 1;

		if (curX == mazeSize - 2 && curY == mazeSize - 2)
			break;

		if (canMoveRight) curX++;
		else if (canMoveLower) curY++;
		else if (canMoveLeft) curX--;
		else if (canMoveUpper) curY--;

		if (!canMoveLeft && !canMoveRight && !canMoveUpper && !canMoveLower) {
			getLastMove(&curX, &curY);
			continue;
		}

		saveMove(curX, curY);

	}

	copyArray(solution[0], moveHistory[0], moveHistoryIterator * 2);

}

void solveMazeAnimation(int *curX, int *curY) {

	moveHistoryIterator = 0;

	setCellVisited(0);

	int firstX = *curX;
	int firstY = *curY;

	bool canMoveLeft, canMoveRight, canMoveUpper, canMoveLower;

	if (!showProcess) {
		textColor(10); cout << endl << "--SOLVING--" << endl; textColor(7);
	}

	while (true) {

		if (firstX != 1 && firstY != 1 && *curX == 1 && *curY == 1)
			setCellVisited(0);

		canMoveLeft = *curX > 1 && m(*curY, *curX - 1)->c != wallType::wall && m(*curY, *curX - 1)->visited != wallType::wall && m(*curY, *curX - 1)->c != wallType::enemy ? true : false;
		canMoveRight = *curX < mazeSize - 2 && m(*curY, *curX + 1)->c != wallType::wall && m(*curY, *curX + 1)->visited != wallType::wall && m(*curY, *curX + 1)->c != wallType::enemy ? true : false;
		canMoveUpper = *curY > 1 && m(*curY - 1, *curX)->c != wallType::wall && m(*curY - 1, *curX)->visited != wallType::wall && m(*curY - 1, *curX)->c != wallType::enemy ? true : false;
		canMoveLower = *curY < mazeSize - 2 && m(*curY + 1, *curX)->c != wallType::wall && m(*curY + 1, *curX)->visited != wallType::wall && m(*curY + 1, *curX)->c != wallType::enemy ? true : false;

		if (m(*curY, *curX)->c == wallType::coin) totalCoins++;

		if (showProcess) {
			gameMenu();
			cout << endl;
			m(*curY, *curX)->c = wallType::cursor;
			printMaze();
			Sleep(50);
			system("cls");
		}

		m(*curY, *curX)->c = wallType::road;
		m(*curY, *curX)->visited = 1;

		if (*curX == mazeSize - 2 && *curY == mazeSize - 2)
			break;

		if (canMoveRight) (*curX)++;
		else if (canMoveLower) (*curY)++;
		else if (canMoveLeft) (*curX)--;
		else if (canMoveUpper) (*curY)--;

		if (!canMoveLeft && !canMoveRight && !canMoveUpper && !canMoveLower) {
			getLastMove(curX, curY);
			continue;
		}

		saveMove(*curX, *curY);

	}

	autoSolverEnabled = false;

	system("cls");

	gameMenu();
	cout << endl;
	m(*curY, *curX)->c = wallType::cursor;
	printMaze();

}

void removeEnemiesOnTheWay() {
	for (int i = 0; i < moveHistoryIterator; i++)
		if (m(solution[i][1], solution[i][0])->c == wallType::enemy)
			m(solution[i][1], solution[i][0])->c = wallType::road;
}

void copyArray(int *to, int *from, int size) {
	for (int i = 0; i < size; i++)
		*to++ = *from++;
}

void setCellVisited(int x) {
	for (int i = 0; i < mazeSize; i++)
		for (int j = 0; j < mazeSize; j++)
			m(i, j)->visited = x;
}

void gameMenu() {

	textColor(10); cout << "GAME STARTED" << endl << endl;
	textColor(14); cout << wallTypeC::coin; textColor(7); cout << " Coin" << endl;
	textColor(12); cout << wallTypeC::enemy; textColor(7); cout << " Enemy" << endl;

	textColor(14);	cout << endl << wallTypeC::coin; textColor(7); cout << " Total Coins: " << totalCoins << endl;
	if (autoSolverEnabled) {
		textColor(10); cout << endl << "--SOLVING--" << endl; textColor(7);
	}
	else {
		cout << endl << "[TAB] to Auto-Solver" << endl;
		cout << "[ESC] to exit" << endl;
	}
}

string saveSolution() {

	LPCSTR dir = "MazeGame_solutions";

	DWORD dwAttrib = GetFileAttributes(dir);

	if (!(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) {
		CreateDirectoryA(dir, NULL);
	}

	string file_name = "./";
	file_name.append(dir);
	file_name.append("/game_solution_");
	file_name.append(to_string(rand()));
	file_name.append(".txt");

	ofstream solutionFile(file_name);
	if (solutionFile.is_open())
	{
		solutionFile << "--------MAZE--------" << endl;

		for (int i = 0; i < mazeSize; i++)
		{
			for (int j = 0; j < mazeSize; j++)
			{
				if (m(i, j)->c == wallType::wall) // wall
					solutionFile << "##";
				else if (m(i, j)->c == wallType::coin) // coin
					solutionFile << "C ";
				else if (m(i, j)->c == wallType::enemy) // enemy
					solutionFile << "$ ";
				else
					solutionFile << "  ";
			}
			solutionFile << endl;
		}

		solutionFile << endl;

		solutionFile << "-----SHORTEST SOLUTION-----" << endl;
		solutionFile << "0, 1" << endl;
		for (int i = 0; i < moveHistoryIterator; i++)
			solutionFile << solution[i][0] << ", " << solution[i][1] << endl;
		solutionFile << solution[moveHistoryIterator - 1][0] << ", " << solution[moveHistoryIterator - 1][1] + 1 << endl;
		solutionFile.close();
		return file_name;
	}
	else return "";

}

void updateSolution(string file_name) {

	ofstream solutionFile(file_name, ios_base::app);
	if (solutionFile.is_open())
	{
		solutionFile << endl << endl;

		solutionFile << "-----YOUR SOLUTION-----" << endl;
		for (int i = 0; i < moveHistoryIterator; i++)
			solutionFile << moveHistory[i][0] << ", " << moveHistory[i][1] << endl;
		solutionFile << moveHistory[moveHistoryIterator - 1][0] << ", " << moveHistory[moveHistoryIterator - 1][1] + 1 << endl;
		solutionFile.close();
	}

}

cell *m(int y, int x) {
	return *maze + y + x * mazeSize;
}