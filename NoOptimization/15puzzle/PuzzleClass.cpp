#include "PuzzleClass.h"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <map>
#include <random>

puzzle_class::puzzle_class()
{
	this->_size = 0;
	this->_score = 0;

};

puzzle_class::puzzle_class(const int size)
{
	if (size < 2)
	{
		throw std::invalid_argument("Size of the puzzle must be bigger than 1\n");
	}
	this->_size = size;
	this->_score = 0;
	this->_board.resize(size);
	for (int i = 0; i < size; i++)
	{
		this->_board[i].resize(size);
	}

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			this->_board[i][j] = (i * size) + j + 1;
		}
	}

	this->_board[size - 1][size - 1] = -1;

	this->_empty = std::make_pair(size - 1, size - 1);

}


puzzle_class::~puzzle_class()
{
	for (int i = 0; i < _size; ++i)
	{
		_board[i].clear();
		
	}
	_board.clear();
	_size = NULL;
	_score = NULL;
	_empty = std::pair<int,int>(NULL,NULL);
};

puzzle_class::puzzle_class(const puzzle_class& obj)
{

	if (!_board.empty())
	{
		for (int i = 0; i < _size; i++)
		{
			_board[i].clear();
		}
		_board.clear();

	}
	_board.resize(obj._size);
	for (int i = 0; i < obj._size; i++)
	{
		this->_board[i].resize(obj._size);
	}
	
	
	_size = obj._size;
	_score = obj._score;



	for (int i = 0; i < obj._size; i++)
	{
		for (int j = 0; j < obj._size; j++)
		{
			_board[i][j] = obj._board[i][j];
		}
	}

	_empty = obj._empty;
}


puzzle_class& puzzle_class::operator=(const puzzle_class& obj)
{
	if (this != &obj)
	{
		if (!_board.empty())
		{
			for (int i = 0; i < _size; i++)
			{
				_board[i].clear();
			}
			_board.clear();

		}
		_board.resize(obj._size);
		for (int i = 0; i < obj._size; i++)
		{
			this->_board[i].resize(obj._size);
		}


		_size = obj._size;
		_score = obj._score;



		for (int i = 0; i < obj._size; i++)
		{
			for (int j = 0; j < obj._size; j++)
			{
				_board[i][j] = obj._board[i][j];
			}
		}


	}

	_empty = obj._empty;

	return *this;
}



bool puzzle_class::operator!=(const puzzle_class& right) const
{
	if (_size == right._size)
	{
		for (int i = 0; i < _size; i++)
		{
			for (int j = 0; j < _size; j++)
			{
				if (_board[i][j] != right._board[i][j])
				{
					return true;
				}
			}
		}
		return false;
	}
	return true;
}


bool puzzle_class::operator==(const puzzle_class& right) const
{
	//std::cout << "+" << std::flush;
	return !(*this != right);
}


bool puzzle_class::operator<(const puzzle_class& right) const
{

	return _score < right._score;
}


bool puzzle_class::operator>(const puzzle_class& right) const
{

	return right._score < _score;
}


bool puzzle_class::operator<=(const puzzle_class& right) const
{
	return !(_score > right._score);
}


bool puzzle_class::operator>=(const puzzle_class& right) const
{
	return !(_score < right._score);
}


bool puzzle_class::operator!=(const int& right) const
{
	return this->_score != right;
}


bool puzzle_class::operator==(const int& right) const
{
	return this->_score == right;
}


bool puzzle_class::operator<(const int& right) const
{
	return  this->_score < right;
}


bool puzzle_class::operator>(const int& right) const
{
	return right < this->_score;
}


bool puzzle_class::operator<=(const int& right) const
{
	return !(this->_score > right);
}


bool puzzle_class::operator>=(const int& right) const
{
	return !(this->_score < right);
}


int puzzle_class::GetSize() const
{
	return this->_size;
}


