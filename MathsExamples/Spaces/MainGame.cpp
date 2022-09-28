#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;
constexpr int LOCAL_GRID_SIZE = 192;
constexpr int WORLD_GRID_SIZE = 48;
constexpr int CAMERA_GRID_SIZE = 48;

// Handles the update and draw of the application
void UpdateApplication();

enum CoordinateSpace
{
	SPACE_LOCAL = 0,
	SPACE_WORLD,
	SPACE_CAMERA
};

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
	int id = -1;
	GameObjectType type = OBJ_NONE;
	Matrix2D world;
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

static std::vector< GameObject > g_objects =
{
	{ 0, OBJ_SHIP, MatrixIdentity() },
	{ 1, OBJ_LASER, MatrixIdentity() },
	{ 2, OBJ_METEOR1, MatrixIdentity() },
	{ 3, OBJ_METEOR2, MatrixIdentity() },
	{ 4, OBJ_METEOR3, MatrixIdentity() },
	{ 5, OBJ_SAUCER, MatrixIdentity() },
	{ 6, OBJ_BOMB, MatrixIdentity() },
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

	for( GameObject& obj : g_objects )
		obj.world.row[2] = { ( obj.id * 2.0f ) + 1.0f, ( obj.id * 2.0f ) + 1.0f , 1.0f };

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
	static Axes localAxes =
	{
		LOCAL_GRID_SIZE, // gridSize
		{ LOCAL_GRID_SIZE * 2, LOCAL_GRID_SIZE * 2 }, // origin
		{ -1.75f, -1.75f }, // axisMin
		{ 1.75f, 1.75f }, // axisMax
		{ -1.5f, -1.5f }, // labelsMin
		{ 1.5f, 1.5f }, // labelsMax
		{ "-1.5", "-1.0", "-0.5", "0", "0.5", "1.0", "1.5" }, // vXLabels
		{ "-1.5", "-1.0", "-0.5", "", "0.5", "1.0", "1.5" } // vYLabels
	};

	static Axes worldAxes =
	{
		WORLD_GRID_SIZE, // gridSize
		{ WORLD_GRID_SIZE, WORLD_GRID_SIZE * 2 }, // origin
		{ -0.75f, -0.75f }, // axisMin
		{ 14.5f, 13.0f }, // axisMax
		{ 0, 0 }, // labelsMin
		{ 14, 12 }, // labelsMax
		{ "0", "2", "4", "6", "8", "10", "12", "14" }, // vXLabels
		{ "", "2", "4", "6", "8", "10", "12" } // vYLabels
	};

	static Axes cameraAxes =
	{
		CAMERA_GRID_SIZE, // gridSize
		{ CAMERA_GRID_SIZE * 8, CAMERA_GRID_SIZE * 13 }, // origin
		{ -7.5f, -11.0f }, // axisMin
		{ 7.5f, 2.0f }, // axisMax
		{ -6, -10 }, // labelsMin
		{ 6, 2 }, // labelsMax
		{ "-6", "-4", "-2", "0", "2", "4", "6" }, // vXLabels
		{ "-10", "-8", "-6", "-4", "-2", "", "2" } // vYLabels
	};

	static CoordinateSpace space = SPACE_LOCAL;
	static int selected_object = 0;

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();
	Point2f gridMousePos = mousePos;

	switch( space )
	{
		case SPACE_LOCAL:
			gridMousePos = ( ( ( ( mousePos - localAxes.origin ) * 4 ) + Vector2f( LOCAL_GRID_SIZE / 2, LOCAL_GRID_SIZE / 2 ) ) / LOCAL_GRID_SIZE );
			gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) ) / 4.0f;

			break;
		case SPACE_WORLD:
			gridMousePos = ( ( ( mousePos - worldAxes.origin ) + Vector2f( WORLD_GRID_SIZE / 2, WORLD_GRID_SIZE / 2 ) ) / WORLD_GRID_SIZE );
			gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) );

			if( Play::KeyPressed( '2' ) )
				g_objects[selected_object].world.row[2] = { gridMousePos.x, gridMousePos.y, 1.0f };

			if( Play::KeyPressed( '0' ) )
				g_objects[selected_object].world.row[0] = gridMousePos - g_objects[selected_object].world.row[2].As2D();

			if( Play::KeyPressed( '1' ) )
				g_objects[selected_object].world.row[1] = gridMousePos - g_objects[selected_object].world.row[2].As2D();

			if( Play::KeyPressed( 'N' ) )
			{
				g_objects[selected_object].world.row[0] = normalize( g_objects[selected_object].world.row[0] );
				g_objects[selected_object].world.row[1] = normalize( g_objects[selected_object].world.row[1] );
			}
			break;

		case SPACE_CAMERA:
			gridMousePos = ( ( ( mousePos - cameraAxes.origin ) + Vector2f( CAMERA_GRID_SIZE / 2, CAMERA_GRID_SIZE / 2 ) ) / CAMERA_GRID_SIZE );
			gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) );
			break;
	}

	if( Play::KeyPressed( VK_LEFT ) )
		selected_object--;

	if( Play::KeyPressed( VK_RIGHT ) )
		selected_object++;

	if( selected_object < 0 )
		selected_object = 0;

	if( selected_object > g_objects.size() - 1 )
		selected_object = g_objects.size() - 1;

	if( Play::KeyPressed( 'L' ) )
		space = SPACE_LOCAL;

	if( Play::KeyPressed( 'W' ) )
		space = SPACE_WORLD;

	if( Play::KeyPressed( 'C' ) )
		space = SPACE_CAMERA;

	static bool swap_order = false;

	if( Play::KeyPressed( 'S' ) )
		swap_order = !swap_order;

	Play::ColourSprite( "36px", Play::cBlack );


	Matrix2D matrix = MatrixIdentity();

	float grid_size = 1;

	switch( space )
	{
		case SPACE_LOCAL:
		{
			DrawAxes( localAxes, "36px" );
			Play::DrawFontText( "36px", "LOCAL SPACE", { 20 ,20 }, Play::LEFT );
			Play::DrawFontText( "36px", "______________", { 20 ,25 }, Play::LEFT );
			Play::cameraPos = -localAxes.origin;
			grid_size = LOCAL_GRID_SIZE;

			Shape shape = g_shapes[g_objects[selected_object].type]; // Make a copy so we can transform the data 

			for( size_t i = 0; i < shape.vertices.size() - 1; ++i )
				Play::DrawSpriteLine( shape.vertices[i] * LOCAL_GRID_SIZE, shape.vertices[i + 1] * LOCAL_GRID_SIZE, "pen2px", Play::cBlack );

			break;
		}
		case SPACE_WORLD:
		{
			DrawAxes( worldAxes, "36px" );
			Play::DrawFontText( "36px", "WORLD SPACE", { 20 ,20 }, Play::LEFT );
			Play::DrawFontText( "36px", "______________", { 20 ,25 }, Play::LEFT );
			Play::cameraPos = -worldAxes.origin;
			grid_size = WORLD_GRID_SIZE;

			for( GameObject& obj : g_objects )
			{
				Shape shape = g_shapes[obj.type]; // Make a copy so we can transform the data 

				for( Point2f& p : shape.vertices )
					p = obj.world.Transform( p );

				for( size_t i = 0; i < shape.vertices.size() - 1; ++i )
					Play::DrawSpriteLine( shape.vertices[i] * WORLD_GRID_SIZE, shape.vertices[i + 1] * WORLD_GRID_SIZE, "pen2px", selected_object == obj.id ? Play::cYellow : Play::cBlack );
			}
			matrix = g_objects[selected_object].world;
			break;
		}
		case SPACE_CAMERA:
		{
			DrawAxes( cameraAxes, "36px" );
			Play::DrawFontText( "36px", "CAMERA SPACE", { 20 ,20 }, Play::LEFT );
			Play::DrawFontText( "36px", "______________", { 20 ,25 }, Play::LEFT );
			Play::cameraPos = -cameraAxes.origin;
			grid_size = CAMERA_GRID_SIZE;

			Matrix2D camera = g_objects[selected_object].world;
			camera.Inverse();

			for( GameObject& obj : g_objects )
			{
				Shape shape = g_shapes[obj.type]; // Make a copy so we can transform the data 

				Matrix2D worldCamera = camera * obj.world;

				if( swap_order )
					worldCamera = obj.world * camera;

				for( Point2f& p : shape.vertices )
					p = worldCamera.Transform( p );

				for( size_t i = 0; i < shape.vertices.size() - 1; ++i )
					Play::DrawSpriteLine( shape.vertices[i] * WORLD_GRID_SIZE, shape.vertices[i + 1] * WORLD_GRID_SIZE, "pen2px", selected_object == obj.id ? Play::cYellow : Play::cBlack );
			}
			matrix = camera;
			break;
		}
	}



	std::stringstream ss; // Used for forcing precision of numbers in strings

	if( space != SPACE_LOCAL )
	{
		if( length( matrix.row[0] ) > 0 )
			DrawSpriteArrow( matrix.row[2].As2D() * grid_size, ( matrix.row[2] + matrix.row[0] ).As2D() * grid_size, "pen2px", Play::cRed );

		if( length( matrix.row[1] ) > 0 )
			DrawSpriteArrow( matrix.row[2].As2D() * grid_size, ( matrix.row[2] + matrix.row[1] ).As2D() * grid_size, "pen2px", Play::cGreen );

		if( matrix.row[2].As2D() != Point2f( 0, 0 ) )
			DrawSpriteArrow( { 0, 0 }, matrix.row[2].As2D() * grid_size, "pen2px", Play::cWhite );

		Play::cameraPos = { 0, 0 };
		Play::ColourSprite( "36px", Play::cRed );

		if( swap_order && space == SPACE_CAMERA )
			Play::DrawFontText( "36px", "(SWAPPED!)", { 240 ,20 }, Play::LEFT );

		ss.str( "" );
		ss << "R0={ ";
		ss << std::fixed << std::setprecision( 2 ) << matrix.row[0].x << ", " << matrix.row[0].y << ", " << matrix.row[0].w << " }";
		Play::DrawFontText( "36px", ss.str(), { 440,685.0f }, Play::LEFT );

		Play::ColourSprite( "36px", Play::cGreen );
		ss.str( "" );
		ss << "R1={ ";
		ss << std::fixed << std::setprecision( 2 ) << matrix.row[1].x << ", " << matrix.row[1].y << ", " << matrix.row[1].w << " }";
		Play::DrawFontText( "36px", ss.str(), { 440, 715.0f }, Play::LEFT );

		Play::ColourSprite( "36px", Play::cWhite );
		ss.str( "" );
		ss << "R2={ ";
		ss << std::fixed << std::setprecision( 2 ) << matrix.row[2].x << ", " << matrix.row[2].y << ", " << matrix.row[2].w << " }";
		Play::DrawFontText( "36px", ss.str(), { 440, 745.0f }, Play::LEFT );
	}

	Play::cameraPos = { 0, 0 };

	Play::ColourSprite( "36px", Play::cBlack);
	ss.str( "" );
	ss << "Mouse = { ";
	ss << std::fixed << std::setprecision( 2 ) << gridMousePos.x << ", " << gridMousePos.y << " }";
	Play::DrawFontText( "36px", ss.str(), { 420, 20.0f }, Play::LEFT );
}
