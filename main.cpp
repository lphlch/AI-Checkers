/************************************************************************************************
 The project is about AI checkers.

 The code is based on "baseline"(https://github.com/sse2018-makyek-fun/std-client)
 Not only the given region code has changed, but also other functions.

 Copyright © 2021 LPH.
************************************************************************************************/
// make scanf safely
#define _CRT_SECURE_NO_WARNINGS

#define DEBUG_MODE 0
#define INIT_MODE 0
#define BOARD_MODE 0
#define SUBMIT_MODE 1
#define CMD_MODE 0
#define MAX_SHOW_LEVEL 1

// board information
#define BOARD_SIZE 8
#define EMPTY 0
#define WHITE_FLAG 2
#define WHITE_KING 4
#define BLACK_FLAG 1
#define BLACK_KING 3

#define MAX_STEP 14
#define MAX_CHESS 14
#define MAX_SEARCH 100	//最多搜索100种，否则时间也来不及
#define MAX_SEARCH_LEVEL 6

#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"

//#pragma region 分数表
const int WHITE_SCORE[BOARD_SIZE][BOARD_SIZE] = {
	{0,1,0,9,0,1,0,1},
	{1,0,3,0,3,0,3,0},
	{0,4,0,6,0,6,0,3},
	{6,0,7,0,7,0,5,0},
	{0,8,0,8,0,8,0,9},
	{7,0,8,0,8,0,8,0},
	{0,8,0,8,0,8,0,7},
	{0,0,0,0,0,0,0,0}
};
const int BLACK_SCORE[BOARD_SIZE][BOARD_SIZE] = {
	{0,0,0,0,0,0,0,0},
	{7,0,8,0,8,0,8,0},
	{0,8,0,8,0,8,0,7},
	{9,0,8,0,8,0,8,0},
	{0,5,0,7,0,7,0,6},
	{3,0,6,0,6,0,4,0},
	{0,3,0,3,0,3,0,1},
	{1,0,1,0,9,0,1,0}
};
const int WHITE_KING_SCORE[BOARD_SIZE][BOARD_SIZE] = {
	{0,12,0,12,0,12,0,12},
	{13,0,13,0,13,0,13,0},
	{0,14,0,14,0,14,0,14},
	{15,0,15,0,15,0,15,0},
	{0,15,0,15,0,15,0,15},
	{14,0,14,0,14,0,14,0},
	{0,13,0,13,0,13,0,13},
	{12,0,12,0,12,0,12,0}
};
const int BLACK_KING_SCORE[BOARD_SIZE][BOARD_SIZE] = {
	{0,12,0,12,0,12,0,12},
	{13,0,13,0,13,0,13,0},
	{0,14,0,14,0,14,0,14},
	{15,0,15,0,15,0,15,0},
	{0,15,0,15,0,15,0,15},
	{14,0,14,0,14,0,14,0},
	{0,13,0,13,0,13,0,13},
	{12,0,12,0,12,0,12,0},
};
//#pragma endregion

//const char CHESS_MANUAL[] = {
//	{};
//}

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include <cstring>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::max;
using std::min;

/*
*  对每一个己方的合法走法：
*	拷贝一块棋盘,己方棋子的id就是棋盘的id
*		在这个棋盘上检测敌方的走法，并取合法走法；
*		对每一敌方合法走法判断吃子数；
*
*  对每一敌方合法走法：
*	再拷贝一块棋盘；
*		再判断我方合法走法。
*/

//命令结构体
struct Command
{
	int x[MAX_STEP];
	int y[MAX_STEP];
	int numStep;
	bool isJump;
};

//合法命令结构体
struct LegalCommand
{
	Command cmd;
	int score;
};

//单一棋子所有命令结构体
struct SingleCommand
{
	vector<Command> cmds;
	int maxJumpStep;
	bool isJump;
};

//棋子结构体
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

int aiTurn(int level, char board[BOARD_SIZE][BOARD_SIZE], int curFlag, Command& cmd, bool& isEnd);
void initNextBoard(int level, int id);
void initNextLevel(int level, int id, Command cmd);
void place(struct Command cmd, char board[BOARD_SIZE][BOARD_SIZE], bool isVirtual = false, bool isEnemyTurn = false);