std::vector<std::vector<int>> puzzle_class::GetPuzzle() const
{
	return this->_board;
}


void puzzle_class::PrintPuzzle() const
{
	
	for(int i = 0; i< this->_size;i++)
	{
		for(int j = 0;j<this->_size; j++)
		{
			std::cout << std::setfill(' ') << std::setw(3);
			std::cout << this->_board[i][j] << " ";
		}
		std::cout << "\n\n";
	}
	//std::cout << std::flush;
}


int puzzle_class::GetScore() const
{
	int score = 0;

	for (int i = 0; i < this->_size; i++)
	{
		for (int j = 0; j < this->_size; j++)
		{
			if (this->_board[i][j] != -1)
			{
				const int trueX = (this->_board[i][j] - 1) / this->_size;
				const int trueY = (this->_board[i][j] - 1) - (this->_size * trueX);
				score += abs(i - trueX) + abs(j - trueY);
			}
		}
	}
	score += abs(_empty.first - (_size - 1)) + abs(_empty.second - (_size - 1));
	
	return score;
}


void puzzle_class::UpdateScore()
{
	_score = GetScore();
}

std::string puzzle_class::Serialize() const
{
	std::string serialized;
	serialized += std::to_string(this->_size);
	serialized += "|";
	for (const auto& vector : _board)
	{
		for (auto number: vector)
		{
			serialized += std::to_string(number);
			serialized += "|";
		}
	}
	return serialized;
}


void puzzle_class::Deserialize(std::string serializedObject)
{
	try
	{
		if (serializedObject.empty())
		{
			throw std::invalid_argument("Invalid serialized puzzle");
		}
		char* next_string = NULL;
		char* token = strtok_s(&serializedObject[0], "|",&next_string);
	
		this->_size = std::stoi(token);
		std::cout<<this->_size<<"\n";
		if (this->_size < 2)
		{
			throw std::invalid_argument("Puzzle size must be bigger than 1");
		}
		this->_board.resize(this->_size);
		token = strtok_s(NULL, "|", &next_string);
		for (int i = 0; i < this->_size; i++)
		{
			this->_board[i].resize(this->_size);
			for (int j = 0; j < this->_size; j++)
			{
				if (token == nullptr)
				{
					throw std::invalid_argument("Invalid serialized puzzle");
				}
				this->_board[i][j] = std::stoi(token);
				if (_board[i][j] == -1)
				{
					_empty = std::make_pair(i, j);
				}
				token = strtok_s(NULL, "|", &next_string);
			}
		}
		
	}
	catch (std::exception &e)
	{
		std::cerr<<e.what()<<"\n";
		exit(-1);
	}
	

	UpdateScore();
}


void puzzle_class::SetBoard(std::vector<std::vector<int>> newBoard, int size)
{
	this->_size = size;
	if (!_board.empty())
	{
		for (int i = 0; i < _size; i++)
		{
			_board[i].clear();
		}
		_board.clear();

	}
	
	_board.resize(size);
	
	for (int i = 0; i < size; i++)
	{
		this->_board[i].resize(size);
	}

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			this->_board[i][j] = newBoard[i][j];
		}
	}
}


int puzzle_class::CountInversions()
{
	std::vector<int> placements;
	for (int i = 0; i < _size; i++)
	{
		for (int j = 0; j < _size; j++)
		{
			if (_board[i][j] > -1)
			{
				placements.push_back(_board[i][j]);
			}
		}
	}

	int count = 0;

	for (int i = 0; i < (_size * _size) - 2; i++)
	{
		for (int j = i + 1; j < (_size * _size) - 1; j++)
		{
			if (placements[i] > placements[j])
			{
				count++;
			}
		}
	}

	return count;
}


