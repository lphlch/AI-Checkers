/************************************************************************************************
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

using std::cout;
using std::endl;

//命令结构体
struct Command
{
	int x[MAX_STEP];
	int y[MAX_STEP];
	int numStep;
};
//己方棋子结构体
struct Chess
{
	int x;
	int y;
	bool isKing;
	bool isEaten;	//标记是否被吃了
	Command jump[MAX_STEP];	//标记每个棋子的合法走法
	int maxJumpStep;
	int maxJumpNum;
	bool isJump;	//标记这一跳是否为吃
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
struct Chess myChesses[12];

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

//清空上一次所有己方棋子存储的走法
void clearAllMyChessesStep()
{
	/* debug */
	cout << "DEBUG: ALL MY CHESS CLEAR!\n";
	for (int i = 0; i < MAX_CHESS; i++)
	{
		memset(myChesses[i].jump, 0, sizeof(myChesses[i].jump));
		myChesses[i].maxJumpStep = 0;
		myChesses[i].maxJumpNum = 0;
		myChesses[i].isJump = false;
	}
}
//清空上一个棋子的走法
void clearOneChessStep(Chess& chess)
{
	memset(chess.jump, 0, sizeof(chess.jump));
	chess.maxJumpStep = 0;
	chess.maxJumpNum = 0;
	chess.isJump = false;
}

//单格移动
bool tryToMove(int x, int y,int id)
{
	int newX, newY;

	//for (int i = 0; i < board[x][y]; i++)
	/* 没升王之前，黑棋me==1只能往上跳(减,i=2,3)，白棋me==2只能往下跳(加,i=0,1) */
	int dir = (2 - me) * 2 * (!isKing(x, y));
	for (int i = dir; i < dir + 2 + (isKing(x, y)) * 2; i++)
	{
		newX = x + moveDir[i][0];
		newY = y + moveDir[i][1];
		/* 若可以移动，则把移动步骤加入走法中 */
		if (isInBound(newX, newY) && board[newX][newY] == EMPTY)
		{
			Command& moveCmd = myChesses[id].jump[myChesses[id].maxJumpNum];
			moveCmd.x[0] = x;
			moveCmd.y[0] = y;
			moveCmd.x[1] = newX;
			moveCmd.y[1] = newY;
			moveCmd.numStep = 2;
			myChesses[id].maxJumpNum++;
		}
	}

	if (myChesses[id].maxJumpNum > 0)
	{
		return true;
	}
	return false;
}

