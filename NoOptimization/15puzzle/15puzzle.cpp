#include <iostream>
#include <fstream>
#include <regex>
#include <stack>
#include "PuzzleClass.h"



int search(std::vector<puzzle_class>& path, int g, int bound)
{

	auto f = g + path.back()._score;
	if (f > bound)
	{
		return f;
	}
	if (path.back().IsSolved())
	{
		return 0;
	}
	int min = 10000;
	for (auto move : path.back().GetValidMoves())
	{
		std::shared_ptr<puzzle_class> neighbour(&path.back().GetNeighbour(move));
		if (std::find(path.begin(),path.end(),*neighbour) == path.end())
		{
			path.push_back(*neighbour);

			auto t = search(path, g + 1, bound);

			if (t == 0)
			{
				return 0;
			}
			if (t < min)
			{
				min = t;
			}
			path.pop_back();
		}
	}
	return min;
}


void ida(const puzzle_class& initial)
{
	auto bound = initial._score;
	std::vector<puzzle_class> path;
	
	path.push_back(initial);
	while (true)
	{
		auto t = search(path, 0, bound);
		if (t == 0)
		{
			int count = 0;
			std::stack<puzzle_class> h;

			while (!path.empty())
			{
				count++;
				h.push(path.back());
				path.pop_back();

			}
			while (!h.empty())
			{
				h.top().PrintPuzzle();
				std::cout << "\n";
				h.pop();
			}

			std::cout << "took me " << count - 1  << " steps\n";
			return;
		}
		if (t == 10000)
		{
			std::cout << "can't solve this puzzle\n";
			return;
		}
		bound = t;
	}
}

int main(int argc, char* argv[])
{
	puzzle_class puzzle;
	if (argc > 1)
	{
		// there are arguments

		const auto serialized_regex = std::regex("(-{0,1}[0-9]+\\|){3,}");
		const auto positiveNumber_regex = std::regex("^-?[0-9]+$");

		auto firstArgument = std::string(argv[1]);

		if (std::regex_match(firstArgument.begin(), firstArgument.end(), positiveNumber_regex))
		{
			// first argument is a number
			puzzle = puzzle_class(std::stoi(firstArgument));
			puzzle.Shuffle();

		}
		else if (std::regex_match(firstArgument.begin(), firstArgument.end(), serialized_regex))
		{
			// first argument is a serialized puzzle, puzzle is considered already shuffled
			puzzle.Deserialize(firstArgument);
		}
		else
		{
			try
			{
				// first argument is a file path, puzzle is considered already shuffled
				std::ifstream input_file;
				input_file.open(firstArgument, std::ios::in);
				if (input_file.fail())
				{
					std::cerr<<"failed to open given file\n";
					return -1;
				}
				std::string line;
				std::getline(input_file, line);
				puzzle.Deserialize(line);

			}
			catch (std::exception& e)
			{
				throw std::invalid_argument("The file path is invalid.\n The first argument can be either a number, serialized puzzle, \
					path or nothing case in which the puzzle will have size 4 and be randomly arranged.\n");
			}

		}
	}
	else
	{
		puzzle = puzzle_class(4);
		puzzle.Shuffle();
	}


	if (! puzzle.IsSolvable())
	{
		std::cerr<<"The given puzzle is unsolvable\n";
		return -1;
	}
	
	std::cout << "\n";
	ida(puzzle);

	return 0;

}
