#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;


struct GameState 
{
	int score = 0;
};

GameState gameState;

enum GameObjectType {
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	//This creates the window the game will be displayed in using the set parameters in lines 5-7
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	//This centres the sprites origin so that it translates from the center instead of the top left corner
	Play::CentreAllSpriteOrigins();
	//This loads the background PNG
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	//This plays the music for the game
	//Play::StartAudioLoop("music");
	//These create the player and fan sprite also setting the velocity and animation speed of the fan
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140,217 }, 0, "fan");
	Play::GetGameObject(id_fan).velocity = { 0, 3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();
	HandlePlayerControls();
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void HandlePlayerControls() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	//This moves the players sprite upwards when the up key is pressed and changes the sprites PNG to its climbing variant
	if (Play::KeyDown(VK_UP)) 
	{
		obj_agent8.velocity = { 0,-4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}
	//This moves the players sprite downwards when the down key is pressed and changes the sprites PNG to its falling variant
	else if (Play::KeyDown(VK_DOWN)) 
	{
		obj_agent8.acceleration = { 0,1 };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
	}
	//This resets the players velocity and acceleration when neither buttons are pressed
	else 
	{
		Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
		obj_agent8.velocity *= 0.5f;
		obj_agent8.acceleration = { 0,0 };
	}
	Play::UpdateGameObject(obj_agent8);

	//This stops the player from leaving the game window
	if (Play::IsLeavingDisplayArea(obj_agent8)) obj_agent8.pos = obj_agent8.oldPos;

	//This draws the string that the players sprite hangs from
	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);
	Play::DrawObjectRotated(obj_agent8);
}

void UpdateFan() 
{
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);
	//This spawns either a spanner or driver sprite depending on the rolled number and throws it towards the players direction
	if (Play::RandomRoll(50) == 50) {
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

		if (Play::RandomRoll(2) == 1) {
			Play::SetSprite(obj_tool, "spanner", 0);
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}
		Play::PlayAudio("tool");
	}
	//This spawns coins depending on whether the die rolls correctly
	if (Play::RandomRoll(150) == 1) {
		int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");
		GameObject& obj_coin = Play::GetGameObject(id);
		obj_coin.velocity = { -3, 0 };
		obj_coin.rotSpeed = 0.1f;
	}
	Play::UpdateGameObject(obj_fan);

	//This stops the fan from leaving the game window and reverts its direction on the y-axis
	if (Play::IsLeavingDisplayArea(obj_fan)) {
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1; 
	}
	Play::DrawObject(obj_fan);
}

void UpdateTools() 
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	for (int id : vTools) {
		GameObject& obj_tool = Play::GetGameObject(id);
		//If a tool collides with the player this code will stop the music, play death sound effect and moves the players sprite off the screen
		if (Play::IsColliding(obj_tool, obj_agent8)) {
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			obj_agent8.pos = { -100, -100 };
		}
		Play::UpdateGameObject(obj_tool);

		//If a tools coordinates reach the windows top or bottom border this code will essentially relfect it away to keep it in the game
		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL)) {
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}
		Play::DrawObjectRotated(obj_tool);

		//If a tool goes off the screen on the left side this code deletes it to keep the games memory and processing to acceptable levels
		if (!Play::IsVisible(obj_tool)) Play::DestroyGameObject(id);
	}
}

void UpdateCoinsAndStars() {

}