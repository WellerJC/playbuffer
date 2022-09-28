#pragma once

enum AngleMode
{
	DEGREES = 0,
	RADIANS
};

struct Axes
{
	int gridSize{ 1 };
	Vector2f origin{ 0.0f, 0.0f };
	Vector2f axisMin{ -1.0f, -1.0f };
	Vector2f axisMax{ 1.0f, 1.0f };
	Vector2f labelsMin{ -1.0f, -1.0f };
	Vector2f labelsMax{ 1.0f, 1.0f };
	std::vector< std::string > vXLabels;
	std::vector< std::string > vYLabels;
};

void DrawAxes( Axes& a, std::string font );

void DrawSpriteArrow( Vector2f startPoint, Vector2f endPoint, const char* penSprite, Play::Colour c );
void DrawSpriteCross( Vector2f centrePoint, const char* penSprite, Play::Colour c );