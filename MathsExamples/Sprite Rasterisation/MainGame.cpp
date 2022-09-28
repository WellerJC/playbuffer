#define PLAY_IMPLEMENTATION
#include "Play.h"
#include< algorithm >

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;


struct TexturedQuad
{
	const PixelData* pixelData{ nullptr };
	Matrix2D m;
	std::vector< Point2f > vertices;
};

struct Colour
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	int i;
};

void UpdateApplication();
void DrawTexturedQuad( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& offset );
void DrawTexturedQuad_Inverse( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& offset );
void DrawPixel( Point2f pos, Pixel p );


enum Discipline
{
	DEV_UNKNOWN = -1,
	DEV_CODE = 0,
	DEV_ART,
	DEV_DESIGN,
	DEV_PROD,
	MAX_DEV_ROLES,
};

constexpr int MAX_DEVS = 10;
std::vector< std::string > g_Names = { "Rod", "Jane", "Freddy", "Rick", "Carole", "Brian", "Eric", "Julie", "Terrence", "Maria" };
std::vector< std::string > g_Roles = { "Coder", "Concept Artist", "Designer", "Producer" };

class Someone
{
public:
	Someone( std::string n ) { name = n; }
	~Someone();
	std::string Introduce() { return "Someone\n"; } // Whoops :-)
protected:
	std::string name = "";
};

Someone* everyone[MAX_DEVS];


// Add your own Developer class here which inherits from Someone
//class Developer : public Someone
//{
//public:
//	Developer( std::string n, Discipline d ) : Someone( n ) { discipline = d; }
//	~Developer();
//	std::string Introduce() { return "My name is " + name + " and I am a " + g_Roles[discipline] + "\n"; }
//private:
//	Discipline discipline = DEV_UNKNOWN;
//
//};

//Someone::~Someone()
//{
//	//I died
//}


// Add your own Developer class here which inherits from Someone
class Developer : public Someone
{
private:
	std::string disc;

public:
	Developer( std::string n, int d ) : Someone( n )
	{
		//disc = g_Roles[d];
	}
	~Developer(){};

	std::string Introduce() { return "My name is " + name + " and I am a " + disc; }
};

// The standard entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	// Don't change these three lines of code
	int devs = 0;
	for( Someone*& s : everyone )
		s = new Developer( g_Names[devs % MAX_DEVS], (Discipline)( devs++ % MAX_DEV_ROLES ) );

	// You must keep this loop, but can change the code inside it
	//for( Someone* s : everyone )
	//{
	//	std::cout << s->Introduce();
	//}

	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );

	Play::LoadBackground( "Data\\Backgrounds\\maths_bg768sq.png" );

	// Certain sprites need their origins to be in their centres
	Play::CentreAllSpriteOrigins();

	// Changes the colour of the font sprites - slow, so not recommended in your update loop
	Play::ColourSprite( "36px", Play::cBlack );
	Play::ColourSprite( "54px", Play::cBlack );

	Colour c;

	
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
	static int sel = 0;
	static bool rotating = false;

