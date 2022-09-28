#define PLAY_IMPLEMENTATION
#include "Play.h"
#include< algorithm >

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;
constexpr int GRID_SIZE = 12;

struct TexturedQuad
{
	const PixelData* pixelData{ nullptr };
	Matrix2D m;
	std::vector< Point2f > vertices;
};

void UpdateApplication();
void DrawTexturedQuad( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& offset );
void DrawTexturedQuad_Inverse( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& offset );
void DrawBounds( std::vector< Point2f >& vertices );
void DrawBigPixel( Point2f pos, Pixel colour );
void DrawHighlightPixel( void );


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
	Axes axes =
	{
		GRID_SIZE, // gridSize
		{ GRID_SIZE * 4, GRID_SIZE * 12 }, // origin
		{ -2.0f, -2.0f }, // axisMin
		{ 56.0f, 48.0f }, // axisMax
		{ 0, 0 }, // labelsMin
		{ 50, 40 }, // labelsMax
		{ "0", "10", "20", "30", "40", "50" }, // vXLabels
		{ "", "10", "20", "30", "40" } // vYLabels
	};

	static int sel = 0;
	static bool useInverseMatrix = false;
	static bool rotating = false;

#define W( A )	(Play::GetSpriteWidth( A )-1.0f)
#define H( A )	(Play::GetSpriteHeight( A )-1.0f)
#define W0( A )	Play::GetSpriteWidth( A )
#define H0( A )	Play::GetSpriteHeight( A )

	// Set up from quads based on the first three loaded sprites
	static std::vector< TexturedQuad > quads =
	{
		{ Play::GetSpritePixelData( 0 ), MatrixTranslation( W0( 0 ) / 2, H0( 0 ) / 2 ), { {-W( 0 ) / 2,-H( 0 ) / 2}, {W( 0 ) / 2,-H( 0 ) / 2}, {W( 0 ) / 2,H( 0 ) / 2}, {-W( 0 ) / 2,H( 0 ) / 2}, {-W( 0 ) / 2, -H( 0 ) / 2}} },
		{ Play::GetSpritePixelData( 1 ), MatrixTranslation( W0( 1 ) / 2, H0( 1 ) / 2 ), { {-W( 1 ) / 2,-H( 1 ) / 2}, {W( 1 ) / 2,-H( 1 ) / 2}, {W( 1 ) / 2,H( 1 ) / 2}, {-W( 1 ) / 2,H( 1 ) / 2}, {-W( 1 ) / 2, -H( 1 ) / 2}} },
		{ Play::GetSpritePixelData( 2 ), MatrixTranslation( W0( 2 ) / 2, H0( 2 ) / 2 ), { {-W( 2 ) / 2,-H( 2 ) / 2}, {W( 2 ) / 2,-H( 2 ) / 2}, {W( 2 ) / 2,H( 2 ) / 2}, {-W( 2 ) / 2,H( 2 ) / 2}, {-W( 2 ) / 2, -H( 2 ) / 2}} },
	};

	// *********************************************
	// Keyboard and Mouse Controls
	// *********************************************

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();

	Point2f gridMousePos = ( ( ( mousePos - axes.origin ) + Vector2f( GRID_SIZE / 2, GRID_SIZE / 2 ) ) / GRID_SIZE );
	gridMousePos = Point2f( floor( gridMousePos.x ), floor( gridMousePos.y ) );

	if( Play::KeyPressed( '2' ) )
		quads[sel].m.row[2] = { gridMousePos.x, gridMousePos.y, 1.0f };

	if( Play::KeyPressed( '0' ) )
		quads[sel].m.row[0] = ( gridMousePos - quads[sel].m.row[2].As2D() ) / 10.0f;

	if( Play::KeyPressed( '1' ) )
		quads[sel].m.row[1] = ( gridMousePos - quads[sel].m.row[2].As2D() ) / 10.0f;

	if( Play::KeyPressed( 'N' ) )
	{
		quads[sel].m.row[0] = normalize( quads[sel].m.row[0] );
		quads[sel].m.row[1] = normalize( quads[sel].m.row[1] );
	}

	if( Play::KeyPressed( 'R' ) )
		rotating = !rotating;
	
	if( rotating )
	{
		quads[sel].m.row[0] += quads[sel].m.row[1] / 100.0f;
		quads[sel].m.row[0] = normalize( quads[sel].m.row[0] );
		quads[sel].m.row[1] = { -quads[sel].m.row[0].y, quads[sel].m.row[0].x, 0.0f };
	}

	if( Play::KeyPressed( 'I' ) )
		useInverseMatrix = !useInverseMatrix;

	if( Play::KeyPressed( VK_LEFT ) ) sel--;
	if( Play::KeyPressed( VK_RIGHT ) ) sel++;

	if( sel < 0 ) sel = 0;

	if( sel > quads.size() - 1 )
		sel = quads.size() - 1;

	// *********************************************
	// Draw Axes
	// *********************************************

	Play::ColourSprite( "36px", Play::cBlack );
	Play::cameraPos = { 0, 0 };
	DrawAxes( axes, "36px" );
	Play::cameraPos = -axes.origin;

	// *********************************************
	// Transform and Render Quads	
	// *********************************************

	const Matrix2D& matrix = quads[sel].m;
	std::vector< Point2f > vertices = quads[sel].vertices; // Make a copy so we can transform the data 
	Point2f spriteOrigin{ (quads[sel].pixelData->width) / 2, (quads[sel].pixelData->height) / 2 };

	for( Point2f& p : vertices )
		p = matrix.Transform( p );

	for( size_t i = 0; i < vertices.size() - 1; ++i )
		Play::DrawSpriteLine( vertices[i] * GRID_SIZE, vertices[i + 1] * GRID_SIZE, "pen2px", Play::cBlack );

	if( useInverseMatrix )
		DrawTexturedQuad_Inverse( vertices, quads[sel], spriteOrigin );
	else
		DrawTexturedQuad( vertices, quads[sel], spriteOrigin );

	// *********************************************
	// Draw Arrows
	// *********************************************

	if( length( matrix.row[0] ) > 0 )
		DrawSpriteArrow( matrix.row[2].As2D() * GRID_SIZE, ( matrix.row[2] + ( matrix.row[0] * 10 ) ).As2D() * GRID_SIZE, "pen2px", Play::cRed );

	if( length( matrix.row[1] ) > 0 )
		DrawSpriteArrow( matrix.row[2].As2D() * GRID_SIZE, ( matrix.row[2] + ( matrix.row[1] * 10 ) ).As2D() * GRID_SIZE, "pen2px", Play::cGreen );

	if( matrix.row[2].As2D() != Point2f( 0, 0 ) )
		DrawSpriteArrow( { 0, 0 }, matrix.row[2].As2D() * GRID_SIZE, "pen2px", Play::cWhite );

	Play::cameraPos = { 0, 0 };

	// *********************************************
	// Draw Text
	// *********************************************

	std::stringstream ss; // Used for forcing precision of numbers in strings
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
	Play::DrawFontText( "36px", "0/1/2 to set Matrix Row L/R Change Sprite", { DISPLAY_WIDTH / 2 , DISPLAY_HEIGHT - 25.0f }, Play::CENTRE );

	Play::ColourSprite( "36px", Play::cBlack );
	ss.str( "" );
	ss << "Mouse = { ";
	ss << std::fixed << std::setprecision( 2 ) << gridMousePos.x << ", " << gridMousePos.y << " }";
	Play::DrawFontText( "36px", ss.str(), { 420, 30.0f }, Play::LEFT );
}


