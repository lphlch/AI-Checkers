﻿/************************************************************************************************
 The project is about AI checkers.

 The code is based on "baseline"(https://github.com/sse2018-makyek-fun/std-client)
 Not only the given region code has changed, but also other functions.

 Copyright © 2021 LPH.
************************************************************************************************/
// make scanf safely
#define _CRT_SECURE_NO_WARNINGS

// board information
#define BOARD_SIZE 8
#define EMPTY 0
#define WHITE_FLAG 2
#define WHITE_KING 4
#define BLACK_FLAG 1
#define BLACK_KING 3

#define MAX_STEP 15
#define MAX_CHESS 12

#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
//命令结构体
struct Command
{
	int x[MAX_STEP];
	int y[MAX_STEP];
	int numStep;
};
//己方棋子结构体
struct myChess
{
	int x;
	int y;
	bool isKing;
	bool isEaten;	//标记是否被吃了
};

char board[BOARD_SIZE][BOARD_SIZE] = { 0 };
int myFlag;
int moveDir[4][2] = { {1, -1}, {1, 1}, {-1, -1}, {-1, 1} };	//移动一次一格
int jumpDir[4][2] = { {2, -2}, {2, 2}, {-2, -2}, {-2, 2} };	//跳跃一次两格
int numMyFlag;
int me;
struct Command moveCmd = { {0},{0},2 };
struct Command jumpCmd = { {0}, {0},  0 };
struct Command longestJumpCmd = { {0},{0}, 1 };
struct myChess myChesses[12];

void debug(std::string str)
{
	std::cout << "DEBUG " << str << std::endl;
	fflush(stdout);
}

//打印棋盘 1黑2白 上白下黑 黑0@，白X* 坐标先列后行
void printBoard()
{
	char visualBoard[BOARD_SIZE][BOARD_SIZE + 1] = { 0 };
	std::cout << "DEBUG:   01234567\n";
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			switch (board[i][j])
			{
				case EMPTY:
					visualBoard[i][j] = '.';
					break;
				case BLACK_FLAG:
					visualBoard[i][j] = 'O';
					break;
				case WHITE_FLAG:
					visualBoard[i][j] = 'X';
					break;
				case BLACK_KING:
					visualBoard[i][j] = '@';
					break;
				case WHITE_KING:
					visualBoard[i][j] = '*';
					break;
				default:
					break;
			}
		}
		printf("DEBUG: %d %s\n", i, visualBoard[i]);
	}
}

//是否在边界内
bool isInBound(int x, int y)
{
	return x >= 0 && x < BOARD_SIZE&& y >= 0 && y < BOARD_SIZE;
}

//是否为王
bool isKing(int x, int y)
{
	return board[x][y] == WHITE_KING || board[x][y] == BLACK_KING;
}

void rotateCommand(struct Command* cmd)
{
	if (myFlag == BLACK_FLAG)
	{
		for (int i = 0; i < cmd->numStep; i++)
		{
			cmd->x[i] = BOARD_SIZE - 1 - cmd->x[i];
			cmd->y[i] = BOARD_SIZE - 1 - cmd->y[i];
		}
	}
}

//单格移动
int tryToMove(int x, int y)
{
	int newX, newY;

	//for (int i = 0; i < board[x][y]; i++)
	/* 没升王之前，黑棋me==1只能往上跳(减,i=2,3)，白棋me==2只能往下跳(加,i=0,1) */
	int dir = (2 - me) * 2 * (!isKing(x, y));
	for (int i = dir; i < dir + 2 + (isKing(x, y)) * 2; i++)
	{
		newX = x + moveDir[i][0];
		newY = y + moveDir[i][1];
		if (isInBound(newX, newY) && board[newX][newY] == EMPTY)
		{
			moveCmd.x[0] = x;
			moveCmd.y[0] = y;
			moveCmd.x[1] = newX;
			moveCmd.y[1] = newY;
			return i;
		}
	}

	return -1;
}