char board[BOARD_SIZE][BOARD_SIZE] = { 0 };
char stepBoard[MAX_SEARCH_LEVEL][MAX_SEARCH][BOARD_SIZE][BOARD_SIZE] = { 0 };	//移动后产生的棋盘
int myFlag;
int moveDir[4][2] = { {1, -1}, {1, 1}, {-1, -1}, {-1, 1} };	//移动一次一格
int jumpDir[4][2] = { {2, -2}, {2, 2}, {-2, -2}, {-2, 2} };	//跳跃一次两格
int me;
struct Command moveCmd = { {0},{0},2, false };
struct Command jumpCmd = { {0}, {0},  0, true };
struct Command longestJumpCmd = { {0},{0}, 1 ,false };
//struct LegalCommand legalCommand[MAX_SEARCH_LEVEL] = { 0 };
vector< vector<LegalCommand> > legalCommands(MAX_SEARCH_LEVEL);
struct Chess myChesses[14];
struct Chess enemyChesses[14];


void debug(std::string str)
{
	cout << "DEBUG: " << str << endl;
	fflush(stdout);
}

//打印棋盘 1黑2白 上白下黑 黑0@，白X* 坐标先列后行
void printBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
	char visualBoard[BOARD_SIZE][BOARD_SIZE + 1] = { 0 };
	if (0)
	{
		cout << "DEBUG: ";
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				cout << (int)board[i][j];
			}
			cout << ",";
		}
		cout << endl;
		return;
	}
	cout << "DEBUG:   01234567\n";
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
bool isKing(int x, int y, char board[BOARD_SIZE][BOARD_SIZE])
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

//清空上一次所有己方棋子存储的走法 0=all 1=my 2=enemy
void clearChessesStep(int mode)
{
	;
}
//清空上一个棋子的走法
void clearOneChessStep(Chess& chess)
{
	memset(chess.jump, 0, sizeof(chess.jump));
	chess.maxJumpStep = 0;
	chess.maxJumpNum = 0;
	chess.isJump = false;
}
//清空深层搜索 0全清 1清1层
void clearLevelSearch(int mode = 0)
{
	if (mode == 0)
	{
		//memset(legalCommand, 0, sizeof(legalCommand));
		legalCommands.clear();
		legalCommands.resize(MAX_SEARCH_LEVEL);
		memset(stepBoard, 0, sizeof(stepBoard));
		memset(enemyChesses, 0, sizeof(enemyChesses));
	}
	else
	{
		//memset(&legalCommand[1], 0, sizeof(legalCommand[1]));
		legalCommands[mode].clear();
	}


	return;
}

//单格移动
//bool tryToMove(int x, int y, Chess& chess, char board[BOARD_SIZE][BOARD_SIZE], int curFlag)
bool tryToMove(int x, int y, char board[BOARD_SIZE][BOARD_SIZE], int curFlag, SingleCommand& scmd)
{
	int newX, newY;

	//for (int i = 0; i < board[x][y]; i++)
	/* 没升王之前，黑棋curFlag==1只能往上跳(减,i=2,3)，白棋curFlag==2只能往下跳(加,i=0,1) */
	int dir = (2 - curFlag) * 2 * (!isKing(x, y, board));
	for (int i = dir; i < dir + 2 + (isKing(x, y, board)) * 2; i++)
	{
		//cout << x << ' ' << i;
		newX = x + moveDir[i][0];
		newY = y + moveDir[i][1];
		/* 若可以移动，则把移动步骤加入走法中 */
		if (isInBound(newX, newY) && board[newX][newY] == EMPTY)
		{
			//Command& moveCmd = chess.jump[chess.maxJumpNum];
			Command moveCmd;
			moveCmd.x[0] = x;
			moveCmd.y[0] = y;
			moveCmd.x[1] = newX;
			moveCmd.y[1] = newY;
			moveCmd.numStep = 2;
			scmd.cmds.push_back(moveCmd);
			//chess.maxJumpNum++;
		}
	}

	if (scmd.cmds.size() > 0)
	{
		return true;
	}
	return false;
}

