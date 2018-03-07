//Assignment 3 Yiyang Zeng yz3622
//Hold A and D to move to the left or to the right. Press Space to fire. 

#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <time.h>

#include "ShaderProgram.h"
#include "Matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include "tinyxml2.h"

//Identifiers and definitions
GLuint LoadTexture(const char *filePath);
enum textDisplayMode { ALIGN_LEFT, ALIGN_MIDDLE };
enum GameMode{ TITLE_SCREEN, GAME_LEVEL};

const GLint WIN_WIDTH = 800, WIN_HEIGHT = 1000;
const GLfloat ASPECT_RATIO = 0.8;
const GLfloat COORD_X = 1, COORD_Y = 2;

void Setup();

GameMode gameMode = TITLE_SCREEN;

class Vector3 {
private:
	float x;
	float y;
	float z;

public:
	Vector3(float x, float y, float z):x(x), y(y), z(z) {

	}

	GLfloat getX() const {
		return x;
	}

	GLfloat getY() const {
		return y;
	}

	GLfloat getZ() const {
		return z;
	}

	GLboolean set(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
		return true;
	}

	GLboolean set(const Vector3& other) {
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		return true;
	}
};

class Entity {
private:
	Vector3 position;
	Vector3 velocity;
	Vector3 size;

	GLfloat rotation;

	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;

