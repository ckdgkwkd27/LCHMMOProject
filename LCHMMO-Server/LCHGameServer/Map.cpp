#include "pch.h"
#include "Map.h"
#include "ZoneManager.h"
#include "Player.h"
#include "Section.h"

void Map::LoadMap(int mapId, std::string pathPrefix)
{
    std::string mapFullPath = pathPrefix + "/Map_" + std::to_string(mapId) + ".txt";
    std::ifstream openFile(mapFullPath);
    CRASH_ASSERT(openFile.is_open());

    std::string line;
    getline(openFile, line);
    MinX = stoi(line);

	getline(openFile, line);
	MaxX = stoi(line);

	getline(openFile, line);
	MinY = stoi(line);

	getline(openFile, line);
	MaxY = stoi(line);

	xCount = MaxX - MinX + 1;
	yCount = MaxY - MinY + 1;
    printf("[INFO] LoadMap MapId=%d, MinX=%d, MaxX=%d, MinY=%d, MaxY=%d\n", mapId, MinX, MaxX, MinY, MaxY);

    for (uint32 y = 0; y < yCount; y++)
    {
        std::string mapLine;
        getline(openFile, mapLine);
        for (uint32 x = 0; x < xCount; x++)
        {
            CollisionBuf[y][x] = (mapLine[x] == '1' ? true : false);
        }
    }

    openFile.close();
}

bool Map::CanGo(Vector2Int cellPos, bool checkActor)
{
    if (cellPos.x < MinX || cellPos.x > MaxX)
        return false;
    if (cellPos.y < MinY || cellPos.y > MaxY)
        return false;

    int32 x = cellPos.x - MinX;
    int32 y = MaxY - cellPos.y;
    bool ret = !CollisionBuf[y][x] && (!checkActor || ObjectBuf[y][x] == nullptr);
    return ret;
}

ActorPtr Map::Find(Vector2Int cellPos)
{
    if (cellPos.x < MinX || cellPos.x > MaxX)
        return nullptr;
    if (cellPos.y < MinY || cellPos.y > MaxY)
        return nullptr;

	int32 x = cellPos.x - MinX;
    int32 y = MaxY - cellPos.y;
    return ObjectBuf[y][x];
}

bool Map::ApplyLeave(ActorPtr actor)
{
    auto _zone = GZoneManager.FindZoneByID(actor->zoneID);
    if (_zone == nullptr)
        return false;
    
	protocol::PositionInfo posInfo = actor->ActorInfo.posinfo();
	if (posInfo.posx() < MinX || posInfo.posx() > MaxX)
		return false;
	if (posInfo.posy() < MinY || posInfo.posy() > MaxY)
		return false;

    //Remove from section
    Vector2Int cellPos(posInfo.posx(), posInfo.posy());
    SectionPtr section = _zone->GetSection(cellPos);
    section->Remove(actor);

	{
		int x = posInfo.posx() - MinX;
		int y = MaxY - posInfo.posy();
        if (ObjectBuf[y][x] == actor)
            ObjectBuf[y][x] = nullptr;
	}
    return true;
}

bool Map::ApplyMove(ActorPtr actor, Vector2Int dest)
{
    //ApplyLeave(actor);
    auto _zone = GZoneManager.FindZoneByID(actor->zoneID);
    if (_zone == nullptr)
        return false;

    if (CanGo(dest, true) == false)
    {
        return false;
    }

    protocol::PositionInfo posInfo = actor->ActorInfo.posinfo();
    {
		int32 x = posInfo.posx() - MinX;
		int32 y = MaxY - posInfo.posy();
        if (ObjectBuf[y][x] == actor)
            ObjectBuf[y][x] = nullptr;
    }

    {
        int32 x = dest.x - MinX;
        int32 y = MaxY - dest.y;
        ObjectBuf[y][x] = actor;
    }

    //Section
	Vector2Int cellPos(actor->ActorInfo.posinfo().posx(), actor->ActorInfo.posinfo().posy());
    SectionPtr now = _zone->GetSection(cellPos);

    SectionPtr after;
    if(actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER)
        after = _zone->GetSection(dest);
    else
        after = _zone->GetSection(dest);

    if (now != after)
    {
        now->Remove(actor);
        after->Add(actor);
    }

    actor->ActorInfo.mutable_posinfo()->set_posx(dest.x);
    actor->ActorInfo.mutable_posinfo()->set_posy(dest.y);
    return true;
}

