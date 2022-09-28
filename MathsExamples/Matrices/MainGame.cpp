#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;
constexpr int GRID_SIZE = 192;

// Handles the update and draw of the application
void UpdateApplication();

struct Shape
{
	std::vector< Point2f > vertices;
};

enum GameObjectType
{
	OBJ_NONE = -1,
	OBJ_SHIP = 0,
	OBJ_LASER,
	OBJ_METEOR1,
	OBJ_METEOR2,
	OBJ_METEOR3,
	OBJ_SAUCER,
	OBJ_BOMB
};

struct GameObject
{
	GameObjectType type = OBJ_NONE;
	Matrix2D m;
};

std::vector< Shape > g_shapes =
{
	{{ { 0, -2 }, { -1, 1 }, { 1, 1 }, { 0, -2 } }}, // player ship
	{{ { 0, 1 }, { 0, -1 } }}, // laser
	{{ { 0, -4 }, { 3, -3 }, {4, -1}, { 3, 0 }, { 4, 1 }, { 2, 3 }, { 2, 4 }, {-1, 3 }, {-3, 3 }, {-4, 0 }, {-3,-3 }, {-1,-2 },{0,-4} }}, // large asteroid
	{{ { 0, -3 }, { 2, -2 }, { 3, 0 }, { 2, 1 }, { 2, 2 }, { 0, 3 }, {-1, 3 }, {-2, 1 }, {-3, 0 }, {-2, -1}, {-2,-2 }, { 0,-3 } }}, // medium asteroid
	{{ { 0, -2 }, { 1, -2 }, { 2, 0 }, { 0, 2 }, { -2, 1 }, { -1, 0 }, {-2, -1 }, {0, -2 } }}, // small asteroid
	{{ { -2, -0.5f }, { 0, -1.5f }, { 2, -0.5f }, { -2, -0.5f }, { -4, 0.5f }, { -2, 0 }, { 2, 0 }, { 4, 0.5f }, { 2, -0.5f } }}, // saucer
	{{ { 0, -0.5f }, { 0.5f, 0 }, { 0, 0.5f }, { -0.5f, 0 }, { 0, -0.5f } }}, // bomb
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

	for( Shape& s : g_shapes )
		for( Point2f& p : s.vertices )
			p /= 4;

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
	Axes localAxes;
	localAxes.gridSize = GRID_SIZE;
	localAxes.origin = { GRID_SIZE * 2, GRID_SIZE * 2 };
	localAxes.axisMin = { -1.75f, -1.75f };
	localAxes.axisMax = { 1.75f, 1.75f };
	localAxes.labelsMin = { -1.5f, -1.5f };
	localAxes.labelsMax = { 1.5f, 1.5f };
	localAxes.vXLabels = { "-1.5", "-1.0", "-0.5", "0", "0.5", "1.0", "1.5" };
	localAxes.vYLabels = { "-1.5", "-1.0", "-0.5", "0", "0.5", "1.0", "1.5" };

	static int selected_object = 0;

	static std::vector< GameObject > objects =
	{
		{ OBJ_SHIP, MatrixIdentity() },
		{ OBJ_LASER, MatrixIdentity() },
		{ OBJ_METEOR1, MatrixIdentity() },
		{ OBJ_METEOR2, MatrixIdentity() },
		{ OBJ_METEOR3, MatrixIdentity() },
		{ OBJ_SAUCER, MatrixIdentity() },
		{ OBJ_BOMB, MatrixIdentity() },
	};

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();

	Point2f gridMousePos = (( ((mousePos - localAxes.origin)*4) + Vector2f( GRID_SIZE / 2, GRID_SIZE / 2 ) ) / GRID_SIZE);
	gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) ) / 4.0f;

	if( Play::KeyPressed( '2' ) )
		objects[selected_object].m.row[2] = { gridMousePos.x, gridMousePos.y, 1.0f };

	if( Play::KeyPressed( '0' ) )
		objects[selected_object].m.row[0] = gridMousePos - objects[selected_object].m.row[2].As2D();

	if( Play::KeyPressed( '1' ) )
		objects[selected_object].m.row[1] = gridMousePos - objects[selected_object].m.row[2].As2D();

	if( Play::KeyPressed( 'N' ) )
	{
		objects[selected_object].m.row[0] = normalize( objects[selected_object].m.row[0] );
		objects[selected_object].m.row[1] = normalize( objects[selected_object].m.row[1] );
	}

	if( Play::KeyPressed( VK_LEFT ) )
		selected_object--;

	if( Play::KeyPressed( VK_RIGHT ) )
		selected_object++;

	if( selected_object < 0 )
		selected_object = 0;

	if( selected_object > objects.size() - 1 )
		selected_object = objects.size() - 1;

	Play::ColourSprite( "36px", Play::cBlack );

	Play::cameraPos = { 0, 0 };

	DrawAxes( localAxes, "36px" );
	Play::cameraPos = -localAxes.origin;
	Shape shape = g_shapes[ objects[selected_object].type ]; // Make a copy so we can transform the data 
	Matrix2D& matrix = objects[selected_object].m;

	for( Point2f& p : shape.vertices )
		p = matrix.Transform( p );

	for( size_t i = 0; i < shape.vertices.size() - 1; ++i )
		Play::DrawSpriteLine( shape.vertices[i] * GRID_SIZE, shape.vertices[i + 1] * GRID_SIZE, "pen2px", Play::cBlack );
	
	if( length( matrix.row[0] ) > 0 )
		DrawSpriteArrow( matrix.row[2].As2D() * GRID_SIZE, (matrix.row[2] + matrix.row[0]).As2D() * GRID_SIZE, "pen2px", Play::cRed );
	
	if( length( matrix.row[1] ) > 0 )
		DrawSpriteArrow( matrix.row[2].As2D() * GRID_SIZE, ( matrix.row[2] + matrix.row[1] ).As2D() * GRID_SIZE, "pen2px", Play::cGreen );
	
	if( matrix.row[2].As2D() != Point2f( 0, 0 ) )
		DrawSpriteArrow( { 0, 0 }, matrix.row[2].As2D() * GRID_SIZE, "pen2px", Play::cWhite );


	Play::cameraPos = { 0, 0 };


	std::stringstream ss; // Used for forcing precision of numbers in strings
	// Draw the title
	Play::ColourSprite( "36px", Play::cRed );
	ss.str( "" );
	ss << "R0={ ";
	ss << std::fixed << std::setprecision( 2 ) << matrix.row[0].x << ", " << matrix.row[0].y << ", " << matrix.row[0].w << " }";
	Play::DrawFontText( "36px", ss.str(), { 20, 30.0f }, Play::LEFT );

	Play::ColourSprite( "36px", Play::cGreen );
	ss.str( "" );
	ss << "R1={ ";
	ss << std::fixed << std::setprecision( 2 ) << matrix.row[1].x << ", " << matrix.row[1].y << ", " << matrix.row[1].w << " }";
	Play::DrawFontText( "36px", ss.str(), { 20, 60.0f }, Play::LEFT );

	Play::ColourSprite( "36px", Play::cWhite );
	ss.str( "" );
	ss << "R2={ ";
	ss << std::fixed << std::setprecision( 2 ) << matrix.row[2].x << ", " << matrix.row[2].y << ", " << matrix.row[2].w << " }";
	Play::DrawFontText( "36px", ss.str(), { 20, 90.0f }, Play::LEFT );
	Play::DrawFontText( "36px", "0/1/2 Copy Mouse to Matrix Row L/R Change Shape", { DISPLAY_WIDTH / 2 , DISPLAY_HEIGHT - 25.0f }, Play::CENTRE );

	Play::ColourSprite( "36px", Play::cBlack);
	ss.str( "" );
	ss << "Mouse = { ";
	ss << std::fixed << std::setprecision( 2 ) << gridMousePos.x << ", " << gridMousePos.y << " }";
	Play::DrawFontText( "36px", ss.str(), { 420, 30.0f }, Play::LEFT );
}