//跳跃
//void tryToJump(int x, int y, int currentStep, Chess& chess, char board[BOARD_SIZE][BOARD_SIZE], int curFlag)
void tryToJump(int x, int y, int currentStep, char board[BOARD_SIZE][BOARD_SIZE], int curFlag, SingleCommand& scmd)
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
		if (isInBound(newX, newY) && board[midX][midY] != EMPTY && ((board[midX][midY] & 1) == (curFlag - 1)) && (board[newX][newY] == EMPTY))
		{
			board[newX][newY] = board[x][y];
			board[x][y] = EMPTY;
			tmpFlag = board[midX][midY];
			board[midX][midY] = EMPTY;
			tryToJump(newX, newY, currentStep + 1, board, curFlag, scmd);
			board[x][y] = board[newX][newY];
			board[newX][newY] = EMPTY;
			board[midX][midY] = tmpFlag;
		}
	}
	/* 如果这一跳是最长的，存为最长 */
	/* 最长的可能有多个 */
	if ((jumpCmd.numStep >= 2) && (jumpCmd.numStep >= scmd.maxJumpStep))
	{
		//cout << "DEBUG: it can jump\n";
		//chess.isJump = true;
		scmd.maxJumpStep = jumpCmd.numStep;
		/* 如果是最长的，且长于2（防止清空其余单步），清空所有已存储的走法 */
		if ((jumpCmd.numStep > 2) && jumpCmd.numStep > scmd.maxJumpStep)
		{
			scmd.cmds.clear();
			scmd.isJump = true;
			//cout << "DEBUG: CLEAR my chess id: " << id << " for maxStep " << jumpCmd.numStep << endl;
			scmd.maxJumpStep = jumpCmd.numStep;
		}
		//memcpy(&chess.jump[chess.maxJumpNum], &jumpCmd, sizeof(struct Command));
		scmd.cmds.push_back(jumpCmd);
		//chess.maxJumpNum++;
	}

	jumpCmd.numStep--;
}

//根据坐标找到我的棋
Chess& findChess(int x, int y, int curFlag)
{
	if (curFlag > 2)
	{
		curFlag = curFlag - 2;
	}
	if (curFlag == me)
	{
		for (int i = 0; i < MAX_CHESS; i++)
		{
			if (!myChesses[i].isEaten && myChesses[i].x == x && myChesses[i].y == y)
			{
				return myChesses[i];
			}
		}
	}
	else if (curFlag == (3 - me))
	{
		for (int i = 0; i < MAX_CHESS; i++)
		{
			if (!enemyChesses[i].isEaten && enemyChesses[i].x == x && enemyChesses[i].y == y)
			{
				return enemyChesses[i];
			}
		}
	}
}

//评估当前价值,均是相当于黑方来说,正数表示利于黑方
int getCurrentScore(const char board[BOARD_SIZE][BOARD_SIZE])
{
	int score = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			if (board[i][j] == WHITE_KING)
			{
				score -= WHITE_KING_SCORE[i][j];
			}
			else if (board[i][j] == WHITE_FLAG)
			{
				score -= WHITE_SCORE[i][j];
			}
			if (board[i][j] == BLACK_KING)
			{
				score += BLACK_KING_SCORE[i][j];
			}
			else if (board[i][j] == BLACK_FLAG)
			{
				score += BLACK_SCORE[i][j];
			}
			//}
		}
	}
	return score;
}

//根据命令返回我方价值
int getCommandScore(Command& command, const char curBoard[BOARD_SIZE][BOARD_SIZE], int curFlag)
{
	/* 初始化棋盘 */
	char nextBoard[BOARD_SIZE][BOARD_SIZE] = { 0 };
	memcpy(nextBoard, curBoard, sizeof(nextBoard));

	/* 虚拟放置 */
	place(command, nextBoard, true);

	/* 评估价值 */
	return getCurrentScore(nextBoard);
}

//储存合法走法
void saveLegalCommand(Command cmd, int level, int score = 0)
{
	//LegalCommand& lcmd = legalCommand[level];
	//int& num = legalCommand[level].count;
	//lcmd.chessID[num] = id;
	//lcmd.legalCommand[num] = cmd;
	//lcmd.score[num] = score;
	//num++;

	LegalCommand lcmd;
	lcmd.cmd = cmd;
	lcmd.score = score;
	legalCommands[level].push_back(lcmd);
}

//检测走法
vector< Command > searchStep(int x, int y, int& maxStep, bool& isJump, char board[BOARD_SIZE][BOARD_SIZE], int curFlag)
{
	/* 存储合法走法 */
	vector< Command > cmds;

	if (!(board[x][y] == curFlag || board[x][y] == curFlag + 2))
	{
		return cmds;
	}

	SingleCommand scmd = {};

	/* 尝试跳跃吃子,最后每个棋子的合法走法在scmd数组里面 */
	//tryToJump(x, y, 0, chess, board, curFlag);
	tryToJump(x, y, 0, board, curFlag, scmd);

	/* 如果还不能吃，但是这个棋子可以吃 或 这个棋子最长的走法大于所有棋子的走法,该走法设为最长 */
	if ((isJump == false && scmd.maxJumpStep >= 2) || scmd.maxJumpStep > maxStep)
	{
		maxStep = scmd.maxJumpStep;
		isJump = true;
		cout << "DEBUG: GROBLE max step= " << maxStep << " isJump= " << isJump << endl;

		for (int i = 0; i < scmd.cmds.size(); i++)
		{
			cmds.push_back(scmd.cmds[i]);
		}
		return cmds;
	}

	/* 可跳，返回搜索成功 */
	if (scmd.isJump == true && isJump == true && scmd.maxJumpStep >= maxStep)
	{
		for (int i = 0; i < scmd.cmds.size(); i++)
		{
			cmds.push_back(scmd.cmds[i]);
		}
		return cmds;
	}

	/* 如果无法跳跃，则开始移动 */
	if (isJump == false)
	{
		/* debug */
		//cout << "DEBUG: Can not jump, judge move" << endl;
		/* 清空结构体 */
		memset(&scmd, 0, sizeof(scmd));

		if (tryToMove(x, y, board, curFlag, scmd))
		{
			scmd.maxJumpStep = 2;
			maxStep = 2;

			for (int i = 0; i < scmd.cmds.size(); i++)
			{
				cmds.push_back(scmd.cmds[i]);
			}
			return cmds;
		}
	}

	return cmds;
}

