
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <raylib.h>
using namespace std;

const int width = 20;
const int height = 20;
const int CELL_SIZE = 20;

Color purpleTop = { 60, 20, 90, 255 };
Color blackBottom = { 8, 8, 12, 255 };
Color textMain = { 180, 160, 200, 255 };
Color textAccent = { 220, 200, 240, 255 };
Color barBg = { 25, 20, 35, 255 };
Color barFill = { 120, 60, 180, 255 };
Color barBorder = { 90, 70, 130, 255 };

class Cell {
public:
	bool walls[4] = { true, true, true, true }; // top, bottom, left, right
	bool visited = false;
};

class WaterSystem {
public:
	float waterLevel = 0.0f;  // Current water height in pixels
	float riseSpeed = 0.3f;   // Pixels per frame
	float maxWaterLevel;
	float oxygenLevel = 100.0f;
	float oxygenDepletionRate = 0.15f;
	bool isPlayerUnderwater = false;

	struct AirBubble {
		float x, y;
		bool collected;
	};

	struct DrainSwitch {
		float x, y;
		bool activated;
		float activationRadius = 20.0f; // How close player must be to activate
	};

	vector<AirBubble> airBubbles;
	vector<DrainSwitch> drainSwitches;
	float waveOffset = 0;

	WaterSystem() {
		maxWaterLevel = height * CELL_SIZE;
		Reset();
	}

	void Reset() { // This is when starting new game to reset everything. 
		waterLevel = 0.0f;
		oxygenLevel = 100.0f;
		isPlayerUnderwater = false;
		airBubbles.clear();
		drainSwitches.clear();

		// Placing 8 air bubbles randomly in maze
		for (int i = 0; i < 8; i++) {
			AirBubble bubble;
			bubble.x = (rand() % width) * CELL_SIZE + CELL_SIZE / 2;
			bubble.y = (rand() % height) * CELL_SIZE + CELL_SIZE / 2;
			bubble.collected = false;
			airBubbles.push_back(bubble);
		}

		// Placing 3 drain switches at strategic locations
		DrainSwitch drain1 = {
			CELL_SIZE*2.5f,
			CELL_SIZE*2.5f,
			false
		};
		DrainSwitch drain2 = {
			CELL_SIZE * (width - 2.5f),
			CELL_SIZE * (height - 2.5f),
			false
		};
		DrainSwitch drain3 = {
			CELL_SIZE * (width / 2),
			CELL_SIZE * (height / 2),
			false
		};

		drainSwitches.push_back(drain1);
		drainSwitches.push_back(drain2);
		drainSwitches.push_back(drain3);
	}

	void Update(float playerX, float playerY, float deltaTime) { 
		waveOffset += deltaTime * 50; 
		if (waveOffset > 360) waveOffset -= 360;

		// Count active drains
		int activeDrains = 0;
		for (auto drain : drainSwitches) {
			if (drain.activated) activeDrains++;
		}

		// Adjust water level based on drains
		if (activeDrains == 0) {
			waterLevel += riseSpeed;
		}
		else if (activeDrains == 1) {
			waterLevel += riseSpeed * 0.3f; // Slower rise
		}
		else if (activeDrains == 2) {
			waterLevel -= riseSpeed * 0.2f; // Slowly drains
		}
		else {
			waterLevel -= riseSpeed * 1.5f; // Fast drain with all switches
		}

		// Clamp water level --> Limiting the water level within bounds
		if (waterLevel < 0) waterLevel = 0;
		if (waterLevel > maxWaterLevel) waterLevel = maxWaterLevel;

		// Check if player is underwater
		float waterTopY = maxWaterLevel - waterLevel;
		isPlayerUnderwater = (playerY > waterTopY); // Why not directly compare with waterLevel? why not playerY > waterLevel?

		// Update oxygen
		if (isPlayerUnderwater) {
			oxygenLevel -= oxygenDepletionRate;
			if (oxygenLevel < 0) oxygenLevel = 0;
		}
		else {
			oxygenLevel += 0.5f; // Slowly recover when above water
			if (oxygenLevel > 100) oxygenLevel = 100;
		}

		// Check air bubble collection
		for (auto& bubble : airBubbles) {
			if (!bubble.collected) {
				float dist = sqrt(pow(playerX - bubble.x, 2) +
					pow(playerY - bubble.y, 2));
				if (dist < CELL_SIZE / 2) {
					bubble.collected = true;
					oxygenLevel = min(100.0f, oxygenLevel + 50.0f);
				}
			}
		}

		// Check drain switch activation
		for (auto& drain : drainSwitches) {
			if (!drain.activated) {
				float dist = sqrt(pow(playerX - drain.x, 2) +
					pow(playerY - drain.y, 2));
				if (dist < drain.activationRadius) {
					drain.activated = true;
				}
			}
		}
	}

