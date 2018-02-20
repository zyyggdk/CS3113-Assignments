//Assignment 2 Yiyang Zeng yz3622
//Player 1 is on the left and Player 2 is on the right. 

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>

#include "ShaderProgram.h"
#include "Matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//Identifiers and definitions
GLuint LoadTexture(const char *filePath);
void Setup();
void ProcessEvents(SDL_Event& event);
void Update();
void Render();
void Cleanup();

class Entity {
private:
	GLfloat x;
	GLfloat y;
	GLfloat rotation;
	GLint textureID = -1;

	GLfloat default_x;
	GLfloat default_y;
	GLfloat default_rotation;

	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;

	GLfloat width;
	GLfloat height;

	Matrix modelMatrix;

public:
	Entity(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
		GLfloat red = 0, GLfloat green = 0, GLfloat blue = 0, GLfloat alpha = 0) :
		width(width), height(height), x(x), y(y), default_x(x), default_y(y),
		red(red), green(green), blue(blue), alpha(alpha) {
		modelMatrix.Identity();
		modelMatrix.Translate(x, y, 0);
	}

	const GLfloat getX() {
		return x;
	}

	const GLfloat getY() {
		return y;
	}

	const GLfloat getHeight() {
		return height;
	}

	const GLfloat getWidth() {
		return width;
	}

	void Update() {
		Reset();
		modelMatrix.Translate(x, y, 0);
	}

	void Move(GLfloat x, GLfloat y) {
		this->x += x;
		this->y += y;
	}

	void Reset() {
		modelMatrix.Identity();
	}

	void virtual ResetDefault() {
		Reset();
		modelMatrix.Translate(default_x, default_y, 0);
		x = default_x;
		y = default_y;
	}

	void Draw(ShaderProgram& program) {
		glUseProgram(program.programID);
		program.SetColor(red, green, blue, alpha);
		program.SetModelMatrix(modelMatrix);
		GLfloat vertices2[] = { -width / 2 , -height / 2, width / 2, -height / 2 , width / 2, height / 2,
			-width / 2, -height / 2, width / 2, height / 2, -width / 2, height / 2 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	GLboolean CheckCollision(Entity* other) {
		/*a) is R1¡¯s bottom higher than R2¡¯s top ?
		b) is R1¡¯s top lower than R2¡¯s bottom ?
		c) is R1¡¯s left larger than R2¡¯s right ?
		d) is R1¡¯s right smaller than R2¡¯s left ?*/
		return !(
			this->y - this->height / 2 > other->y + other->height / 2 ||
			this->y + this->height / 2 < other->y - other->height / 2 ||
			this->x - this->width / 2 > other->x + other->width / 2 ||
			this->x + this->width / 2 < other->x - other->width / 2
			);
	}
};class Board : public Entity {private:	GLfloat units_a_second = 1;public:	Board(GLfloat width, GLfloat height, GLfloat x, GLfloat y) :		Entity(width, height, x, y) {	}	void Update(GLfloat elapsed, GLboolean ifUp) {		if (ifUp) {			Entity::Move(0, elapsed * units_a_second);		}		else {			Entity::Move(0, -elapsed * units_a_second);		}		Entity::Update();	}};class Ball : public Entity {private:	GLfloat velocity_x;
	GLfloat velocity_y;	GLfloat default_velocity_x;
	GLfloat default_velocity_y;public:	Ball(GLfloat width, GLfloat height, GLfloat x, GLfloat y,		GLfloat velocity_x, GLfloat velocity_y) :		Entity(width, height, x, y),		velocity_x(velocity_x), velocity_y(velocity_y), 		default_velocity_x(velocity_x), default_velocity_y(velocity_y) {	}	void Update(GLfloat elapsed, std::vector<Entity*>& entities) {		if (Entity::getY() + Entity::getHeight() / 2 >= 2 || Entity::getY() - Entity::getHeight() / 2 <= -2)			velocity_y = -velocity_y;		if (CheckCollision(entities)) {			velocity_x = -velocity_x;		}		Entity::Move(elapsed * velocity_x, elapsed * velocity_y);		Entity::Update();	}	void Reset() {		Entity::Reset();	}	void ResetDefault() {		Entity::ResetDefault();		velocity_x = default_velocity_x;		velocity_y = default_velocity_y;	}	GLboolean CheckCollision(std::vector<Entity*>& entities) {		for (int i = 0; i < entities.capacity(); ++i) {
			if (entities[i] != this) {
				if (Entity::CheckCollision(entities[i]))
					return true;
			}
		}		return false;	}	GLboolean CheckWin() {		if (Entity::getX() > 3.55) {			std::cout << "Player 1 wins!" << std::endl;			return true;		}					if (Entity::getX() < -3.55) {			std::cout << "Player 2 wins!" << std::endl;			return true;		}		return false;	}};

//Pong

SDL_Window* displayWindow;
ShaderProgram program;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

Matrix projectionMatrix;
Matrix viewMatrix;

GLfloat lastFrameTicks = 0.0f;
SDL_Event event;
bool done = false;

Ball ball(0.1, 0.1, 0, 0, 1, 1);

Board board1(0.1, 1, -3.2, 0);
Board board2(0.1, 1, 3.2, 0);

std::vector<Entity*> entities;

GLboolean board1Move = false;
GLboolean board2Move = false;
GLboolean board1Up = false;
GLboolean board2Up = false;

GLint main(GLint argc, char *argv[]) {

	Setup();
	while (!done) {

		ProcessEvents(event);
		Update();
		Render();
	}
	Cleanup();
	return 0;
}

void Setup() {
	// setup SDL and OpenGL
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	glViewport(0, 0, 640, 360);

	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	entities.push_back(&ball);
	entities.push_back(&board1);
	entities.push_back(&board2);
}

void ProcessEvents(SDL_Event& event) {
	// our SDL event loop

	while (SDL_PollEvent(&event)) {
		
		// check quit event
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}

		// check input events
		// Scancodes: https://wiki.libsdl.org/SDL_Scancode
		board1Move = false;
		if (keys[SDL_SCANCODE_W]) {
			board1Move = true;
			board1Up = true;
		}
		else if (keys[SDL_SCANCODE_S]) {
			board1Move = true;
			board1Up = false;
		}

		board2Move = false;
		// check input events
		if (keys[SDL_SCANCODE_UP]) {
			board2Move = true;
			board2Up = true;
		}
		else if (keys[SDL_SCANCODE_DOWN]) {
			board2Move = true;
			board2Up = false;
		}
	}

}

void Update() {
	//Get time
	GLfloat ticks = (GLfloat)SDL_GetTicks() / 1000.0f;
	GLfloat elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	ball.Update(elapsed, entities);
	if(board1Move)
		board1.Update(elapsed, board1Up);
	if(board2Move)
		board2.Update(elapsed, board2Up);

	if (ball.CheckWin()) {
		for (int i = 0; i < entities.capacity(); ++i) {
			entities[i]->ResetDefault();
		}
	}
}

void Render() {
	//CLear the screen
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < entities.capacity(); ++i) {
		entities[i]->Draw(program);
	}

	//Display the screen
	SDL_GL_SwapWindow(displayWindow);
}

void Cleanup() {
	// cleanup joysticks, textures, etc.
	SDL_Quit();
}

GLuint LoadTexture(const char *filePath) {
	GLint w, h, comp;
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