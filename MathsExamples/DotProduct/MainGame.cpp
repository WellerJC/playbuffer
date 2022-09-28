#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;
constexpr int GRID_SIZE = 48;


// Handles the update and draw of the application
void UpdateApplication();


// The standard entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );

	Play::LoadBackground( "Data\\Backgrounds\\maths_bg768sq.png" );

	// Certain sprites need their origins to be in their centres
	Play::CentreSpriteOrigin( "rocket" );
	Play::CentreMatchingSpriteOrigins( "pen" );
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
	mainAxes.axisMin = { -6.5f, -6.5f };
	mainAxes.axisMax = { 6.5f, 6.5f };
	mainAxes.labelsMin = { -6, -6 };
	mainAxes.labelsMax = { 6, 6 };
	mainAxes.vXLabels = { "-6", "-4", "-2", "0", "2", "4", "6" };
	mainAxes.vYLabels = { "-6", "-4", "-2", "0", "2", "4", "6" };

	// Static variables which are persistent between calls
	static Point2f startPoint = { -3, 1 };
	static Point2f endPoint = { 3, 1 };
	static Point2f testPoint = { 1, 3 };

	static float dist = 0.0f;

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();

	Point2f gridMousePos = ( mousePos - mainAxes.origin + Vector2f( GRID_SIZE / 2, GRID_SIZE / 2 ) ) / GRID_SIZE;
	gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) );



	if( Play::KeyPressed( '1' ) && endPoint != gridMousePos && testPoint != gridMousePos )
		startPoint = gridMousePos;

	if( Play::KeyPressed( '2' ) && startPoint != gridMousePos )
		endPoint = gridMousePos;

	if( Play::GetMouseButton( Play::LEFT ) && startPoint != gridMousePos  )
		testPoint = gridMousePos;

	Vector2f lineVec = endPoint - startPoint;
	Vector2f unitLineVec = normalize( lineVec );
	Vector2f perpendicularVec = { lineVec.y, -lineVec.x };
	Vector2f unitPerpVec = normalize( perpendicularVec );
	Vector2f testVec = testPoint - startPoint;
	float dp = dot( testVec, unitPerpVec );
	float d = dot( testVec, unitPerpVec );

	Play::ColourSprite( "36px", Play::cBlack );
	DrawAxes( mainAxes, "36px" );

	Play::DrawLine( mainAxes.origin + (startPoint * GRID_SIZE) - (lineVec * 1000.0f), mainAxes.origin + ( startPoint * GRID_SIZE ) + (lineVec * 1000.0f ), Play::cWhite );
	DrawSpriteArrow( mainAxes.origin + ( startPoint * GRID_SIZE ), mainAxes.origin + ( endPoint * GRID_SIZE ), "pen4px", Play::cYellow );
	DrawSpriteArrow( mainAxes.origin + (startPoint * GRID_SIZE), mainAxes.origin + (testPoint * GRID_SIZE), "pen4px", Play::cBlue );
	DrawSpriteArrow( mainAxes.origin + (startPoint * GRID_SIZE), mainAxes.origin + ((startPoint + unitPerpVec ) * GRID_SIZE), "pen4px", Play::cBlue );
	if( d != 0 )
		DrawSpriteArrow( mainAxes.origin + (testPoint * GRID_SIZE), mainAxes.origin + ((testPoint - ( unitPerpVec * d))*GRID_SIZE), "pen4px", Play::cBlack );

	Play::DrawSpriteCircle( mainAxes.origin + ( startPoint * GRID_SIZE ), 1, "pen4px", Play::cWhite );
	Play::DrawSpriteCircle( mainAxes.origin + ( startPoint * GRID_SIZE ), 4, "pen4px", Play::cWhite );

	Play::DrawSpriteCircle( mainAxes.origin + ( endPoint * GRID_SIZE ), 1, "pen4px", Play::cWhite );
	Play::DrawSpriteCircle( mainAxes.origin + ( endPoint * GRID_SIZE ), 4, "pen4px", Play::cWhite );

	Play::DrawSpriteCircle( mainAxes.origin + ( testPoint * GRID_SIZE ), 1, "pen4px", Play::cRed );
	Play::DrawSpriteCircle( mainAxes.origin + ( testPoint * GRID_SIZE ), 4, "pen4px", Play::cRed );

	// Draw the title
	Play::DrawFontText( "54px", "Half-space/Distance Test", { DISPLAY_WIDTH / 2, GRID_SIZE / 2 }, Play::CENTRE );
	Play::DrawFontText( "54px", "________________", { DISPLAY_WIDTH / 2,  (GRID_SIZE / 2) + 8 }, Play::CENTRE );

	Play::DrawFontText( "36px", "P1 = { " + std::to_string( (int)startPoint.x ) + ", " + std::to_string( (int)startPoint.y ) + " }", {50, 90.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "P2 = { " + std::to_string( (int)endPoint.x ) + ", " + std::to_string( (int)endPoint.y ) + " }", { 50, 120.0f }, Play::LEFT );

	Play::DrawFontText( "36px", "Test = { " + std::to_string( (int)testPoint.x ) + ", " + std::to_string( (int)testPoint.y ) + " }", { 50, 180.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "Mouse = { " + std::to_string( (int)gridMousePos.x ) + ", " + std::to_string( (int)gridMousePos.y ) + " }", { 50, 700.0f }, Play::LEFT );


	Play::ColourSprite( "36px", Play::cWhite );

	Play::DrawFontText( "36px", "dp = " + std::to_string( dp ), { 50, 210.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "dist = " + std::to_string( d ), { 50, 240.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "Vec = { " + std::to_string( (int)lineVec.x ) + ", " + std::to_string( (int)lineVec.y ) + " }", { 50, 150.0f }, Play::LEFT );

	//// Draw the remaining text
	Play::DrawFontText( "36px", "Press keys 1/2 to set line and LMB for test point", { DISPLAY_WIDTH /2 , DISPLAY_HEIGHT - 25.0f }, Play::CENTRE );

              


}