#define W( A )	(Play::GetSpriteWidth( A )-1.0f)
#define H( A )	(Play::GetSpriteHeight( A )-1.0f)
#define W0( A )	Play::GetSpriteWidth( A )
#define H0( A )	Play::GetSpriteHeight( A )

	// Set up from quads based on the first three loaded sprites
	static std::vector< TexturedQuad > quads =
	{
		{ Play::GetSpritePixelData( 0 ), MatrixTranslation( DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 ), { {-W( 0 ) / 2,-H( 0 ) / 2}, {W( 0 ) / 2,-H( 0 ) / 2}, {W( 0 ) / 2,H( 0 ) / 2}, {-W( 0 ) / 2,H( 0 ) / 2}, {-W( 0 ) / 2, -H( 0 ) / 2}} },
		{ Play::GetSpritePixelData( 1 ), MatrixTranslation( DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 ), { {-W( 1 ) / 2,-H( 1 ) / 2}, {W( 1 ) / 2,-H( 1 ) / 2}, {W( 1 ) / 2,H( 1 ) / 2}, {-W( 1 ) / 2,H( 1 ) / 2}, {-W( 1 ) / 2, -H( 1 ) / 2}} },
		{ Play::GetSpritePixelData( 2 ), MatrixTranslation( DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 ), { {-W( 2 ) / 2,-H( 2 ) / 2}, {W( 2 ) / 2,-H( 2 ) / 2}, {W( 2 ) / 2,H( 2 ) / 2}, {-W( 2 ) / 2,H( 2 ) / 2}, {-W( 2 ) / 2, -H( 2 ) / 2}} },
	};

	// *********************************************
	// Keyboard and Mouse Controls
	// *********************************************

	// Get the mouse position
	Point2f mousePos = Play::GetMousePos();

	if( Play::KeyPressed( '2' ) )
		quads[sel].m.row[2] = { mousePos.x, mousePos.y, 1.0f };

	if( Play::KeyPressed( '0' ) )
		quads[sel].m.row[0] = ( mousePos - quads[sel].m.row[2].As2D() ) / (quads[sel].pixelData->width/2);

	if( Play::KeyPressed( '1' ) )
		quads[sel].m.row[1] = ( mousePos - quads[sel].m.row[2].As2D() ) / (quads[sel].pixelData->height/2);

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

	if( Play::KeyPressed( VK_LEFT ) ) sel--;
	if( Play::KeyPressed( VK_RIGHT ) ) sel++;

	if( sel < 0 ) sel = 0;

	if( sel > quads.size() - 1 )
		sel = quads.size() - 1;

	// *********************************************
	// Transform and Render Quads	
	// *********************************************

	const Matrix2D& matrix = quads[sel].m;
	std::vector< Point2f > vertices = quads[sel].vertices; // Make a copy so we can transform the data 
	Point2f spriteOrigin{ ( quads[sel].pixelData->width ) / 2, ( quads[sel].pixelData->height ) / 2 };

	for( Point2f& p : vertices )
		p = matrix.Transform( p );

	for( size_t i = 0; i < vertices.size() - 1; ++i )
		Play::DrawLine( vertices[i], vertices[i + 1], Play::cBlack );

	DrawTexturedQuad_Inverse( vertices, quads[sel], spriteOrigin );

	// *********************************************
	// Draw Arrows
	// *********************************************

	if( length( matrix.row[0] ) > 0 )
		DrawSpriteArrow( matrix.row[2].As2D(), ( matrix.row[2] + ( matrix.row[0] * quads[sel].pixelData->width / 2 ) ).As2D(), "pen2px", Play::cRed );

	if( length( matrix.row[1] ) > 0 )
		DrawSpriteArrow( matrix.row[2].As2D(), ( matrix.row[2] + ( matrix.row[1] * quads[sel].pixelData->height / 2 ) ).As2D(), "pen2px", Play::cGreen );

	if( matrix.row[2].As2D() != Point2f( 0, 0 ) )
		DrawSpriteArrow( { 0, 0 }, matrix.row[2].As2D(), "pen2px", Play::cWhite );

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
	ss << std::fixed << std::setprecision( 2 ) << mousePos.x << ", " << mousePos.y << " }";
	Play::DrawFontText( "36px", ss.str(), { 420, 30.0f }, Play::LEFT );
}

// **********************************************************************
// Draw the sprite by transforming each screen pixel into sprite space
// **********************************************************************
void DrawTexturedQuad_Inverse( std::vector< Point2f >& vertices, TexturedQuad& q, Point2f& spriteOrigin )
{
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
				DrawPixel( screenPixelPos, q.pixelData->pPixels[pixel] );
			}

			screenPixelPos.x++;
			spritePixelPos += inverse.row[0].As2D();
		}
		screenPixelPos.y++;
		screenPixelPos.x -= screenPixelWidth;

		spritePixelPos -= inverse.row[0].As2D() * screenPixelWidth;
		spritePixelPos += inverse.row[1].As2D();
	}
}


void DrawPixel( Point2f pixelPos, Pixel src )
{
	if( pixelPos.x < 0 || pixelPos.x >= DISPLAY_WIDTH || pixelPos.y < 0 || pixelPos.y >= DISPLAY_HEIGHT )
		return;

	PixelData* pBackBuffer = PlayGraphics::Instance().GetDrawingBuffer();

	src.bits = ( src.bits >> 1 ) & 0xFF7F7F7F;

	if( src.a != 0 )
		pBackBuffer->pPixels[(int)pixelPos.x + ( (int)pixelPos.y * pBackBuffer->width )] = src;
}