std::vector<Vector2Int> Map::FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool CheckActor)
{
	int32 _deltaY[] = { 1, -1, 0, 0 };
	int32 _deltaX[] = { 0, 0, -1, 1 };

    std::vector<Pos> path;

	std::vector<std::vector<bool>> closed(MaxY - MinY + 1, std::vector<bool>(MaxX - MinX + 1, false));
    std::vector<std::vector<int32>> best(MaxY - MinY + 1, std::vector<int32>(MaxX - MinX + 1, INT32_MAX));
    std::vector<std::vector<int32>> opened(MaxY - MinY + 1, std::vector<int32>(MaxX - MinX + 1, INT32_MAX));
    std::map<Pos, Pos> parent;

    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;

    Pos pos = Cell2Pos(startCellPos);
    Pos dest = Cell2Pos(destCellPos);

    int32 g = 0;
    int32 h = 10 * (abs(dest.y - pos.y) + abs(dest.x - pos.x));
    opened[pos.y][pos.x] = h;

    pq.push({ h, 0, pos });
    parent[pos] = pos;

    while (pq.size() > 0)
    {
        PQNode node = pq.top();
        pq.pop();
        if(closed[node.pos.y][node.pos.x] == true)
            continue;

        closed[node.pos.y][node.pos.x] = true;
        if(node.pos.y == dest.y && node.pos.x == dest.x)
            break;

        for (uint32 i = 0; i < ARRAY_CNT(_deltaY); i++)
        {
            Pos next = Pos(node.pos.y + _deltaY[i], node.pos.x + _deltaX[i]);
            if (next.y != dest.y || next.x != dest.x)
            {
                if(CanGo(Pos2Cell(next), CheckActor) == false)
                    continue;
            }

            if(closed[next.y][next.x])
                continue;

            g = 0;
            h = 10 * ((dest.y - next.y) * (dest.y - next.y) + (dest.x - next.x) * (dest.x - next.x));
            if(opened[next.y][next.x] < g + h)
                continue;

            opened[dest.y][dest.x] = g + h;

            pq.push({ g + h, g, next });
            parent[next] = node.pos;
        }
    }

    return CalcCellPathFromParent(parent, dest);
}

std::vector<Vector2Int> Map::CalcCellPathFromParent(std::map<Pos, Pos> parent, Pos dest)
{
    std::vector<Vector2Int> cells = std::vector<Vector2Int>();
    Pos pos = dest;
    while (true)
    {
		if (pos == parent[pos])
			break;

        cells.push_back(Pos2Cell(pos));
        pos = parent[pos];
    }
    cells.push_back(Pos2Cell(pos));
    
    std::reverse(std::begin(cells), std::end(cells));
    return cells;
}

Pos Map::Cell2Pos(Vector2Int cell)
{
 //   int32 posY = cell.y;
 //   if (posY < 0)
 //       posY = 0;
 //   if(posY > MaxY - MinY)
 //       posY = MaxY - MinY;

 //   int32 posX = cell.x;
	//if (posX < 0)
	//	posX = 0;
	//if (posX > MaxX - MinX)
	//	posX = MaxX - MinX;

 //   return Pos(posY, posX);
    return Pos(MaxY - cell.y, cell.x - MinX);
}

Vector2Int Map::Pos2Cell(Pos pos)
{
    return Vector2Int(pos.x + MinX, MaxY - pos.y);
}