	void Draw(int offsetX, int offsetY) {
		float waterTopY = offsetY + maxWaterLevel - waterLevel; // Inner Left edge 

		if (waterLevel > 0) {
			// Draw main water body
			Color waterColor = { 30, 60, 150, 160 };
			DrawRectangle(offsetX, waterTopY, width * CELL_SIZE, waterLevel, waterColor);
		}

		// Draw air bubbles
		for (auto& bubble : airBubbles) {
			if (!bubble.collected) {
				DrawCircleGradient(offsetX + bubble.x,offsetY + bubble.y,12, { 200, 200, 255, 200 }, { 100, 100, 200, 100 });
				DrawCircle(offsetX + bubble.x - 3,offsetY + bubble.y - 3,3, { 255, 255, 255, 255 });
			}
		}

		// Draw drain switches
		for (auto& drain : drainSwitches) {
			Color switchColor = drain.activated ? GREEN : RED;
			Color glowColor = drain.activated ? Color{ 0, 255, 0, 50 } : Color{ 255, 0, 0, 50 };

			// Glow effect also Labeling an also swith bogy
			DrawCircle(offsetX + drain.x, offsetY + drain.y,drain.activationRadius, glowColor);

			DrawCircle(offsetX + drain.x, offsetY + drain.y, 12, BLACK);
			DrawCircle(offsetX + drain.x, offsetY + drain.y, 10, switchColor);

			
			const char* text = drain.activated ? "ON" : "OFF";
			DrawText(text, offsetX + drain.x - 10,
				offsetY + drain.y - 25, 10, switchColor);
		}
	}

	void DrawUI(int screenW, int screenH) {
		// Water level indicator
		int indicatorX = 10;
		int indicatorY = 120;
		DrawRectangle(indicatorX, indicatorY, 30, 200, Fade(BLACK, 0.5f));
		DrawRectangle(indicatorX, indicatorY + 200 - (waterLevel / maxWaterLevel) * 200,30, (waterLevel / maxWaterLevel) * 200, Fade(BLUE, 0.7f));
		DrawRectangleLines(indicatorX, indicatorY, 30, 200, WHITE);
		DrawText("WATER", indicatorX - 5, indicatorY - 20, 16, BLUE);

		// Oxygen bar (only whn prson is underwater or low oxygen)
		if (isPlayerUnderwater || oxygenLevel < 100) {
			int barX = screenW / 2 - 150;
			int barY = 80;

			// Background
			DrawRectangle(barX - 2, barY - 2, 304, 24, BLACK);
			
			// Oxygen bar
			Color oxyColor;
			if (oxygenLevel > 60) oxyColor = SKYBLUE;
			else if (oxygenLevel > 30) oxyColor = YELLOW;
			else oxyColor = RED;

			DrawRectangle(barX, barY, (int)(300 * oxygenLevel / 100), 20, oxyColor);
			DrawRectangleLines(barX, barY, 300, 20, WHITE);
			// Text
			DrawText("OXYGEN", barX + 120, barY + 2, 16, WHITE);
			// Critical warning
			if (oxygenLevel < 30) {
				if ((int)(GetTime() * 3) % 2 == 0) {
					DrawText("WARNING: LOW OXYGEN!",
						screenW / 2 - MeasureText("WARNING: LOW OXYGEN!", 20) / 2,
						barY + 30, 20, RED);
				}
			}
		}

		// Drain status
		int drainY = 350;
		DrawText("DRAIN SWITCHES:", 10, drainY, 14, WHITE);
		for (int i = 0; i < drainSwitches.size(); i++) {
			Color statusColor = drainSwitches[i].activated ? GREEN : RED;
			DrawCircle(25 + i * 40, drainY + 27, 10, statusColor);
			DrawText(TextFormat("%d", i + 1), 22 + i * 40, drainY + 22, 12, WHITE);
		}
	}

