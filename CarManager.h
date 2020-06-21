#pragma once
#pragma comment(lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <vector>
#include <string>

class CarManager
{
public:
	CarManager();
	static const std::string getHelpText();
	static const std::vector<Vector> getHitboxPoints(CARBODY car_type, GameWrapper& gameWrapper);
	~CarManager();
};