	Matrix modelMatrix;

public:
	Entity(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
		GLfloat red = 0, GLfloat green = 0, GLfloat blue = 0, GLfloat alpha = 0) :
		position(x, y, 0), velocity(0, 0, 0), size(width, height, 0),
		red(red), green(green), blue(blue), alpha(alpha) {
		modelMatrix.Identity();
		modelMatrix.Translate(x, y, 0);
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

	void virtual Update() {
		ResetModel();
		Move(position.getX() + velocity.getX(), position.getY() + velocity.getY(), position.getZ() + velocity.getZ());
		modelMatrix.Translate(position.getX(), position.getY(), position.getZ());
	}

	void virtual Update(GLfloat elapsed) {
		ResetModel();
		Move(position.getX() + velocity.getX() * elapsed, position.getY() + velocity.getY() * elapsed, position.getZ() + velocity.getZ() * elapsed);
		modelMatrix.Translate(position.getX(), position.getY(), position.getZ());
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

	void ResetModel() {
		modelMatrix.Identity();
	}

	void virtual Draw(ShaderProgram& program) {
		glUseProgram(program.programID);
		program.SetColor(red, green, blue, alpha);
		program.SetModelMatrix(modelMatrix);
		GLfloat width = size.getX(), height = size.getY();
		GLfloat vertices2[] = { -width / 2 , -height / 2, width / 2, -height / 2 , width / 2, height / 2,
		-width / 2, -height / 2, width / 2, height / 2, -width / 2, height / 2 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	void virtual Draw(ShaderProgram& program, GLfloat* vertices, GLint verticesCount) {
		program.SetModelMatrix(modelMatrix);
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
		GLfloat thisWidth = size.getX(), thisHeight = size.getY();
		GLfloat otherWidth = other->size.getX(), otherHeight = other->size.getY();
		return !(
			this->position.getY() - thisHeight / 2 > other->position.getY() + otherHeight / 2 ||
			this->position.getY() + thisHeight / 2 < other->position.getY() - otherHeight / 2 ||
			this->position.getX() - thisWidth / 2 > other->position.getX() + otherWidth / 2 ||
			this->position.getX() + thisWidth / 2 < other->position.getX() - otherWidth / 2
			);
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

class SpriteEntity :public TexturedEntity {
private:
	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> texCoordData;

public:
	SpriteEntity(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
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
		GLfloat size = Entity::getSize().getY();
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

class SpaceObject;

class Bullet : public SpriteEntity {
private:
	GLfloat timeAlive = 0;
	GLfloat timeAliveLimit = 10;
	SpaceObject* shooter;
	std::vector<Bullet*>* bullets;

public:
	Bullet(GLfloat width, GLfloat height, GLfloat x, GLfloat y, GLint textureID, std::string spriteName, SpaceObject* shooter, std::vector<Bullet*>* bullets, GLfloat bulletVelocityY) :
		SpriteEntity(width, height, x, y, textureID, 1024, 1024, "sheet.xml", spriteName, true), shooter(shooter), bullets(bullets) {
		Entity::ChangeVelocity(0, bulletVelocityY, 0);
		std::cout << "bullet!";
	}

	GLboolean IfTimeout() {
		return this->timeAlive > timeAliveLimit;
	}

	void Update(GLfloat elapsed) {
		timeAlive += elapsed;
		if (IfTimeout())
			RemoveBullet();

		//Check hits between bullets
		for (GLuint i = 0; i < bullets->size(); i++) {
			if ((*bullets)[i] != this) {
				if (CheckHit((*bullets)[i])) {
					RemoveBullet();
					(*bullets)[i]->RemoveBullet();
					return;
				}
			}
		}

		Entity::Update(elapsed);
	}

	GLboolean RemoveBullet() {
		for (GLuint i = 0; i < bullets->size(); i++) {
			if ((*bullets)[i] == this) {
				Bullet* temp = (*bullets)[i];
				(*bullets)[i] = (*bullets)[bullets->size() - 1];
				bullets->pop_back();
				//delete temp;
				return true;
			}
		}
		return false;
	}

	GLboolean CheckHit(Entity* other) {
		if (other != (Entity*)shooter)
		{
			return Entity::RecCollision(other);
		}
		return false;
	}
};

class SpaceObject : public SpriteEntity {
private:
	GLfloat fireRate;
	GLfloat fireTimer;
	std::vector<Bullet*>* bullets;
	std::string bulletSprite;
	std::vector<Entity*>* entities;
	GLfloat bulletVelocityY;
	GLboolean isAlive = true;

public:
	SpaceObject(GLfloat width, GLfloat height, GLfloat x, GLfloat y, GLfloat fireRate, GLint textureID, GLfloat bulletVelocityY, 
		std::string spriteName, std::string bulletSprite, std::vector<Bullet*>* bullets, std::vector<Entity*>* entities):
		SpriteEntity(width, height, x, y, textureID, 1024, 1024, "sheet.xml", spriteName, true), entities(entities), 
		bullets(bullets), fireRate(fireRate), bulletSprite(bulletSprite), bulletVelocityY(bulletVelocityY){
		/*shootX = Entity::getPos().getX();
		shootY = Entity::getPos().getY() + Entity::getSize().getY() * 0.5;*/
		fireTimer = rand() % GLint(fireRate);
	}

	void ShootBullet() {
		if (fireTimer > fireRate) {
			std::cout << "shoot: " << bulletVelocityY << "\n";
			bullets->push_back(new Bullet(0.02, 0.1, Entity::getPos().getX(), Entity::getPos().getY(), TexturedEntity::getTextureID(), bulletSprite, this, bullets, bulletVelocityY));
			fireTimer = 0;
		}
			
	}

	GLboolean CheckBulletHit() {
		for (GLuint i = 0; i < bullets->size(); i++)
		{
			if ((*bullets)[i]->CheckHit(this)) {
				(*bullets)[i]->RemoveBullet();
				this->RemoveObject();
				this->isAlive = false;
				return true;
			}
		}
		return false;
	}

	GLboolean RemoveObject() {
		for (GLuint i = 0; i < entities->size(); i++) {
			if ((*entities)[i] == this) {
				//delete (*entities)[i];
				(*entities)[i] = (*entities)[entities->size() - 1];
				entities->pop_back();
				return true;
			}
		}
		return false;
	}

	void Update(GLfloat elapsed) {
		fireTimer += elapsed;
		//std::cout << fireTimer << std::endl;
		if(CheckBulletHit())
			return;
	}

	GLboolean ifAlive() {
		return isAlive;
	}
};

class Enemy : public SpaceObject {
public:

	Enemy(GLfloat x, GLfloat y, GLint textureID, std::vector<Bullet*>* bullets, std::vector<Entity*>* entities) :
		SpaceObject(0.25, 0.25, x, y, 10, textureID, -0.5f, "enemyBlack1.png", "laserGreen06.png", bullets, entities) {
		Entity::ChangeVelocity(0, -0.1, 0);
	}

	void Update(GLfloat elapsed) {
		SpaceObject::Update(elapsed);
		SpaceObject::ShootBullet();
		Entity::Update(elapsed);
	}
};

class Player : public SpaceObject {
private:
	GLfloat moveSpeed = 0.5;

public:
	Player(GLfloat x, GLfloat y, GLint textureID, std::vector<Bullet*>* bullets, std::vector<Entity*>* entities) :
		SpaceObject(0.25, 0.25, x, y, 2, textureID, 0.5f, "playerShip1_blue.png", "laserBlue16.png", bullets, entities) {
	}

	void Update(GLfloat elapsed, GLboolean IfFire) {
		SpaceObject::Update(elapsed);
		if(IfFire)
			SpaceObject::ShootBullet();
		Entity::Update(elapsed);
	}

	GLboolean Move(GLfloat x) {
		return Entity::Move(Entity::getPos().getX() + moveSpeed * x, Entity::getPos().getY(), 0);
	}

};

SDL_Window* displayWindow;
ShaderProgram program;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;

SDL_Event event;
bool done = false;

GLint fontTexture;
GLint spaceTextureSheet;

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
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
};

class GameLevel : public GameState{
private:
	GLboolean ifFire = false;
	GLboolean moveLeft = false;
	GLboolean moveRight = false;
	std::vector<Entity*> entities;
	std::vector<Bullet*> bullets;
	Player* player;

public:
	GameLevel() {
		for (GLint i = -4; i < 5; i++)
		{
			entities.push_back(new Enemy(i * 0.3 * COORD_X, 0.8 * COORD_Y, spaceTextureSheet, &bullets, &entities));
		}

		player = new Player(0, -0.8 * COORD_Y, spaceTextureSheet, &bullets, &entities);
		entities.push_back(player);
	}

	void ProcessEvents(SDL_Event& event) {
		while (SDL_PollEvent(&event)) {

			// check quit event
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

			// check input events
			// Scancodes: https://wiki.libsdl.org/SDL_Scancode
			ifFire = false;
			moveLeft = false;
			moveRight = false;

			if (keys[SDL_SCANCODE_A]) {
				moveLeft = true;
			}
			else if (keys[SDL_SCANCODE_D]) {
				moveRight = true;
			}

			// check input events
			if (keys[SDL_SCANCODE_SPACE]) {
				ifFire = true;
			}
		}
	}

	void Update() {
		if (!player->ifAlive()) {
			entities.clear();
			bullets.clear();
			entities.push_back(new TextBox(-0.25, 0, 0.3, 0.005, ALIGN_MIDDLE, "GAME OVER"));
		}

		GLfloat elapsed = GameState::GetElapsed();

		for (GLuint i = 0; i < entities.size(); i++)
		{
			if (entities[i] != player)
				entities[i]->Update(elapsed);
		}

		if (moveLeft) {
			player->Move(-elapsed);
		}

		else if (moveRight) {
			player->Move(elapsed);
		}

		player->Update(elapsed, ifFire);

		for (GLuint i = 0; i < bullets.size(); i++)
		{
			bullets[i]->Update(elapsed);
		}
	}

	void Render() {
		GameState::Render();

		for (GLint i = 0; i < entities.size(); i++)
		{
			entities[i]->Draw(program);
		}

		for (GLuint i = 0; i < bullets.size(); i++)
		{
			bullets[i]->Draw(program);
		}

		SDL_GL_SwapWindow(displayWindow);
	}
};

class TitleScreen : public GameState {
private:
	std::vector<Entity*> entities;

public:
	TitleScreen() {
		entities.push_back(new TextBox(-1, 1, 0.3, 0.005, ALIGN_MIDDLE, "SPACE"));
		entities.push_back(new TextBox(0, 0.5, 0.3, 0.005, ALIGN_MIDDLE, "INVADER"));
		entities.push_back(new TextBox(0, -1, 0.1, 0.0025, ALIGN_MIDDLE, "PRESS SPACE TO START"));
	}

	void ProcessEvents(SDL_Event& event) {
		while (SDL_PollEvent(&event)) {

			// check quit event
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

			// check input events
			// Scancodes: https://wiki.libsdl.org/SDL_Scancode

			// check input events
			if (keys[SDL_SCANCODE_SPACE]) {
				gameMode = GAME_LEVEL;
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

//Space Invader

void ProcessEvents(SDL_Event& event);
void Update();
void Render();

TitleScreen* title;
GameLevel* level;

GLint main(GLint argc, char *argv[]) {

	Setup();
	while (!done) {
		ProcessEvents(event);
		Update();
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

	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	projectionMatrix.SetOrthoProjection(-COORD_Y * ASPECT_RATIO, COORD_Y * ASPECT_RATIO, -COORD_Y, COORD_Y, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetModelMatrix(modelMatrix);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	srand(time(NULL));

	spaceTextureSheet = LoadTexture("sheet.png");	
	level = new GameLevel();
	title = new TitleScreen();
}

void ProcessEvents(SDL_Event& event) {
	switch (gameMode) {
		case TITLE_SCREEN:
			title->ProcessEvents(event);
			break;
		case GAME_LEVEL:
			level->ProcessEvents(event);
			break;
	}
}

void Update() {
	switch (gameMode) {
	case GAME_LEVEL:
		level->Update();
		break;
	}
}

void Render() {
	switch (gameMode) {
	case TITLE_SCREEN:
		title->Render();
		break;
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