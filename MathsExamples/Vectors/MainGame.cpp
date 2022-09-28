#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;
constexpr int GRID_SIZE = 48;


// Handles the update and draw of the application
void UpdateApplication();

enum Operation
{
	OP_ADD = 0,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE
};


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
	mainAxes.axisMin = { -6.5f, -6.5f };
	mainAxes.axisMax = { 6.5f, 6.5f };
	mainAxes.labelsMin = { -6, -6 };
	mainAxes.labelsMax = { 6, 6 };
	mainAxes.vXLabels = { "-6", "-4", "-2", "0", "2", "4", "6" };
	mainAxes.vYLabels = { "-6", "-4", "-2", "0", "2", "4", "6" };

	// Static variables which are persistent between calls
	static std::vector< std::pair < Operation, Vector2f > > operations;
	static std::vector< Vector2f > results;

	static float dist = 0.0f;

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();

	Point2f gridMousePos = ( mousePos - mainAxes.origin + Vector2f( GRID_SIZE / 2, GRID_SIZE / 2 ) ) / GRID_SIZE;
	gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) );

	static Point2f workingVector = { 3, 3 };

	if( Play::KeyPressed( 'A' ) )
		operations.push_back( std::pair( OP_ADD, workingVector ) );

	if( Play::KeyPressed( 'S' ) )
		operations.push_back( std::pair( OP_SUBTRACT, workingVector ) );

	if( Play::KeyPressed( 'M' ) )
		operations.push_back( std::pair( OP_MULTIPLY, workingVector ) );

	if( Play::KeyPressed( 'D' ) )
	{
		if( operations.size() != 0 ) // Divide cannot be the first operation
			operations.push_back( std::pair( OP_DIVIDE, workingVector ) );
	}

	if( Play::KeyPressed( 'C' ) )
		operations.clear();

	if( Play::KeyPressed( 'R' ) )
		std::reverse( operations.begin(), operations.end() );

	if( Play::GetMouseButton( Play::LEFT ) && gridMousePos != Point2f{ 0, 0 } )
		workingVector = gridMousePos;

	Play::ColourSprite( "36px", Play::cBlack );
	DrawAxes( mainAxes, "36px" );

	DrawSpriteArrow( mainAxes.origin, mainAxes.origin + ( workingVector * GRID_SIZE ), "pen4px", Play::cYellow );

	results.clear();
	float multiply = 1.0f;

	// A fairly rudimentary attempt at a left associative calculation of vector operations
	// Multiplications and divides are applied to all the vectors already in the list
	// Special case for multipications at the very start which are apploed to the first vector only

	for( std::pair p : operations )
	{
		if( results.size() == 0 )
		{
			if( p.first == OP_MULTIPLY )
			{
				multiply *= length( p.second );
				results.push_back( { 0, 0 } ); // appears as a cross
				continue; // go to the next operation
			}

			if( p.first == OP_DIVIDE )
			{
				multiply *= 1.0f / length( p.second );
				results.push_back( { 0, 0 } ); // appears as a cross
				continue; // go to the next operation
			}
		}

		switch( p.first )
		{
			case OP_ADD:
				results.push_back( p.second * multiply );
				break;
			case OP_SUBTRACT:
				results.push_back( -p.second );
				break;
			case OP_MULTIPLY:
				for( Vector2f& v : results )
					v *= length( p.second );
				results.push_back( { 0, 0 } ); // appears as a cross
				break;
			case OP_DIVIDE:
				for( Vector2f& v : results )
					v /= length( p.second );
				results.push_back( { 0, 0 } ); // appears as a cross
				break;

		}

		multiply = 1.0f;
	}

	Point2f product = { 0, 0 };
	for( Vector2f& v : results )
	{
		if( v == Vector2f( 0, 0 ) )
			DrawSpriteCross( mainAxes.origin + ( product * GRID_SIZE ), "pen4px", Play::cRed );
		else
			DrawSpriteArrow( mainAxes.origin + ( product * GRID_SIZE), mainAxes.origin + (( product +  v ) * GRID_SIZE), "pen4px", Play::cBlack );

		product += v;
	}

	if( product != Vector2f( 0, 0 ) )
		DrawSpriteArrow( mainAxes.origin , mainAxes.origin + ( product * GRID_SIZE ), "pen4px", Play::cBlue );

	// Draw the title
	Play::DrawFontText( "54px", "Vector Addition/Multiplication", { DISPLAY_WIDTH / 2, GRID_SIZE / 2 }, Play::CENTRE );
	Play::DrawFontText( "54px", "____________________________", { DISPLAY_WIDTH / 2,  (GRID_SIZE / 2) + 5 }, Play::CENTRE );


	Play::ColourSprite( "36px", Play::cYellow );
	Play::DrawFontText( "36px", "Vec = { " + std::to_string( (int)workingVector.x ) + ", " + std::to_string( (int)workingVector.y ) + " }", { 30, 630.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "Scalar = " + std::to_string( length( workingVector ) ), { 30, 660.0f }, Play::LEFT );
	Play::ColourSprite( "36px", Play::cWhite );
	Play::DrawFontText( "36px", "Mouse = { " + std::to_string( (int)gridMousePos.x ) + ", " + std::to_string( (int)gridMousePos.y ) + " }", { 30, 600.0f }, Play::LEFT );


	//// Draw the remaining text
	Play::DrawFontText( "36px", "(A)dd, (S)ubtract, (M)ultiply, (D)ivide, (R)everse, (C)lear", { DISPLAY_WIDTH /2 , DISPLAY_HEIGHT - 25.0f }, Play::CENTRE );

	Play::ColourSprite( "36px", Play::cBlue );

	std::stringstream ss; // Used for forcing precision of numbers in strings
	ss.str( "" );
	ss << "Result = { ";
	ss << ( product.x < 0.0f ? "-" : "" ) << std::fixed << std::setprecision( 3 ) << abs( product.x ) << ", ";
	ss << ( product.y < 0.0f ? "-" : "" ) << std::fixed << std::setprecision( 3 ) << abs( product.y ) << " }";
	Play::DrawFontText( "36px", ss.str(), { 30, 690.0f }, Play::LEFT );



}