//跳跃
void tryToJump(int x, int y, int currentStep)
{
	int newX, newY, midX, midY;
	char tmpFlag;
	jumpCmd.x[currentStep] = x;
	jumpCmd.y[currentStep] = y;
	jumpCmd.numStep++;

	for (int i = 0; i < 4; i++)
	{
		newX = x + jumpDir[i][0];	//跳跃后的坐标
		newY = y + jumpDir[i][1];
		midX = (x + newX) / 2;		//跳跃时跨越的坐标
		midY = (y + newY) / 2;

		/*白棋010 100，黑棋001 011，空000
		* &1	0 0	       1 1		0
		* 黑me-1==0,白me-1==1
		* 若我黑，则&1==0为敌人,me-1==0
		* 若我白，则&1==1为敌人,me-1==1
		*/
		/* 如果在边界内              且   跨越的格子非空				且     跨越的格子是敌人的				且		跳跃后格子为空 */
		if (isInBound(newX, newY) && board[midX][midY] != EMPTY && ((board[midX][midY] & 1) == (me - 1)) && (board[newX][newY] == EMPTY))
		{
			board[newX][newY] = board[x][y];
			board[x][y] = EMPTY;
			tmpFlag = board[midX][midY];
			board[midX][midY] = EMPTY;
			tryToJump(newX, newY, currentStep + 1);
			board[x][y] = board[newX][newY];
			board[newX][newY] = EMPTY;
			board[midX][midY] = tmpFlag;
		}
	}
	/* 如果这一跳是最大长的，存为最长 */
	if (jumpCmd.numStep > longestJumpCmd.numStep)
	{
		memcpy(&longestJumpCmd, &jumpCmd, sizeof(struct Command));
	}

	jumpCmd.numStep--;
}

//根据坐标找到我的棋
int findMyChess(int x, int y)
{
	for (int i = 0; i < MAX_CHESS; i++)
	{
		if (!myChesses[i].isEaten && myChesses[i].x == x && myChesses[i].y == y)
		{
			return i;
		}
	}
	return -1;
}

void place(struct Command cmd)
{
	/*debug*/
	std::cout << "DEBUG: PLACE " << cmd.numStep;
	for (int i = 0; i < cmd.numStep; i++)
	{
		std::cout << " " << cmd.x[i] << "," << cmd.y[i] << " ";
	}
	std::cout << std::endl;
	int midX, midY, curFlag;
	curFlag = board[cmd.x[0]][cmd.y[0]];
	for (int i = 0; i < cmd.numStep - 1; i++)
	{
		/* 移动棋子 */
		board[cmd.x[i]][cmd.y[i]] = EMPTY;
		board[cmd.x[i + 1]][cmd.y[i + 1]] = curFlag;
		if (curFlag == me)	//如果是我方棋子，同时移动结构体
		{
			myChess& curChess = myChesses[findMyChess(cmd.x[i], cmd.y[i])];
			curChess.x = cmd.x[i + 1];
			curChess.y = cmd.y[i + 1];
		}

		/* 如果是跳 */
		if (abs(cmd.x[i] - cmd.x[i + 1]) == 2)
		{
			midX = (cmd.x[i] + cmd.x[i + 1]) / 2;
			midY = (cmd.y[i] + cmd.y[i + 1]) / 2;
			/* 如果跨越是我方棋子，己方棋子少一个 */
			/*白棋010 100，黑棋001 011，空000
			* &1	0 0	       1 1		0
			* 黑me-1==0,白me-1==1
			* 若我黑，则&1==0为敌人,me-1==0
			* 若我白，则&1==1为敌人,me-1==1
			*/
			if (board[midX][midY] != EMPTY && (board[midX][midY] & 1) != (me - 1))
			{
				numMyFlag--;
				/* 找到我方被吃的那个棋，吃掉 */
				int id = findMyChess(midX, midY);
				myChesses[id].isEaten = true;
				std::cout << "DEBUG: 我方编号" << id << "被吃" << std::endl;;
			}
			/* 无论跨越是不是我方的，都吃掉（吃自己人233） */
			std::cout << "DEBUG: 吃子" << midX << "," << midY << std::endl;

			board[midX][midY] = EMPTY;
		}
	}
	/* 升王 */
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (board[0][i] == BLACK_FLAG)
		{
			board[0][i] = BLACK_KING;
		}
		if (board[BOARD_SIZE - 1][i] == WHITE_FLAG)
		{
			board[BOARD_SIZE - 1][i] = WHITE_KING;
		}
	}
}

void initAI()
{
	/* 我方棋子初始化 */
	numMyFlag = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			if (board[i][j] == me)
			{
				myChesses[numMyFlag].x = i;
				myChesses[numMyFlag].y = j;
				myChesses[numMyFlag].isKing = false;
				myChesses[numMyFlag].isEaten = false;
				numMyFlag++;
			}
		}
	}
	debug("初始化成功");
}

/**
 * 轮到你落子。
 * 棋盘上0表示空白，1表示黑棋，2表示白棋
 * me表示你所代表的棋子(1或2)
 * 你需要返回一个结构体Command，其中numStep是你要移动的棋子经过的格子数（含起点、终点），
 * x、y分别是该棋子依次经过的每个格子的横、纵坐标
 */