//对每一己方合法走法进行搜索(暂时先搜一层）,返回收益id值,还应该返回收益值
int searchNextLevel(int level, int& thisLevelScore, int nodeId)
{
	level++;

	int curFlag;
	/* 第二层 */
	/* 奇数层为敌方搜索 偶数层（包括0）是我方搜索 */
	if (level % 2 == 0)
	{
		curFlag = me;
	}
	else
	{
		curFlag = (3 - me);
	}

	if (0)
	{
		cout << "DEBUG:\nDEBUG: Now Search LEVEL= " << level << endl;
	}

	/* 或许应该用Command来存储 */
	int bestStepID = 0;
	int bestScore;

	bool isJump = true;
	/* 对每一合法步骤判断 */
	//for (int i = 0; i < legalCommand[level].count; i++)
	for (int i = 0; i < legalCommands[level - 1].size(); i++)
	{
		clearLevelSearch(level);
		initNextBoard(level, nodeId);

		if (!SUBMIT_MODE && CMD_MODE)
		{
			cout << "DEBUG: Search cmd: PLACE " << legalCommands[level - 1][i].cmd.numStep;
			for (int j = 0; j < legalCommands[level - 1][i].cmd.numStep; j++)
			{
				cout << " " << legalCommands[level - 1][i].cmd.x[j] << "," << legalCommands[level - 1][i].cmd.y[j] << " ";
			}
			cout << endl;
		}

		/* 初始化 */
		initNextLevel(level, i, legalCommands[level - 1][i].cmd);

		Command cmd;
		bool isEnd = false;
		/* 下一步棋,存储合法步骤 */
		aiTurn(level, stepBoard[level][i], curFlag, cmd, isEnd);

		//if (!isEnd && i == 0)
		//{
		//	//bestScore = curScore;
		//	bestScore = legalCommands[level+1][0].score;
		//}

		if (!SUBMIT_MODE && CMD_MODE)
		{
			cout << "DEBUG: OPP cmd: PLACE " << cmd.numStep;
			for (int i = 0; i < cmd.numStep; i++)
			{
				cout << " " << cmd.x[i] << "," << cmd.y[i] << " ";
			}
			cout << endl;
		}

		int nextScore;
		/* 当该棋有下一步 且 层数未达最深层 */
		if (!isEnd && level != MAX_SEARCH_LEVEL - 1)
		{
			/* 继续往下搜索 */
			if (legalCommands[level].size() != 0)
			{
				searchNextLevel(level, nextScore, i);	//返回下一层的最优分数

				if (!SUBMIT_MODE)
				{
					cout << "DEBUG: level= " << level << " NextlevelScore " << nextScore << endl;
				}
				//cout << "DEBUG: NextLevelScore= " << nextScore << endl;

				/* 最大最小值算法，不知道有没有写反 */

				if (i == 0)
				{
					bestScore = nextScore;
					if (!SUBMIT_MODE)
					{
						cout << "DEBUG: level= " << level << " init1bestScore " << bestScore << endl;
					}
				}
				else if (me == BLACK_FLAG)	//黑棋最终要求最大
				{
					if (level % 2 == 0)	//偶数层分数最优为最小
					{
						bestScore = min(nextScore, bestScore);
						if (!SUBMIT_MODE)
						{
							cout << "DEBUG: level= " << level << " minbestScore " << bestScore << endl;
						}
					}
					else
					{
						bestScore = max(nextScore, bestScore);
						if (!SUBMIT_MODE)
						{
							cout << "DEBUG: level= " << level << " maxbestScore " << bestScore << endl;
						}
					}
				}
				else if (me == WHITE_FLAG)
				{
					if (level % 2 == 0)	//相反
					{
						bestScore = max(nextScore, bestScore);
						if (!SUBMIT_MODE)
						{
							cout << "DEBUG: level= " << level << " maxbestScore " << bestScore << endl;
						}
					}
					else
					{
						bestScore = min(nextScore, bestScore);
						if (!SUBMIT_MODE)
						{
							cout << "DEBUG: level= " << level << " minbestScore " << bestScore << endl;
						}
					}
				}

				if (nextScore == bestScore)
				{
					bestStepID = i;
				}
			}
			else
			{
				for (int k = 0; k < legalCommands[level - 1].size(); k++)
				{
					if (k == 0)
					{
						bestScore = legalCommands[level - 1][k].score;
					}
					else if (me == BLACK_FLAG)	//黑棋最终要求最大
					{
						if (level % 2 == 0)	//偶数层分数最优为最小
						{
							bestScore = min(legalCommands[level - 1][k].score, bestScore);
						}
						else
						{
							bestScore = max(legalCommands[level - 1][k].score, bestScore);
						}
					}
					else if (me == WHITE_FLAG)
					{
						if (level % 2 == 0)	//相反
						{
							bestScore = max(legalCommands[level - 1][k].score, bestScore);
						}
						else
						{
							bestScore = min(legalCommands[level - 1][k].score, bestScore);
						}
					}
				}
			}

		}
		/* 如果层数到达最深 */
		else if (!isEnd)
		{
			if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
			{
				for (int j = 0; j < legalCommands[level].size(); j++)
				{
					cout << "DEBUG: Judged level= " << level << ",flag= " << curFlag << ", this= " << j << " up= " << i << " score = " << legalCommands[level][j].score << endl;
				}
			}
			/* 否则返回该层分数的最优值 */

			/* i=0特判 */
			if (i == 0)
			{
				bestScore = legalCommands[level][0].score;
				if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
				{
					cout << "DEBUG: level= " << level << " init2bestScore " << bestScore << endl;
				}
				if (me == BLACK_FLAG)	//黑棋最终要求最大
				{
					if (level % 2 == 0)	//偶数层分数最优为最小
					{
						for (int j = 0; j < legalCommands[level].size(); j++)
						{
							bestScore = min(legalCommands[level][j].score, bestScore);
							if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
							{
								cout << "DEBUG: level= " << level << " min2bestScore " << bestScore << endl;
							}
						}
					}
					else
					{
						for (int j = 0; j < legalCommands[level].size(); j++)
						{
							bestScore = max(legalCommands[level][j].score, bestScore);
							if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
							{
								cout << "DEBUG: level= " << level << " max2bestScore " << bestScore << endl;
							}
						}
					}
				}
				else if (me == WHITE_FLAG)
				{
					if (level % 2 == 0)	//相反
					{
						for (int i = 0; i < legalCommands[level].size(); i++)
						{
							bestScore = max(legalCommands[level][i].score, bestScore);
							if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
							{
								cout << "DEBUG: level= " << level << " max2bestScore " << bestScore << endl;
							}
						}
					}
					else
					{
						for (int i = 0; i < legalCommands[level].size(); i++)
						{
							bestScore = min(legalCommands[level][i].score, bestScore);
							if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
							{
								cout << "DEBUG: level= " << level << " min2bestScore " << bestScore << endl;
							}
						}
					}
				}
			}

			/* 最大最小值算法，不知道有没有写反 */
			else if (me == BLACK_FLAG)	//黑棋最终要求最大
			{
				if (level % 2 == 0)	//偶数层分数最优为最小
				{
					for (int j = 0; j < legalCommands[level].size(); j++)
					{
						bestScore = min(legalCommands[level][j].score, bestScore);
						if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
						{
							cout << "DEBUG: level= " << level << " min2bestScore " << bestScore << endl;
						}
					}
				}
				else
				{
					for (int j = 0; j < legalCommands[level].size(); j++)
					{
						bestScore = max(legalCommands[level][j].score, bestScore);
						if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
						{
							cout << "DEBUG: level= " << level << " max2bestScore " << bestScore << endl;
						}
					}
				}
			}
			else if (me == WHITE_FLAG)
			{
				if (level % 2 == 0)	//相反
				{
					for (int i = 0; i < legalCommands[level].size(); i++)
					{
						bestScore = max(legalCommands[level][i].score, bestScore);
						if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
						{
							cout << "DEBUG: level= " << level << " max2bestScore " << bestScore << "id=" << i << endl;
						}
					}
				}
				else
				{
					for (int i = 0; i < legalCommands[level].size(); i++)
					{
						bestScore = min(legalCommands[level][i].score, bestScore);
						if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
						{
							cout << "DEBUG: level= " << level << " min2bestScore " << bestScore << "id=" << i << endl;
						}
					}
				}
			}

			if (!SUBMIT_MODE && level <= MAX_SHOW_LEVEL)
			{
				cout << "DEBUG: level= " << level << " bestScore " << bestScore << endl;
			}

			bestStepID = i;
		}
		/* 如果这步棋走不了 */
		else
		{
			bestScore = legalCommands[level - 1][0].score;
			bestStepID = 0;
		}
	}

	thisLevelScore = bestScore;
	return bestStepID;
}