	bool IsGameOver() {
		return oxygenLevel <= 0;
	}

	float GetWaterPercentage() {
		return (waterLevel / maxWaterLevel) * 100;
	}
};

class Player2D {
public:
	float x, y;
	float speed = 2.0f;
	float size = CELL_SIZE * 0.6f;

	Player2D() {
		x = CELL_SIZE / 2;
		y = CELL_SIZE / 2;
	}

	void Reset() { // This is for the new game. 
		x = CELL_SIZE / 2;
		y = CELL_SIZE / 2;
	}

	void Update(vector<Cell>& grid) {
		float newX = x;
		float newY = y;

		// Movement input
		if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) newY -= speed;
		if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) newY += speed;
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) newX -= speed;
		if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) newX += speed;

		// Check collision with walls
		if (CanMoveTo(newX, newY, grid)) {
			x = newX;
			y = newY;
		}
		else {
			// Try moving on single axis
			if (CanMoveTo(newX, y, grid)) x = newX;
			else if (CanMoveTo(x, newY, grid)) y = newY;
		}
	}

	bool CanMoveTo(float newX, float newY, vector<Cell>& grid) {
		// Geting grid position
		int gridX = (int)(newX / CELL_SIZE);
		int gridY = (int)(newY / CELL_SIZE);

		// Checking th boundary
		if (newX < size / 2 || newX > width * CELL_SIZE - size / 2) return false;
		if (newY < size / 2 || newY > height * CELL_SIZE - size / 2) return false;

		if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height) return false;

		int idx = gridX + gridY * width;

		// Check collision with walls
		float cellX = fmod(newX, CELL_SIZE);
		float cellY = fmod(newY, CELL_SIZE);

		float buffer = size / 2;

		// Check walls
		if (grid[idx].walls[0] && cellY < buffer) return false; // Top
		if (grid[idx].walls[1] && cellY > CELL_SIZE - buffer) return false; // Bottom
		if (grid[idx].walls[2] && cellX < buffer) return false; // Left
		if (grid[idx].walls[3] && cellX > CELL_SIZE - buffer) return false; // Right

		return true;
	}

	void Draw(int offsetX, int offsetY, bool isUnderwater) {
		Color playerColor = isUnderwater ? Color{ 100, 150, 255, 255 } : Color{ 255, 200, 100, 255 };

		// Draw player
		DrawCircle(offsetX + x, offsetY + y, size / 2, playerColor);
		DrawCircleLines(offsetX + x, offsetY + y, size / 2, BLACK);

		// Draw bubble effect if underwater
		if (isUnderwater) {
			for (int i = 0; i < 3; i++) {
				float bubbleY = y - 10 - i * 5 - sin(GetTime() * 3) * 3;
				DrawCircle(offsetX + x, offsetY + bubbleY, 2 - i * 0.5f,
					Fade(WHITE, 0.5f - i * 0.1f));
			}
		}
	}

	bool HasReachedExit() {
		float exitX = (width - 0.5f) * CELL_SIZE;
		float exitY = (height - 0.5f) * CELL_SIZE;
		float dist = sqrt(pow(x - exitX, 2) + pow(y - exitY, 2));
		return dist < CELL_SIZE / 2;
	}
};

struct Particle {
	Vector2 position;
	Vector2 velocity;
	float size;
	float alpha;
	float life;
	Color baseColor;
};

class ParticleSystem {
public:
	vector<Particle> particles;
	int screenW = 0;
	int screenH = 0;

	//ParticleSystem() : screenW(0), screenH(0) {}

	void Init(int numParticles, int screenWidth, int screenHeight) {
		screenW = screenWidth;
		screenH = screenHeight;
		particles.clear();

		for (int i = 0; i < numParticles; i++) {
			particles.push_back(CreateParticle());
		}
	}

