#pragma once
#pragma comment(lib, "pluginsdk.lib")

#define LINMATH_H //Conflicts with linmath.h if we done declare this here

#include <bakkesmod/plugin/bakkesmodplugin.h>

#include "pch.h"
#include "ReplayHitboxPlugin.h"
#include "Hitbox.h"
#include <bakkesmod/wrappers/GameEvent/ServerWrapper.h>
#include <bakkesmod/wrappers/GameObject/BallWrapper.h>
#include <bakkesmod/wrappers/GameObject/CarWrapper.h>
#include <sstream>
#include <vector>



BAKKESMOD_PLUGIN(ReplayHitboxPlugin, "Show hitboxes in replays", plugin_version, PLUGINTYPE_REPLAY)

ReplayHitboxPlugin::ReplayHitboxPlugin()
{

}


ReplayHitboxPlugin::~ReplayHitboxPlugin()
{
}


void ReplayHitboxPlugin::onLoad()
{
	hitboxOn = std::make_shared<bool>(true);
	cvarManager->registerCvar("replay_showhitbox", "0", "Show Hitbox", true, true, 0, true, 1).bindTo(hitboxOn);
	cvarManager->getCvar("replay_showhitbox").addOnValueChanged(std::bind(&ReplayHitboxPlugin::OnHitboxOnValueChanged, this, std::placeholders::_1, std::placeholders::_2));

	gameWrapper->HookEvent("Function TAGame.Replay_TA.Tick", bind(&ReplayHitboxPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Replay_TA.StopPlayback", bind(&ReplayHitboxPlugin::OnFreeplayDestroy, this, std::placeholders::_1));

}

void ReplayHitboxPlugin::OnFreeplayLoad(std::string eventName)
{
	// draw the car's hitboxes
	if (*hitboxOn) {
		gameWrapper->RegisterDrawable(std::bind(&ReplayHitboxPlugin::Render, this, std::placeholders::_1));
	}
}

void ReplayHitboxPlugin::OnFreeplayDestroy(std::string eventName)
{
	gameWrapper->UnregisterDrawables();
}

void ReplayHitboxPlugin::OnHitboxOnValueChanged(std::string oldValue, CVarWrapper cvar)
{
	if (cvar.getBoolValue() && gameWrapper->IsInReplay()) {
		OnFreeplayLoad("Load");
	}
	else
	{
		OnFreeplayDestroy("Destroy");
	}
}

#include <iostream>     // std::cout
#include <fstream> 

Vector Rotate(Vector aVec, double roll, double yaw, double pitch)
{

	// this rotate is kind of messed up, because UE's xyz coordinates didn't match the axes i expected
   /*
   float sx = sin(pitch);
   float cx = cos(pitch);
   float sy = sin(yaw);
   float cy = cos(yaw);
   float sz = sin(roll);
   float cz = cos(roll);
   */
	float sx = sin(roll);
	float cx = cos(roll);
	float sy = sin(yaw);
	float cy = cos(yaw);
	float sz = sin(pitch);
	float cz = cos(pitch);

	aVec = Vector(aVec.X, aVec.Y * cx - aVec.Z * sx, aVec.Y * sx + aVec.Z * cx);  //2  roll?


	aVec = Vector(aVec.X * cz - aVec.Y * sz, aVec.X * sz + aVec.Y * cz, aVec.Z); //1   pitch?
	aVec = Vector(aVec.X * cy + aVec.Z * sy, aVec.Y, -aVec.X * sy + aVec.Z * cy);  //3  yaw?

	// ugly fix to change coordinates to Unreal's axes
	float tmp = aVec.Z;
	aVec.Z = aVec.Y;
	aVec.Y = tmp;
	return aVec;
}


void ReplayHitboxPlugin::Render(CanvasWrapper canvas)
{
	if (*hitboxOn && gameWrapper->IsInReplay())
	{

		ServerWrapper game = gameWrapper->GetGameEventAsReplay();

		if (game.IsNull())
			return;
		ArrayWrapper<CarWrapper> cars = game.GetCars();

		for (int i = 0; i < cars.Count(); i++) {
			CarWrapper car = cars.Get(i);
			if (car.IsNull())
				return;
		
			// Gets hitbox with some cached values
			//std::vector<Vector> hitbox = CarManager::getHitboxPoints(static_cast<CARBODY>(car.GetLoadoutBody()), *gameWrapper, i);

			// This is the most accurate way to get hitboxes
			Vector extent = car.GetLocalCollisionExtent();
			Vector offset = car.GetLocalCollisionOffset();
			Hitbox* hb = new Hitbox(extent.X, extent.Y, extent.Z, offset.X, offset.Y, offset.Z);
			std::vector<Vector> hitbox;
			hb->getPoints(hitbox);
			delete hb;

			canvas.SetColor(255, 255, 0, 200);

			Vector v = car.GetLocation();
			Rotator r = car.GetRotation();

			double dPitch = (double)r.Pitch / 32764.0 * 3.14159;
			double dYaw = (double)r.Yaw / 32764.0 * 3.14159;
			double dRoll = (double)r.Roll / 32764.0 * 3.14159;

			Vector2 carLocation2D = canvas.Project(v);
			Vector2 hitbox2D[8];
			for (int i = 0; i < 8; i++) {
				hitbox2D[i] = canvas.Project(Rotate(hitbox[i], dRoll, -dYaw, dPitch) + v);
			}

			canvas.DrawLine(hitbox2D[0], hitbox2D[1]);
			canvas.DrawLine(hitbox2D[1], hitbox2D[2]);
			canvas.DrawLine(hitbox2D[2], hitbox2D[3]);
			canvas.DrawLine(hitbox2D[3], hitbox2D[0]);
			canvas.DrawLine(hitbox2D[4], hitbox2D[5]);
			canvas.DrawLine(hitbox2D[5], hitbox2D[6]);
			canvas.DrawLine(hitbox2D[6], hitbox2D[7]);
			canvas.DrawLine(hitbox2D[7], hitbox2D[4]);
			canvas.DrawLine(hitbox2D[0], hitbox2D[4]);
			canvas.DrawLine(hitbox2D[1], hitbox2D[5]);
			canvas.DrawLine(hitbox2D[2], hitbox2D[6]);
			canvas.DrawLine(hitbox2D[3], hitbox2D[7]);

			canvas.SetPosition(carLocation2D.minus((Vector2{ 10,10 })));
			canvas.FillBox((Vector2{ 20, 20 }));
		}
	}
}

void ReplayHitboxPlugin::onUnload()
{
}