#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <random>
#include <vector>
#include <thread>
#include <fstream>
#include <regex>
#include <sstream>
#include "PuzzleClass.h"

#pragma comment (lib, "msmpi.lib")

#define MASTER 0




void SwitchElementBetweenContainers(std::vector<int>& from, std::vector<int>& to, const int element)
{
	from.erase(std::remove(from.begin(), from.end(), element), from.end());
	to.push_back(element);
}

enum CommunicationTag
{
	COMM_TAG_MASTER_SEND_CONTINUE,
	COMM_TAG_MASTER_SEND_TERMINATE,
	COMM_TAG_SLAVE_SEND_FINISHED,
	COMM_TAG_SLAVE_FINISHED_FOUND_SOLUTION,
	COMM_TAG_MASTER_SEND_INITIAL,
	COMM_TAG_RESET,
	COMM_TAG_WRITE_PUZZLE,
	COMM_TAG_SLAVE_CRASHED,
};

int SearchToGoal(std::vector<puzzle_class>& path, int g, int bound, const puzzle_class& goal)
{

	auto f = g + path.back()._score;
	if (f > bound)
	{
		return f;
	}
	if (path.back() == goal)
	{
		return 0;
	}
	int min = 10000;
	for (auto move : path.back().GetValidMoves())
	{
		std::shared_ptr<puzzle_class> neighbour(&path.back().GetNeighbour(move));
		if (std::find(path.begin(), path.end(), *neighbour) == path.end())
		{
			path.push_back(*neighbour);

			auto t = SearchToGoal(path, g + 1, bound, goal);

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


std::vector<puzzle_class> IDAStar(const puzzle_class& initial, const puzzle_class& goal)
{
	auto bound = initial._score;
	std::vector<puzzle_class> path;

	path.push_back(initial);
	while (true)
	{
		auto t = SearchToGoal(path, 0, bound, goal);
		if (t == 0)
		{

			return path;
		}
		if (t == 10000)
		{
			std::cout << "can't solve this puzzle\n";
			return std::vector<puzzle_class>();
		}
		bound = t;
	}
}


int SearchToSolved(std::vector<puzzle_class>& path, int g, int bound)
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
		if (std::find(path.begin(), path.end(), *neighbour) == path.end())
		{
			path.push_back(*neighbour);

			auto t = SearchToSolved(path, g + 1, bound);

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


puzzle_class InterpretArguments(int argc, char* argv[], bool &go_in_depth, int& depth)
{
	puzzle_class puzzle;
	if (argc > 1)
	{
		// there are arguments

		auto serialized_regex = std::regex("(-{0,1}[0-9]+\\|){3,}");
		auto positiveNumber_regex = std::regex("^-?[0-9]+$");

		auto firstArgument = std::string(argv[1]);
		if (firstArgument != "DEPTH")
		{
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
						throw std::invalid_argument("failed to open given file\n");
					}
					std::string line;
					std::getline(input_file, line);
					puzzle.Deserialize(line);

				}
				catch (std::exception& e)
				{
					throw std::invalid_argument("The file path is invalid.\n The first argument can be either a number, serialized puzzle, \
						path or nothing case in which the puzzle will have size 4 and be randomly arranged.\n \
						If no first argument regarding the puzzle was desired but the algorithm should go in depth at the start then the \
						\"DEPTH\" argument must be given as first argument and the depth to go to as the second.");
				}

			}

			if (argc >= 4)
			{
				if (std::strcmp(argv[2], "DEPTH") == 0)
				{
					go_in_depth = true;
				}
				try
				{
					auto depthString = std::string(argv[3]);
					depth = std::stoi(depthString);
				}
				catch (std::exception& e)
				{
					throw std::invalid_argument("Given depth is invalid");
				}

			}


		}
		else
		{
			if (argc >= 3)
			{
				if (std::strcmp(argv[1], "DEPTH") == 0)
				{
					go_in_depth = true;
				}
				try
				{
					auto depthString = std::string(argv[2]);
					depth = std::stoi(depthString);
				}
				catch (std::exception& e)
				{
					throw std::invalid_argument("Given depth is invalid");
				}

			}
			puzzle = puzzle_class(4);
			puzzle.Shuffle();

		}

	}
	else
	{
		puzzle = puzzle_class(4);
		puzzle.Shuffle();
	}
	if (depth < 1 && go_in_depth)
	{
		throw std::invalid_argument("Depth must be bigger than 1\n");
	}
	return puzzle;
}


int main(int argc, char* argv[]) {

	const int rc = MPI_Init(&argc, &argv);
	if (rc != MPI_SUCCESS)
	{
		std::cout << "Error starting MPI program. Terminating.\n";

		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	int  numtasks, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (numtasks < 2)
		{
			printf("Error: create at least 2 processes for this test\n");
			MPI_Finalize();
			return -1;
		}


	try
	{
		
		if (rank == 0)
		{
			bool goInDepth = false;
			int depth = 5;
			std::string path_to_file;
			std::string serialized_puzzle;
			std::vector<int> freeSlaves;
			std::vector<int> workingSlaves;

			freeSlaves.resize(numtasks - 1);

			for (int i = 1; i < numtasks; i++)
			{
				freeSlaves[i - 1] = i;
			}
			
			puzzle_class puzzle = InterpretArguments(argc, argv, goInDepth, depth);

			std::cout<<"Start:\n"<<std::flush;
			puzzle.PrintPuzzle();
			std::cout << "\n" << std::flush;
			if (!puzzle.IsSolvable())
			{
				throw std::invalid_argument("The given puzzle is unsolvable\n");
			}
			std::string puzzle_string = puzzle.Serialize();
			int puzzle_size = puzzle_string.size() + 1;
			int shortest = 10000;
			int slave_to_complete = -1;
			
			MPI_Bcast(&puzzle_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
			MPI_Bcast(&puzzle_string[0], puzzle_size, MPI_CHAR, MASTER, MPI_COMM_WORLD);
			MPI_Bcast(&goInDepth, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);
			if (goInDepth)
			{
				MPI_Bcast(&depth, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
			}

			std::vector<puzzle_class> aux;
			std::vector<puzzle_class> to_use;
			std::vector<puzzle_class> visited;
			to_use.push_back(puzzle);
			visited.push_back(puzzle);

			// while for number of tasks
			if (goInDepth)
			{
				for (int i = 1; i <= depth; i++)
				{
					while (!to_use.empty())
					{
						for (auto move : to_use.back().GetValidMoves())
						{
							auto current = to_use.back().GetNeighbour(move);
							if (current.IsSolved())
							{
								std::cout<<"Either the given puzzle is so simple that it will be better to solve it on a single process or the \
max number of jobs/depth given is very big, either way I will try to find the solution.\n"<<std::flush;
								auto solution = IDAStar(puzzle, current);
								for (auto node : solution)
								{
									node.PrintPuzzle();
									std::cout<<"\n";
								}
								int dummy1,dummy2;
								std::cout<<std::flush;
								for (int j = 1; j < numtasks; j++)
								{
									MPI_Send(&dummy1, 1, MPI_INT, j, COMM_TAG_MASTER_SEND_TERMINATE, MPI_COMM_WORLD);
									MPI_Send(&dummy2, 1, MPI_INT, j, COMM_TAG_MASTER_SEND_TERMINATE, MPI_COMM_WORLD);
								}
								MPI_Finalize();
								return 0;
								
							}
							if (std::find(visited.begin(), visited.end(), current) == visited.end())
							{
								aux.push_back(current);
								visited.push_back(current);
							}

						}
						to_use.pop_back();
					}
					to_use = aux;
					aux.clear();

				}

			}
			else
			{
				while (to_use.size() < (numtasks - 1))
				{

					while (!to_use.empty())
					{
						for (auto move : to_use.back().GetValidMoves())
						{
							auto current = to_use.back().GetNeighbour(move);
							if (current.IsSolved())
							{
								std::cout<<"Either the given puzzle is so simple that it will be better to solve it on a single process or the\
max number of jobs/depth given is very big, either way I will try to find the solution.\n"<<std::flush;
								auto solution = IDAStar(puzzle, current);
								for (auto node : solution)
								{
									node.PrintPuzzle();
									std::cout<<"\n";
								}
								int dummy1,dummy2;
								for (int i = 1; i < numtasks; i++)
								{
									MPI_Send(&dummy1, 1, MPI_INT, i, COMM_TAG_MASTER_SEND_TERMINATE, MPI_COMM_WORLD);
									MPI_Send(&dummy2, 1, MPI_INT, i, COMM_TAG_MASTER_SEND_TERMINATE, MPI_COMM_WORLD);
								}
								MPI_Finalize();
								return 0;
								
							}
							if (std::find(visited.begin(), visited.end(), current) == visited.end())
							{
								aux.push_back(current);
								visited.push_back(current);
							}

						}
						to_use.pop_back();
					}
					to_use = aux;
					aux.clear();
				}
			}
			
			// vector of bounds
			std::vector<int> limits;
			int limit_to_use = puzzle.GetScore();


			//vectors for jobs 
			std::vector<int> jobs;
			std::vector<int> jobs_done;
			for (int i = 0; i < to_use.size(); ++i)
			{
				jobs.push_back(i);
			}

			int finished = 0;

			//iterate while solution is not found
			while (true)
			{
				// reset jobs to be done and bounds if all current jobs are finished
				if (jobs_done.size() == to_use.size())
				{
					for (int i = 0; i < to_use.size(); ++i)
					{
						jobs.push_back(i);
					}
					jobs_done.clear();
					std::pop_heap(limits.begin(), limits.end(), std::greater<>());
					limit_to_use = limits.back();
					limits.clear();

				}

				// while there are jobs to be done and free slaves give jobs to free slaves
				while (!freeSlaves.empty() && !jobs.empty())
				{

					MPI_Send(&jobs.back(), 1, MPI_INT, freeSlaves.front(), COMM_TAG_MASTER_SEND_CONTINUE, MPI_COMM_WORLD);
					MPI_Send(&limit_to_use, 1, MPI_INT, freeSlaves.front(), COMM_TAG_MASTER_SEND_CONTINUE, MPI_COMM_WORLD);
					// switch slaves between waiting and working
					SwitchElementBetweenContainers(freeSlaves, workingSlaves, freeSlaves.front());
					jobs.pop_back();
				}

				MPI_Status stat;

				int job;
				int limit;

				// wait for any response from slaves
				MPI_Recv(&job, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
				MPI_Recv(&limit, 1, MPI_INT, stat.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				limits.push_back(limit);
				std::push_heap(limits.begin(), limits.end(), std::greater<>());

				if (stat.MPI_TAG == COMM_TAG_SLAVE_FINISHED_FOUND_SOLUTION)
				{
					finished = 1;
					if (shortest > limit)
					{
						shortest = limit;
						slave_to_complete = stat.MPI_SOURCE;
					}
				}


				SwitchElementBetweenContainers(workingSlaves, freeSlaves, stat.MPI_SOURCE);
				jobs_done.push_back(job);


				if (finished == 1)
				{

					while (!workingSlaves.empty())
					{
						int dummy1, dummy2;
						MPI_Recv(&dummy1, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
						MPI_Recv(&dummy2, 1, MPI_INT, stat.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						SwitchElementBetweenContainers(workingSlaves, freeSlaves, stat.MPI_SOURCE);
					}

					MPI_Send(&shortest, 1, MPI_INT, slave_to_complete, COMM_TAG_WRITE_PUZZLE, MPI_COMM_WORLD);
					MPI_Send(&shortest, 1, MPI_INT, slave_to_complete, COMM_TAG_WRITE_PUZZLE, MPI_COMM_WORLD);

					break;
				}
			}

			int dummy1, dummy2;
			for (int i = 1; i < numtasks; i++)
			{
				if (i != slave_to_complete)
				{
					MPI_Send(&dummy1, 1, MPI_INT, i, COMM_TAG_MASTER_SEND_TERMINATE, MPI_COMM_WORLD);
					MPI_Send(&dummy2, 1, MPI_INT, i, COMM_TAG_MASTER_SEND_TERMINATE, MPI_COMM_WORLD);
				}


			}

			MPI_Finalize();
			return 0;

		}
		else
		{
			bool goInDepth = false;
			int depth = 5;

			MPI_Status stat;
			char puzzle_string[100]="";
			int puzzle_size=0;

			MPI_Bcast(&puzzle_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
			
			MPI_Bcast(&puzzle_string, puzzle_size, MPI_CHAR, MASTER, MPI_COMM_WORLD);

			MPI_Bcast(&goInDepth, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);
			
			if (goInDepth)
			{
				MPI_Bcast(&depth, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
			}


			puzzle_class puzzle;
			puzzle.Deserialize(std::string(puzzle_string));

			std::vector<puzzle_class> aux;
			std::vector<puzzle_class> to_use;
			std::vector<puzzle_class> visited;
			to_use.push_back(puzzle);
			visited.push_back(puzzle);

			std::vector<puzzle_class> solution;
			
			// while for number of tasks
			if (goInDepth)
			{
				for (int i = 1; i <= depth; i++)
				{
					while (!to_use.empty())
					{
						for (auto move : to_use.back().GetValidMoves())
						{
							auto current = to_use.back().GetNeighbour(move);
							if (std::find(visited.begin(), visited.end(), current) == visited.end())
							{
								aux.push_back(current);
								visited.push_back(current);
							}

						}
						to_use.pop_back();
					}
					to_use = aux;
					aux.clear();

				}

			}
			else
			{
				while (to_use.size() < (numtasks - 1))
				{

					while (!to_use.empty())
					{
						for (auto move : to_use.back().GetValidMoves())
						{
							auto current = to_use.back().GetNeighbour(move);
							if (std::find(visited.begin(), visited.end(), current) == visited.end())
							{
								aux.push_back(current);
								visited.push_back(current);
							}

						}
						to_use.pop_back();
					}
					to_use = aux;
					aux.clear();
				}
			}


			// only slaves will actually try to solve the problem
			int job;

			int limit;
			while (true)
			{

				MPI_Recv(&job, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
				MPI_Recv(&limit, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
				if (stat.MPI_TAG == COMM_TAG_MASTER_SEND_TERMINATE)
				{
					std::cout << rank << " out\n" << std::flush;
					MPI_Finalize();
					return 0;
				}
				if (stat.MPI_TAG == COMM_TAG_RESET)
				{
					break;
				}
				if (stat.MPI_TAG == COMM_TAG_WRITE_PUZZLE)
				{
					std::cout << rank << " should write solution\n" << std::flush;

					for (auto i : solution)
					{
						i.PrintPuzzle();
						std::cout << "\n";
					}
					std::cout << " took me " << solution.size() - 1 << " steps\n";
					std::cout << std::flush;

					std::cout << rank << " out\n" << std::flush;
					MPI_Finalize();
					return 0;
				}

				auto bound = limit;

				std::vector<puzzle_class> path;
				path.push_back(to_use[job]);

				auto t = SearchToSolved(path, 0, bound);
				if (t == 0)
				{
					//std::cout << rank << ":\n";
					//path.back().PrintPuzzle();
					//std::cout << "\n\n" << std::flush;
					solution = IDAStar(puzzle, to_use[job]);
					for (int i = 1; i < path.size(); i++)
					{
						solution.push_back(path[i]);
					}
					int steps = solution.size() - 1;
					MPI_Send(&job, 1, MPI_INT, MASTER, COMM_TAG_SLAVE_FINISHED_FOUND_SOLUTION, MPI_COMM_WORLD);
					MPI_Send(&steps, 1, MPI_INT, MASTER, COMM_TAG_SLAVE_FINISHED_FOUND_SOLUTION, MPI_COMM_WORLD);
				}
				else if (t == 10000)
				{
					std::cout << rank << " thinks there is no solution for this puzzle\n" << std::flush;
				}
				else
				{
					bound = t;
					MPI_Send(&job, 1, MPI_INT, MASTER, COMM_TAG_SLAVE_SEND_FINISHED, MPI_COMM_WORLD);
					MPI_Send(&bound, 1, MPI_INT, MASTER, COMM_TAG_SLAVE_SEND_FINISHED, MPI_COMM_WORLD);
				}

			}

		}
	}
	catch (std::exception& e)
	{
		std::cerr << "ERROR: The process " << rank << " crashed, the reason is :\n" << e.what() << "\n" << std::flush;
		MPI_Abort(MPI_COMM_WORLD, -1);
		return -1;
	}
}