struct Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE])
{
	struct Command command =
	{
		{0},
		{0},
		0
	};
	int numChecked = 0;
	int maxStep = 1;
	/* 随机落子 */
	bool israned[MAX_CHESS] = { false };
	while (true)
	{
		int ran = rand() % MAX_CHESS;
		/* 如果已经被随机过了		或		被吃了 ，则进行下一次随机*/
		if (israned[ran] == true || myChesses[ran].isEaten == true)
		{
			continue;
		}
		int x = myChesses[ran].x;
		int y = myChesses[ran].y;
		numChecked++;
		longestJumpCmd.numStep = 1;

		/* 尝试跳跃吃子 */
		tryToJump(x, y, 0);
		if (longestJumpCmd.numStep > maxStep)
		{
			//复制命令
			memcpy(&command, &longestJumpCmd, sizeof(struct Command));
			break;
		}

		/* 如果无法跳跃，则开始移动 */
		if (command.numStep == 0)
		{
			if (tryToMove(x, y) >= 0)
			{
				//复制命令
				memcpy(&command, &moveCmd, sizeof(struct Command));
			}
		}

		if (numChecked >= numMyFlag)
		{
			return command;
		}

		/* 如果没有成功落子，标记被随机，进行下一次循环 */
		israned[ran] = true;
	}
	//for (int i = 0; i < BOARD_SIZE; i++)
	//{
	//	for (int j = 0; j < BOARD_SIZE; j++)
	//	{
	//		//白棋010 100，黑棋001 011，空000
	//		//代表 有棋			且	棋是我方的
	//		if (board[i][j] > 0 && (board[i][j] & 1) != (me - 1))
	//		{
	//			numChecked++;
	//			longestJumpCmd.numStep = 1;
	//			/* 尝试跳跃吃子 */
	//			tryToJump(i, j, 0);
	//			if (longestJumpCmd.numStep > maxStep)
	//			{
	//				memcpy(&command, &longestJumpCmd, sizeof(struct Command));
	//			}
	//			/* 如果无法跳跃，则开始移动 */
	//			if (command.numStep == 0)
	//			{
	//				if (tryToMove(i, j) >= 0)
	//				{
	//					memcpy(&command, &moveCmd, sizeof(struct Command));
	//				}
	//			}
	//		}
	//		if (numChecked >= numMyFlag)
	//		{
	//			return command;
	//		}
	//	}
	//}
	return command;
}

//1黑2白
//.X.X.X.X
//X.X.X.X.
//.X.X.X.X
//........
//........
//O.O.O.O.
//.O.O.O.O
//O.O.O.O.
//初始化棋盘
void start()
{
	memset(board, 0, sizeof(board));
	/* 初始化上半棋盘 白色 */
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 8; j += 2)
		{
			board[i][j + (i + 1) % 2] = WHITE_FLAG;
		}
	}
	/* 初始化下半棋盘 黑色 */
	for (int i = 5; i < 8; i++)
	{
		for (int j = 0; j < 8; j += 2)
		{
			board[i][j + (i + 1) % 2] = BLACK_FLAG;
		}
	}

	initAI();
}

void turn()
{
	// AI
	struct Command command = aiTurn((const char(*)[BOARD_SIZE])board);
	place(command);
	//rotateCommand(&command);
	printf("%d", command.numStep);
	for (int i = 0; i < command.numStep; i++)
	{
		printf(" %d,%d", command.x[i], command.y[i]);
	}
	printf("\n");
	fflush(stdout);
}

void end(int x)
{
	exit(0);
}

void loop()
{
	char tag[10] = { 0 };
	struct Command command =
	{
		{0},
		{0},
		0
	};
	int status;
	while (true)
	{
		memset(tag, 0, sizeof(tag));
		scanf("%s", tag);
		if (strcmp(tag, START) == 0)
		{
			scanf("%d", &myFlag);
			me = myFlag;
			start();
			printf("OK\n");
			fflush(stdout);
		}
		else if (strcmp(tag, PLACE) == 0)
		{
			scanf("%d", &command.numStep);
			for (int i = 0; i < command.numStep; i++)
			{
				scanf("%d,%d", &command.x[i], &command.y[i]);
			}
			//rotateCommand(&command);
			place(command);
		}
		else if (strcmp(tag, TURN) == 0)
		{
			turn();
		}
		else if (strcmp(tag, END) == 0)
		{
			scanf("%d", &status);
			end(status);
		}
		printBoard();
	}
}

int main(int argc, char* argv[])
{
	srand(time(0));
	loop();
	return 0;
}
