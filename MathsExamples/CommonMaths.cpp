#include "Play.h"
#include "CommonMaths.h"

// Draws and labels a pair of axes
void DrawAxes( Axes& a, std::string font )
{
	Point2f xAxisStart{ a.origin.x + ( a.axisMin.x * a.gridSize ), a.origin.y };
	Point2f xAxisEnd{ a.origin.x + ( a.axisMax.x * a.gridSize ), a.origin.y };
	Point2f yAxisStart{ a.origin.x, a.origin.y + ( a.axisMin.y * a.gridSize ) };
	Point2f yAxisEnd{ a.origin.x, a.origin.y + ( a.axisMax.y * a.gridSize ) };

	Play::DrawSpriteLine( xAxisStart, xAxisEnd, "pen4px", Play::cWhite );
	Play::DrawSpriteLine( yAxisStart, yAxisEnd, "pen4px", Play::cWhite );

	int fontSpriteHeight = Play::GetSpriteHeight( font.c_str() );

	int xLabelCount = static_cast<int>( a.vXLabels.size() );

	if( xLabelCount > 0 )
	{
		float x = a.origin.x + ( a.labelsMin.x * a.gridSize );
		float xPixelInc = ( ( a.labelsMax.x - a.labelsMin.x ) * a.gridSize ) / ( xLabelCount - 1 );

		for( int label = 0; label < xLabelCount; label++ )
		{
			Play::DrawFontText( font.c_str(), a.vXLabels[label], { static_cast<int>( x ), static_cast<int>( a.origin.y - fontSpriteHeight / 2 ) }, Play::CENTRE );
			x += xPixelInc;
		}
	}

	int yLabelCount = static_cast<int>( a.vYLabels.size() );

	if( yLabelCount > 0 )
	{
		float y = a.origin.y + ( a.labelsMin.y * a.gridSize );
		float yPixelInc = ( ( a.labelsMax.y - a.labelsMin.y ) * a.gridSize ) / ( yLabelCount - 1 );

		for( int label = 0; label < yLabelCount; label++ )
		{
			Play::DrawFontText( font.c_str(), a.vYLabels[label], { static_cast<int>( a.origin.x ), static_cast<int>( y - fontSpriteHeight / 2 ) }, Play::CENTRE );
			y += yPixelInc;
		}
	}
}

void DrawSpriteArrow( Vector2f startPoint, Vector2f endPoint, const char* penSprite, Play::Colour c )
{
	Vector2f unitVec = normalize( endPoint - startPoint );
	Vector2f unitParallelVec = { unitVec.y, -unitVec.x };

	Play::DrawSpriteLine( startPoint, endPoint, penSprite, c );
	Play::DrawSpriteLine( endPoint - ( unitVec * 25 ) + ( unitParallelVec * 10 ), endPoint, penSprite, c );
	Play::DrawSpriteLine( endPoint - ( unitVec * 25 ) - ( unitParallelVec * 10 ), endPoint, penSprite, c );
}

void DrawSpriteCross( Vector2f centrePoint, const char* penSprite, Play::Colour c )
{
	Play::DrawSpriteLine( centrePoint + Vector2f( -20, -20 ), centrePoint + Vector2f( 20, 20 ), penSprite, c );
	Play::DrawSpriteLine( centrePoint + Vector2f( -20, 20 ), centrePoint + Vector2f( 20, -20 ), penSprite, c );
}

