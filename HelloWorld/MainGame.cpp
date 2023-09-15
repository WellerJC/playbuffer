#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Agent8State {
	STATE_APPEAR = 0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,
};

struct GameState 
{
	int score = 0;
	Agent8State agentState = STATE_APPEAR;
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
void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();

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
	Play::StartAudioLoop("music");
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
	UpdateAgent8();
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();
	Play::DrawFontText("64px", "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
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
		if (obj_agent8.velocity.y > 5) {
			gameState.agentState = STATE_HALT;
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0,0 };
		}
		else {
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0,0 };
		}
	}
	//This shoots a laser from the sprite
	if (Play::KeyPressed(VK_SPACE)) {
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 30, "laser");
		Play::GetGameObject(id).velocity = { 32,0 };
		Play::PlayAudio("shoot");
	}
	Play::UpdateGameObject(obj_agent8);
	
	//This stops the player from leaving the game window
	//I have decided to leave this in as the one in the UpdateAgent8() function doesnt seem to be stopping the player from leaving the display boundaries
	if (Play::IsLeavingDisplayArea(obj_agent8)) obj_agent8.pos = obj_agent8.oldPos;
	
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
		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_tool, obj_agent8)) {
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			gameState.agentState = STATE_DEAD;
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
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	for (int id_coin : vCoins) {
		GameObject& obj_coin = Play::GetGameObject(id_coin);
		bool hasCollided = false;

		//This if statement detect if the coin has collided with the player and if it has it adds points to the players score and spawns in star.pngs
		if (Play::IsColliding(obj_coin, obj_agent8)) {
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f) {
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f, 0.5f };
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}
			hasCollided = true;
			gameState.score += 500;
			Play::PlayAudio("collect");
		}
		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);

		//If the coin has collided with the player this removes the coin from the scene
		if (!Play::IsVisible(obj_coin) || hasCollided) Play::DestroyGameObject(id_coin);
	}

	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);

	//This creates the star animation when a player collects a coin
	for (int id_star : vStars) {
		GameObject& obj_star = Play::GetGameObject(id_star);
		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);

		if (!Play::IsVisible(obj_star)) Play::DestroyGameObject(id_star);
	}
}

void UpdateLasers() {
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	for (int id_laser : vLasers) {
		GameObject& obj_laser = Play::GetGameObject(id_laser);
		bool hasCollided = false;
		//This determines if the laser collided with a tool or coin and filters it to run the necessary code
		for (int id_tool : vTools) {
			GameObject& obj_tool = Play::GetGameObject(id_tool);
			if (Play::IsColliding(obj_laser, obj_tool)) {
				hasCollided = true;
				obj_tool.type = TYPE_DESTROYED;
				gameState.score += 100;
			}
		}

		for (int id_coin : vCoins) {
			GameObject& obj_coin = Play::GetGameObject(id_coin);
			if (Play::IsColliding(obj_laser, obj_coin)) {
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				Play::PlayAudio("error");
				gameState.score -= 300;
			}
		}

		//This if statement makes sure that the players score doesnt fall beneath zero in case they were to get a -300 penalty with less than 300 points in their score
		if (gameState.score < 0) gameState.score = 0;

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);

		//This will detect whether the lasers are off the players screen or have collided with another sprite, if so this remove them from the scene
		if (!Play::IsVisible(obj_laser) || hasCollided) Play::DestroyGameObject(id_laser);
	}
}

void UpdateDestroyed() {
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	//This code comes into a effect when a player shoots either a tool or coin giving the object the effect that it is fading out
	//instead of just dissappearing upon collision
	for (int id_dead : vDead) {
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f;
		Play::UpdateGameObject(obj_dead);

		if (obj_dead.frame % 2) Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);

		if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10) Play::DestroyGameObject(id_dead);
	}
}

void UpdateAgent8() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	switch (gameState.agentState) {
	//This case is used at the start of the game to move the player 1/3 of the way down the screen
	case STATE_APPEAR:
		obj_agent8.velocity = { 0,12 };
		obj_agent8.acceleration = { 0,0.5f };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
		obj_agent8.rotation = 0;
		if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3) gameState.agentState = STATE_PLAY;
		break;
	//This case is used when the player stops moving slowing the players velocity so that the moving animation finishes then it chase to the play state
	case STATE_HALT:
		obj_agent8.velocity *= 0.9f;
		if (Play::IsAnimationComplete(obj_agent8)) gameState.agentState = STATE_PLAY;
		break;
	//This case is used in general giving the player access to the HandlePlayerControls() which lets them control agent 8
	case STATE_PLAY:
		HandlePlayerControls();
		break;
	//This case is used when the player collides with a tool, this gievs the player the option to restart the game via space
	//It restarts the music and destroy any objects still in the scene and returns the agents state to its first state and position
	case STATE_DEAD:
		obj_agent8.acceleration = { -0.3f, 0.5f };
		obj_agent8.rotation += 0.25f;
		if (Play::KeyPressed(VK_SPACE) == true) {
			gameState.agentState = STATE_APPEAR;
			obj_agent8.pos = { 115,0 };
			obj_agent8.velocity = { 0,0 };
			obj_agent8.frame = 0;
			Play::StartAudioLoop("music");
			gameState.score = 0;

			for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_TOOL)) Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		break;
	}

	Play::UpdateGameObject(obj_agent8);

	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != STATE_DEAD) obj_agent8.pos = obj_agent8.oldPos;

	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);
	Play::DrawObjectRotated(obj_agent8);
}