bool puzzle_class::IsSolvable()
{
	if (_size % 2 == 1)
	{
		if (CountInversions() % 2 == 0)
		{
			//std::cout << "inversions:" << CountInversions() << std::endl;
			return true;
		}
		return false;
	}

	for (int i = 0; i < _size; i++)
	{
		for (int j = 0; j < _size; j++)
		{
			if (_board[i][j] == -1)
			{
				if ((CountInversions() + i) % 2 == 1)
				{
					return true;
				}
				return false;
			}
		}
	}

	return false;
}


void puzzle_class::Shuffle()
{
	// makes hard puzzles, shortest path see was 36 for n = 4
	std::random_device rd;
	std::mt19937 generator = std::mt19937(rd());
	const std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(0, _size * _size - 1);
	std::vector<int> pieces;
	do
	{
		pieces.resize(_size * _size);
		for (int i = 0; i < _size * _size; i++)
		{
			pieces[i] = i + 1;
		}
		pieces[(_size * _size) - 1] = -1;

		for (int i = 0; i < _size; i++)
		{
			for (int j = 0; j < _size; j++)
			{
				int el = distribution(generator) % pieces.size();
				_board[i][j] = pieces[el] ;
				pieces.erase(pieces.begin() + el);
			}
		}
		UpdateScore();
	} while (!IsSolvable() || _score > 25);

	for (int i = 0; i < _size; i++)
	{
		for (int j = 0; j < _size; j++)
		{
			if (_board[i][j] == -1)
			{
				_empty = std::make_pair(i, j);
				break;
			}
		}
	}
	
}

void puzzle_class::EasyShuffle()
{
	// easier to control difficulty of puzzles 
	std::random_device rd;
	std::mt19937 generator = std::mt19937(rd());
	const std::uniform_int_distribution<int> distribution_moves = std::uniform_int_distribution<int>(40, 60);
	const std::uniform_int_distribution<int> distribution_neighbor = std::uniform_int_distribution<int>(0, 4) ;
	int moves = distribution_moves(generator);
	while (moves > 0 || _score < 23)
	{
		auto neighbours = this->GetValidMoves();
		this->Deserialize(this->GetNeighbour(neighbours[distribution_neighbor(generator) % neighbours.size()]).Serialize());
		moves--;
	}
}


puzzle_class& puzzle_class::GetNeighbour(int move) const
{
	puzzle_class* neighbour = new puzzle_class();
	*neighbour = *this;
	int newX = (move - 1)  / this->_size;
	int newY = (move - 1) - (this->_size * newX);

	if (newX < 0 || newX > _size - 1 || newY < 0 || newY > _size - 1)
	{
		std::cout << "Out of the grid\n";
		exit(-1);
	}

	if ((abs(_empty.first - newX) == 0 && abs(_empty.second - newY) == 0) || (abs(_empty.first - newX) + abs(_empty.second - newY) > 1))
	{
		std::cout << "Tried to make a switch between two pieces\n";
		exit(-1);
	}

	const int aux = neighbour->_board[_empty.first][_empty.second];
	neighbour->_board[_empty.first][_empty.second] = neighbour->_board[newX][newY];
	neighbour->_board[newX][newY] = aux;

	neighbour->_empty = std::make_pair(newX, newY);

	neighbour->UpdateScore();
	return *neighbour;
}



std::vector<int> puzzle_class::GetValidMoves() const
{
	std::vector<int> moves;
	if (_empty.second > 0)
	{
		//can move left
		moves.push_back((_empty.first * _size) + _empty.second);
	}
	if (_empty.second < _size - 1)
	{
		// can move right
		moves.push_back((_empty.first * _size) + _empty.second + 2);
	}
	if (_empty.first > 0)
	{
		// can move down
		moves.push_back(((_empty.first  - 1 )* _size) + _empty.second + 1);
	}
	if (_empty.first < _size - 1)
	{
		//can move up
		moves.push_back(((_empty.first + 1) * _size) + _empty.second + 1);
	}
	return moves;
}


bool puzzle_class::IsSolved() const
{
	return (_score == 0);
}

void puzzle_class::SetH(int value)
{

	UpdateScore();
}
