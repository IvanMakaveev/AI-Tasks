// Ivan Makaveev, 2MI0600203
// Solution 1 - Simple DFS solution with code reuse
#include <iostream>
#include <chrono>

const char LEFT = '<';
const char RIGHT = '>';
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

bool isGoalState(const char* board, size_t boardSize)
{
    for (size_t i = 0; i < boardSize; i++)
    {
        if (board[i] != RIGHT || board[2 * boardSize - i] != LEFT)
        {
            return false;
        }
    }

    return board[boardSize] == SPACE;
}

bool performStep(char* board, size_t boardSize, int slabPosition);

bool nextStep(char* board, size_t boardSize, int slabPosition, int offset)
{
    std::swap(board[slabPosition + offset], board[slabPosition]);
    bool isSolved = performStep(board, boardSize, slabPosition + offset);
    std::swap(board[slabPosition + offset], board[slabPosition]);

    if (isSolved)
    {
        print(board);
        return true;
    }

    return false;
}

bool performStep(char* board, size_t boardSize, int slabPosition)
{
    if (isGoalState(board, boardSize / 2))
    {
        print(board);
        return true;
    }

    if (slabPosition - 2 >= 0 && board[slabPosition - 2] == LEFT && board[slabPosition - 1] != LEFT)
    {
        if (nextStep(board, boardSize, slabPosition, -2))
        {
            return true;
        }
    }

    if (slabPosition + 2 < boardSize && board[slabPosition + 2] == RIGHT && board[slabPosition + 1] != RIGHT)
    {
        if (nextStep(board, boardSize, slabPosition, 2))
        {
            return true;
        }
    }

    if (slabPosition - 1 >= 0 && board[slabPosition - 1] == LEFT)
    {
        if(nextStep(board, boardSize, slabPosition, -1))
        {
            return true;
        }
    }

    if (slabPosition + 1 < boardSize && board[slabPosition + 1] == RIGHT)
    {
        if(nextStep(board, boardSize, slabPosition, 1))
        {
            return true;
        }
    }

    return false;
}

void findPath(char* board, size_t boardSize)
{
    performStep(board, 2 * boardSize + 1, boardSize);
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