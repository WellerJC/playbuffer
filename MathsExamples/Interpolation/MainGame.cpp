#define PLAY_IMPLEMENTATION
#include "Play.h"

#include "../CommonMaths.h"

constexpr int DISPLAY_WIDTH = 768;
constexpr int DISPLAY_HEIGHT = 768;
constexpr int DISPLAY_SCALE = 1;


enum 
{
	INTERP_LINEAR = 0,
	INTERP_EASE_IN,
	INTERP_EASE_OUT,
	INTERP_EASE_INOUT,
	INTERP_PROPORTIONAL,
	INTERP_SPRING,
	MAX_INTERPS
};

std::string g_interpMethods[]
{
	"Linear",
	"Ease In",
	"Ease Out",
	"Ease In/Out",
	"Proportional",
	"Spring"
};

// Handles the update and draw of the application
void UpdateApplication( float );


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
	UpdateApplication( elapsedTime );
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
void UpdateApplication( float elapsedTime )
{
	static int interp = INTERP_LINEAR;
	static std::vector< Point2f > points;

	int xstart = -DISPLAY_WIDTH / 2;
	int xend = DISPLAY_WIDTH / 2;

	static int x = xstart;
	static float theta = 0.0f;
	static float speed = 0.0f;

	theta += elapsedTime*2.0f;

	if( theta > 1.0f )
		theta = 1.0f;

	if( theta < 0.0f )
		theta = 0.0f;

	int dx = xend - xstart;

	switch( interp )
	{
		case INTERP_LINEAR:
			x = (int)xstart + ( dx * theta );
			break;
		case INTERP_EASE_IN:
		{
			float scaled_theta = theta / 2; 
			float inverted_cosine = -cos( scaled_theta * PLAY_PI );
			float positive_inverted_cosine = inverted_cosine + 1;
			x = xstart + ( dx * positive_inverted_cosine );
			break;
		}
		case INTERP_EASE_OUT:
		{
			float scaled_theta = ( theta / 2 ) + 0.5f;
			float inverted_cosine = -cos( scaled_theta * PLAY_PI );
			x = xstart + ( dx * inverted_cosine );
			break;
		}
		case INTERP_EASE_INOUT:
		{
			float inverted_cosine = -cos( theta * PLAY_PI );
			float positive_inverted_cosine = ( inverted_cosine + 1 ) / 2.0f;
			x = xstart + ( dx * positive_inverted_cosine );
			break;
		}
		case INTERP_PROPORTIONAL:
		{
			dx = ( xend - x ) / 8;
			x += dx;
			break;
		}
		case INTERP_SPRING:
		{
			float force = (xend - x)/8;
			speed += force;
			x += speed;
			speed *= 0.7f;
			break;
		}
	}

	if( Play::KeyDown( VK_SPACE ) )
	{
		x = xstart;
		theta = 0.0f;
		points.clear();
		speed = 0.0f;
	}


	if( Play::KeyPressed( VK_RIGHT ) )
	{
		interp++;
		if( interp >= MAX_INTERPS )
			interp = 0;
		speed = 0.0f;
	}

	if( Play::KeyPressed( VK_LEFT ) )
	{
		interp--;
		if( interp < 0 )
			interp = MAX_INTERPS - 1;
		speed = 0.0f;
	}

	if( theta < 1.0f )
		points.push_back( { ( x * 0.8f ) + ( DISPLAY_WIDTH / 2 ), DISPLAY_HEIGHT / 4 } );

	for( Point2f p : points )
		Play::DrawSprite( Play::GetSpriteId( "pen6px" ), { p.x, p.y }, 0 );

	Play::DrawSprite( Play::GetSpriteId( "spyder" ), { x, DISPLAY_HEIGHT * 0.55f }, 0 );
	Play::DrawSprite( Play::GetSpriteId( "spyder_small" ), { ( x * 0.8f ) + ( DISPLAY_WIDTH / 2 ), DISPLAY_HEIGHT / 4 }, 0 );

	// Draw the title
	Play::DrawFontText( "54px", "Interpolation", { DISPLAY_WIDTH / 2, 24 }, Play::CENTRE );
	Play::DrawFontText( "54px", "________________", { DISPLAY_WIDTH / 2,  24 + 8 }, Play::CENTRE );

	// Draw the mode
	Play::DrawFontText( "36px", g_interpMethods[interp], { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 120.0f }, Play::CENTRE );

	//// Draw the remaining text
	Play::ColourSprite( "36px", Play::cWhite );
	Play::DrawFontText( "36px", "Left / right to change method and space to reset", { DISPLAY_WIDTH / 2 , DISPLAY_HEIGHT - 25.0f }, Play::CENTRE );



}
