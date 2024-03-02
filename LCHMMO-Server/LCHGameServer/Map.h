#pragma once
#include "Actor.h"
#include <fstream>
#define MAX_MAP_SIZE 1024

class Vector2Int
{
public:
	int32 x;
	int32 y;
	int32 sqrMagnitude;
	int32 CellDistFromZero;

public:
	Vector2Int(int32 x, int32 y) 
	{ 
		this->x = x; 
		this->y = y; 
		this->CellDistFromZero = abs(x * x + y * y);
	}

	float Magnitude() { return (float)sqrt(sqrMagnitude); }

public:
	static Vector2Int UP() { return Vector2Int(0,1); }
	static Vector2Int DOWN() { return Vector2Int(0, -1); }
	static Vector2Int LEFT() { return Vector2Int(-1, 0); }
	static Vector2Int RIGHT() { return Vector2Int(1, 0); }

	static double Distance(Vector2Int A, Vector2Int B)
	{
		int32 xsquare = (B.x - A.x) * (B.x - A.x);
		int32 ysquare = (B.y - A.y) * (B.y - A.y);
		double dist = sqrt(xsquare + ysquare);
		return dist;
	}

	static bool IsEqual(Vector2Int a, Vector2Int b)
	{
		if (a.x == b.x && a.y == b.y)
			return true;
		return false;
	}

	static Vector2Int GetVectorFromActorPos(protocol::PositionInfo posInfo)
	{
		return Vector2Int(posInfo.posx(), posInfo.posy());
	}

	friend Vector2Int operator+(Vector2Int a, Vector2Int b)
	{
		return Vector2Int(a.x + b.x, a.y + b.y);
	}

	friend Vector2Int operator-(Vector2Int a, Vector2Int b)
	{
		return Vector2Int(a.x - b.x, a.y - b.y);
	}
};

class Pos
{
public:
	Pos() { this->y = 0; this->x = 0; }
	Pos(int32 y, int32 x) { this->y = y; this->x = x; }
	bool operator==(Pos& other)
	{
		return y == other.y && x == other.x;
	}

	bool operator!=(Pos& other)
	{
		return !(*this == other);
	}

	bool operator<(const Pos& other) const
	{
		if (y != other.y)
			return y < other.y;
		return x < other.x;
	}

	Pos operator+(Pos& other)
	{
		Pos ret;
		ret.y = y + other.y;
		ret.x = x + other.x;
		return ret;
	}

	Pos& operator+=(Pos& other)
	{
		y += other.y;
		x += other.x;
		return *this;
	}

	int32 y = 0;
	int32 x = 0;
};

struct PQNode
{
	bool operator<(const PQNode& other) const { return F < other.F; }
	bool operator>(const PQNode& other) const { return F > other.F; }

	int32 F;
	int32 G;
	Pos pos;
};

class Map
{
public:
	int32 MinX;
	int32 MinY;
	int32 MaxX;
	int32 MaxY;

	bool CollisionBuf[MAX_MAP_SIZE][MAX_MAP_SIZE];
	ActorPtr ObjectBuf[MAX_MAP_SIZE][MAX_MAP_SIZE];

private:
	uint32 xCount, yCount;
	 
public:
	void LoadMap(int mapId, std::string pathPrefix = "../../../../Common/MapData");
	bool CanGo(Vector2Int cellPos, bool checkActor = true);
	ActorPtr Find(Vector2Int cellPos);
	bool ApplyLeave(ActorPtr actor);
	bool ApplyMove(ActorPtr actor, Vector2Int dest);
	std::vector<Vector2Int> FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool CheckActor = true);
	std::vector<Vector2Int> CalcCellPathFromParent(std::map<Pos, Pos> parent, Pos dest);
	Pos Cell2Pos(Vector2Int cell);
	Vector2Int Pos2Cell(Pos pos);
};

using MapPtr = std::shared_ptr<Map>;