void initNextBoard(int level, int id)
{
	for (int i = 0; i < MAX_CHESS; i++)
	{
		/* 复制一块棋盘 */
		//memcpy(&firstStepBoard[i], board, sizeof(board));
		if (level == 1)
		{
			memcpy(&stepBoard[level][i], board, sizeof(board));
		}
		else
		{
			memcpy(&stepBoard[level][i], &stepBoard[level - 1][id], sizeof(board));
		}
	}
	return;
}

//深层搜索初始化
void initNextLevel(int level, int id, Command cmd)
{
	if (0)
	{
		cout << "DEBUG: Start Init next level ,Num= " << id << endl;
	}
	/* 模拟下棋 */
	place(cmd, stepBoard[level][id], true, false);
	if (0)
	{
		cout << "DEBUG: After place board:\n";
		printBoard(stepBoard[level][id]);
	}
	///* 初始化下一手行动结构体 */
	//int numEnemyFlag = 0;
	//for (int i = 0; i < BOARD_SIZE; i++)
	//{
	//	for (int j = 0; j < BOARD_SIZE; j++)
	//	{
	//		/* 敌方棋子初始化 */
	//		//黑1白2 白王2，4 黑王1，3  me=1，3-me=2 need 2 or 4
	//		//						me=2，3-me=1 need 1 or 3
	//		if (board[i][j] != EMPTY && board[i][j] % 2 == (3 - me) % 2)	//暂时先这么判断
	//		{
	//			enemyChesses[numEnemyFlag].x = i;
	//			enemyChesses[numEnemyFlag].y = j;
	//			if (board[i][j] > 2)
	//			{
	//				enemyChesses[numEnemyFlag].isKing = true;
	//			}
	//			else
	//			{
	//				enemyChesses[numEnemyFlag].isKing = false;
	//			}
	//			enemyChesses[numEnemyFlag].isEaten = false;
	//			numEnemyFlag++;
	//		}
	//	}
	//}
	//clearChessesStep(2);

	//debug("Init Next Level Success");
	return;
}

