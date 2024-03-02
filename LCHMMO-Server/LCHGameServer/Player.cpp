#include "pch.h"
#include "Player.h"
#include "ClientSession.h"
#include "Viewport.h"

void Player::ViewportUpdate()
{
	this->viewport->Update();
}
