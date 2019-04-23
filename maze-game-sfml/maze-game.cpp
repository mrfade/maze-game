/*
*	Author: Enes Solak
*	Web: https://enessolak.com.tr
*	Github: https://github.com/mrfade/maze-game
*
*/

#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <conio.h>
#include <windows.h>

using namespace std;
using namespace sf;

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


RenderWindow window;

cell maze[100][100];
int mazeSize = 20; // maze size

int moveHistory[10000][2];
int solution[10000][2];
int moveHistoryIterator = 0;

bool autoSolverEnabled = false;
int totalCoins = 0;

void generateMaze();
void printMaze(int, int);
void solveMaze();
void solveMazeRecursive(int *, int *, int, int);
void playGame();
void removeEnemiesOnTheWay();
void setCellVisited(int);
void saveMove(int, int);
void getLastMove(int *, int *);
void copyArray(int *, int *, int);
string saveSolution(); // returns file name
void updateSolution(string);
cell *m(int, int); // returns maze cell address


int main() {

	window.create(VideoMode(800, 600), "MAZE GAME by Enes Solak");
	window.setFramerateLimit(30); // 30 frame is enough to play

	srand(time(NULL));

	playGame();

	return 0;

}

void playGame() {

	Font font;
	font.loadFromFile("resources/sansation.ttf");

	Texture coinTexture;
	coinTexture.loadFromFile("resources/images/coin.png");

	Sprite coin(coinTexture);

	string mazeInput;
	bool gameStarted = false;
	bool gameFinished = false;
	bool loseGame = false;
	bool solutionUpdated = false;
	bool autoSolverEnabled = false;

	int upperOffset = 50;
	int leftOffset = 350;

	int curX = 0, curY = 1;
	int lastX = 0, lastY = 1;
	int ASfirstX = 0, ASfirstY = 1;
	int holdX = 0, holdY = 1;
	bool hitWall = false;
	string file_name;

	Clock clock;
	ostringstream osstr;

	while (window.isOpen()) {

		Event event;

		while (window.pollEvent(event)) {

			if (event.type == Event::Closed || (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape && !autoSolverEnabled))
				window.close();

			if (event.type == Event::TextEntered) {
				if (event.text.unicode >= 48 && event.text.unicode <= 57) {
					if (mazeInput.length() < 3)
						mazeInput += event.text.unicode;
				}
			}

			if (event.type == Event::KeyPressed) {

				if (!gameStarted && event.key.code == Keyboard::BackSpace) {
					if (mazeInput.length() > 0)
						mazeInput.pop_back();
				}

				if ((!gameStarted && event.key.code == Keyboard::Enter) || (gameFinished && event.key.code == Keyboard::Space)) {
					if (mazeInput.length() > 1) {

						mazeSize = stoi(mazeInput);

						if (mazeSize < 10) mazeSize = 10;
						if (mazeSize > 100) mazeSize = 100;

						setCellVisited(0);
						totalCoins = 0;
						moveHistoryIterator = 0;

						curX = 0, curY = 1;
						lastX = 0, lastY = 1;
						ASfirstX = 0, ASfirstY = 1;
						holdX = 0, holdY = 1;

						generateMaze();

						file_name = saveSolution();

						gameStarted = true;
						gameFinished = false;
					}
				}

				if (gameStarted && (event.key.code == Keyboard::Up || event.key.code == Keyboard::Down || event.key.code == Keyboard::Left || event.key.code == Keyboard::Right)) {

					m(curY, curX)->c = wallType::road;

					if (event.key.code == Keyboard::Up) curY--;
					if (event.key.code == Keyboard::Down) curY++;
					if (event.key.code == Keyboard::Left) curX--;
					if (event.key.code == Keyboard::Right) curX++;

					if (curX == mazeSize - 1 && curY == mazeSize - 2) gameFinished = true;

					if (m(curY, curX)->c == wallType::wall || curX < 0) { // hit wall
						curX = lastX;
						curY = lastY;
						hitWall = true;
					}
					else if (m(curY, curX)->c == wallType::coin) // hit coin
						totalCoins++;
					else if (m(curY, curX)->c == wallType::enemy) // hit enemy
						loseGame = true; // restart game

					m(curY, curX)->c = wallType::cursor;

					lastX = curX;
					lastY = curY;

					if (hitWall) {
						cout << endl << "You Hit Wall!" << endl;
						hitWall = false;
					}
					else {
						saveMove(curX, curY);
					}

				}

				if (gameStarted && !autoSolverEnabled && event.key.code == Keyboard::Tab) {

					moveHistoryIterator = 0;
					setCellVisited(0);

					ASfirstX = curX;
					ASfirstY = curY;

					autoSolverEnabled = true;

				}

				if (autoSolverEnabled && event.key.code == Keyboard::Escape) {
					autoSolverEnabled = false;
				}

				if (gameStarted && !gameFinished && event.key.code == Keyboard::C) {
					gameStarted = false;
					mazeInput = "";
				}

			}

		}

		if (gameFinished) window.clear(Color(39, 174, 96));
		else window.clear(Color(44, 62, 80));

		osstr.str("");
		osstr << "Frame: " << clock.restart().asMilliseconds() << "ms";

		Text frame;
		frame.setFillColor(Color::White);
		frame.setFont(font);
		frame.setOutlineColor(Color::Black);
		frame.setOutlineThickness(0.3);
		frame.setString(osstr.str());
		frame.setPosition(10, 10);
		frame.setCharacterSize(16);
		window.draw(frame);

		Text text;
		text.setFillColor(Color::White);
		text.setFont(font);
		text.setOutlineColor(Color::Black);
		text.setOutlineThickness(0.1);

		if (autoSolverEnabled) {

			m(holdY, holdX)->c = wallType::road;

			holdX = curX;
			holdY = curY;

			solveMazeRecursive(&curX, &curY, ASfirstX, ASfirstY);

			m(curY, curX)->c = wallType::cursor;

			Sleep(20 / mazeSize * mazeSize);

			if (curX == mazeSize - 2 && curY == mazeSize - 2)
				autoSolverEnabled = false;

		}

		if (!gameStarted) {

			text.setString("WELCOME TO MAZE GAME");
			text.setPosition(150, 150);
			text.setCharacterSize(40);
			window.draw(text);

			text.setString("by Enes Solak");
			text.setPosition(500, 200);
			text.setCharacterSize(25);
			window.draw(text);

			text.setString("Enter maze size (10-100): ");
			text.setPosition(150, 300);
			text.setCharacterSize(20);
			window.draw(text);

			text.setString(mazeInput);
			text.setPosition(380, 300);
			text.setCharacterSize(20);
			text.setStyle(Text::Bold | Text::Underlined | Text::Italic);
			window.draw(text);
			text.setStyle(Text::Regular);

		}

		if (!gameFinished && !loseGame && gameStarted) {

			text.setString("Game Started");
			text.setCharacterSize(30);
			text.setPosition(50, 50);
			window.draw(text);

			coin.setPosition(50, 125);
			window.draw(coin);

			text.setString("Total coins: " + to_string(totalCoins));
			text.setCharacterSize(20);
			text.setPosition(75, 120);
			window.draw(text);

			text.setString("[C] to change maze size");
			text.setPosition(50, 170);
			text.setCharacterSize(20);
			window.draw(text);

			text.setString("[Tab] to Auto-Solver");
			text.setPosition(50, 200);
			text.setCharacterSize(20);
			window.draw(text);

			text.setString("[Escape] to exit");
			text.setPosition(50, 230);
			text.setCharacterSize(20);
			window.draw(text);

			printMaze(leftOffset, upperOffset);

		}

		if (loseGame) {

			window.clear(Color(192, 57, 43));

			text.setString("YOU LOST!");
			text.setPosition(150, 50);
			text.setCharacterSize(100);
			text.setOutlineThickness(0.5);
			text.setStyle(Text::Bold);
			window.draw(text);
			text.setStyle(Text::Regular);

			text.setString("Restarting...");
			text.setPosition(310, 200);
			text.setCharacterSize(40);
			text.setOutlineThickness(0.1);
			window.draw(text);

			window.display();

			remove(file_name.c_str()); // remove solution file
			Sleep(1000);
			playGame();

		}

		if (gameFinished) {

			text.setString("YOU WIN!");
			text.setPosition(170, 50);
			text.setCharacterSize(100);
			text.setOutlineThickness(0.5);
			text.setStyle(Text::Bold);
			window.draw(text);
			text.setStyle(Text::Regular);

			coin.setPosition(110, 195);
			window.draw(coin);

			text.setString("Total coins: " + to_string(totalCoins));
			text.setCharacterSize(20);
			text.setPosition(130, 190);
			text.setOutlineThickness(0.1);
			window.draw(text);

			if (file_name != "") {
				text.setString("Solution file created: " + file_name);
				if (!solutionUpdated) {
					updateSolution(file_name);
					solutionUpdated = true;
				}
			}
			else {
				text.setString("An error occurred while trying to save solution file!");
				text.setFillColor(Color::Red);
			}
			text.setPosition(110, 240);
			text.setCharacterSize(20);
			text.setStyle(Text::Underlined);
			window.draw(text);
			text.setStyle(Text::Regular);
			text.setFillColor(Color::White);

			text.setString("[Space] to play again");
			text.setPosition(110, 280);
			text.setCharacterSize(20);
			window.draw(text);

			text.setString("[Escape] to exit");
			text.setPosition(110, 310);
			text.setCharacterSize(20);
			window.draw(text);

		}


		window.display();

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

		/* window.clear(Color(44, 62, 80));
		m(curY, curX)->c = wallType::cursor;
		printMaze(50, 50);
		Sleep(20/mazeSize*mazeSize);
		window.display(); */

		m(curY, curX)->c = wallType::road;
		m(curY, curX)->visited = 1;

		if (curY > 0 && curY < mazeSize - 1 && curX > 1 && m(curY, curX - 1)->c != wallType::road && m(curY, curX - 1)->visited != 1) {
			leftBlock = true;
			if (curX > 2 && m(curY, curX - 2)->c != wallType::wall)
				leftBlock = false;
			else if (m(curY - 1, curX - 1)->c != wallType::wall || m(curY + 1, curX - 1)->c != wallType::wall)
				leftBlock = false;
			else if (m(curY - 1, curX - 2)->c != wallType::wall || m(curY + 1, curX - 2)->c != wallType::wall)
				leftBlock = false;
		}
		else leftBlock = false;

		if (curY > 0 && curY < mazeSize - 1 && curX < mazeSize - 2 && m(curY, curX + 1)->c != wallType::road && m(curY, curX + 1)->visited != 1) {
			rightBlock = true;
			if (curX < mazeSize - 3 && m(curY, curX + 2)->c != wallType::wall)
				rightBlock = false;
			else if (m(curY - 1, curX + 1)->c != wallType::wall || m(curY + 1, curX + 1)->c != wallType::wall)
				rightBlock = false;
			else if (m(curY - 1, curX + 2)->c != wallType::wall || m(curY + 1, curX + 2)->c != wallType::wall)
				rightBlock = false;
		}
		else rightBlock = false;

		if (curX > 0 && curX < mazeSize - 1 && curY > 1 && m(curY - 1, curX)->c != wallType::road && m(curY - 1, curX)->visited != 1) {
			upperBlock = true;
			if (curY > 2 && m(curY - 2, curX)->c != wallType::wall)
				upperBlock = false;
			else if (m(curY - 1, curX + 1)->c != wallType::wall || m(curY - 1, curX - 1)->c != wallType::wall)
				upperBlock = false;
			else if (m(curY - 2, curX + 1)->c != wallType::wall || m(curY - 2, curX - 1)->c != wallType::wall)
				upperBlock = false;
		}
		else upperBlock = false;

		if (curX > 0 && curX < mazeSize - 1 && curY < mazeSize - 2 && m(curY + 1, curX)->c != wallType::road && m(curY + 1, curX)->visited != 1) {
			lowerBlock = true;
			if (curY < mazeSize - 3 && m(curY + 2, curX)->c != wallType::wall)
				lowerBlock = false;
			else if (m(curY + 1, curX + 1)->c != wallType::wall || m(curY + 1, curX - 1)->c != wallType::wall)
				lowerBlock = false;
			else if (m(curY + 2, curX + 1)->c != wallType::wall || m(curY + 2, curX - 1)->c != wallType::wall)
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

void printMaze(int leftOffset, int upperOffset) {

	Texture wallTexture, coinTexture, enemyTexture, cursorTexture;
	wallTexture.loadFromFile("resources/images/wall.png");
	coinTexture.loadFromFile("resources/images/coin.png");
	enemyTexture.loadFromFile("resources/images/enemy.png");
	cursorTexture.loadFromFile("resources/images/cursor.png");

	Sprite wall(wallTexture), coin(coinTexture), enemy(enemyTexture), cursor(cursorTexture);

	RectangleShape rectangle(Vector2f(18, 18));
	rectangle.setOutlineColor(Color::Black);
	rectangle.setOutlineThickness(0.5);
	rectangle.setFillColor(Color(236, 240, 241));

	int curX = 0, curY = 0;

	for (int y = 0; y < mazeSize; y++)
	{
		for (int x = 0; x < mazeSize; x++)
		{
			if (m(y, x)->c == wallType::cursor) {
				curX = x;
				curY = y;
				break;
			}
		}
	}

	int width = 20, height = 20;
	int startX = 0, startY = 0;

	if (mazeSize > width) {

		if (curX > width / 2)
			startX = curX - width / 2;

		if (curY > height / 2)
			startY = curY - height / 2;

		if (mazeSize - startX < width)
			startX = mazeSize - width;

		if (mazeSize - startY < height)
			startY = mazeSize - height;
	}

	int lastX = startX + width, lastY = startY + height;

	if (lastX > mazeSize)
		lastX = mazeSize;

	if (lastY > mazeSize)
		lastY = mazeSize;

	for (int y = startY; y < lastY; y++)
	{
		for (int x = startX; x < lastX; x++)
		{

			if (m(y, x)->c != wallType::wall) {
				rectangle.setPosition(leftOffset + (x - startX) * 18, upperOffset + (y - startY) * 18);
				window.draw(rectangle);
			}

			if (m(y, x)->c == wallType::wall) { // wall
				wall.setPosition((x - startX) * 18, (y - startY) * 18);
				wall.move(leftOffset, upperOffset); //offset
				window.draw(wall);
			}
			else if (m(y, x)->c == wallType::coin) { // coin
				coin.setPosition((x - startX) * 18, (y - startY) * 18);
				coin.move(leftOffset, upperOffset); //offset
				window.draw(coin);
			}
			else if (m(y, x)->c == wallType::enemy) { // enemy
				enemy.setPosition((x - startX) * 18, (y - startY) * 18);
				enemy.move(leftOffset, upperOffset); //offset
				window.draw(enemy);
			}
			else if (m(y, x)->c == wallType::cursor) { // cursor
				cursor.setPosition((x - startX) * 18, (y - startY) * 18);
				cursor.move(leftOffset, upperOffset); //offset
				window.draw(cursor);
			}

		}
	}
}

void solveMaze() {

	moveHistoryIterator = 0;

	setCellVisited(0);

	int curX = 0;
	int curY = 1;

	while (true) {

		bool canMoveLeft = curX > 1 && m(curY, curX - 1)->c != wallType::wall && m(curY, curX - 1)->visited != 1 ? true : false;
		bool canMoveRight = curX < mazeSize - 2 && m(curY, curX + 1)->c != wallType::wall && m(curY, curX + 1)->visited != 1 ? true : false;
		bool canMoveUpper = curY > 1 && m(curY - 1, curX)->c != wallType::wall && m(curY - 1, curX)->visited != 1 ? true : false;
		bool canMoveLower = curY < mazeSize - 2 && m(curY + 1, curX)->c != wallType::wall && m(curY + 1, curX)->visited != 1 ? true : false;

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

void solveMazeRecursive(int *curX, int *curY, int firstX, int firstY) {

	bool canMoveLeft, canMoveRight, canMoveUpper, canMoveLower;

	if (firstX != 1 && firstY != 1 && *curX == 1 && *curY == 1)
		setCellVisited(0);

	canMoveLeft = *curX > 1 && m(*curY, *curX - 1)->c != wallType::wall && m(*curY, *curX - 1)->visited != 1 && m(*curY, *curX - 1)->c != wallType::enemy ? true : false;
	canMoveRight = *curX < mazeSize - 2 && m(*curY, *curX + 1)->c != wallType::wall && m(*curY, *curX + 1)->visited != 1 && m(*curY, *curX + 1)->c != wallType::enemy ? true : false;
	canMoveUpper = *curY > 1 && m(*curY - 1, *curX)->c != wallType::wall && m(*curY - 1, *curX)->visited != 1 && m(*curY - 1, *curX)->c != wallType::enemy ? true : false;
	canMoveLower = *curY < mazeSize - 2 && m(*curY + 1, *curX)->c != wallType::wall && m(*curY + 1, *curX)->visited != 1 && m(*curY + 1, *curX)->c != wallType::enemy ? true : false;

	m(*curY, *curX)->c = wallType::road;
	m(*curY, *curX)->visited = 1;

	if (canMoveRight) (*curX)++;
	else if (canMoveLower) (*curY)++;
	else if (canMoveLeft) (*curX)--;
	else if (canMoveUpper) (*curY)--;

	if (m(*curY, *curX)->c == wallType::coin) totalCoins++;

	if (!canMoveLeft && !canMoveRight && !canMoveUpper && !canMoveLower)
		getLastMove(curX, curY);
	else
		saveMove(*curX, *curY);

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