//放置棋子，若是虚拟放置，则放置后不会进行结构体的移动,需要另行初始化
void place(struct Command cmd, char board[BOARD_SIZE][BOARD_SIZE], bool isVirtual, bool isEnemyTurn)
{
	/*debug*/
	if (0)
	{
		cout << "DEBUG: PLACE " << cmd.numStep;
		for (int i = 0; i < cmd.numStep; i++)
		{
			cout << " " << cmd.x[i] << "," << cmd.y[i] << " ";
		}
		cout << endl;
	}

	int midX, midY, curFlag;
	curFlag = board[cmd.x[0]][cmd.y[0]];

	for (int i = 0; i < cmd.numStep - 1; i++)
	{
		/* 移动棋子 */
		board[cmd.x[i]][cmd.y[i]] = EMPTY;
		board[cmd.x[i + 1]][cmd.y[i + 1]] = curFlag;
		//if (!isVirtual && !isEnemyTurn)
		//{
		//	Chess& curChess = findChess(cmd.x[i], cmd.y[i], curFlag);
		//	curChess.x = cmd.x[i + 1];
		//	curChess.y = cmd.y[i + 1];
		//}

		/* 如果是跳 */
		if (abs(cmd.x[i] - cmd.x[i + 1]) == 2)
		{
			midX = (cmd.x[i] + cmd.x[i + 1]) / 2;
			midY = (cmd.y[i] + cmd.y[i + 1]) / 2;
			/*
			* 白棋010 100，黑棋001 011，空000
			* &1	0 0	       1 1		0
			* 黑me-1==0,白me-1==1
			* 若我黑，则&1==0为敌人,me-1==0
			* 若我白，则&1==1为敌人,me-1==1
			*/
			//if (board[midX][midY] != EMPTY && (board[midX][midY] & 1) != (me - 1))
			//if (board[midX][midY] != EMPTY && !isVirtual && isEnemyTurn)
			//{
			//	/* 找到被吃的那个棋，吃掉 */
			//	int findCurFlag = curFlag;
			//	if (findCurFlag > 2)
			//	{
			//		findCurFlag = findCurFlag % 2;
			//	}
			//	Chess& curChess = findChess(midX, midY, 3 - findCurFlag);
			//	curChess.isEaten = true;
			//	cout << "DEBUG: Eat" << midX << "," << midY << endl;
			//}

			board[midX][midY] = EMPTY;
		}
	}

	/* 升王 */
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (board[0][i] == BLACK_FLAG)
		{
			board[0][i] = BLACK_KING;
			//if (!isEnemyTurn && !isVirtual)
			//{
			//	Chess& chess = findChess(0, i, BLACK_FLAG);
			//	chess.isKing = true;
			//}
		}
		if (board[BOARD_SIZE - 1][i] == WHITE_FLAG)
		{
			board[BOARD_SIZE - 1][i] = WHITE_KING;
			//if (!isEnemyTurn && !isVirtual)
			//{
			//	Chess& chess = findChess(BOARD_SIZE - 1, i, WHITE_FLAG);
			//	chess.isKing = true;
			//}
		}
	}

	return;
}

