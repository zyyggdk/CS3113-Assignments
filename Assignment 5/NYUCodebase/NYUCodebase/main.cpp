//Assignment 5 Yiyang Zeng yz3622
//a simple Separated Axis Collision demo

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <time.h>
#include "FlareMap.h"

#include "ShaderProgram.h"
#include "Matrix.h"
#include "SatCollision.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include "tinyxml2.h"


//Identifiers and definitions
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
GLuint LoadTexture(const char *filePath);
GLfloat lerp(float v0, float v1, float t);
enum textDisplayMode { ALIGN_LEFT, ALIGN_MIDDLE };
enum GameMode{ GAME_LEVEL};

const GLint WIN_WIDTH = 1280, WIN_HEIGHT = 720;
const GLfloat ASPECT_RATIO = 1.77777777778;
const GLfloat COORD_Y = 9, COORD_X = 16;
const GLfloat TILE_SIZE = 1;
const GLint SPRITE_COUNT_X = 16;
const GLint SPRITE_COUNT_Y = 8;

Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;

#define LEVEL_HEIGHT 16
#define LEVEL_WIDTH 22

void Setup();

GameMode gameMode = GAME_LEVEL;

class Vector3 {
public:
	float x;
	float y;
	float z;

	Vector3() :x(0), y(0), z(0) {

	}

	Vector3(float x, float y, float z):x(x), y(y), z(z) {

	}

	void set(GLfloat x, GLfloat y, GLfloat z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Matrix toMatrix() {
		Matrix m;
		m.Clear();
		m.m[0][0] = x;
		m.m[1][0] = y;
		m.m[2][0] = z;
		m.m[3][0] = 1;
		return m;
	}
};

class Vector2 {
public:
	float x;
	float y;

	Vector2() :x(0), y(0) {

	}

	Vector2(float x, float y) :x(x), y(y) {

	}

	void set(GLfloat x, GLfloat y) {
		this->x = x;
		this->y = y;
	}

	Matrix toMatrix() {
		Matrix m;
		m.Clear();
		m.m[0][0] = x;
		m.m[1][0] = y;
		m.m[2][0] = 0;
		m.m[3][0] = 1;
		return m;
	}
};

class Entity {
private:
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 friction;
	Vector3 size;

	GLfloat rotation;

	const GLfloat gravity = 0;
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;

	Matrix modelMatrix;

	GLboolean isStatic = true;

	std::vector<Vector2> edges;

public:
	Entity(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
		GLfloat red = 0, GLfloat green = 0, GLfloat blue = 0, GLfloat alpha = 1) :
		position(x, y, 0), size(width, height, 1),
		red(red), green(green), blue(blue), alpha(alpha) {
		modelMatrix.Identity();
		modelMatrix.Translate(x, y, 0);
		edges.push_back(Vector2(0.5, 0.5));
		edges.push_back(Vector2(-0.5, 0.5));
		edges.push_back(Vector2(0.5, -0.5));
		edges.push_back(Vector2(-0.5, -0.5));
		modelMatrix.Display();
	}

	const Vector3 getPos() const {
		return position;
	}

	const Vector3 getVelocity() const {
		return velocity;
	}

	const Vector3 getSize() const {
		return size;
	}

	void virtual Update(GLfloat elapsed) {
		ResetModel();
		if (!isStatic) {
			
			ChangeVelocity(lerp(velocity.x, 0.0f, elapsed * friction.x),
				lerp(velocity.y, 0.0f, elapsed * friction.y),
				lerp(velocity.z, 0.0f, elapsed * friction.z));
			ChangeVelocity(velocity.x + acceleration.x * elapsed,
				velocity.y + (acceleration.y + gravity) * elapsed,
				velocity.z + acceleration.z * elapsed);
			Move(position.x + velocity.x * elapsed,
				position.y + velocity.y * elapsed,
				position.z + velocity.z * elapsed);
			/*std::cout << "Pos:" << position.x << "," << position.y << std::endl;
			std::cout << "Vel:" << velocity.x << "," << velocity.y << std::endl;
			std::cout << "Acc:" << acceleration.x << "," << acceleration.y << std::endl;*/
		}

		
		modelMatrix.Translate(position.x, position.y, position.z);
		
		modelMatrix.Rotate(rotation);
		modelMatrix.Scale(size.x, size.y, size.z);
		
	}

