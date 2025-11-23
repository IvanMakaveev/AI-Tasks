// Ivan Makaveev, 2MI0600203
#include <iostream>
#include <vector>
#include <string>

class TicTacToe
{
	static const size_t BOARD_SIZE = 3;
	static const char EMPTY = '_';
	static const char DEFAULT_START = 'X';
	static const char DEFAULT_SECOND = 'O';

	char playerSymbol = DEFAULT_START;
	char computerSymbol = DEFAULT_SECOND;

	std::vector<char> board;
	size_t movesLeft = BOARD_SIZE * BOARD_SIZE;

	bool isValidSymbol(char sym) const
	{
		return sym == DEFAULT_START || sym == DEFAULT_SECOND;
	}

	bool isValidBoardSymbol(char sym) const
	{
		return isValidSymbol(sym) || sym == EMPTY;
	}

	char readStartingSymbol() const
	{
		std::string str;
		std::cin >> str;
		if (str == "FIRST")
		{
			char startSymbol;
			std::cin >> startSymbol;

			if (isValidSymbol(startSymbol))
				return startSymbol;
		}

		return DEFAULT_START;
	}

	void readTurnSymbol()
	{
		std::string str;
		std::cin >> str;
		if (str == "TURN")
		{
			char symbol;
			std::cin >> symbol;

			if (!isValidSymbol(symbol))
				return;

			computerSymbol = symbol;
			playerSymbol = symbol == DEFAULT_START ? DEFAULT_SECOND : DEFAULT_START;
		}
	}

	void readPlayerSymbol()
	{
		std::string str;
		std::cin >> str;
		if (str == "HUMAN")
		{
			char symbol;
			std::cin >> symbol;

			if (!isValidSymbol(symbol))
				return;

			playerSymbol = symbol;
			computerSymbol = symbol == DEFAULT_START ? DEFAULT_SECOND : DEFAULT_START;
		}
	}

	void readBoard()
	{
		std::cin.ignore();
		board.clear();
		
		std::string line;
		size_t linesToRead = BOARD_SIZE * 2 + 1;
		movesLeft = BOARD_SIZE * BOARD_SIZE;

		while (linesToRead--)
		{
			std::getline(std::cin, line);
			if (line.size() > 0 && line[0] == '|')
			{
				for (int i = 0; i < line.size(); i++)
				{
					if (isValidBoardSymbol(line[i]))
					{
						board.push_back(line[i]);
						if (isValidSymbol(line[i]))
							movesLeft--;
					}
				}
			}
		}
	}

	char getSymbolAt(size_t x, size_t y) const
	{
		return board[x * BOARD_SIZE + y];
	}

	char& getSymbolAt(size_t x, size_t y)
	{
		return board[x * BOARD_SIZE + y];
	}

	bool checkLineWinner(const std::vector<char>& line) const
	{
		char first = line[0];
		if (first == EMPTY)
			return false;

		for (const auto& sym : line)
			if (sym != first)
				return false;

		return true;
	}

	char getWinner() const
	{
		for (size_t x = 0; x < BOARD_SIZE; x++)
		{
			std::vector<char> row;
			std::vector<char> col;
			for (size_t y = 0; y < BOARD_SIZE; y++)
			{
				row.push_back(getSymbolAt(x, y));
				col.push_back(getSymbolAt(y, x));
			}

			if (checkLineWinner(row))
				return row.front();
			if (checkLineWinner(col))
				return col.front();
		}

		std::vector<char> diag;
		std::vector<char> secDiag;
		for (size_t i = 0; i < BOARD_SIZE; i++)
		{
			diag.push_back(getSymbolAt(i, i));
			secDiag.push_back(getSymbolAt(i, BOARD_SIZE - i - 1));
		}

		if (checkLineWinner(diag))
			return diag.front();
		if (checkLineWinner(secDiag))
			return secDiag.front();

		return EMPTY;
	}

	bool isGameTerminated() const
	{
		return getWinner() != EMPTY || movesLeft == 0;
	}

	void printBorder() const
	{
		for (size_t i = 0; i < BOARD_SIZE; i++)
		{
			std::cout << "+---";
		}
		std::cout << '+' << std::endl;
	}

	void printRow(size_t row) const 
	{
		for (size_t i = 0; i < BOARD_SIZE; i++)
		{
			std::cout << "| ";
			std::cout << getSymbolAt(row, i);
			std::cout << ' ';
		}

		std::cout << '|' << std::endl;
	}

	void printBoard() const
	{
		size_t rowsToPrint = BOARD_SIZE * 2 + 1;
		for (size_t i = 0; i < rowsToPrint; i++)
		{
			if (i % 2 == 0)
				printBorder();
			else
				printRow(i / 2);
		}
	}

	void printGameResult() const
	{
		char winner = getWinner();
		if (winner == EMPTY)
			std::cout << "DRAW" << std::endl;
		else
			std::cout << "WINNER: " << winner << std::endl;
	}