// **********************************************************************
// Draw the sprite by transforming each sprite pixel into screen space
// **********************************************************************
void DrawTexturedQuad( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& spriteOrigin )
{
	DrawBounds( vertices );

	Point2f screenPixelPos{ floor( vertices[0].x ), floor( vertices[0].y ) };
	const Pixel* pixel = q.pixelData->pPixels;
	int spritePixelWidth = q.pixelData->width;
	int spritePixelHeight = q.pixelData->height;

	// Iterate through each pixel in the sprite in turn
	for( int y = 0; y < spritePixelHeight; y++ )
	{
		for( int x = 0; x < spritePixelWidth; x++ )
		{
			DrawBigPixel( screenPixelPos, *pixel++ );
			screenPixelPos += q.m.row[0].As2D();
		}
		screenPixelPos -= q.m.row[0].As2D() * spritePixelWidth;
		screenPixelPos += q.m.row[1].As2D();
	}

	DrawHighlightPixel();
}

// **********************************************************************
// Draw the sprite by transforming each screen pixel into sprite space
// **********************************************************************
void DrawTexturedQuad_Inverse( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& spriteOrigin )
{
	DrawBounds( vertices );

	static float inf = std::numeric_limits<float>::infinity();
	float minX{ inf }, minY{ inf }, maxX{ -inf }, maxY{ -inf };

	//calculate the extremes of the rotated corners.
	for( int i = 0; i < 4; i++ )
	{
		minX = floor( minX < vertices[i].x ? minX : vertices[i].x );
		maxX = ceil( maxX > vertices[i].x ? maxX : vertices[i].x );
		minY = floor( minY < vertices[i].y ? minY : vertices[i].y );
		maxY = ceil( maxY > vertices[i].y ? maxY : vertices[i].y );
	}

	if( Determinant( q.m ) == 0.0f ) return;
	Matrix2D inverse = q.m;
	inverse.Inverse();

	int screenPixelWidth = maxX - minX;
	int screenPixelHeight = maxY - minY;
	int spriteWidth = q.pixelData->width;
	int spriteHeight = q.pixelData->height;

	Point2f screenPixelPos{ minX, minY };
	Point2f spritePixelPos = inverse.Transform( screenPixelPos ) + spriteOrigin;

	// Iterate through each pixel on the screen in turn
	for( int y = 0; y < screenPixelHeight; y++ )
	{
		for( int x = 0; x < screenPixelWidth; x++ )
		{
			Point2f rounded{ round( spritePixelPos.x ), round( spritePixelPos.y ) };
			
			if( rounded.x >= 0 && rounded.y >= 0 && rounded.x < spriteWidth && rounded.y < spriteHeight )
			{
				int pixel = rounded.x + ( rounded.y * spriteWidth );
				DrawBigPixel( screenPixelPos, q.pixelData->pPixels[pixel] );
			} 

			screenPixelPos.x++;
			spritePixelPos += inverse.row[0].As2D();
		}
		screenPixelPos.y++;
		screenPixelPos.x -= screenPixelWidth;

		spritePixelPos -= inverse.row[0].As2D() * screenPixelWidth;
		spritePixelPos += inverse.row[1].As2D();
	}

	DrawHighlightPixel();
}