	GLboolean Rotate(GLfloat x) {
		rotation += x;
		return true;
	}

	GLboolean virtual Move(GLfloat x, GLfloat y, GLfloat z) {
		position.set(x, y, z);
		return true;
	}

	GLboolean Resize(GLfloat x, GLfloat y, GLfloat z) {
		size.set(x, y, z);
		return true;
	}

	GLboolean ChangeVelocity(GLfloat x, GLfloat y, GLfloat z) {
		velocity.set(x, y, z);
		return true;
	}

	GLboolean ReverseVelocity(GLint x, GLint y, GLint z) {
		this->velocity.x = x * this->velocity.x;
		this->velocity.y = y * this->velocity.y;
		this->velocity.z = z * this->velocity.z;
		return true;
	}

	GLboolean ChangeAcc(GLfloat x, GLfloat y, GLfloat z) {
		acceleration.set(x, y, z);
		return true;
	}

	GLboolean ChangeFric(GLfloat x, GLfloat y, GLfloat z) {
		friction.set(x, y, z);
		return true;
	}

	void ResetModel() {
		modelMatrix.Identity();
	}

	void virtual Draw(ShaderProgram& program) {

		glUseProgram(program.programID);
		program.SetColor(red, green, blue, alpha);
		program.SetModelMatrix(modelMatrix);
		program.SetViewMatrix(viewMatrix);
		GLfloat width = 1, height = 1;
		GLfloat vertices2[] = { -width / 2 , -height / 2, width / 2, -height / 2 , width / 2, height / 2,
		-width / 2, -height / 2, width / 2, height / 2, -width / 2, height / 2 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	void virtual Draw(ShaderProgram& program, GLfloat* vertices, GLint verticesCount) {
		program.SetModelMatrix(modelMatrix);
		program.SetViewMatrix(viewMatrix);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, verticesCount);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	GLboolean RecCollision(Entity* other) {
		/*a) is R1¡¯s bottom higher than R2¡¯s top ?
		b) is R1¡¯s top lower than R2¡¯s bottom ?
		c) is R1¡¯s left larger than R2¡¯s right ?
		d) is R1¡¯s right smaller than R2¡¯s left ?*/
		GLfloat thisWidth = size.x, thisHeight = size.y;
		GLfloat otherWidth = other->size.x, otherHeight = other->size.y;

		return !(
			this->position.y - thisHeight / 2 > other->position.y + otherHeight / 2 ||
			this->position.y + thisHeight / 2 < other->position.y - otherHeight / 2 ||
			this->position.x - thisWidth / 2 > other->position.x + otherWidth / 2 ||
			this->position.x + thisWidth / 2 < other->position.x - otherWidth / 2
			);
	}

	//GLboolean ResolveCollision(Entity* other) {
	//	GLfloat thisWidth = size.x, thisHeight = size.y;
	//	GLfloat otherWidth = other->size.x, otherHeight = other->size.y;
	//	GLfloat penetrationY = fabs(this->position.y - other->position.y)
	//		- thisHeight / 2 - otherHeight / 2;
	//	GLfloat penetrationX = fabs(this->position.x - other->position.x)
	//		- thisWidth / 2 - otherWidth / 2;
	//	if (penetrationY > 0)
	//		penetrationY = 0;
	//	else
	//		penetrationY -= 0.001;
	//	if (penetrationX > 0)
	//		penetrationX = 0;
	//	else
	//		penetrationX -= 0.001;
	//	//Move up
	//	if (this->position.y > other->position.y) {
	//		penetrationY = -penetrationY;
	//	}
	//	//Move right
	//	if (this->position.x > other->position.x) {
	//		penetrationX = -penetrationX;
	//	}
	//	Move(penetrationX, penetrationY, 0);
	//}

	GLboolean RecSATCollision(Entity* other) {
		std::pair<float, float> penetration;

		std::vector<std::pair<float, float>> e1Points;
		std::vector<std::pair<float, float>> e2Points;

		for (int i = 0; i < this->edges.size(); i++) {

			GLfloat x, y;
			x = this->modelMatrix.m[0][0] * this->edges[i].toMatrix().m[0][0]
				+ this->modelMatrix.m[1][0] * this->edges[i].toMatrix().m[1][0]
				+ this->modelMatrix.m[2][0] * this->edges[i].toMatrix().m[2][0]
				+ this->modelMatrix.m[3][0];

			y = this->modelMatrix.m[0][1] * this->edges[i].toMatrix().m[0][0]
				+ this->modelMatrix.m[1][1] * this->edges[i].toMatrix().m[1][0]
				+ this->modelMatrix.m[2][1] * this->edges[i].toMatrix().m[2][0]
				+ this->modelMatrix.m[3][1];
			
			e1Points.push_back(std::make_pair(x, y));
		}		

		for (int i = 0; i < other->edges.size(); i++) {

			GLfloat x, y;
			x = other->modelMatrix.m[0][0] * other->edges[i].toMatrix().m[0][0]
				+ other->modelMatrix.m[1][0] * other->edges[i].toMatrix().m[1][0]
				+ other->modelMatrix.m[2][0] * other->edges[i].toMatrix().m[2][0]
				+ other->modelMatrix.m[3][0];

			y = other->modelMatrix.m[0][1] * other->edges[i].toMatrix().m[0][0]
				+ other->modelMatrix.m[1][1] * other->edges[i].toMatrix().m[1][0]
				+ other->modelMatrix.m[2][1] * other->edges[i].toMatrix().m[2][0]
				+ other->modelMatrix.m[3][1];

			e2Points.push_back(std::make_pair(x, y));
		}

		if (CheckSATCollision(e1Points, e2Points, penetration)) {
			this->position.x += (penetration.first * 0.5f);
			this->position.y += (penetration.second * 0.5f);

			other->position.x -= (penetration.first * 0.5f);
			other->position.y -= (penetration.second * 0.5f);

			this->ReverseVelocity(-1, -1, 1);
			other->ReverseVelocity(-1, -1, 1);
		}

		return true;
	}

	GLboolean ifStatic() {
		return isStatic;
	}

	void setStatic() {
		isStatic = true;
	}

	void setDynamic() {
		isStatic = false;
	}
};

class TexturedEntity :public Entity {
private:
	GLuint textureID;

public:
	TexturedEntity(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
		GLint textureID) :
		Entity(width, height, x, y),
		textureID(textureID)
		{
	}

	const GLint getTextureID() {
		return textureID;
	}

	void ResetModel() {
		Entity::ResetModel();
	}

	void Draw(ShaderProgram& program, GLfloat* vertices, GLint verticesCount, GLfloat* texCoords) {
		
		glUseProgram(program.programID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		
		Entity::Draw(program, vertices, verticesCount);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}
};

class NonUniformSpriteEntity :public TexturedEntity {
private:
	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> texCoordData;

public:
	NonUniformSpriteEntity(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
		GLint textureID, GLint sheetWidth, GLint sheetHeight, 
		std::string atlasFile, std::string spriteName, GLboolean adjustToSprite = false):
		TexturedEntity(width, height, x, y, textureID) {
		float u, v, spriteWidth, spriteHeight;
		tinyxml2::XMLDocument doc;
		doc.LoadFile(&atlasFile[0]);
		for (tinyxml2::XMLElement* element = doc.FirstChildElement("TextureAtlas")->FirstChildElement("SubTexture");
			element != NULL; element = element->NextSiblingElement())
		{
			if (std::string(element->Attribute("name")).compare(spriteName) == 0) {
				element->QueryFloatAttribute("x", &u);
				element->QueryFloatAttribute("y", &v);
				element->QueryFloatAttribute("width", &spriteWidth);
				element->QueryFloatAttribute("height", &spriteHeight);
				u /= sheetWidth;
				v /= sheetHeight;
				spriteWidth /= sheetWidth;
				spriteHeight /= sheetHeight;
				break;
			}		
		}

		texCoordData.insert(texCoordData.end(), {
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight
		});

		GLfloat aspect = spriteWidth / spriteHeight;
		GLfloat size = Entity::getSize().y;
		vertexData.insert(vertexData.end(), {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size ,
			0.5f * size * aspect, -0.5f * size });
		if (adjustToSprite) {	
			Entity::Resize(size * aspect, size, 0);
		}
	}

	void Draw(ShaderProgram& program) {
		
		TexturedEntity::Draw(program, vertexData.data(), 6, texCoordData.data());
	}
};

class UniformSpriteEntity:public TexturedEntity {
private:
	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> texCoordData;

public:
	UniformSpriteEntity(GLfloat tileSize, GLfloat x, GLfloat y,
		GLint textureID, GLint spriteCountX, GLint spriteCountY,
		GLint spriteId) :
		TexturedEntity(tileSize, tileSize, x, y, textureID) {
		float u = (float)(((int)spriteId) % spriteCountX) / (float)spriteCountX;
		float v = (float)(((int)spriteId) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0 / (float)spriteCountX;
		float spriteHeight = 1.0 / (float)spriteCountY;

		vertexData.insert(vertexData.end(), {
			- 0.5f * tileSize, - 0.5f * tileSize,
			0.5f * tileSize, 0.5f * tileSize,
			- 0.5f * tileSize, 0.5f * tileSize,
			0.5f * tileSize, 0.5f * tileSize, 
			- 0.5f * tileSize, - 0.5f * tileSize, 
			0.5f * tileSize, - 0.5f * tileSize
		});

		texCoordData.insert(texCoordData.end(), {
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight
		});
	}

	void Draw(ShaderProgram& program) {

		TexturedEntity::Draw(program, vertexData.data(), 6, texCoordData.data());
	}
};

class TextBox :public TexturedEntity {
private: 

	std::string text;
	GLfloat size;
	GLfloat spacing;
	GLint displayMode;
	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> texCoordData;

public: 
	TextBox(GLfloat x, GLfloat y, GLfloat size, GLfloat spacing, GLint displayMode = 0, std::string text = "") :
		TexturedEntity(0, 0, x, y, LoadTexture("font1.png")), text(text), size(size), spacing(spacing), displayMode(displayMode){
		
		GLfloat texture_size = 1.0 / 16.0f;
		
		for (GLint i = 0; i < text.size(); i++) {
			GLint spriteIndex = (GLint)text[i];
			GLfloat texture_x = (GLfloat)(spriteIndex % 16) / 16.0f;
			GLfloat texture_y = (GLfloat)(spriteIndex / 16) / 16.0f;
			GLint charNum = text.size();
			if (displayMode == ALIGN_LEFT) {
				vertexData.insert(vertexData.end(), {
					((size + spacing) * i) + (-0.5f * size), 0.5f * size,
					((size + spacing) * i) + (-0.5f * size), -0.5f * size,
					((size + spacing) * i) + (0.5f * size), 0.5f * size,
					((size + spacing) * i) + (0.5f * size), -0.5f * size,
					((size + spacing) * i) + (0.5f * size), 0.5f * size,
					((size + spacing) * i) + (-0.5f * size), -0.5f * size,
				});
			}
			if (displayMode == ALIGN_MIDDLE) {
				GLint shift = (size * (charNum) + spacing * (charNum - 1)) / 2 ;
				vertexData.insert(vertexData.end(), {
					((size + spacing) * i) + (-0.5f * size) - shift, 0.5f * size,
					((size + spacing) * i) + (-0.5f * size) - shift, -0.5f * size,
					((size + spacing) * i) + (0.5f * size) - shift, 0.5f * size,
					((size + spacing) * i) + (0.5f * size) - shift, -0.5f * size,
					((size + spacing) * i) + (0.5f * size) - shift, 0.5f * size,
					((size + spacing) * i) + (-0.5f * size) - shift, -0.5f * size,
				});
			}
			
			texCoordData.insert(texCoordData.end(), {
				texture_x, texture_y,
				texture_x, texture_y + texture_size,
				texture_x + texture_size, texture_y,
				texture_x + texture_size, texture_y + texture_size,
				texture_x + texture_size, texture_y,
				texture_x, texture_y + texture_size,
			});
		}
	}

	void Draw(ShaderProgram& program) {
		// draw this data (use the .data() method of std::vector to get pointer to data)
		// draw this yourself, use text.size() * 6 or vertexData.size()/2 to get number of vertices
		TexturedEntity::Draw(program, vertexData.data(), vertexData.size() / 2, texCoordData.data());
	}
};

SDL_Window* displayWindow;
ShaderProgram program;
ShaderProgram texturedProgram;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

SDL_Event event;
bool done = false;

GLint fontTexture;

class GameState {
private:
	GLfloat lastFrameTicks = 0.0f;

public:
	GLfloat GetElapsed() {
		GLfloat ticks = (GLfloat)SDL_GetTicks() / 1000.0f;
		GLfloat elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		return elapsed;
	}
	void virtual Render() {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
};

class GameLevel : public GameState{
private:

	std::vector<Entity*> entities;

public:
	GameLevel() {
		entities.push_back(new Entity(1, 2, 0, 0, 1, 0, 0));
		entities.push_back(new Entity(1, 2, 10, 0, 0, 1, 0));
		entities.push_back(new Entity(1, 2, -10, 5, 0, 0, 1));
		entities[0]->Resize(2, 4, 1);
		entities[1]->Resize(6, 2, 1);
		entities[2]->Resize(4, 4, 1);
		entities[0]->Rotate(0.5);
		entities[1]->Rotate(3.0);
		entities[2]->Rotate(1.8);
		entities[0]->ChangeVelocity(4, 8, 0);
		entities[1]->ChangeVelocity(-8, 4, 0);
		entities[2]->ChangeVelocity(-4, -4, 0);
		entities[0]->setDynamic();
		entities[1]->setDynamic();
		entities[2]->setDynamic();
	}

	void ProcessEvents(SDL_Event& event) {
		while (SDL_PollEvent(&event)) {

			// check quit event
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

		}
	}

	void Update(GLfloat timeStep) {

		for (GLuint i = 0; i < entities.size(); i++)
		{
			if (entities[i]->getPos().x > COORD_X || entities[i]->getPos().x < -COORD_X)
				entities[i]->ReverseVelocity(-1, 1, 1);
			if (entities[i]->getPos().y > COORD_Y || entities[i]->getPos().y < -COORD_Y)
				entities[i]->ReverseVelocity(1, -1, 1);
			entities[i]->Update(timeStep);
		}

		for (GLuint i = 0; i < entities.size(); i++)
		{
			for (GLuint j = i + 1; j < entities.size(); j++)
			{
				if (j != i) {
					entities[i]->RecSATCollision(entities[j]);

				}
			}
		}
	}

	void Render() {
		GameState::Render();

		for (GLint i = 0; i < entities.size(); i++)
		{
			entities[i]->Draw(program);
			
		}

		SDL_GL_SwapWindow(displayWindow);
	}

};

void ProcessEvents(SDL_Event& event);
void Update(GLfloat timeStep);
void Render();

GameLevel* level;

GLint main(GLint argc, char *argv[]) {
	GLfloat accumulator = 0.0f;
	GLfloat ticks = (GLfloat)SDL_GetTicks() / 1000.0f;
	GLfloat elapsed = ticks;
	GLfloat lastFrameTicks = ticks;

	Setup();
	while (!done) {
		ticks = (GLfloat)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		elapsed += accumulator;
		
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}
		ProcessEvents(event);
		while (elapsed >= FIXED_TIMESTEP) {
			Update(FIXED_TIMESTEP);
			elapsed -= FIXED_TIMESTEP;
		}
		accumulator = elapsed;
		Render();
	}
	SDL_Quit();
	return 0;
}

void Setup() {

	// setup SDL and OpenGL
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	texturedProgram.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	projectionMatrix.SetOrthoProjection(-COORD_Y * ASPECT_RATIO, COORD_Y * ASPECT_RATIO, -COORD_Y, COORD_Y, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetModelMatrix(modelMatrix);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	srand(time(NULL));

	level = new GameLevel();
}

void ProcessEvents(SDL_Event& event) {
	switch (gameMode) {
		case GAME_LEVEL:
			level->ProcessEvents(event);
			break;
	}
}

void Update(GLfloat timeStep) {
	switch (gameMode) {
	case GAME_LEVEL:
		level->Update(timeStep);
		break;
	}
}

void Render() {
	switch (gameMode) {
	case GAME_LEVEL:
		level->Render();
		break;
	}
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

GLfloat lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}