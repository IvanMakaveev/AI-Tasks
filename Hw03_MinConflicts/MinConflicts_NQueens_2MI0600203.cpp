// Ivan Makaveev, 2MI0600203
#include <iostream>
#include <chrono>
#include <vector>
#include <random>

class NQueensSolver
{
	bool isInitialized = false;
	std::vector<unsigned> board;
	std::vector<unsigned> rowQueens;
	std::vector<unsigned> mainDiagQueens;
	std::vector<unsigned> secDiagQueens;

	unsigned getMainDiag(unsigned col, unsigned row)
	{
		return row - col + board.size() - 1;
	}

	unsigned getSecDiag(unsigned col, unsigned row)
	{
		return row + col;
	}

	void setConflictData(unsigned col, unsigned row, int stateChange)
	{
		rowQueens[row] += stateChange;
		mainDiagQueens[getMainDiag(col, row)] += stateChange;
		secDiagQueens[getSecDiag(col, row)] += stateChange;
	}

	void addColQueen(unsigned col, unsigned row)
	{
		board[col] = row;
		setConflictData(col, row, 1);
	}

	void removeColQueen(unsigned col)
	{
		setConflictData(col, board[col], -1);
	}

	unsigned getRandomIndexFromBehind(size_t collectionSize, size_t elementsCount)
	{
		return (collectionSize - 1) - std::rand() % elementsCount;
	}

	unsigned getConflictsCount(unsigned col, unsigned row)
	{
		return rowQueens[row] + mainDiagQueens[getMainDiag(col, row)] + secDiagQueens[getSecDiag(col, row)];
	}

	unsigned getMinConflictsRow(unsigned col, bool skipCurrent = false)
	{
		unsigned minConflicts = INT_MAX;

		size_t candidatesCount = 0;
		std::vector<unsigned> rowCandidates;
		rowCandidates.reserve(board.size());

		for (size_t row = 0; row < board.size(); row++)
		{
			if (row == board[col])
			{
				continue;
			}

			unsigned conflictsCount = getConflictsCount(col, row);
			if (minConflicts >= conflictsCount)
			{
				if (minConflicts > conflictsCount)
				{
					candidatesCount = 0;
					minConflicts = conflictsCount;
				}

				candidatesCount++;
				rowCandidates.push_back(row);
			}
		}

		//if (skipCurrent)
		//{
		//	// If the min-conflict column can only be moved at the current position only - we take the 2nd most min-conflict
		//	if (candidatesCount == 1 && rowCandidates[rowCandidates.size() - 1] == board[col])
		//		return rowCandidates[rowCandidates.size() - 2];

		//	unsigned result;
		//	do
		//	{
		//		result = rowCandidates[getRandomIndexFromBehind(rowCandidates.size(), candidatesCount)];
		//	} while (skipCurrent && result == board[col]);

		//	return result;
		//}

		return rowCandidates[getRandomIndexFromBehind(rowCandidates.size(), candidatesCount)];
	}

	void initializeBoard()
	{
		for (size_t col = 0; col < board.size(); col++)
		{
			unsigned row = getMinConflictsRow(col);
			addColQueen(col, row);
		}

		isInitialized = true;
	}

	bool hasConflicts()
	{
		for (size_t col = 0; col < board.size(); col++)
		{
			if (getConflictsCount(col, board[col]) != 3)
				return true;
		}

		return false;
	}

	unsigned getMaxConflictsCol()
	{
		unsigned maxConflicts = 0;

		size_t candidatesCount = 0;
		std::vector<unsigned> colCandidates;
		colCandidates.reserve(board.size());

		for (size_t col = 0; col < board.size(); col++)
		{
			unsigned conflictsCount = getConflictsCount(col, board[col]);
			if (maxConflicts <= conflictsCount)
			{
				if (maxConflicts < conflictsCount)
				{
					candidatesCount = 0;
					maxConflicts = conflictsCount;
				}

				candidatesCount++;
				colCandidates.push_back(col);
			}
		}

		return colCandidates[getRandomIndexFromBehind(colCandidates.size(), candidatesCount)];
	}

	void resolveConflicts()
	{
		unsigned targetCol = getMaxConflictsCol();
		unsigned targetRow = getMinConflictsRow(targetCol, true);

		removeColQueen(targetCol);
		addColQueen(targetCol, targetRow);
	}

	bool solve()
	{
		if (board.size() <= 1)
			return true;

		if (board.size() <= 3)
			return false;

		initializeBoard();

		while (hasConflicts())
		{
			resolveConflicts();
		}

		return true;
	}

	void printSolution(bool printFullBoard)
	{
		if (printFullBoard)
		{
			for (int row = 0; row < board.size(); row++)
			{
				for (int col = 0; col < board.size(); col++)
				{
					if (board[col] == row)
						std::cout << "* ";
					else
						std::cout << "_ ";
				}
				std::cout << std::endl;
			}
		}
		else
		{
			for (const auto& queen : board)
				std::cout << queen << ' ';
		}
	}

public:
	NQueensSolver(size_t boardSize)
		: board(boardSize, 0), rowQueens(boardSize, 0), mainDiagQueens(2 * boardSize - 1, 0), secDiagQueens(2 * boardSize - 1, 0)
	{ }

	void printSolve(bool printResult = true, bool fullSolution = false)
	{
		if (isInitialized)
		{
			return;
		}

		auto start = std::chrono::high_resolution_clock::now();
		bool hasSolution = solve();
		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> duration = end - start;
		std::cout << "# TIMES_MS: alg=" << duration.count() << std::endl;

		if (!printResult)
		{
			return;
		}

		if (!hasSolution)
		{
			std::cout << -1 << std::endl;
		}
		else
		{
			printSolution(fullSolution);
		}
	}

	void reset()
	{
		this->operator=(NQueensSolver(board.size()));
	}
};

int main()
{
	size_t n;
	std::cin >> n;

	NQueensSolver solver(n);
	solver.printSolve();
}