	void Resize(int screenWidth, int screenHeight) {
		screenW = screenWidth;
		screenH = screenHeight;
	}

	Particle CreateParticle() {
		Particle p;
		int w = (screenW > 0) ? screenW : 800;
		int h = (screenH > 0) ? screenH : 600;

		p.position = { (float)(rand() % w), (float)(rand() % h) };
		p.velocity = {((float)(rand() % 100) - 50) / 50.0f,-((float)(rand() % 50) + 20) / 30.0f};

		p.size = (float)(rand() % 4) + 1.5f;
		p.alpha = (float)(rand() % 100) / 100.0f * 0.7f + 0.1f;

		int colorChoice = rand() % 4;
		switch (colorChoice) {
		case 0: p.baseColor = { 180, 120, 255, 255 }; break;
		case 1: p.baseColor = { 140, 80, 200, 255 }; break;
		case 2: p.baseColor = { 200, 180, 255, 255 }; break;
		case 3: p.baseColor = { 255, 255, 255, 255 }; break;
		}
		return p;
	}

	void Update() {
		int w = (screenW > 0) ? screenW : 800;
		int h = (screenH > 0) ? screenH : 600;

		for (size_t i = 0; i < particles.size(); i++) {
			Particle& p = particles[i];

			p.position.x += p.velocity.x;
			p.position.y += p.velocity.y;
			p.position.x += sin(p.life * 0.02f) * 0.3f;
			if ( p.position.y < -20 ||p.position.x < -20 || p.position.x > w + 20) {
				p.position = { (float)(rand() % w), (float)(h + rand() % 50) };
				p.velocity = {
					((float)(rand() % 100) - 50) / 50.0f,
					-((float)(rand() % 50) + 20) / 30.0f
				};
			}
		}
	}

	void Draw() {
		for (const Particle& p : particles) {
			Color drawColor = {
				p.baseColor.r,
				p.baseColor.g,
				p.baseColor.b,
				(unsigned char)(p.alpha * 255)
			};

			DrawCircle((int)p.position.x, (int)p.position.y, p.size * 2.0f,
				Fade(drawColor, p.alpha * 0.3f));
			DrawCircle((int)p.position.x, (int)p.position.y, p.size, drawColor);
			DrawCircle((int)p.position.x, (int)p.position.y, p.size * 0.5f,
				Fade(WHITE, p.alpha * 0.5f));
		}
	}
};

void DrawGradientBackground(int screenWidth, int screenHeight) {
	for (int y = 0; y < screenHeight; y++) {
		float t = (float)y / screenHeight;
		Color lineCol = {
			(purpleTop.r * (1 - t) + blackBottom.r * t),
			(purpleTop.g * (1 - t) + blackBottom.g * t),
			(purpleTop.b * (1 - t) + blackBottom.b * t),
			255
		};
		DrawLine(0, y, screenWidth, y, lineCol);
	}
}

int index(int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return -1;
	}
	return x + y * width;
}