//初始化AI
void initAI()
{
	int numMyFlag = 0;
	int numEnemyFlag = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			/* 我方棋子初始化 */
			if (board[i][j] == me || board[i][j] == me + 2)
			{
				myChesses[numMyFlag].x = i;
				myChesses[numMyFlag].y = j;
				if (board[i][j] == me + 2)
				{
					myChesses[numMyFlag].isKing = true;
				}
				else
				{
					myChesses[numMyFlag].isKing = false;
				}
				myChesses[numMyFlag].isEaten = false;
				numMyFlag++;
			}
			/* 敌方棋子初始化 */
			if (board[i][j] == (3 - me) || board[i][j] == (3 - me) + 2)
			{
				enemyChesses[numEnemyFlag].x = i;
				enemyChesses[numEnemyFlag].y = j;
				if (board[i][j] == (3 - me) + 2)
				{
					enemyChesses[numEnemyFlag].isKing = true;
				}
				else
				{
					enemyChesses[numEnemyFlag].isKing = false;
				}
				enemyChesses[numEnemyFlag].isEaten = false;
				numEnemyFlag++;
			}
		}
	}
	for (int i = numMyFlag; i < MAX_CHESS; i++)
	{
		myChesses[i].isEaten = true;
	}
	for (int i = numEnemyFlag; i < MAX_CHESS; i++)
	{
		enemyChesses[i].isEaten = true;
	}
	clearChessesStep(0);

	//debug("Init Success");
}

/**
 * 轮到你落子。
 * 棋盘上0表示空白，1表示黑棋，2表示白棋
 * me表示你所代表的棋子(1或2)
 * 你需要返回一个结构体Command，其中numStep是你要移动的棋子经过的格子数（含起点、终点），
 * x、y分别是该棋子依次经过的每个格子的横、纵坐标
 */
 //int aiTurn(Chess chesses[], char board[BOARD_SIZE][BOARD_SIZE], int curFlag, Command& cmd)
