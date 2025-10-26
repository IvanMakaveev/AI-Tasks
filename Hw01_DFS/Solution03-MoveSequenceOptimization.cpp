// Ivan Makaveev, 2MI0600203
// Solution 3 - Rule-based solution by simulating only the exact moves
#include <iostream>
#include <chrono>

const char RIGHT = '<';
const char LEFT = '>';
const char SPACE = '_';

void fillBoard(char* board, size_t boardSize)
{
    for (size_t i = 0; i < boardSize; i++)
    {
        board[i] = LEFT;
        board[2 * boardSize - i] = RIGHT;
    }

    board[boardSize] = SPACE;
    board[boardSize * 2 + 1] = '\0';
}

void print(char* board)
{
    std::cout << board << std::endl;
}

void smallLeftStep(char* board, size_t boardSize, size_t& slabPosition)
{
    std::swap(board[slabPosition - 1], board[slabPosition]);
    print(board);
    slabPosition -= 1;
}

void smallRightStep(char* board, size_t boardSize, size_t& slabPosition)
{
    std::swap(board[slabPosition + 1], board[slabPosition]);
    print(board);
    slabPosition += 1;
}

void bigLeftStep(char* board, size_t boardSize, size_t& slabPosition)
{
    std::swap(board[slabPosition - 2], board[slabPosition]);
    print(board);
    slabPosition -= 2;
}

void bigRightStep(char* board, size_t boardSize, size_t& slabPosition)
{
    std::swap(board[slabPosition + 2], board[slabPosition]);
    print(board);
    slabPosition += 2;
}

void performRightStep(char* board, size_t boardSize, size_t& slabPosition, size_t moveCount, bool reverse)
{
    if (reverse)
    {
        smallRightStep(board, boardSize, slabPosition);
    }

    for (size_t i = 1; i < moveCount; i++)
    {
        bigRightStep(board, boardSize, slabPosition);
    }

    if (!reverse)
    {
        smallRightStep(board, boardSize, slabPosition);
    }
}

void performLeftStep(char* board, size_t boardSize, size_t& slabPosition, size_t moveCount, bool reverse)
{
    if (reverse)
    {
        smallLeftStep(board, boardSize, slabPosition);
    }

    for (size_t i = 1; i < moveCount; i++)
    {
        bigLeftStep(board, boardSize, slabPosition);
    }

    if (!reverse)
    {
        smallLeftStep(board, boardSize, slabPosition);
    }
}

void performMiddleStep(char* board, size_t boardSize, size_t& slabPosition, size_t moveCount, bool isLeft)
{
    if (isLeft)
    {
        for (size_t i = 0; i < moveCount; i++)
        {
            bigLeftStep(board, boardSize, slabPosition);
        }
    }
    else
    {
        for (size_t i = 0; i < moveCount; i++)
        {
            bigRightStep(board, boardSize, slabPosition);
        }
    }
}

void findPath(char* board, size_t boardSize)
{
    print(board);

    bool isNextMoveLeft = true;
    size_t slabPosition = boardSize;
    for (size_t i = 1; i <= boardSize; i++)
    {
        if (isNextMoveLeft)
        {
            performLeftStep(board, 2 * boardSize + 1, slabPosition, i, false);
        }
        else
        {
            performRightStep(board, 2 * boardSize + 1, slabPosition, i, false);
        }
        isNextMoveLeft = !isNextMoveLeft;
    }

    performMiddleStep(board, 2 * boardSize + 1, slabPosition, boardSize, isNextMoveLeft);
    isNextMoveLeft = !isNextMoveLeft;

    for (size_t i = boardSize; i >= 1; i--)
    {
        if (isNextMoveLeft)
        {
            performLeftStep(board, 2 * boardSize + 1, slabPosition, i, true);
        }
        else
        {
            performRightStep(board, 2 * boardSize + 1, slabPosition, i, true);
        }
        isNextMoveLeft = !isNextMoveLeft;
    }
}

int main()
{
    size_t boardSize = 0;
    std::cin >> boardSize;

    auto start = std::chrono::high_resolution_clock::now();

    char* board = new char[boardSize * 2 + 2];
    fillBoard(board, boardSize);
    findPath(board, boardSize);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    // std::cout << "Time to compute and print: " << duration.count() << " seconds";
}