//跳跃
void tryToJump(int x, int y, int currentStep,int id)
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
			tryToJump(newX, newY, currentStep + 1,id);
			board[x][y] = board[newX][newY];
			board[newX][newY] = EMPTY;
			board[midX][midY] = tmpFlag;
		}
	}
	/* 如果这一跳是最长的，存为最长 */
	/* 最长的可能有多个 */
	if ((jumpCmd.numStep >= 2)&&(jumpCmd.numStep >= myChesses[id].maxJumpStep))
	{
		cout << "DEBUG: it can jump\n";
		myChesses[id].isJump = true;
		myChesses[id].maxJumpStep = jumpCmd.numStep;
		/* 如果是最长的，且长于2（防止清空其余单步），清空所有已存储的走法 */
		if ((jumpCmd.numStep > 2)&&jumpCmd.numStep > myChesses[id].maxJumpStep)
		{
			clearOneChessStep(myChesses[id]);
			cout << "DEBUG: CLEAR my chess id: " << id << " for maxStep " << jumpCmd.numStep << endl;
			myChesses[id].maxJumpStep = jumpCmd.numStep;
		}
		memcpy(&myChesses[id].jump[myChesses[id].maxJumpNum], &jumpCmd, sizeof(struct Command));
		myChesses[id].maxJumpNum++;
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
	cout << "DEBUG: PLACE " << cmd.numStep;
	for (int i = 0; i < cmd.numStep; i++)
	{
		cout << " " << cmd.x[i] << "," << cmd.y[i] << " ";
	}
	cout << endl;
	int midX, midY, curFlag;
	curFlag = board[cmd.x[0]][cmd.y[0]];
	for (int i = 0; i < cmd.numStep - 1; i++)
	{
		/* 移动棋子 */
		board[cmd.x[i]][cmd.y[i]] = EMPTY;
		board[cmd.x[i + 1]][cmd.y[i + 1]] = curFlag;
		if (curFlag == me)	//如果是我方棋子，同时移动结构体
		{
			Chess& curChess = myChesses[findMyChess(cmd.x[i], cmd.y[i])];
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
				std::cout << "DEBUG: My Chess" << id << "was eaten" << std::endl;
			}
			/* 无论跨越是不是我方的，都吃掉（吃自己人233） */
			std::cout << "DEBUG: Eat" << midX << "," << midY << std::endl;

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

//初始化AI
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
	clearAllMyChessesStep();
	debug("Init Success");
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
	bool isJump = false;	//标记是否可以跳而不是移动
	int maxStep = 1;

	/* 清空上一次己方棋子存储的走法 */
	clearAllMyChessesStep();

	/* 随机落子 */
	bool israned[MAX_CHESS] = { false };
	bool isAllRaned = false;
	while (!isAllRaned)
	{
		int ran = rand() % MAX_CHESS;
		/* 如果已经被随机过了		或		被吃了 ，则进行下一次随机*/
		if (israned[ran] == true || myChesses[ran].isEaten == true)
		{
			continue;
		}
		int x = myChesses[ran].x;
		int y = myChesses[ran].y;
		//longestJumpCmd.numStep = 1;

		/* 尝试跳跃吃子,最后每个棋子的合法走法在myChess的jump数组里面 */
		cout << "DEBUG: Now judge jump my chess id: " << ran << " x=" << myChesses[ran].x << " y=" << myChesses[ran].y << endl;
		tryToJump(x, y, 0,ran);
		/* 如果还不能吃，但是这个棋子可以吃 或 这个棋子最长的走法大于所有棋子的走法,该走法设为最长 */
		if ((isJump==false && myChesses[ran].maxJumpStep >= 2) ||myChesses[ran].maxJumpStep > maxStep)
		{
			maxStep = myChesses[ran].maxJumpStep;
			isJump = true;
			cout << "DEBUG: GROBLE max step= " << maxStep << " isJump= " << isJump << endl;
			////复制命令
			//maxStep = longestJumpCmd.numStep;
			//memcpy(&command, &longestJumpCmd, sizeof(struct Command));
		}

		/* 如果无法跳跃，则开始移动 */
		if (isJump==false)
		{
			/* debug */
			cout << "DEBUG: Now judge move my chess id: " << ran << " x="<< myChesses[ran].x<<" y="<<myChesses[ran].y<<endl;
			clearOneChessStep(myChesses[ran]);
			if (tryToMove(x, y,ran))
			{
				myChesses[ran].maxJumpStep = 2;
				maxStep = 2;
				////复制命令
				//memcpy(&command, &moveCmd, sizeof(struct Command));
			}
		}

		/* 标记被随机，准备进行下一次循环 */
		israned[ran] = true;

		/* 检测是否已经全部被随机 */
		for (int i = 0; i < MAX_CHESS; i++)
		{
			if (israned[i] == true || myChesses[i].isEaten == true)
			{
				/* 若已经被随机 */
				isAllRaned = true;
			}
			else
			{
				isAllRaned = false;
				break;
			}
		}
	}

	/* 检索所有最大走法，并返回第一个（暂时） */
	cout << "DEBUG: \nDEBUG: MaxStep=" << maxStep <<" with isJump= "<< isJump<< endl;
	for (int i = 0; i < MAX_CHESS; i++)
	{
		if (myChesses[i].maxJumpStep == maxStep && myChesses[i].isJump==isJump)
		{
			cout << "DEBUG: id=" << i << " x=" << myChesses[i].x << " y=" << myChesses[i].y<<" isJump= "<<myChesses[i].isJump<<endl;
			for (int j = 0; j < myChesses[i].maxJumpNum; j++)
			{
				cout << "DEBUG: Way-" << j;
				//暂时返回第一个
				memcpy(&command, &myChesses[i].jump[j], sizeof(struct Command));
				for (int k = 0; k < myChesses[i].jump[j].numStep; k++)
				{
					cout << " " << myChesses[i].jump[j].x[k] << "," << myChesses[i].jump[j].y[k] << " ";
				}
				cout << endl;
			}
		}
	}
	cout << "DEBUG: Step out END\nDEBUG: \n";
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
