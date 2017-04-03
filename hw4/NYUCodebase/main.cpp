#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <math.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>



#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

float GRAVITY_Y = -750.0;
int FPS = 20;
SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}



struct entity
{
	void Draw(ShaderProgram program) {
		Matrix M;
		M.Translate(x / (320.0f/3.55f), y / (180.0f/2.0f), 0.0f);
		M.Scale(width / 90.0f, height / 90.0f, 1.0f);
		M.Rotate(rotation);
		model = M;

		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		program.setModelMatrix(model);
		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

	}

	void setLocation(float a, float b) {
		x = a;
		y = b;
	}

	void setSize(float x, float y) {
		width = x;
		height = y;
	}

	int checkCollision(entity enemy) {
		float leftBound = x - (width / 2);
		float rightBound = (width / 2) + x;
		float upBound = (height / 2) + y;
		float downBound = y- (height / 2);
		float eLeftBound = enemy.x - (enemy.width / 2);
		float eRightBound = (enemy.width / 2) + enemy.x;
		float eUpBound = (enemy.height / 2) + enemy.y;
		float eDownBound = enemy.y - (enemy.height / 2);
		if (leftBound <= eLeftBound && eLeftBound <= rightBound) {
			if ((downBound <= eUpBound && eUpBound <= upBound)) { 
				return 1; //enemy is hitting the right bottom
			}
			if (downBound <= eDownBound && eDownBound <= upBound) { 
				return 2; //enemy is hitting the right top
			}
			else return 0;
		}
		else if (leftBound <= eRightBound && eRightBound <= rightBound) {
			if ((downBound <= eUpBound && eUpBound <= upBound)) {
				return 3; //enemy is hitting left bottom
			}
			if (downBound <= eDownBound && eDownBound <= upBound) {
				return 4; //enemy is hitting left top
			}
			else return 0;
		}
		else return 0;
	}

	void update(float elapsed) {
		x += velocity_x*elapsed;
		y += velocity_y*elapsed;
		velocity_x += acceleration_x*elapsed;
		velocity_y += (acceleration_y)*elapsed;
	}

	float x = 0;
	float y = 0;
	float rotation = 0;
	//int textureID;
	float width = 90;
	float height = 90;
	float speed = 180;
	float velocity_x = 0;
	float velocity_y= 0;
	float acceleration_x = 0;
	float acceleration_y = 0;
	float angle = 0;


	Matrix model;
	GLuint texture;

};


struct sprite : entity {
	void Draw(ShaderProgram program, int index = 0) {
		Matrix M;
		M.Translate(x / (320.0f / 3.55f), y / (180.0f / 2.0f), 0.0f);
		M.Scale(width / 90.0f, height / 90.0f, 1.0f);
		M.Rotate(rotation);
		model = M;

		float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
		float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0f / (float)spriteCountX;
		float spriteHeight = 1.0f / (float)spriteCountY;
		GLfloat texCoords[] = {
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight
		};
		float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f,
			-0.5f, 0.5f, -0.5f };

		program.setModelMatrix(model);
		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

	int spriteCountX = 1;
	int spriteCountY = 1;
};

struct character : entity {
	void update(float elapsed) {
		x += velocity_x*elapsed;
		y += velocity_y*elapsed;
		velocity_x += acceleration_x*elapsed;
		velocity_y += (acceleration_y + GRAVITY_Y)*elapsed;
		animationCD += elapsed;
		if (animationCD >= 1.0 / FPS) {
			if (animationState >= animations[currentAnimation].size() - 1) {
				animationState = 0;
			}
			else {
				animationState++;
			}
			animationCD = 0.0;
			texture = animations[currentAnimation][animationState];
		}
	}
	void addAnimation(std::vector<GLuint> anim) {
		animations.push_back(anim);
	}

	int currentAnimation = 0;
	int animationState = 0;
	std::vector<std::vector<GLuint>> animations;
	float animationCD = 0;
	bool allowjump = true;

};