void maze_generation(vector<Cell>& grid, vector<int> stack) {
	// Clear grid
	for (int i = 0; i < width * height; i++) {
		grid[i].visited = false;
		grid[i].walls[0] = true;
		grid[i].walls[1] = true;
		grid[i].walls[2] = true;
		grid[i].walls[3] = true;
	}
	stack.clear();

	int currentCell_x = 0;
	int currentCell_y = 0;

	int check_coordinate = index(currentCell_x, currentCell_y);
	grid[check_coordinate].visited = true;
	stack.push_back(check_coordinate);

	while (stack.empty() == false) {
		vector<int> neighbours;

		int top = index(currentCell_x, currentCell_y - 1);
		int left = index(currentCell_x - 1, currentCell_y);
		int bottom = index(currentCell_x, currentCell_y + 1);
		int right = index(currentCell_x + 1, currentCell_y);

		if (top != -1 && grid[top].visited == false) neighbours.push_back(top);
		if (left != -1 && grid[left].visited == false) neighbours.push_back(left);
		if (bottom != -1 && grid[bottom].visited == false) neighbours.push_back(bottom);
		if (right != -1 && grid[right].visited == false) neighbours.push_back(right);

		if (neighbours.empty() == false) {
			int randomIndex = rand() % neighbours.size();
			int value = neighbours[randomIndex];

			if (value == top) {
				grid[check_coordinate].walls[0] = false;
				grid[value].walls[1] = false;
				currentCell_y -= 1;
			}
			else if (value == left) {
				grid[check_coordinate].walls[2] = false;
				grid[value].walls[3] = false;
				currentCell_x -= 1;
			}
			else if (value == bottom) {
				grid[check_coordinate].walls[1] = false;
				grid[value].walls[0] = false;
				currentCell_y += 1;
			}
			else if (value == right) {
				grid[check_coordinate].walls[3] = false;
				grid[value].walls[2] = false;
				currentCell_x += 1;
			}

			check_coordinate = index(currentCell_x, currentCell_y);
			grid[check_coordinate].visited = true;
			stack.push_back(check_coordinate);
		}
		else {
			stack.pop_back();
			if (stack.empty() == false) {
				check_coordinate = stack.back();
				currentCell_x = check_coordinate % width;
				currentCell_y = check_coordinate / width;
			}
		}
	}
}

