#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "MathsExamples/CommonMaths.h"

constexpr int DISPLAY_WIDTH = 720;
constexpr int DISPLAY_HEIGHT = 720;
constexpr int DISPLAY_SCALE = 1;
constexpr int GRID_SIZE = 48;


// Handles the update and draw of the application
void UpdateApplication();


// The standard entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );

	Play::LoadBackground( "Data\\Backgrounds\\maths_bg720sq.png" );

	// Certain sprites need their origins to be in their centres
	Play::CentreSpriteOrigin( "rocket" );
	Play::CentreMatchingSpriteOrigins( "pen" );

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
	Axes circleAxes;
	circleAxes.gridSize = GRID_SIZE;
	circleAxes.origin = { GRID_SIZE * 3.5f, GRID_SIZE * 4 };
	circleAxes.axisMin = { -2.5f, -2.5f };
	circleAxes.axisMax = { 2.5f, 2.5f };
	circleAxes.labelsMin = { -2.0f, -2.0f };
	circleAxes.labelsMax = { 2.0f, 2.0f };
	circleAxes.vXLabels = { "-1", "0", "1" };
	circleAxes.vYLabels = { "-1", "", "1" };

	Axes cosAxes;
	cosAxes.gridSize = GRID_SIZE;
	cosAxes.origin = { GRID_SIZE * 7.5f, GRID_SIZE * 4 };
	cosAxes.axisMin = { -0.5f, -2.5f };
	cosAxes.axisMax = { 6.0f, 2.5f };
	cosAxes.labelsMin = { 0.0f, -2.0f };
	cosAxes.labelsMax = { 6.0f, 2.0f };
	cosAxes.vYLabels = { "-1", "0", "1" };

	Axes sinAxes;
	sinAxes.gridSize = GRID_SIZE;
	sinAxes.origin = { GRID_SIZE * 3.5f, GRID_SIZE * 8 };
	sinAxes.axisMin = { -2.5f, -0.5f };
	sinAxes.axisMax = { 2.5f, 6.0f };
	sinAxes.labelsMin = { -2.0f, 0.0f };
	sinAxes.labelsMax = { 2.0f, 6.0f };
	sinAxes.vXLabels = { "-1", "0", "1" };

	// Static variables which are persistent between calls
	const int circleRadius = GRID_SIZE * 2;
	static Point2f sinPoint = circleAxes.origin;
	static Point2f cosPoint = circleAxes.origin;
	static Vector2f mouseVec = { 0.0f, -1.0f };
	static Vector2f rocketVec = circleAxes.origin + ( circleRadius * mouseVec );
	static float angle = 0.0f;
	static AngleMode angleMode = RADIANS;

	if( Play::KeyDown( 'R' ) )
		angleMode = RADIANS;

	if( Play::KeyDown( 'D' ) )
		angleMode = DEGREES;

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();
	float mouseDist = lengthSqr( mousePos - circleAxes.origin );

	// Check to see if the mouse is in the Rocket's circle
	if( mouseDist > 1.0f && mouseDist < circleRadius * circleRadius )
	{
		// Get the angle between the mouse vector and up (0 degrees)
		mouseVec = normalize( mousePos - circleAxes.origin );
		Vector2f upVec( 0.0f, -1.0f );
		angle = acos( dot( mouseVec, upVec ) ); // dot product cosine rule
		// cosine rule gives smallest angle between point and up (i.e. < 180 degrees) so this needs adjusting for 180-360 degrees
		if( mouseVec.x < 0 ) angle = 2 * PLAY_PI - angle;
		// Note: you could just calculate angle as atan2( pointVec.x, -pointVec.y ) but we're practicing the dot product!

		// Scale up the unit vector to the required circle size
		rocketVec = circleRadius * mouseVec;

		// The cos and sin graphs are drawn using the x and y coordinate at the circumference of the circle
		int offset = ( Play::RadToDeg( angle ) * GRID_SIZE ) / 60;
		sinPoint = { sinAxes.origin.x + rocketVec.x, sinAxes.origin.y + offset };
		cosPoint = { cosAxes.origin.x + offset, cosAxes.origin.y + rocketVec.y };

		// Transpose the rotated vector to the circle origin
		rocketVec += circleAxes.origin;
	}

	// Draw the sine and cosine graphs
	Play::ColourSprite( "pen4px", Play::cYellow );
	for( int i = 0; i < Play::RadToDeg( angle ); i++ )
	{
		Play::DrawSprite( "pen4px", { cosAxes.origin.x + ( ( i * GRID_SIZE ) / 60 ), cosAxes.origin.y - ( circleRadius * cos( i * PLAY_PI / 180 ) ) }, 1 );
		Play::DrawSprite( "pen4px", { sinAxes.origin.x + ( circleRadius * sin( i * PLAY_PI / 180 ) ), sinAxes.origin.y + ( ( i * GRID_SIZE ) / 60 ) }, 1 );
	}
	Play::ColourSprite( "pen4px", Play::cWhite );

	// Draw the rotated rockets
	static int rocketSpriteId = Play::GetSpriteId( "rocket" );
	Play::DrawSpriteRotated( rocketSpriteId, rocketVec, 0, angle, 1.0f, 0.4f );
	Play::DrawSpriteRotated( rocketSpriteId, circleAxes.origin, 0, angle, 1.0f, 1.0f );

	// Draw the rocket's director vector and the rotation circle
	Play::DrawSpriteLine( { circleAxes.origin.x, circleAxes.origin.y }, { rocketVec.x , rocketVec.y }, "pen4px", Play::cYellow );
	Play::DrawSpriteCircle( circleAxes.origin, GRID_SIZE * 2, "pen4px", Play::cYellow );

	// Draw the lines between the rocket's vector and the sine/cosine graphs
	Play::DrawSpriteLine( { rocketVec.x, rocketVec.y }, { cosPoint.x , cosPoint.y }, "pen2px", Play::cBlack );
	Play::DrawSpriteLine( { rocketVec.x, rocketVec.y }, { sinPoint.x , sinPoint.y }, "pen2px", Play::cBlack );
	Play::DrawCircle( { static_cast<int>( cosPoint.x ), static_cast<int>( cosPoint.y ) }, 8, Play::cWhite );
	Play::DrawCircle( { static_cast<int>( sinPoint.x ), static_cast<int>( sinPoint.y ) }, 8, Play::cWhite );

	// Draw the title
	Play::DrawFontText( "54px", "Rotated Movement", { DISPLAY_WIDTH / 2, -5 }, Play::CENTRE );
	Play::DrawFontText( "54px", "________________", { DISPLAY_WIDTH / 2, 3 }, Play::CENTRE );

	const float xCentre = 500.0f;
	const float yStart = 330.0f;

	std::stringstream ss; // Used for forcing precision of numbers in strings

	if( angleMode == DEGREES )
	{
		ss << "Angle = " << std::fixed << std::setprecision( 1 ) << Play::RadToDeg( angle );
		Play::DrawFontText( "54px", ss.str(), { xCentre - 110.0f, yStart }, Play::LEFT );
		cosAxes.vXLabels = { "0", "90", "180", "270", "360" };
		sinAxes.vYLabels = { "0", "90", "180", "270", "360" };
	}
	else
	{
		ss << "Angle = " << std::fixed << std::setprecision( 3 ) << angle / PLAY_PI << "Pi";
		Play::DrawFontText( "54px", ss.str(), { xCentre - 160.0f, yStart }, Play::LEFT );
		cosAxes.vXLabels = std::vector<std::string>{ "0", "0.5Pi", "Pi", "1.5Pi", "2Pi" };
		sinAxes.vYLabels = std::vector<std::string>{ "0", "0.5Pi", "Pi", "1.5Pi", "2Pi" };
	}

	ss.str( "" );
	ss << ( mouseVec.x < 0.0f ? " - " : " + " ) << std::fixed << std::setprecision( 3 ) << abs( mouseVec.x );
	Play::DrawFontText( "36px", "x = x" + ss.str(), { xCentre - 90.0f, yStart + 50.0f }, Play::LEFT );

	ss.str( "" );
	ss << ( mouseVec.y < 0.0f ? " - " : " + " ) << std::fixed << std::setprecision( 3 ) << abs( mouseVec.y );

	// Draw the remaining text
	Play::DrawFontText( "36px", "y = y" + ss.str(), { xCentre - 90.0f, yStart + 80.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "_______", { xCentre, yStart + 100.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "How much do we add", { xCentre, yStart + 153.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "to x and y to move the", { xCentre, yStart + 183.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "rocket forwards in the", { xCentre, yStart + 213.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "direction it is facing?", { xCentre, yStart + 243.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "_______", { xCentre, yStart + 263.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "'R' to use radians", { xCentre, yStart + 300.0f }, Play::CENTRE );
	Play::DrawFontText( "36px", "'D' to use degrees", { xCentre, yStart + 330.0f }, Play::CENTRE );

	// Draw the axis on top of everything else
	DrawAxes( circleAxes, "36px" );
	DrawAxes( sinAxes, "36px" );
	DrawAxes( cosAxes, "36px" );
}
