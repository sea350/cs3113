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

struct entity
{
	void Draw() {
		Matrix M;
		M.Translate(x / (320.0f/3.55), y / (180.0f/2.0f), 0.0f);
		M.Scale(width / 90.0f, height / 90.0f, 1.0f);
		M.Rotate(rotation);
		model = M;
	}

	void setLocation(float a, float b) {
		x = a;
		y = b;
	}

	void setSize(float a, float b) {
		width = a;
		height = b;
	}

	bool checkCollision(entity enemy) {
		float leftBound = x - (width / 2);
		float rightBound = (width / 2) + x;
		float upBound = (height / 2) + y;
		float downBound = y- (height / 2);
		float eLeftBound = enemy.x - (enemy.width / 2);
		float eRightBound = (enemy.width / 2) + enemy.x;
		float eUpBound = (enemy.height / 2) + enemy.y;
		float eDownBound = enemy.y - (enemy.height / 2);
		if (leftBound < eLeftBound && eLeftBound < rightBound) {
			if ((downBound < eUpBound && eUpBound < upBound) || (downBound < eDownBound && eDownBound < upBound)) { return true; }
			else return false;
		}
		else if (leftBound < eRightBound && eRightBound < rightBound) {
			if ((downBound < eUpBound && eUpBound < upBound) || (downBound < eDownBound && eDownBound < upBound)) { return true; }
			else return false;
		}
		else return false;
	}

	float x = 0;
	float y = 0;
	float rotation = 0;
	//int textureID;
	float width = 90;
	float height = 90;
	float speed = 180;
	float direction_x = 0;
	float direction_y= 0;
	float angle = 0;


	Matrix model;
	GLuint texture;

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
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);
	float lastFrameTicks = 0.0f;


	//entity left;
	//left.model.Translate(-3.0f, 0.0f, 0.0f);
	//left.model.Scale(0.1f, 0.75f, 1.0f);
	//left.texture = LoadTexture(RESOURCE_FOLDER"Red.png");

	//entity marker;
	//marker.model.Translate(3.0f, 1.0f, 0.0f);
	//marker.model.Scale(0.1f, 0.1f, 1.0f);
	//marker.texture = LoadTexture(RESOURCE_FOLDER"Red.png");

	//entity right;
	//right.model.Translate(3.0f, 0.0f, 0.0f);
	//right.model.Scale(0.1f, 0.75f, 1.0f);
	//right.texture = LoadTexture(RESOURCE_FOLDER"Blue.png");

	entity left;
	left.setSize(10.0f, 80.0f);
	left.setLocation(-300, 0);
	left.texture = LoadTexture(RESOURCE_FOLDER"Red.png");

	entity right;
	right.setSize(10.0f, 80.0f);
	right.setLocation(300, 0);
	right.texture = LoadTexture(RESOURCE_FOLDER"Blue.png");

	entity ball;
	ball.setSize(40.0f, 40.0f);
	ball.setLocation(0, 0);
	ball.texture = LoadTexture(RESOURCE_FOLDER"Chief.jpg");

	entity angry;
	angry.texture = LoadTexture(RESOURCE_FOLDER"angry.jpg");
	angry.Draw();

	entity winner;
	winner.setSize(40.0f, 40.0f);
	winner.setLocation(-90.0f, 0.0f);

	bool rightWin = false;
	bool leftWin = false;

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
		if (keys[SDL_SCANCODE_UP] && right.height/2 + right.y <= 180.0f) {
			right.y += 150.0f*elapsed;
		}
		else if (keys[SDL_SCANCODE_DOWN] && right.y - (right.height / 2) >= -180.0f) {
			right.y -= 150.0f*elapsed;
		}

		if (keys[SDL_SCANCODE_W] && left.height / 2 + left.y <= 180.0f) {
			left.y += 150.0f*elapsed;
		}
		else if (keys[SDL_SCANCODE_S] && left.y - (left.height / 2) >= -180.0f) {
			left.y -= 150.0f*elapsed;
		}

		/*ball.x += ball.speed*elapsed*cos(ball.angle*3.14159 / 180);
		ball.y += ball.speed*elapsed*sin(ball.angle*3.14159 / 180);*/

		if (!rightWin && !leftWin && ticks>2) {
			ball.x += ball.speed*elapsed*cos(ball.angle*3.14159 / 180);
			ball.y += ball.speed*elapsed*sin(ball.angle*3.14159 / 180);
		}

		if (left.checkCollision(ball)) {
			ball.angle = 45.0f*((ball.y - left.y) / (left.height/2));
		}
		if (right.checkCollision(ball)) {
			ball.angle = -180.0f-(45.0f*((ball.y - right.y) / (right.height / 2)));
		}

		if (ball.y + (ball.height / 2) > 180) {
			ball.angle= -ball.angle;
		}
		if (ball.y - (ball.height / 2) < -180) {
			ball.angle= -ball.angle;
		}

		if (ball.x + ball.width / 2 > 320.0f) {
			leftWin = true;
		}

		if (ball.x - (ball.width / 2) < -320.0f) {
			rightWin = true;
		}
		/*if (ball.y + ball.height / 2 >= 180 || ball.y - ball.height / 2 <= 180) {
			ball.angle = -ball.angle;
		}*/

		/*while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					right.model.Translate(0.0f, 1.0f, 0.0f);
				}
			}
		}*/
		

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

		left.Draw();
		program.setModelMatrix(left.model);
		glBindTexture(GL_TEXTURE_2D, left.texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		right.Draw();
		program.setModelMatrix(right.model);
		glBindTexture(GL_TEXTURE_2D, right.texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		ball.Draw();
		program.setModelMatrix(ball.model);
		glBindTexture(GL_TEXTURE_2D, ball.texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		if (leftWin || rightWin) {
			program.setModelMatrix(angry.model);
			glBindTexture(GL_TEXTURE_2D, angry.texture);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program.texCoordAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

			if (leftWin) {
				winner.texture= LoadTexture(RESOURCE_FOLDER"redflag.jpg");
			}
			else {
				winner.texture = LoadTexture(RESOURCE_FOLDER"blueflag.jpg");
			}

			winner.Draw();
			program.setModelMatrix(winner.model);
			glBindTexture(GL_TEXTURE_2D, winner.texture);
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);
			glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program.texCoordAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
			glDisableVertexAttribArray(program.texCoordAttribute);

		}

		/*program.setModelMatrix(marker.model);
		glBindTexture(GL_TEXTURE_2D, marker.texture);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);*/

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
