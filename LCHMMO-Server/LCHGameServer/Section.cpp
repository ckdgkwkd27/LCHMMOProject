#include "pch.h"
#include "Section.h"
#include "Viewport.h"
#include "ClientPacketHandler.h"

Section::Section(int32 x, int32 y)
{
	this->indexX = x;
	this->indexY = y;
}

void Section::Add(ActorPtr actor)
{
	RecursiveLockGuard guard(sectionLock);
	actorVector.push_back(actor);
}

void Section::Remove(ActorPtr actor)
{
	RecursiveLockGuard guard(sectionLock);
	actorVector.erase(std::remove(actorVector.begin(), actorVector.end(), actor), actorVector.end());
}

ActorPtr Section::FindActor(ActorIDType _actorID)
{
	auto it = std::find_if(actorVector.begin(), actorVector.end(), [_actorID](ActorPtr _actor) {
		if (_actor == nullptr) CRASH_ASSERT(false);
		return _actor->ActorInfo.actorid() == _actorID;
	});

	if (it == actorVector.end())
		return nullptr;

	return *it;
}

PlayerPtr Section::FindPlayerInCondition(std::function<bool(ActorPtr)> _condition)
{
	for (ActorPtr _actor : actorVector)
	{
		if (_actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER)
		{
			if (_condition(_actor) == true)
			{
				return std::dynamic_pointer_cast<Player>(_actor);
			}
		}
	}
	return nullptr;
}

bool Section::PlayerViewportUpdate()
{
	for (auto _actor : this->actorVector)
	{
		if (_actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER && _actor != nullptr)
		{
			TimeStampType currTimeStamp = TimeUtil::GetCurrTimeStamp();
			if (currTimeStamp > _actor->LastViewportUpdateTimeStamp + 100ms)
			{
				_actor->ViewportUpdate();
			}
		}
	}
	return true;
}