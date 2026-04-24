#pragma once

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <mutex>
using namespace std;

const int BOARD_SIZE = 15;

const string EMPTY = "empty";
const string AI_PLAYER = "white";
const string HUMAN_PLAYER = "black";

class AiGame{

public:
    AiGame(int userId);

    // 判断是否平局
    bool isDraw() const{
        lock_guard<mutex> lock(mutex_);
        return moveCount_ >= BOARD_SIZE * BOARD_SIZE;
    }

    bool humanMove(int x, int y);

    bool checkWin(int x, int y, const string& player);

    void aiMove();

    // 获取最后一步移动的坐标
    pair<int, int> getLastMove() const{
        lock_guard<mutex> lock(mutex_);
        return lastMove_;
    }

    const vector<vector<string>> getBoard() const{
        lock_guard<mutex> lock(mutex_);
        return board_;
    }

    bool isGameOver() const{
        lock_guard<mutex> lock(mutex_);
        return gameOver_;
    }

    string getWinner() const{
        lock_guard<mutex> lock(mutex_);
        return winner_;
    }

private:
    // 检查移动是否有效
    bool isValidMove(int x, int y) const{
        if(x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return false;
        if(board_[x][y] != EMPTY) return false;
        if(gameOver_ || isDraw()) return false;
        return true;
    }

    // 检查坐标是否在棋盘内
    bool isInBoard(int x, int y) const {
        return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
    }

    // 获取ai的最佳移动位置
    pair<int, int> getBestMove();

    // 评估威胁
    int evaluateThreat(int r, int c);
    // 判断某个空位是否靠近已有棋子
    bool isNearOccupied(int r, int c);


private:
    bool        gameOver_;
    int         userId_;
    int         moveCount_;
    string      winner_{"none"};
    pair<int, int>   lastMove_{-1, -1};  // 上一次落子位置
    vector<vector<string>>          board_;
    mutable mutex     mutex_;

};