int main() {
	srand(time(0));

	const int screenWidth = 800;
	const int screenHeight = 600;
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "Maze Master - Flood Escape");
	SetTargetFPS(60);

	float loadProgress = 0.0f;
	bool loadingDone = false;
	float waitTimer = 0.0f;

	ParticleSystem particles;
	particles.Init(100, screenWidth, screenHeight);

	InitAudioDevice();
	Music bgmusic = LoadMusicStream("music.mp3");
	PlayMusicStream(bgmusic);
	SetMusicVolume(bgmusic, 1.0f);

	Texture2D button = LoadTexture("maze_master.png");
	Texture2D button1 = LoadTexture("newbutstart.png");
	Texture2D button2 = LoadTexture("newbutabout.png");
	Texture2D button3 = LoadTexture("nerbutexit.png");

	Color border = { 0, 255, 180, 200 };

	vector<Cell> grid(width * height);
	vector<int> stack;

	// Game objects
	Player2D player;
	WaterSystem waterSystem;
	float gameTimer = 0;
	bool hasWon = false;
	int state = 0;

	// Loading screen loop
	while (!WindowShouldClose() && !loadingDone) {
		UpdateMusicStream(bgmusic);

		int currentW = GetScreenWidth();
		int currentH = GetScreenHeight();
		particles.Resize(currentW, currentH);

		if (loadProgress < 100.0f) {
			loadProgress += 0.8f;
			if (loadProgress > 100.0f) loadProgress = 100.0f;
		}
		else {
			waitTimer += GetFrameTime();
			if (waitTimer > 2.0f) loadingDone = true;
		}

		particles.Update();

		BeginDrawing();
		DrawGradientBackground(currentW, currentH);
		particles.Draw();

		DrawText("Presented by",
			(currentW - MeasureText("Presented by", 28)) / 2,
			currentH / 2 - 100, 28, textMain);

		DrawText("SHADOW STUDIOS",
			(currentW - MeasureText("SHADOW STUDIOS", 52)) / 2,
			currentH / 2 - 55, 52, textAccent);

		int barW = 350, barH = 16;
		int barX = (currentW - barW) / 2;
		int barY = currentH / 2 + 100;

		DrawRectangle(barX - 2, barY - 2, barW + 4, barH + 4, barBorder);
		DrawRectangle(barX, barY, barW, barH, barBg);
		DrawRectangle(barX, barY, (int)(barW * loadProgress / 100), barH, barFill);
		DrawRectangle(barX, barY, (int)(barW * loadProgress / 100), barH / 2,
			Fade(WHITE, 0.2f));

		const char* loadText = (loadProgress < 100) ? "Loading..." : "Ready";
		DrawText(loadText,
			(currentW - MeasureText(loadText, 20)) / 2,
			barY + 30, 20, textMain);

		EndDrawing();
	}

	// Main game loop
	while (!WindowShouldClose()) {
		UpdateMusicStream(bgmusic);

		int currentW = GetScreenWidth();
		int currentH = GetScreenHeight();
		particles.Resize(currentW, currentH);
		particles.Update();

		if (state == 0) {
			Vector2 mousePos = GetMousePosition();

			Rectangle rec = {
				currentW * 0.20f - button1.width / 2.0f,
				currentH / 2.0f - button1.height / 2.0f + 50,
				(float)button1.width, (float)button1.height
			};
			Rectangle rec2 = {
				currentW * 0.50f - button2.width / 2.0f,
				currentH / 2.0f - button2.height / 2.0f + 50,
				(float)button2.width, (float)button2.height
			};
			Rectangle rec3 = {
				currentW * 0.80f - button3.width / 2.0f,
				currentH / 2.0f - button3.height / 2.0f + 50,
				(float)button3.width, (float)button3.height
			};

			if (IsKeyPressed(KEY_F)) ToggleFullscreen();

			BeginDrawing();
			DrawGradientBackground(currentW, currentH);
			particles.Draw();

			DrawTexture(button, currentW / 2 - button.width / 2, 50, WHITE);
			DrawTexture(button1, (int)rec.x, (int)rec.y, WHITE);
			DrawTexture(button2, (int)rec2.x, (int)rec2.y, WHITE);
			DrawTexture(button3, (int)rec3.x, (int)rec3.y, WHITE);

			if (CheckCollisionPointRec(mousePos, rec)) {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					state = 1;
					maze_generation(grid, stack);
					player.Reset();
					waterSystem.Reset();
					gameTimer = 0;
					hasWon = false;
				}
				DrawRectangleLinesEx(rec, 3.0f, border);
			}
			else if (CheckCollisionPointRec(mousePos, rec2)) {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					state = 2;
				}
				DrawRectangleLinesEx(rec2, 3.0f, border);
			}
			else if (CheckCollisionPointRec(mousePos, rec3)) {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					state = 3;
				}
				DrawRectangleLinesEx(rec3, 3.0f, border);
			}

			DrawText("Press F for Fullscreen",
				(currentW - MeasureText("Press F for Fullscreen", 16)) / 2,
				currentH - 40, 16, Fade(textMain, 0.7f));

			EndDrawing();
		}

		else if (state == 1) {
			gameTimer += GetFrameTime();

			if (!waterSystem.IsGameOver() && !hasWon) {
				player.Update(grid);
				waterSystem.Update(player.x, player.y, GetFrameTime());
				hasWon = player.HasReachedExit();
			}

			BeginDrawing();
			DrawGradientBackground(currentW, currentH);
			particles.Draw();

			// Calculate maze offset to center it
			int mazeWidth = width * CELL_SIZE;
			int mazeHeight = height * CELL_SIZE;
			int offsetX = (currentW - mazeWidth) / 2;
			int offsetY = (currentH - mazeHeight) / 2;

			// Draw maze background
			DrawRectangle(offsetX - 20, offsetY - 20, mazeWidth + 40, mazeHeight + 40,
				Fade(BLACK, 0.5f));
			DrawRectangleLinesEx({ (float)(offsetX - 20), (float)(offsetY - 20),
								  (float)(mazeWidth + 40), (float)(mazeHeight + 40) },
				2.0f, barBorder);

			// Draw maze cells
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					int px = offsetX + x * CELL_SIZE;
					int py = offsetY + y * CELL_SIZE;
					DrawRectangle(px, py, CELL_SIZE, CELL_SIZE, Fade(purpleTop, 0.2f));
				}
			}

			// Draw maze walls
			Color wallColor = { 200, 180, 255, 255 };
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					int px = offsetX + x * CELL_SIZE;
					int py = offsetY + y * CELL_SIZE;
					int idx = index(x, y);

					if (grid[idx].walls[0])
						DrawLineEx({ (float)px, (float)py },
							{ (float)(px + CELL_SIZE), (float)py }, 2.0f, wallColor);
					if (grid[idx].walls[1])
						DrawLineEx({ (float)px, (float)(py + CELL_SIZE) },
							{ (float)(px + CELL_SIZE), (float)(py + CELL_SIZE) }, 2.0f, wallColor);
					if (grid[idx].walls[2])
						DrawLineEx({ (float)px, (float)py },
							{ (float)px, (float)(py + CELL_SIZE) }, 2.0f, wallColor);
					if (grid[idx].walls[3])
						DrawLineEx({ (float)(px + CELL_SIZE), (float)py },
							{ (float)(px + CELL_SIZE), (float)(py + CELL_SIZE) }, 2.0f, wallColor);
				}
			}

			DrawRectangle(offsetX + 5, offsetY + 5, CELL_SIZE - 10, CELL_SIZE - 10,
				Fade(BLUE, 0.3f));
			DrawText("START", offsetX + 7, offsetY + CELL_SIZE / 2 - 5, 10, BLUE);

			float exitX = (width - 1) * CELL_SIZE + CELL_SIZE / 2;
			float exitY = (height - 1) * CELL_SIZE + CELL_SIZE / 2;
			DrawCircle(offsetX + exitX, offsetY + exitY, CELL_SIZE / 3, Fade(GREEN, 0.3f));
			DrawCircle(offsetX + exitX, offsetY + exitY, CELL_SIZE / 4, GREEN);
			DrawText("EXIT", offsetX + exitX - 12, offsetY + exitY - 5, 10, WHITE);

			waterSystem.Draw(offsetX, offsetY);

			player.Draw(offsetX, offsetY, waterSystem.isPlayerUnderwater);

			DrawRectangle(0, 0, currentW, 60, Fade(BLACK, 0.5f));
			DrawText("FLOOD ESCAPE",
				(currentW - MeasureText("FLOOD ESCAPE", 36)) / 2,
				10, 36, textAccent);

			DrawText(TextFormat("Time: %.1fs", gameTimer), currentW - 120, 10, 16, WHITE);
			DrawText(TextFormat("Water: %.0f%%", waterSystem.GetWaterPercentage()),
				currentW - 120, 30, 16, BLUE);

			waterSystem.DrawUI(currentW, currentH);

			DrawText("WASD/Arrows: Move | Collect AIR BUBBLES | Activate DRAINS | Reach EXIT",
				(currentW - MeasureText("WASD/Arrows: Move | Collect AIR BUBBLES | Activate DRAINS | Reach EXIT", 14)) / 2,
				currentH - 30, 14, textMain);

			if (waterSystem.IsGameOver()) {
				
				for (int y = 0; y < GetScreenHeight()/2; y++) {
					float t = (float)y / GetScreenHeight();
					Color lineCol = {
						(purpleTop.r* (1 - t) + blackBottom.r * t),
						(purpleTop.g* (1 - t) + blackBottom.g * t),
						(purpleTop.b* (1 - t) + blackBottom.b * t),
						255
					};
					DrawLine(0, y, GetScreenWidth(), y, lineCol);
				}
				for (int y = GetScreenHeight(); y >= GetScreenHeight()/2; y--) {
					float	t = (float)y / GetScreenHeight();
					Color lineCol = {
						(purpleTop.r * (1 - t) + blackBottom.r * t),
						(purpleTop.g * (1 - t) + blackBottom.g * t),
						(purpleTop.b * (1 - t) + blackBottom.b * t),
						255
					};
					DrawLine(0, y, GetScreenWidth(), y, lineCol);
				}
				DrawRectangle(currentW / 2 - 200, currentH / 2 - 80, 400, 160, Fade(BLACK, 0.8f));
				DrawText("GAME OVER",
					(currentW - MeasureText("GAME OVER", 40)) / 2,
					currentH / 2 - 60, 40, RED);
				DrawText("You drowned!",
					(currentW - MeasureText("You drowned!", 20)) / 2,
					currentH / 2 - 10, 20, WHITE);
				DrawText("Press ENTER to retry or TAB for menu",
					(currentW - MeasureText("Press ENTER to retry or TAB for menu", 16)) / 2,
					currentH / 2 + 30, 16, textMain);

				if (IsKeyPressed(KEY_ENTER)) {
					maze_generation(grid, stack);
					player.Reset();
					waterSystem.Reset();
					gameTimer = 0;
					hasWon = false;
				}
			}

			// Win screen
			if (hasWon) {
				DrawRectangle(currentW / 2 - 200, currentH / 2 - 80, 400, 160, Fade(BLACK, 0.8f));
				DrawText("YOU ESCAPED!",
					(currentW - MeasureText("YOU ESCAPED!", 40)) / 2,
					currentH / 2 - 60, 40, GREEN);
				DrawText(TextFormat("Time: %.1f seconds", gameTimer),
					(currentW - MeasureText(TextFormat("Time: %.1f seconds", gameTimer), 20)) / 2,
					currentH / 2 - 10, 20, WHITE);
				DrawText("Press ENTER for new maze or TAB for menu",
					(currentW - MeasureText("Press ENTER for new maze or TAB for menu", 16)) / 2,
					currentH / 2 + 30, 16, textMain);

				if (IsKeyPressed(KEY_ENTER)) {
					maze_generation(grid, stack);
					player.Reset();
					waterSystem.Reset();
					gameTimer = 0;
					hasWon = false;
				}
			}

			if (IsKeyPressed(KEY_TAB)) {
				state = 0;
			}

			EndDrawing();
		}

		else if (state == 2) {
			BeginDrawing();
			DrawGradientBackground(currentW, currentH);
			particles.Draw();

			int panelW = 500, panelH = 380;
			int panelX = (currentW - panelW) / 2;
			int panelY = (currentH - panelH) / 2;

			DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.7f));
			DrawRectangleLinesEx({ (float)panelX, (float)panelY, (float)panelW, (float)panelH },
				2.0f, barBorder);

			DrawText("FLOOD ESCAPE - ABOUT",
				(currentW - MeasureText("FLOOD ESCAPE - ABOUT", 28)) / 2,
				panelY + 30, 28, textAccent);

			DrawLine(panelX + 50, panelY + 70, panelX + panelW - 50, panelY + 70,
				Fade(barFill, 0.5f));

			int yPos = panelY + 90;
			DrawText("SURVIVE THE RISING FLOOD!",
				(currentW - MeasureText("SURVIVE THE RISING FLOOD!", 20)) / 2,
				yPos, 20, BLUE);

			yPos += 40;
			DrawText("How to Play:", panelX + 30, yPos, 18, textAccent);

			yPos += 25;
			DrawText("• Use WASD or Arrow Keys to move", panelX + 40, yPos, 14, textMain);

			yPos += 20;
			DrawText("• Water constantly rises - don't drown!", panelX + 40, yPos, 14, textMain);

			yPos += 20;
			DrawText("• Collect AIR BUBBLES to refill oxygen", panelX + 40, yPos, 14, SKYBLUE);

			yPos += 20;
			DrawText("• Activate DRAIN SWITCHES to slow/reverse flooding", panelX + 40, yPos, 14, GREEN);

			yPos += 20;
			DrawText("• Reach the GREEN EXIT to win", panelX + 40, yPos, 14, textMain);

			yPos += 30;
			DrawText("Strategy Tips:", panelX + 30, yPos, 18, textAccent);

			yPos += 25;
			DrawText("• Find drains quickly to control water level", panelX + 40, yPos, 14, textMain);

			yPos += 20;
			DrawText("• Save air bubbles for emergencies", panelX + 40, yPos, 14, textMain);

			yPos += 20;
			DrawText("• Learn the maze layout before water rises", panelX + 40, yPos, 14, textMain);

			DrawText("Press TAB to return",
				(currentW - MeasureText("Press TAB to return", 20)) / 2,
				panelY + panelH - 35, 20, YELLOW);

			if (IsKeyPressed(KEY_TAB)) state = 0;

			EndDrawing();
		}

		else if (state == 3) {
			break;
		}
	}

	UnloadMusicStream(bgmusic);
	CloseAudioDevice();
	UnloadTexture(button);
	UnloadTexture(button1);
	UnloadTexture(button2);
	UnloadTexture(button3);
	CloseWindow();
	return 0;
}
