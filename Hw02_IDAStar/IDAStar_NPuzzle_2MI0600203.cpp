// Ivan Makaveev, 2MI0600203
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

struct BoardState
{
	size_t rows;
	unsigned emptyTilePos;
	unsigned emptyTileTarget;
	std::vector<unsigned>& board;

	unsigned pathCost = 0;
	unsigned heuristicCost = 0;

	BoardState(std::vector<unsigned>& board,
		size_t rows,
		unsigned emptyTilePos,
		unsigned emptyTileTarget,
		unsigned pathCost = 0)
		: board(board), rows(rows), emptyTilePos(emptyTilePos), emptyTileTarget(emptyTileTarget), pathCost(pathCost)
	{ }

	int getManhattanDistance(unsigned index)
	{
		unsigned tileVal = board[index];
		unsigned targetPos = (emptyTileTarget > tileVal - 1) ? tileVal - 1 : tileVal;

		int rowDist = getTileRow(index) - getTileRow(targetPos);
		int colDist = getTileCol(index) - getTileCol(targetPos);
		return std::abs(rowDist) + std::abs(colDist);
	}

	void calculateHeuristic()
	{
		heuristicCost = 0;
		for (int i = 0; i < board.size(); i++)
		{
			if (board[i] == 0)
				continue;

			heuristicCost += getManhattanDistance(i);
		}
	}

	bool makeMove(const std::pair<int, int>& move)
	{
		unsigned nextTileRow = getEmptyTileRow() + move.first;
		unsigned nextTileCol = getEmptyTileCol() + move.second;
		if (!isValidTile(nextTileRow, nextTileCol))
			return false;

		unsigned nextTileIndex = getTileIndex(nextTileRow, nextTileCol);
		unsigned prevDistIndex = emptyTilePos;
		int oldDist = getManhattanDistance(nextTileIndex);

		std::swap(board[emptyTilePos], board[nextTileIndex]);
		emptyTilePos = nextTileIndex;
		pathCost++;

		int newDist = getManhattanDistance(prevDistIndex);
		heuristicCost += newDist - oldDist;
		return true;
	}

	void undoMove(const std::pair<int, int>& move)
	{
		unsigned prevTileRow = getEmptyTileRow() - move.first;
		unsigned prevTileCol = getEmptyTileCol() - move.second;

		unsigned prevTileIndex = getTileIndex(prevTileRow, prevTileCol);
		unsigned prevDistIndex = emptyTilePos;
		int newDist = getManhattanDistance(prevTileIndex);

		std::swap(board[emptyTilePos], board[prevTileIndex]);
		emptyTilePos = prevTileIndex;
		pathCost--;

		int oldDist = getManhattanDistance(prevDistIndex);
		heuristicCost += oldDist - newDist;
	}

	unsigned getTotalCost() const
	{
		return pathCost + heuristicCost;
	}

	unsigned getTileRow(unsigned index) const
	{
		return index / rows;
	}

	unsigned getTileCol(unsigned index) const
	{
		return index % rows;
	}

	unsigned getEmptyTileRow() const
	{
		return getTileRow(emptyTilePos);
	}

	unsigned getEmptyTileCol() const
	{
		return getTileCol(emptyTilePos);
	}

	bool isValidTile(unsigned x, unsigned y) const
	{
		return x < rows&& y < rows;
	}

	unsigned getTileIndex(unsigned x, unsigned y) const
	{
		return x * rows + y;
	}
};

void initBoard(size_t n, std::vector<unsigned>& board, unsigned& emptyTilePos)
{
	board.reserve(n + 1);

	unsigned temp;
	for (unsigned i = 0; i <= n; i++)
	{
		std::cin >> temp;
		board.push_back(temp);
		if (temp == 0)
			emptyTilePos = i;
	}
}

unsigned getInversions(const std::vector<unsigned>& board)
{
	unsigned inversionsCount = 0;

	for (unsigned i = 0; i < board.size() - 1; i++)
	{
		if (board[i] == 0)
			continue;

		for (unsigned j = i + 1; j < board.size(); j++)
		{
			if (board[j] && board[i] > board[j])
				inversionsCount++;
		}
	}
	return inversionsCount;
}

bool hasSolution(const std::vector<unsigned>& board, size_t rows, unsigned emptyTilePos)
{
	unsigned inversionsCount = getInversions(board);

	if (rows & 1)
		return !(inversionsCount & 1);

	unsigned emptyTileRowReversed = rows - (emptyTilePos / rows);
	if (emptyTileRowReversed & 1)
		return !(inversionsCount & 1);

	return (inversionsCount & 1);
}

unsigned idaStar(BoardState& state, unsigned threshold, const std::pair<int, int>& prevMove, std::vector<std::string>& path)
{
	static const std::vector<std::pair<std::string, std::pair<int, int>>> MOVES
		= { {"right", {0, -1}}, {"left", {0, 1}}, {"up", {1, 0}}, {"down", {-1, 0}} };

	if (state.getTotalCost() > threshold)
		return state.getTotalCost();

	if (state.heuristicCost == 0)
		return 0;

	unsigned minSuccThreshold = UINT_MAX;

	for (const auto& moveData : MOVES)
	{
		auto& move = moveData.second;
		if ((prevMove.first == -move.first && prevMove.second == move.second && prevMove.second == 0)
			|| (prevMove.first == move.first && prevMove.first == 0 && prevMove.second == -move.second))
			continue;

		if (!state.makeMove(move))
			continue;
		path.push_back(moveData.first);

		unsigned nextThreshold = idaStar(state, threshold, move, path);
		if (nextThreshold == 0)
			return 0;

		minSuccThreshold = std::min(minSuccThreshold, nextThreshold);

		path.pop_back();
		state.undoMove(move);
	}

	return minSuccThreshold;
}

void printSolution(std::vector<std::string>& pathResult)
{
	std::cout << pathResult.size() << std::endl;
	for (const auto& move : pathResult)
		std::cout << move << std::endl;
}

bool solvePuzzle(std::vector<unsigned>& board, size_t rows, unsigned emptyTilePos, unsigned emptyTileTarget, std::vector<std::string>& result)
{
	if (!hasSolution(board, rows, emptyTilePos))
		return false;

	BoardState root(board, rows, emptyTilePos, emptyTileTarget);
	root.calculateHeuristic();

	int threshold = root.getTotalCost();
	if (threshold == 0)
		return true;

	do
	{
		threshold = idaStar(root, threshold, { 0, 0 }, result);
	} while (threshold != 0);

	return true;
}

int main()
{
	size_t n;
	int emptyTileTarget;
	std::cin >> n >> emptyTileTarget;

	size_t rows = std::sqrt(n + 1);
	if (emptyTileTarget == -1)
		emptyTileTarget = n;

	unsigned emptyTilePos;
	std::vector<unsigned> board;
	initBoard(n, board, emptyTilePos);

	std::vector<std::string> pathResult;

	auto start = std::chrono::high_resolution_clock::now();
	bool isSolved = solvePuzzle(board, rows, emptyTilePos, emptyTileTarget, pathResult);
	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> duration = end - start;
	std::cout << "# TIMES_MS: alg=" << duration.count() << std::endl;

	if (!isSolved)
	{
		std::cout << -1 << std::endl;
	}
	else
	{
		printSolution(pathResult);
	}
}