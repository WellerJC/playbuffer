#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "..\CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;
constexpr int GRID_SIZE = 48;
constexpr float ARC_INCREMENT = 0.0001f;


// Handles the update and draw of the application
void UpdateApplication();


// The standard entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );

	Play::LoadBackground( "Data\\Backgrounds\\maths_bg768sq.png" );

	// Certain sprites need their origins to be in their centres
	Play::CentreAllSpriteOrigins();

	// Changes the colour of the font sprites - slow, so not recommended in your update loop
	Play::ColourSprite( "36px", Play::cBlack );
	Play::ColourSprite( "54px", Play::cBlack );
}

// Called once every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();
	UpdateApplication();
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}


// Handles the update and draw of the application
void UpdateApplication()
{
	// Initialise the axes positions and labels
	Axes mainAxes;
	mainAxes.gridSize = GRID_SIZE;
	mainAxes.origin = { GRID_SIZE * 8, GRID_SIZE * 8 };
	mainAxes.axisMin = { -5.0f, -5.0f };
	mainAxes.axisMax = { 5.0f, 5.0f };
	mainAxes.labelsMin = { -5, -5 };
	mainAxes.labelsMax = { 5, 5 };
	mainAxes.vXLabels = { "-1.0", "-0.5", "0", "0.5", "1.0" };
	mainAxes.vYLabels = { "-1.0", "-0.5", "0", "0.5", "1.0" };

	// Static variables which are persistent between calls
	Point2f unitVector = { 0, 1 };
	Point2f newUnitVector = { 0, 1 };
	static float radians = 0.0f;

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();

	Play::ColourSprite( "36px", Play::cBlack );

	if( Play::KeyPressed( VK_RIGHT ) )
		radians += ( 2 * PLAY_PI ) / 32;

	if( Play::KeyPressed( VK_LEFT ) )
		radians -= ( 2 * PLAY_PI ) / 32;

	unitVector = { 0, -1 };
	Vector2f clockwiseVec = { 1, 0 };
	float r = 0.0f;
	while( r < radians )
	{
		clockwiseVec = { -unitVector.y, unitVector.x };
		Point2f newVector = unitVector + (clockwiseVec * ARC_INCREMENT);
		newVector = normalize( newVector );
		r += length( newVector - unitVector );
		unitVector = newVector;

		float rcapped = r;

		if( rcapped > 2 * PLAY_PI )
			rcapped -= 2 * PLAY_PI;

		if( rcapped < 2 || ( rcapped > 4 && rcapped < 6 ) )
			Play::ColourSprite( "pen2px", Play::cBlue );
		else 
			Play::ColourSprite( "pen2px", Play::cWhite );

		Play::DrawSprite( Play::GetSpriteId( "pen2px" ), mainAxes.origin + ( unitVector * GRID_SIZE * 5), 0 );
	}

	DrawAxes( mainAxes, "36px" );

	DrawSpriteArrow( mainAxes.origin, mainAxes.origin + ( unitVector * GRID_SIZE * 5 ), "pen2px", Play::cYellow );
	DrawSpriteArrow( mainAxes.origin + ( unitVector * GRID_SIZE * 5 ), mainAxes.origin + ( unitVector * GRID_SIZE * 5 ) + ( clockwiseVec * GRID_SIZE * 3), "pen2px", Play::cRed );

	// Draw the title
	Play::DrawFontText( "54px", "Circles and Radians", { DISPLAY_WIDTH / 2, GRID_SIZE / 2 }, Play::CENTRE );
	Play::DrawFontText( "54px", "________________", { DISPLAY_WIDTH / 2,  (GRID_SIZE / 2) + 8 }, Play::CENTRE );

	std::stringstream ss; // Used for forcing precision of numbers in strings
	ss.str( "" );
	ss << "V = { ";
	ss << ( unitVector.x < 0.0f ? "-" : "" ) << std::fixed << std::setprecision( 3 ) << abs( unitVector.x ) << ", ";
	ss << ( unitVector.y < 0.0f ? "-" : "" ) << std::fixed << std::setprecision( 3 ) << abs( unitVector.y ) << " }";
	Play::DrawFontText( "36px", ss.str(), { 50, 90.0f }, Play::LEFT );

	ss.str( "" );
	ss << "Arc Distance = ";
	ss << std::fixed << std::setprecision( 3 ) << abs( r );

	Play::DrawFontText( "36px", ss.str() , { 50, 700.0f }, Play::LEFT );

	//// Draw the remaining text
	Play::ColourSprite( "36px", Play::cWhite );
	Play::DrawFontText( "36px", "Use left and right arrows to increase/decrease angle", { DISPLAY_WIDTH / 2 , DISPLAY_HEIGHT - 25.0f }, Play::CENTRE );
             


}
