#pragma once

#include <string>
#include <vector>

class puzzle_class 
{
public:
	int _size = NULL;
	std::pair<int, int> _empty;
	std::vector<std::vector<int>> _board;

	int _score = NULL;

	puzzle_class();
	puzzle_class(int size);
	~puzzle_class();

	puzzle_class(const puzzle_class& obj);

	puzzle_class& operator =(const puzzle_class& obj);

	bool operator !=(const puzzle_class& right) const;
	bool operator ==(const puzzle_class& right) const;
	bool operator <(const puzzle_class& right) const;
	bool operator >(const puzzle_class& right) const;
	bool operator <=(const puzzle_class& right) const;
	bool operator >=(const puzzle_class& right) const;

	bool operator !=(const int& right) const;
	bool operator ==(const int& right) const;
	bool operator <(const int& right) const;
	bool operator >(const int& right) const;
	bool operator <=(const int& right) const;
	bool operator >=(const int& right) const;
	
	int GetSize() const;
	std::vector<std::vector<int>> GetPuzzle() const;
	void PrintPuzzle() const;
	int GetScore() const;
	void UpdateScore();
	std::string Serialize() const;
	void Deserialize(std::string serializedObject);
	void SetBoard(std::vector<std::vector<int>> newBoard, int size);
	int CountInversions();
	bool IsSolvable();
	void Shuffle();
	void EasyShuffle();
	puzzle_class& GetNeighbour(int move) const;
	std::vector<int> GetValidMoves() const;
	bool IsSolved() const;
	void SetH(int value);
};

inline bool operator!=(const int& left, const puzzle_class& right)
{
	return left != right._score;
}


inline bool operator==(const int& left, const puzzle_class& right)
{
	return !(left != right);
}

inline bool operator <(const int& left, const puzzle_class& right)
{
	return left < right._score;
}

inline bool operator >(const int& left, const puzzle_class& right)
{
	return right._score < left;
}

inline bool operator <=(const int& left, const puzzle_class& right)
{
	return !(left > right._score);
}

inline bool operator >=(const int& left, const puzzle_class& right)
{
	return !(left < right._score);
}