struct tileArray : entity {
	void Draw(ShaderProgram program) {
		for (int tile = 0; tile < 16; tile++) {
			if (tileIndex[tile] != NULL) {
				Matrix M;
				M.Translate((-300.0f + 640.0f * (int)screen + 40.0f * (int)tile) / (320.0f / 3.55f), (-160.0f + 40.0f * (int)layer) / (180.0f / 2.0f), 0.0f);
				M.Scale(40.0f / 90.0f, 40.0f / 90.0f, 1.0f);
				M.Rotate(rotation);
				model = M;

				float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
				float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
				program.setModelMatrix(model);
				glBindTexture(GL_TEXTURE_2D, tileBank[tileIndex[tile]]);
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(program.positionAttribute);
				glDisableVertexAttribArray(program.texCoordAttribute);
			}
			
		}

	}

	void loadIndex(int a[16]) {
		for (int i = 0; i < 16; i++) {
			tileIndex[i] = a[i];
		}

	}

	int layer = 0;
	int screen = 0;
	int tileIndex[16];
	std::vector<GLuint> tileBank = { 0 };

};


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	SDL_Event event;
	bool done = false;

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Matrix projectionMatrix;
	Matrix viewMatrix;
	//viewMatrix.Translate(3.55, 0.0, 0.0);
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);
	float lastFrameTicks = 0.0f;

	//--------init-----------
	sprite alphabet;
	alphabet.texture = LoadTexture("Untitled.png");
	alphabet.spriteCountX = 8;
	alphabet.spriteCountY = 12;

	entity background;
	background.setSize(640, 360);
	background.texture = LoadTexture("BG.png");

	entity background2;
	background2.setSize(640.0f, 360.0f);
	background2.x = 640.0f;
	background2.texture = LoadTexture("BG.png");

	entity background3;
	background3.setSize(640.0f, 360.0f);
	background3.x = 1280.0f;
	background3.texture = LoadTexture("BG.png");

	character dog;
	dog.setSize(60, 60);
	dog.x = -160.0f;

	std::vector<GLuint> idleDog;
	idleDog.push_back(LoadTexture("Idle1.png"));
	idleDog.push_back(LoadTexture("Idle2.png"));
	idleDog.push_back(LoadTexture("Idle3.png"));
	idleDog.push_back(LoadTexture("Idle4.png"));
	idleDog.push_back(LoadTexture("Idle5.png"));
	idleDog.push_back(LoadTexture("Idle6.png"));
	idleDog.push_back(LoadTexture("Idle7.png"));
	idleDog.push_back(LoadTexture("Idle8.png"));
	idleDog.push_back(LoadTexture("Idle9.png"));
	idleDog.push_back(LoadTexture("Idle10.png"));

	std::vector<GLuint> runDog;
	runDog.push_back(LoadTexture("Run (1).png"));
	runDog.push_back(LoadTexture("Run (2).png"));
	runDog.push_back(LoadTexture("Run (3).png"));
	runDog.push_back(LoadTexture("Run (4).png"));
	runDog.push_back(LoadTexture("Run (5).png"));
	runDog.push_back(LoadTexture("Run (6).png"));
	runDog.push_back(LoadTexture("Run (7).png"));
	runDog.push_back(LoadTexture("Run (8).png"));

	std::vector<GLuint> dieDog;
	dieDog.push_back(LoadTexture("Dead1.png"));
	dieDog.push_back(LoadTexture("Dead2.png"));
	dieDog.push_back(LoadTexture("Dead3.png"));
	dieDog.push_back(LoadTexture("Dead4.png"));
	dieDog.push_back(LoadTexture("Dead5.png"));
	dieDog.push_back(LoadTexture("Dead6.png"));
	dieDog.push_back(LoadTexture("Dead7.png"));
	dieDog.push_back(LoadTexture("Dead8.png"));
	dieDog.push_back(LoadTexture("Dead9.png"));
	dieDog.push_back(LoadTexture("Dead10.png"));

	dog.addAnimation(idleDog);
	dog.addAnimation(runDog);
	dog.addAnimation(dieDog);



	tileArray floor;
	floor.tileBank.push_back(LoadTexture("1.png"));
	floor.tileBank.push_back(LoadTexture("2.png"));
	floor.tileBank.push_back(LoadTexture("3.png"));
	int a[16] = {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3};
	floor.loadIndex(a);

	tileArray floor2;
	floor2.tileBank.push_back(LoadTexture("14.png"));
	floor2.tileBank.push_back(LoadTexture("15.png"));
	floor2.tileBank.push_back(LoadTexture("16.png"));
	int b[16] = { 1,2,3,0,1,2,2,2,3,0,0,1,2,2,2,3 };
	floor2.layer = 2;
	floor2.screen = 1;
	floor2.loadIndex(b);

	entity floorHitbox;
	floorHitbox.texture = LoadTexture("red.jpg");
	floorHitbox.setSize(640.0f, 40.0f);
	floorHitbox.y = -160.0f;

	entity floorHitbox2;
	floorHitbox2.texture = LoadTexture("red.jpg");
	floorHitbox2.setSize(120.0f, 40.0f);
	floorHitbox2.setLocation(380.0f, -80.0f);

	entity floorHitbox3;
	floorHitbox3.texture = LoadTexture("red.jpg");
	floorHitbox3.setSize(200.0f, 40.0f);
	floorHitbox3.setLocation(580.0f, -80.0f);

	entity floorHitbox4;
	floorHitbox4.texture = LoadTexture("red.jpg");
	floorHitbox4.setSize(200.0f, 40.0f);
	floorHitbox4.setLocation(860.0f, -80.0f);

	entity cat;
	cat.setSize(60.0f, 60.0f);
	cat.setLocation(930.0f, -30.0f);
	cat.texture = LoadTexture("cat.png");

	bool end = false;

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		//-------------------ENTER STUFFE PAST HERE-----------
		
		
		/*
		if (dog.checkCollision(floorHitbox) == 1 || dog.checkCollision(floorHitbox) == 3) {
			dog.velocity_y = 0;
		}
		*/
		if (floorHitbox.checkCollision(dog)) {
			dog.velocity_y = 0;
			dog.allowjump = true;
			dog.y = floorHitbox.y + floorHitbox.height/2 + dog.height/2;
		}



		if (floorHitbox2.checkCollision(dog)) {
			dog.velocity_y = 0;
			dog.allowjump = true;
			dog.y = floorHitbox2.y + floorHitbox2.height / 2 + dog.height / 2;
		}
		if (floorHitbox3.checkCollision(dog)) {
			dog.velocity_y = 0;
			dog.allowjump = true;
		}
		if (floorHitbox4.checkCollision(dog)) {
			dog.velocity_y = 0;
			dog.allowjump = true;
		}

		if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_UP]) {
			dog.currentAnimation = 1;
			if (keys[SDL_SCANCODE_RIGHT]) {
				dog.velocity_x = 150.0f;
			}
			if (keys[SDL_SCANCODE_UP] && dog.allowjump) {
				dog.velocity_y = 500;
				dog.allowjump = false;
			}
		}
		else if (cat.checkCollision(dog)) {
			dog.velocity_x = 0.0f;
			dog.velocity_y = 0.0f;
			dog.currentAnimation = 2;
			if (dog.animationState == 10 || dog.animationState == 9) {
				end = true;
			}
		}
		else {
			dog.velocity_x = 0.0f;
			dog.currentAnimation = 0;
		}

		if (dog.x > 0) {
			Matrix M;
			M.Translate(-dog.x / (320.0f / 3.55f), 0, 0);
			viewMatrix=M;

		}
		



		//-------------ANYTHING AFTER HERE IS A DRAW CALL-----
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		//float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		//float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		if(!end){ dog.update(elapsed); }
		//alphabet.Draw(program);
		background.Draw(program);
		background2.Draw(program);
		background3.Draw(program);

		floor.Draw(program);
		//floorHitbox.Draw(program);
		floor2.Draw(program);
		//floorHitbox2.Draw(program);
		//floorHitbox3.Draw(program);
		//floorHitbox4.Draw(program);
		cat.Draw(program);
		dog.Draw(program);



		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