int aiTurn(int level, char board[BOARD_SIZE][BOARD_SIZE], int curFlag, Command& cmd, bool& isEnd)
{
	struct Command command =
	{
		{0},
		{0},
		0,
		false
	};
	bool isJump = false;	//标记是否可以跳而不是移动
	int maxStep = 1;

	/* 清空上一次己方棋子存储的走法 */
	if (level == 0)
	{
		clearLevelSearch(level);
	}

	vector <Command> cmds;//存储未经筛选的每一个子的合法走法
	/* 对于棋盘上的每一个点进行走法搜寻,并返回所有针对每一个子的合法走法 */
	/* 先搜吃 */
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			/* 非棋子或非我方 */
			if (!(board[i][j] == curFlag || board[i][j] == curFlag + 2))
			{
				continue;
			}
			SingleCommand scmd = {};
			tryToJump(i, j, 0, board, curFlag, scmd);
			cmds.insert(cmds.end(), scmd.cmds.begin(), scmd.cmds.end());
		}
	}
	if (cmds.size() != 0)
	{
		isJump = true;
	}
	/* 再搜移动 */
	if (cmds.size() == 0)
	{
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				/* 非棋子或非我方 */
				if (!(board[i][j] == curFlag || board[i][j] == curFlag + 2))
				{
					continue;
				}
				SingleCommand scmd = {};
				if (tryToMove(i, j, board, curFlag, scmd))
				{
					cmds.insert(cmds.end(), scmd.cmds.begin(), scmd.cmds.end());
				}
			}
		}
	}

	/* 检索所有合法走法 */
	for (int i = 0; i < cmds.size(); i++)
	{
		if (cmds[i].numStep > maxStep)
		{
			maxStep = cmds[i].numStep;
		}
	}
	if (cmds.size() == 0)
	{
		isEnd = true;
	}
	for (int i = 0; i < cmds.size(); i++)
	{
		if (cmds[i].numStep == maxStep)
		{
			int score = getCommandScore(cmds[i], board, curFlag);
			saveLegalCommand(cmds[i], level, score);
		}
	}

	if (!SUBMIT_MODE && CMD_MODE)
	{
		for (int i = 0; i < cmds.size(); i++)
		{
			cout << "DEBUG: CMD " << i << " " << cmds[i].numStep;
			for (int j = 0; j < cmds[i].numStep; j++)
			{
				cout << ' ' << cmds[i].x[j] << "," << cmds[i].y[j];
			}
			cout << endl;
		}
	}

	/* 下棋前，对所有合法的走法，分析敌人收益 */
	if (level == 0)	//暂时先这么判断第0层
	{
		//LegalCommand& lcmd = legalCommand[0];
		int score;
		int id = searchNextLevel(level, score, 0);
		cout << "DEBUG: id=" << id << endl;
		//cout << "DEBUG: Search Done!\n";
		//memcpy(&command, &lcmd.legalCommand[id], sizeof(struct Command));
		memcpy(&command, &legalCommands[0][id].cmd, sizeof(struct Command));
		printBoard(board);
		if (!BOARD_MODE)
		{
			cout << "DEBUG: Before score: " << getCurrentScore(board) << endl;
		}

		/* 清空所有棋子走法 */
		clearChessesStep(0);

		cmd = command;
		return -1;
	}
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
	if (INIT_MODE)
	{
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				int a;
				cin >> a;
				board[i][j] = a;
			}
		}
	}
	else
	{
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
	}


	//initAI();
}

void specialTurn(char board[][BOARD_SIZE], int myFlag)
{
	;
}

void turn()
{
	struct Command command;
	specialTurn(board, me);
	bool isEnd = false;
	aiTurn(0, board, me, command, isEnd);
	place(command, board, false, false);
	//rotateCommand(&command);
	printf("%d", command.numStep);
	for (int i = 0; i < command.numStep; i++)
	{
		printf(" %d,%d", command.x[i], command.y[i]);
	}
	printf("\n");
	fflush(stdout);
	clearLevelSearch();
}

void end(int x)
{
	exit(0);
}

void loop()
{
	char tag[12] = { 0 };
	struct Command command =
	{
		{0},
		{0},
		0,
		false
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
			place(command, board, false, true);
		}
		else if (strcmp(tag, TURN) == 0)
		{
			DWORD timeStart = ::GetTickCount();
			turn();
			DWORD timeEnd = ::GetTickCount();
			cout << "DEBUG: Time cost " << timeEnd - timeStart << endl;
		}
		else if (strcmp(tag, END) == 0)
		{
			scanf("%d", &status);
			end(status);
		}
		printBoard(board);
		if (!BOARD_MODE)
		{
			cout << "DEBUG: Now score= " << getCurrentScore(board) << endl;

		}
	}

	return;
}

int main(int argc, char* argv[])
{
	srand(time(0));
	loop();
	return 0;
}
/*
function minimax(node, depth, a, b)
	if node is a terminal node or depth = 0
		return the heuristic value of node
	if #opposite
		foreach child of node
			b := min(a, minimax(child, depth-1, a, b))
			if b <= a
				  return b
		return b
	else we
		foreach child of node
			a := max(b, minimax(child, depth-1, a, b))
			if b <= a
				   return a
		 return a
*/

/* 每一个节点都是一种合法走法(cmd)，内部包含接下去的n种合法走法(vector lcmd)，还包含返回的值，和应走的走法 */

//最小最大值算法
//int minMax(node, int curDepth, int a, int b)
//{
//	if (node == MAXDEPTH || curDepth == 0)
//	{
//		return node_value
//	}
//	if (curDepth % 2 == 1)	//Min
//	{
//		for (child : childs_of_node)
//		{
//			b = std::min(a, minMax(child, curDepth - 1, a, b));
//			if (b <= a)
//			{
//				return b;
//			}
//		}
//		return b;
//	}
//	else
//	{
//		for (child : childs_of_node)
//		{
//			a = std::max(a, minMax(child, curDepth - 1, a, b));
//			if (b <= a)
//			{
//				return a;
//			}
//		}
//		return a;
//	}
//}

/* level 0 找level 1 中的最大值
*		1			2	最小值
*/