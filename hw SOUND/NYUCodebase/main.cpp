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
#include <SDL_mixer.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif


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
		/* IF ROTATION IS IN DEGREES USE THIS

		vertices.push_back(Vector(-50*cos(rotation*(3.1415926 / 180)) + (-50*(-sin(rotation*(3.1415926 / 180)))) + x, 
			-50*sin(rotation*(3.1415926 / 180)) - 50*(cos(rotation*(3.1415926 / 180))) + y));
		vertices.push_back(Vector(50*cos(rotation*(3.1415926 / 180)) - 50*(-sin(rotation*(3.1415926 / 180))) + x,
			50*sin(rotation*(3.1415926 / 180)) - 50*(cos(rotation*(3.1415926 / 180))) + y));
		vertices.push_back(Vector(50*cos(rotation*(3.1415926 / 180)) + 50*(-sin(rotation*(3.1415926 / 180))) + x,
			50*sin(rotation*(3.1415926 / 180)) + 50*(cos(rotation*(3.1415926 / 180))) + y));
		vertices.push_back(Vector(-50*cos(rotation*(3.1415926 / 180)) + 50*(-sin(rotation*(3.1415926 / 180))) + x,
			-50*sin(rotation*(3.1415926 / 180)) + 50*(cos(rotation*(3.1415926 / 180))) + y));
			*/

		vertices.push_back(Vector(-50*cos(rotation) + (-50*(-sin(rotation))) + x,
			-50*sin(rotation) - 50*(cos(rotation)) + y));
		vertices.push_back(Vector(50*cos(rotation) - 50 *(-sin(rotation)) + x,
			50 *sin(rotation) - 50 *(cos(rotation)) + y));
		vertices.push_back(Vector(50 *cos(rotation) + 50 *(-sin(rotation)) + x,
			50 *sin(rotation) + 50 *(cos(rotation)) + y));
		vertices.push_back(Vector(-50 *cos(rotation) + 50 *(-sin(rotation)) + x,
			-50 *sin(rotation) + 50 *(cos(rotation)) + y));


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
	entity red;
	red.texture = LoadTexture("Red.jpg");
	red.setSize(100, 100);
	red.setLocation(-55, 0);
	//red.rotation = 45;

	entity blue;
	blue.texture = LoadTexture("Blue.png");
	blue.setSize(100, 100);
	blue.setLocation(55, 0);
	//blue.rotation = 45;

	entity green;
	green.texture = LoadTexture("Green.png");
	green.setSize(100, 100);
	green.setLocation(0, 105);
	//rotation = 45;

	entity Magenta;
	Magenta.texture = LoadTexture("rb.jpg");
	Magenta.setLocation( 0, -100);
	Magenta.setSize(25, 25);

	entity Cyan;
	Cyan.texture = LoadTexture("bg.jpg");
	Cyan.setLocation( 100, 100);
	Cyan.setSize(25, 25);

	entity Yellow;
	Yellow.texture = LoadTexture("rg.jpg");
	Yellow.setLocation(-100, 100);
	Yellow.setSize(25, 25);

	Vector whoCares;
	std::vector<Vector> redVertices;
	std::vector<Vector> blueVertices;
	std::vector<Vector> greenVertices;
	bool end = false;

	Mix_Music *land;
	land = Mix_LoadMUS("land.mp3");
	Mix_PlayMusic(land, -1);
	

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
		
		
		red.rotation += elapsed*.5;
		blue.rotation += elapsed*.3;
		green.rotation += elapsed*.7;

		redVertices = red.getVertices();
		blueVertices = blue.getVertices();
		greenVertices = green.getVertices();


		//-------------ANYTHING AFTER HERE IS A DRAW CALL-----
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		red.Draw(program);
		blue.Draw(program);
		green.Draw(program);

		if (checkSATCollision(redVertices, blueVertices, whoCares)) {
			Magenta.Draw(program);
			Mix_OpenAudio(60, MIX_DEFAULT_FORMAT, 2, 4096);
		}
		if (checkSATCollision(greenVertices, blueVertices, whoCares)) {
			Cyan.Draw(program);
			Mix_OpenAudio(130, MIX_DEFAULT_FORMAT, 2, 4096);
		}
		if (checkSATCollision(redVertices, greenVertices, whoCares)) {
			Yellow.Draw(program);
			Mix_OpenAudio(500, MIX_DEFAULT_FORMAT, 2, 4096);
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	//Mix_FreeChunk(someSound);
	Mix_FreeMusic(land);
	SDL_Quit();
	return 0;
}