	bool isValidPosition(size_t row, size_t col)
	{
		return row < BOARD_SIZE&& col < BOARD_SIZE;
	}

	void readPlayerTurn()
	{
		size_t row, col;
		do
		{
			std::cin >> row >> col;
			row--;
			col--;
		} while (!isValidPosition(row, col) || getSymbolAt(row, col) != EMPTY);

		getSymbolAt(row, col) = playerSymbol;
	}

	std::vector<char*> getPossibleMoves()
	{
		std::vector<char*> moves;
		for (size_t i = 0; i < board.size(); i++)
		{
			if (board[i] == EMPTY)
				moves.push_back(&board[i]);
		}

		return moves;
	}

	int evaluatePosition(char winner, size_t depth)
	{
		if (winner == computerSymbol)
			return 10 - depth;
		
		return ((int)depth) - 10;
	}

	int maximizer(int alpha, int beta, size_t depth)
	{
		char winner = getWinner();
		if (winner != EMPTY)
			return evaluatePosition(winner, depth);

		auto possibleMoves = getPossibleMoves();
		if (possibleMoves.size() == 0)
			return 0;

		int value = INT_MIN;
		for (auto& move : possibleMoves)
		{
			(*move) = computerSymbol;
			int moveScore = minimizer(alpha, beta, depth + 1);
			(*move) = EMPTY;

			value = std::max(moveScore, value);

			if (value >= beta)
				break;

			alpha = std::max(value, alpha);
		}

		return value;
	}

	int minimizer(int alpha, int beta, size_t depth)
	{
		char winner = getWinner();
		if (winner != EMPTY)
			return evaluatePosition(winner, depth);

		auto possibleMoves = getPossibleMoves();
		if (possibleMoves.size() == 0)
			return 0;

		int value = INT_MAX;
		for (auto& move: possibleMoves)
		{
			(*move) = playerSymbol;
			int moveScore = maximizer(alpha, beta, depth + 1);
			(*move) = EMPTY;

			value = std::min(moveScore, value);

			if (alpha >= value)
				break;

			beta = std::min(value, beta);
		}

		return value;
	}

	char& findBestMove()
	{
		int bestScore = INT_MIN;
		char* resultMove = nullptr;
		for (auto& move : getPossibleMoves())
		{
			(*move) = computerSymbol;
			int moveScore = minimizer(bestScore, INT_MAX, (10 - movesLeft));
			(*move) = EMPTY;

			if (moveScore > bestScore)
			{
				bestScore = moveScore;
				resultMove = move;
			}
		}

		return *resultMove;
	}

	std::vector<std::pair<int, int>> getPossibleMovesCoords() const
	{
		std::vector<std::pair<int, int>> moves;
		for (size_t i = 0; i < BOARD_SIZE; i++)
		{
			for (size_t j = 0; j < BOARD_SIZE; j++)
			{
				if (getSymbolAt(i, j) == EMPTY)
					moves.push_back({ i, j });
			}
		}

		return moves;
	}

	std::pair<int, int> findBestMovePosition()
	{
		std::pair<int, int> coords{ -1, -1 };

		int bestScore = INT_MIN;
		auto moves = getPossibleMovesCoords();

		if (moves.size() == 0 || getWinner() != EMPTY)
			return coords;

		for (const auto& moveCoords : getPossibleMovesCoords())
		{
			getSymbolAt(moveCoords.first, moveCoords.second) = computerSymbol;
			int moveScore = minimizer(bestScore, INT_MAX, (10 - movesLeft));
			getSymbolAt(moveCoords.first, moveCoords.second) = EMPTY;

			if (moveScore > bestScore)
			{
				bestScore = moveScore;
				coords = moveCoords;
			}
		}

		return coords;
	}

	void computeNextMove()
	{
		findBestMove() = computerSymbol;
	}

public:
	void startGame()
	{
		char startSymbol = readStartingSymbol();
		readPlayerSymbol();
		readBoard();

		bool isPlayerTurn = playerSymbol == startSymbol;
		while (!isGameTerminated())
		{
			if (isPlayerTurn)
				readPlayerTurn();
			else
				computeNextMove();

			printBoard();
			isPlayerTurn = !isPlayerTurn;
			movesLeft--;
		}

		printGameResult();
	}

	void startJudge()
	{
		readTurnSymbol();
		readBoard();
		
		auto bestMove = findBestMovePosition();
		if (bestMove.first == -1)
			std::cout << -1 << std::endl;
		else
			std::cout << bestMove.first + 1 << ' ' << bestMove.second + 1 << std::endl;
	}
};

int main()
{
	TicTacToe game;

	std::string gameMode;
	std::cin >> gameMode;

	if (gameMode == "GAME")
		game.startGame();
	else if (gameMode == "JUDGE")
		game.startJudge();

	return 0;
}
