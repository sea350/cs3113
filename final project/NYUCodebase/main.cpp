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
#include <string>
#include <algorithm>



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


struct Vector {
	Vector(float width = 0.0f, float height = 0.0f) {
		x = width;
		y = height;
	}

	float length() const {
		return sqrt(x*x + y*y);
	}
	float x;
	float y;
};

struct entity
{
	void Draw(ShaderProgram &program) {
		Matrix M;
		M.Translate(x / (320.0f / 3.55f), y / (180.0f / 2.0f), 0.0f);
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

	void update(float elapsed) {
		x += velocity_x*elapsed;
		y += velocity_y*elapsed;
		velocity_x += acceleration_x*elapsed;
		velocity_y += (acceleration_y)*elapsed;
	}

	std::vector<Vector> getVertices() {
		std::vector<Vector> vertices;

		vertices.push_back(Vector(-(width/2) * cos(rotation) + (-(height/2) * (-sin(rotation))) + x,
			-(width / 2) * sin(rotation) - (height / 2) * (cos(rotation)) + y));
		vertices.push_back(Vector((width / 2) * cos(rotation) - (height / 2) * (-sin(rotation)) + x,
			(width / 2) * sin(rotation) - (height / 2) * (cos(rotation)) + y));
		vertices.push_back(Vector((width / 2) * cos(rotation) + (height / 2) * (-sin(rotation)) + x,
			(width / 2) * sin(rotation) + (height / 2) * (cos(rotation)) + y));
		vertices.push_back(Vector(-(width / 2) * cos(rotation) + (height / 2) * (-sin(rotation)) + x,
			-(width / 2) * sin(rotation) + (height / 2) * (cos(rotation)) + y));


		return vertices;
	}

	float x = 0;
	float y = 0;
	float rotation = 0;
	float width = 90;
	float wi = width / 90;
	float height = 90;
	float hi = height / 90;
	float speed = 180;
	float velocity_x = 0;
	float velocity_y = 0;
	float acceleration_x = 0;
	float acceleration_y = 0;
	float angle = 0;

	bool active = true;

	Matrix model;
	GLuint texture;

};



bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2, Vector &penetration) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];

	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p >= 0) {
		return false;
	}

	float penetrationMin1 = e1Max - e2Min;
	float penetrationMin2 = e2Max - e1Min;

	float penetrationAmount = penetrationMin1;
	if (penetrationMin2 < penetrationAmount) {
		penetrationAmount = penetrationMin2;
	}

	penetration.x = normalX * penetrationAmount;
	penetration.y = normalY * penetrationAmount;

	return true;
}