std::vector< Point2f > g_bigPixels;
int g_pixel = 0;

void DrawBigPixel( Point2f pixelPos, Pixel pix )
{
	Point2f cameraOrigin = -Play::cameraPos;
	Point2f bigPixelSize{ GRID_SIZE, GRID_SIZE };
	Point2f bigPixelPos = Point2f( round( pixelPos.x ), round( pixelPos.y ) ) * GRID_SIZE;
	PlayGraphics::Instance().DrawRect( cameraOrigin + bigPixelPos, cameraOrigin + bigPixelPos + bigPixelSize, pix, true );
	g_bigPixels.push_back( bigPixelPos );
}

void DrawHighlightPixel()
{
	if( g_bigPixels.size() > 0 )
	{
		g_pixel = ++g_pixel % g_bigPixels.size();
		Point2f cameraOrigin = -Play::cameraPos;
		Point2f bigPixelSize{ GRID_SIZE, GRID_SIZE };
		Point2f bigPixelPos = g_bigPixels[g_pixel];
		PlayGraphics::Instance().DrawRect( cameraOrigin + bigPixelPos, cameraOrigin + bigPixelPos + bigPixelSize, PIX_WHITE, false );
	}
}

void DrawBounds( std::vector< Point2f >& vertices )
{
	static float inf = std::numeric_limits<float>::infinity();
	float minX{ inf }, minY{ inf }, maxX{ -inf }, maxY{ -inf };

	//calculate the extremes of the rotated corners.
	for( int i = 0; i < 4; i++ )
	{
		minX = minX < vertices[i].x ? minX : vertices[i].x;
		maxX = maxX > vertices[i].x ? maxX : vertices[i].x;
		minY = minY < vertices[i].y ? minY : vertices[i].y;
		maxY = maxY > vertices[i].y ? maxY : vertices[i].y;
	}

	Point2f cameraOrigin = -Play::cameraPos;
	Point2f minPos = ( Point2f( minX, minY ) * GRID_SIZE );
	Point2f maxPos = ( Point2f( maxX, maxY ) * GRID_SIZE );

	PlayGraphics::Instance().DrawRect( cameraOrigin + minPos, cameraOrigin + maxPos, PIX_BLUE, false );
	g_bigPixels.clear();
}
