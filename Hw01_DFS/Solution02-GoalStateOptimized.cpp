// Ivan Makaveev, 2MI0600203
// Solution 2 - Optimized goal state and lowered number of recursive function calls
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

bool performStep(char* board, size_t boardSize, int slabPosition, size_t depth, size_t goalDepth)
{
    if (depth == goalDepth)
    {
        print(board);
        return true;
    }

    if (slabPosition - 2 >= 0 && board[slabPosition - 2] == LEFT && board[slabPosition - 1] != LEFT)
    {
        std::swap(board[slabPosition - 2], board[slabPosition]);
        bool isSolved = performStep(board, boardSize, slabPosition - 2, depth + 1, goalDepth);
        std::swap(board[slabPosition - 2], board[slabPosition]);
    
        if (isSolved)
        {
            print(board);
            return true;
        }
    }

    if (slabPosition + 2 < boardSize && board[slabPosition + 2] == RIGHT && board[slabPosition + 1] != RIGHT)
    {
        std::swap(board[slabPosition + 2], board[slabPosition]);
        bool isSolved = performStep(board, boardSize, slabPosition + 2, depth + 1, goalDepth);
        std::swap(board[slabPosition + 2], board[slabPosition]);

        if (isSolved)
        {
            print(board);
            return true;
        }
    }

    if (slabPosition - 1 >= 0 && board[slabPosition - 1] == LEFT)
    {
        std::swap(board[slabPosition - 1], board[slabPosition]);
        bool isSolved = performStep(board, boardSize, slabPosition - 1, depth + 1, goalDepth);
        std::swap(board[slabPosition - 1], board[slabPosition]);
    
        if (isSolved)
        {
            print(board);
            return true;
        }
    }

    if (slabPosition + 1 < boardSize && board[slabPosition + 1] == RIGHT)
    {
        std::swap(board[slabPosition + 1], board[slabPosition]);
        bool isSolved = performStep(board, boardSize, slabPosition + 1, depth + 1, goalDepth);
        std::swap(board[slabPosition + 1], board[slabPosition]);
    
        if (isSolved)
        {
            print(board);
            return true;
        }
    }

    return false;
}

void findPath(char* board, size_t boardSize)
{
    performStep(board, 2 * boardSize + 1, boardSize, 1, (boardSize + 1) * (boardSize + 1));
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