bool penetrationSort(const Vector &p1, const Vector &p2) {
	return (p1.length() < p2.length());
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points, Vector &penetration) {
	std::vector<Vector> penetrations;
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}
		Vector penetration;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
		if (!result) {
			return false;
		}
		penetrations.push_back(penetration);
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		Vector penetration;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);

		if (!result) {
			return false;
		}
		penetrations.push_back(penetration);
	}

	std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
	penetration = penetrations[0];

	Vector e1Center;
	for (int i = 0; i < e1Points.size(); i++) {
		e1Center.x += e1Points[i].x;
		e1Center.y += e1Points[i].y;
	}
	e1Center.x /= (float)e1Points.size();
	e1Center.y /= (float)e1Points.size();

	Vector e2Center;
	for (int i = 0; i < e2Points.size(); i++) {
		e2Center.x += e2Points[i].x;
		e2Center.y += e2Points[i].y;
	}
	e2Center.x /= (float)e2Points.size();
	e2Center.y /= (float)e2Points.size();

	Vector ba;
	ba.x = e1Center.x - e2Center.x;
	ba.y = e1Center.y - e2Center.y;

	if ((penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f) {
		penetration.x *= -1.0f;
		penetration.y *= -1.0f;
	}

	return true;
}


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
				M.Translate((-300.0f + 640.0f * (int)screen + 40.0f * (int)tile) / (320.0f / 3.55f), (-170.0f + 20.0f * (int)layer) / (180.0f / 2.0f), 0.0f);
				M.Scale(40.0f / 90.0f, 20.0f / 90.0f, 1.0f);
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
	entity viewFinder;

	sprite alphabet;
	alphabet.texture = LoadTexture("Untitled.png");
	alphabet.spriteCountX = 8;
	alphabet.spriteCountY = 12;
	alphabet.width = 20;
	alphabet.height = 20;


	// dog init
	character dog;
	dog.setSize(30, 30);
	dog.setLocation(180, -80);
	dog.setSize(120, 120);

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

	dog.addAnimation(idleDog);
	dog.addAnimation(runDog);


	//cat init
	character cat;
	cat.setLocation(-180, -80);
	cat.setSize(120, 120);

	std::vector<GLuint> idleCat;
	idleCat.push_back(LoadTexture("catIdle1.png"));
	idleCat.push_back(LoadTexture("catIdle2.png"));
	idleCat.push_back(LoadTexture("catIdle3.png"));
	idleCat.push_back(LoadTexture("catIdle4.png"));
	idleCat.push_back(LoadTexture("catIdle5.png"));
	idleCat.push_back(LoadTexture("catIdle6.png"));
	idleCat.push_back(LoadTexture("catIdle7.png"));
	idleCat.push_back(LoadTexture("catIdle8.png"));
	idleCat.push_back(LoadTexture("catIdle9.png"));
	idleCat.push_back(LoadTexture("catIdle10.png"));

	std::vector<GLuint> runCat;
	runCat.push_back(LoadTexture("catRun1.png"));
	runCat.push_back(LoadTexture("catRun2.png"));
	runCat.push_back(LoadTexture("catRun3.png"));
	runCat.push_back(LoadTexture("catRun4.png"));
	runCat.push_back(LoadTexture("catRun5.png"));
	runCat.push_back(LoadTexture("catRun6.png"));
	runCat.push_back(LoadTexture("catRun7.png"));
	runCat.push_back(LoadTexture("catRun8.png"));

	cat.addAnimation(idleCat);
	cat.addAnimation(runCat);


	//	desert terrain------------------------------------------------------------------------
	entity background;
	background.setSize(640, 180);
	background.y += 90;
	background.texture = LoadTexture("BG.png");

	entity background2;
	background2.setSize(640.0f, 180.0f);
	background2.x = 640.0f;
	background2.y += 90;
	background2.texture = LoadTexture("BG.png");

	entity background3;
	background3.setSize(640.0f, 180.0f);
	background3.x = 1280.0f;
	background3.y += 90;
	background3.texture = LoadTexture("BG.png");

	entity background4;
	background4.setSize(640.0f, 180.0f);
	background4.x = 1920.0f;
	background4.y += 90;
	background4.texture = LoadTexture("BG.png");

	entity background5;
	background5.setSize(640.0f, 180.0f);
	background5.x = 2560.0f;
	background5.y += 90;
	background5.texture = LoadTexture("BG.png");

	tileArray floor;
	floor.tileBank.push_back(LoadTexture("1.png"));
	floor.tileBank.push_back(LoadTexture("2.png"));
	floor.tileBank.push_back(LoadTexture("3.png"));
	int a[16] = {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3};
	floor.layer = 9;
	floor.loadIndex(a);

	tileArray floor2;
	floor2.tileBank.push_back(LoadTexture("14.png"));
	floor2.tileBank.push_back(LoadTexture("15.png"));
	floor2.tileBank.push_back(LoadTexture("16.png"));
	int b[16] = { 1,2,3,0,1,2,2,2,3,0,0,1,2,2,2,3 };
	floor2.layer = 11;
	floor2.screen = 1;
	floor2.loadIndex(b);

	tileArray floor3a;
	floor3a.tileBank.push_back(LoadTexture("14.png"));
	floor3a.tileBank.push_back(LoadTexture("15.png"));
	floor3a.tileBank.push_back(LoadTexture("16.png"));
	int c[16] = { 1,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0 };
	floor3a.layer = 13;
	floor3a.screen = 2;
	floor3a.loadIndex(c);

	tileArray floor3b;
	floor3b.tileBank.push_back(LoadTexture("14.png"));
	floor3b.tileBank.push_back(LoadTexture("15.png"));
	floor3b.tileBank.push_back(LoadTexture("16.png"));
	int d[16] = { 0,0,0,0,0,0,0,0,0,1,2,2,2,2,2,3 };
	floor3b.layer = 9;
	floor3b.screen = 2;
	floor3b.loadIndex(d);

	tileArray floor4a;
	floor4a.tileBank.push_back(LoadTexture("14.png"));
	floor4a.tileBank.push_back(LoadTexture("15.png"));
	floor4a.tileBank.push_back(LoadTexture("16.png"));
	int e[16] = { 0,0,1,3,0,0,0,0,0,0,0,0,0,0,0,0 };
	floor4a.layer = 11;
	floor4a.screen = 3;
	floor4a.loadIndex(e);

	tileArray floor4b;
	floor4b.tileBank.push_back(LoadTexture("14.png"));
	floor4b.tileBank.push_back(LoadTexture("15.png"));
	floor4b.tileBank.push_back(LoadTexture("16.png"));
	int f[16] = { 0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0 };
	floor4b.layer = 12;
	floor4b.screen = 3;
	floor4b.loadIndex(f);

	tileArray floor4c;
	floor4c.tileBank.push_back(LoadTexture("14.png"));
	floor4c.tileBank.push_back(LoadTexture("15.png"));
	floor4c.tileBank.push_back(LoadTexture("16.png"));
	int g[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0 };
	floor4c.layer = 9;
	floor4c.screen = 3;
	floor4c.loadIndex(g);

	entity floorHitbox;
	floorHitbox.texture = LoadTexture("red.jpg");
	floorHitbox.setSize(640.0f, 20.0f);
	floorHitbox.y = -170.0f + 20.0f * floor.layer;

	entity floorHitbox2a;
	floorHitbox2a.texture = LoadTexture("red.jpg");
	floorHitbox2a.setSize(120.0f, 20.0f);
	floorHitbox2a.setLocation(380.0f, -170.0f + 20.0f * floor2.layer);

	entity floorHitbox2b;
	floorHitbox2b.texture = LoadTexture("red.jpg");
	floorHitbox2b.setSize(200.0f, 20.0f);
	floorHitbox2b.setLocation(580.0f, -170.0f + 20.0f * floor2.layer);

	entity floorHitbox2c;
	floorHitbox2c.texture = LoadTexture("red.jpg");
	floorHitbox2c.setSize(200.0f, 20.0f);
	floorHitbox2c.setLocation(860.0f, -170.0f + 20.0f * floor2.layer);

	entity floorHitbox3a;
	floorHitbox3a.texture = LoadTexture("red.jpg");
	floorHitbox3a.setSize(200.0f, 20.0f);
	floorHitbox3a.setLocation(1060.0f, -170.0f + 20.0f * floor3a.layer);

	entity floorHitbox3b;
	floorHitbox3b.texture = LoadTexture("red.jpg");
	floorHitbox3b.setSize(280.0f, 20.0f);
	floorHitbox3b.setLocation(1460.0f, -170.0f + 20.0f * floor3b.layer);

	entity floorHitbox4a;
	floorHitbox4a.texture = LoadTexture("red.jpg");
	floorHitbox4a.setSize(80.0f, 20.0f);
	floorHitbox4a.setLocation(1720.0f, -170.0f + 20.0f * floor4a.layer);

	entity floorHitbox4b;
	floorHitbox4b.texture = LoadTexture("red.jpg");
	floorHitbox4b.setSize(120.0f, 20.0f);
	floorHitbox4b.setLocation(1940.0f, -170.0f + 20.0f * floor4b.layer);

	entity floorHitbox4c;
	floorHitbox4c.texture = LoadTexture("red.jpg");
	floorHitbox4c.setSize(40.0f, 20.0f);
	floorHitbox4c.setLocation(2140.0f, -170.0f + 20.0f * floor4c.layer);

	//	snow terrain-----------------------------------------------------------------------------
	entity backgroundSnow;
	backgroundSnow.setSize(640, 180);
	backgroundSnow.y = -90;
	backgroundSnow.texture = LoadTexture("snowBG.png");

	entity background2Snow;
	background2Snow.setSize(640.0f, 180.0f);
	background2Snow.x = 640.0f;
	background2Snow.y = -90;
	background2Snow.texture = LoadTexture("snowBG.png");

	entity background3Snow;
	background3Snow.setSize(640.0f, 180.0f);
	background3Snow.x = 1280.0f;
	background3Snow.y = -90;
	background3Snow.texture = LoadTexture("snowBG.png");

	entity background4Snow;
	background4Snow.setSize(640.0f, 180.0f);
	background4Snow.x = 1920.0f;
	background4Snow.y = -90;
	background4Snow.texture = LoadTexture("snowBG.png");

	entity background5Snow;
	background5Snow.setSize(640.0f, 180.0f);
	background5Snow.x = 2560.0f;
	background5Snow.y = -90;
	background5Snow.texture = LoadTexture("snowBG.png");

	tileArray floorSnow;
	floorSnow.tileBank.push_back(LoadTexture("snow1.png"));
	floorSnow.tileBank.push_back(LoadTexture("snow2.png"));
	floorSnow.tileBank.push_back(LoadTexture("snow3.png"));
	floorSnow.layer = 0;
	floorSnow.loadIndex(a);

	tileArray floor2Snow;
	floor2Snow.tileBank.push_back(LoadTexture("snow14.png"));
	floor2Snow.tileBank.push_back(LoadTexture("snow15.png"));
	floor2Snow.tileBank.push_back(LoadTexture("snow16.png"));
	floor2Snow.layer = 2;
	floor2Snow.screen = 1;
	floor2Snow.loadIndex(b);

	tileArray floor3aSnow;
	floor3aSnow.tileBank.push_back(LoadTexture("snow14.png"));
	floor3aSnow.tileBank.push_back(LoadTexture("snow15.png"));
	floor3aSnow.tileBank.push_back(LoadTexture("snow16.png"));
	floor3aSnow.layer = 4;
	floor3aSnow.screen = 2;
	floor3aSnow.loadIndex(c);

	tileArray floor3bSnow;
	floor3bSnow.tileBank.push_back(LoadTexture("snow14.png"));
	floor3bSnow.tileBank.push_back(LoadTexture("snow15.png"));
	floor3bSnow.tileBank.push_back(LoadTexture("snow16.png"));
	floor3bSnow.layer = 0;
	floor3bSnow.screen = 2;
	floor3bSnow.loadIndex(d);

	tileArray floor4aSnow;
	floor4aSnow.tileBank.push_back(LoadTexture("snow14.png"));
	floor4aSnow.tileBank.push_back(LoadTexture("snow15.png"));
	floor4aSnow.tileBank.push_back(LoadTexture("snow16.png"));
	floor4aSnow.layer = 2;
	floor4aSnow.screen = 3;
	floor4aSnow.loadIndex(e);

	tileArray floor4bSnow;
	floor4bSnow.tileBank.push_back(LoadTexture("snow14.png"));
	floor4bSnow.tileBank.push_back(LoadTexture("snow15.png"));
	floor4bSnow.tileBank.push_back(LoadTexture("snow16.png"));
	floor4bSnow.layer = 3;
	floor4bSnow.screen = 3;
	floor4bSnow.loadIndex(f);

	tileArray floor4cSnow;
	floor4cSnow.tileBank.push_back(LoadTexture("snow14.png"));
	floor4cSnow.tileBank.push_back(LoadTexture("snow15.png"));
	floor4cSnow.tileBank.push_back(LoadTexture("snow16.png"));
	floor4cSnow.layer = 0;
	floor4cSnow.screen = 3;
	floor4cSnow.loadIndex(g);

	entity floorHitboxSnow;
	floorHitboxSnow.texture = LoadTexture("red.jpg");
	floorHitboxSnow.setSize(640.0f, 20.0f);
	floorHitboxSnow.y = -170.0f + 20.0f * floorSnow.layer;

	entity floorHitbox2aSnow;
	floorHitbox2aSnow.texture = LoadTexture("red.jpg");
	floorHitbox2aSnow.setSize(120.0f, 20.0f);
	floorHitbox2aSnow.setLocation(380.0f, -170.0f + 20.0f * floor2Snow.layer);

	entity floorHitbox2bSnow;
	floorHitbox2bSnow.texture = LoadTexture("red.jpg");
	floorHitbox2bSnow.setSize(200.0f, 20.0f);
	floorHitbox2bSnow.setLocation(580.0f, -170.0f + 20.0f * floor2Snow.layer);

	entity floorHitbox2cSnow;
	floorHitbox2cSnow.texture = LoadTexture("red.jpg");
	floorHitbox2cSnow.setSize(200.0f, 20.0f);
	floorHitbox2cSnow.setLocation(860.0f, -170.0f + 20.0f * floor2Snow.layer);

	entity floorHitbox3aSnow;
	floorHitbox3aSnow.texture = LoadTexture("red.jpg");
	floorHitbox3aSnow.setSize(200.0f, 20.0f);
	floorHitbox3aSnow.setLocation(1060.0f, -170.0f + 20.0f * floor3aSnow.layer);

	entity floorHitbox3bSnow;
	floorHitbox3bSnow.texture = LoadTexture("red.jpg");
	floorHitbox3bSnow.setSize(280.0f, 20.0f);
	floorHitbox3bSnow.setLocation(1460.0f, -170.0f + 20.0f * floor3bSnow.layer);

	entity floorHitbox4aSnow;
	floorHitbox4aSnow.texture = LoadTexture("red.jpg");
	floorHitbox4aSnow.setSize(80.0f, 20.0f);
	floorHitbox4aSnow.setLocation(1720.0f, -170.0f + 20.0f * floor4aSnow.layer);

	entity floorHitbox4bSnow;
	floorHitbox4bSnow.texture = LoadTexture("red.jpg");
	floorHitbox4bSnow.setSize(120.0f, 20.0f);
	floorHitbox4bSnow.setLocation(1940.0f, -170.0f + 20.0f * floor4bSnow.layer);

	entity floorHitbox4cSnow;
	floorHitbox4cSnow.texture = LoadTexture("red.jpg");
	floorHitbox4cSnow.setSize(40.0f, 20.0f);
	floorHitbox4cSnow.setLocation(2140.0f, -170.0f + 20.0f * floor4cSnow.layer);

	bool end = false;

	Vector whoCares;

	int gameState = 0;
	int roundCounter = 0;

	entity finish;
	finish.texture = LoadTexture("finish.gif");
	finish.setSize(80, 360);
	finish.x = 960-640;

	float aribitraryTimer = 0.0f;

	int dogPoints = 0;
	int catPoints = 0;


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
		if (keys[SDL_SCANCODE_ESCAPE]) {
			break;
		}
		if (gameState == 0) {
			// TITLE SCREEN
			aribitraryTimer += elapsed;
			alphabet.setLocation(-120, 90);
			std::vector<int> dubble_runnr = { 3,20,1,1,11,4,95,17,20,13,13,17 };
			for (int i = 0; i < dubble_runnr.size(); i++) {
				alphabet.Draw(program, dubble_runnr[i]);
				alphabet.x += 20;
			}

			if (aribitraryTimer < 1.0f) {
				alphabet.setLocation(-200, 40);

				std::vector<int> prs_space = { 15,17,18,95,18,15,0,2,4,95,54,95,2,14,13,19,8,13,20,4 };
				for (int i = 0; i < prs_space.size(); i++) {
					alphabet.Draw(program, prs_space[i]);
					alphabet.x += 20;
				}
			}
			if (aribitraryTimer > 1.0f) {
				alphabet.setLocation(-140, 0);
				std::vector<int> prs_space = { 15,17,18,95,4,18,2,95,54,95,16,20,8,19 };
				for (int i = 0; i < prs_space.size(); i++) {
					alphabet.Draw(program, prs_space[i]);
					alphabet.x += 20;
				}
			}
			if (aribitraryTimer > 2.0f) {
				aribitraryTimer = 0.0f;
			}
			
			dog.currentAnimation = 1;
			dog.velocity_y = 0;
			dog.update(elapsed);
			dog.Draw(program);
			alphabet.setLocation(140, -140);
			std::vector<int> use_dog = { 20,18,4,95,20,15,74,92 };
			for (int i = 0; i < use_dog.size(); i++) {
				alphabet.Draw(program, use_dog[i]);
				alphabet.x += 20;
			}
			
			cat.currentAnimation = 1;
			cat.velocity_y = 0;
			cat.update(elapsed);
			cat.Draw(program);
			alphabet.setLocation(-220, -140);
			std::vector<int> use_cat = { 20,18,4,95,22,74,3 };
			for (int i = 0; i < use_cat.size(); i++) {
				alphabet.Draw(program, use_cat[i]);
				alphabet.x += 20;
			}

			if (keys[SDL_SCANCODE_SPACE]) {
				gameState = 2;
				cat.setSize(40, 40);
				dog.setSize(40, 40);
			}
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		if (gameState == 1) {
			// STAGE ONE
			if (checkSATCollision(dog.getVertices(), floorHitbox.getVertices(), whoCares)) {
				dog.velocity_y = 0;
				dog.allowjump = true;
				dog.y = floorHitbox.y + floorHitbox.height / 2 + dog.height / 2;
			}



			if (checkSATCollision(dog.getVertices(), floorHitbox2a.getVertices(), whoCares)) {
				dog.velocity_y = 0;
				dog.allowjump = true;
				dog.y = floorHitbox2a.y + floorHitbox2a.height / 2 + dog.height / 2;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox2b.getVertices(), whoCares)) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox2c.getVertices(), whoCares)) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox3a.getVertices(), whoCares) && roundCounter>=2) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox3b.getVertices(), whoCares) && roundCounter >= 2) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox4a.getVertices(), whoCares) && roundCounter >= 3) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox4b.getVertices(), whoCares) && roundCounter >= 3) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}
			if (checkSATCollision(dog.getVertices(), floorHitbox4c.getVertices(), whoCares) && roundCounter >= 3) {
				dog.velocity_y = 0;
				dog.allowjump = true;
			}



			if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_UP]) {
				dog.currentAnimation = 1;
				if (keys[SDL_SCANCODE_RIGHT]) {
					dog.velocity_x = 150.0f;
				}
				if (keys[SDL_SCANCODE_UP] && dog.allowjump) {
					dog.velocity_y = 250;
					dog.allowjump = false;
				}
			}
			else {
				dog.velocity_x = 0.0f;
				dog.currentAnimation = 0;
			}



			//RUN CAT
			if (checkSATCollision(cat.getVertices(), floorHitboxSnow.getVertices(), whoCares)) {
				cat.velocity_y = 0;
				cat.allowjump = true;
				//cat.y = floorHitbox.y + floorHitboxSnow.height / 2 + cat.height / 2;
			}



			if (checkSATCollision(cat.getVertices(), floorHitbox2aSnow.getVertices(), whoCares)) {
				cat.velocity_y = 0;
				cat.allowjump = true;
				//cat.y = floorHitbox2aSnow.y + floorHitbox2aSnow.height / 2 + cat.height / 2;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox2bSnow.getVertices(), whoCares)) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox2cSnow.getVertices(), whoCares)) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox3aSnow.getVertices(), whoCares) && roundCounter >= 2) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox3bSnow.getVertices(), whoCares) && roundCounter >= 2) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox4aSnow.getVertices(), whoCares) && roundCounter >= 3) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox4bSnow.getVertices(), whoCares) && roundCounter >= 3) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}
			if (checkSATCollision(cat.getVertices(), floorHitbox4cSnow.getVertices(), whoCares) && roundCounter >= 3) {
				cat.velocity_y = 0;
				cat.allowjump = true;
			}

			if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_W]) {
				cat.currentAnimation = 1;
				if (keys[SDL_SCANCODE_D]) {
					cat.velocity_x = 150.0f;
				}
				if (keys[SDL_SCANCODE_W] && cat.allowjump) {
					cat.velocity_y = 250;
					cat.allowjump = false;
				}
			}
			else {
				cat.velocity_x = 0.0f;
				cat.currentAnimation = 0;
			}

			// --------------END CASE--------------------------
			if (dog.active || cat.active) {
				viewFinder.x += elapsed * 50 * roundCounter;
				Matrix M;
				M.Translate(-viewFinder.x / (320.0f / 3.55f), 0, 0);
				viewMatrix = M;

			}
			if(!(dog.active || cat.active)) {
				aribitraryTimer += elapsed;
				if (aribitraryTimer > 2) {
					if (roundCounter >= 3) {
						gameState = 3;
					}
					else{ gameState = 2; }
					
				}
			}
			//---------------------------------------------------


			//-------------ANYTHING AFTER HERE IS A DRAW CALL-----
			if (dog.active) {
				dog.update(elapsed);
			}
			if (cat.active) {
				cat.update(elapsed);
			}

			//if (!end) { dog.update(elapsed); }

			//desert draw
			background.Draw(program);
			background2.Draw(program);
			background3.Draw(program);
			background4.Draw(program);
			background5.Draw(program);

			floor.Draw(program);
			floor2.Draw(program);
			if (roundCounter >= 2) {
				floor3a.Draw(program);
				floor3b.Draw(program);
			}
			if (roundCounter >= 3) {
				floor4a.Draw(program);
				floor4b.Draw(program);
				floor4c.Draw(program);
			}

			dog.Draw(program);


			//snow draw
			backgroundSnow.Draw(program);
			background2Snow.Draw(program);
			background3Snow.Draw(program);
			background4Snow.Draw(program);
			background5Snow.Draw(program);

			floorSnow.Draw(program);
			floor2Snow.Draw(program);
			if (roundCounter >= 2) {
				floor3aSnow.Draw(program);
				floor3bSnow.Draw(program);
			}
			if (roundCounter >= 3) {
				floor4aSnow.Draw(program);
				floor4bSnow.Draw(program);
				floor4cSnow.Draw(program);
			}

			//---for debug purposes---
			//floorHitbox.Draw(program);
			//floorHitbox2a.Draw(program);
			//floorHitbox2b.Draw(program);
			//floorHitbox2c.Draw(program);
			//if (roundCounter > 1) { floorHitbox3a.Draw(program); }
			//if (roundCounter > 1) { floorHitbox3b.Draw(program); }
			//if (roundCounter > 2) { floorHitbox4a.Draw(program); }
			//if (roundCounter > 2) { floorHitbox4b.Draw(program); }
			//if (roundCounter > 2) { floorHitbox4c.Draw(program); }


			//floorHitboxSnow.Draw(program);
			//floorHitbox2aSnow.Draw(program);
			//floorHitbox2bSnow.Draw(program);
			//floorHitbox2cSnow.Draw(program);
			//if (roundCounter > 1) { floorHitbox3aSnow.Draw(program); }
			//if (roundCounter > 1) { floorHitbox3bSnow.Draw(program); }
			//--------------


			cat.Draw(program);

			finish.Draw(program);

			// ----------check for finish
			if (dog.y < 0 || dog.x <viewFinder.x-320) {
				alphabet.setLocation(dog.x + 20, 90);
				std::vector<int> dog_lose = { 3,14,6,95,11,14,18,4 };
				for (int i = 0; i < dog_lose.size(); i++) {
					alphabet.Draw(program, dog_lose[i]);
					alphabet.x += 20;
				}
				dog.active = false;
			}
			else if (checkSATCollision(dog.getVertices(), finish.getVertices(), whoCares)) {
				alphabet.setLocation(dog.x - 60, 90);
				std::vector<int> dog_win = { 3,14,6,95,22,8,13 };
				for (int i = 0; i < dog_win.size(); i++) {
					alphabet.Draw(program, dog_win[i]);
					alphabet.x += 20;
				}
				if (dog.active) { dogPoints += 1; }
				dog.active = false;
				
			}

			if (cat.y < -180 || cat.x <viewFinder.x - 320) {
				alphabet.setLocation(cat.x + 20, -90);
				std::vector<int> cat_lose = { 2,0,19,95,11,14,18,4 };
				for (int i = 0; i < cat_lose.size(); i++) {
					alphabet.Draw(program, cat_lose[i]);
					alphabet.x += 20;
				}
				cat.active = false;
			}
			else if (checkSATCollision(cat.getVertices(), finish.getVertices(), whoCares)) {
				alphabet.setLocation(cat.x - 60, -90);
				std::vector<int> cat_win = { 2,0,19,95,22,8,13 };
				for (int i = 0; i < cat_win.size(); i++) {
					alphabet.Draw(program, cat_win[i]);
					alphabet.x += 20;
				}
				if (cat.active) { catPoints += 1; }
				cat.active = false;
				
			}
			
		}

		if (gameState == 2) {
			// RESET GAME FOR ANOTHER ROUND
			roundCounter += 1;
			aribitraryTimer = 0;
			if (roundCounter <= 3) {
				gameState = 1;
			}
			else{ gameState = 3; }

			dog.x = -160.0f;
			dog.y = 90;
			dog.active = true;
			dog.allowjump = true;

			cat.x = -160.0f;
			cat.y = -90;
			cat.active = true;
			cat.allowjump = true;

			viewFinder.x = 0;

			finish.x += 640;

			


		}

		if (gameState == 3) {
			alphabet.setLocation(-140, 60);
			std::vector<int> cat_pts = { 2,0,19,95,15,19,18 };
			for (int i = 0; i < cat_pts.size(); i++) {
				alphabet.Draw(program, cat_pts[i]);
				alphabet.x += 20;
			}
			alphabet.setLocation(-80, 20);
			alphabet.Draw(program, 52 + catPoints);


			alphabet.setLocation(140, 60);
			std::vector<int> dog_pts = { 3,14,6,95,15,19,18 };
			for (int i = 0; i < dog_pts.size(); i++) {
				alphabet.Draw(program, dog_pts[i]);
				alphabet.x += 20;
			}
			alphabet.setLocation(140, 20);
			alphabet.Draw(program, 52 + dogPoints);
			/*
			if (catPoints > dogPoints) {
				
			}
			if (catPoints < dogPoints) {

			}
			if (catPoints == dogPoints) {

			}
			*/
		}
		
		
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
