//Assignment 4 Yiyang Zeng yz3622
//Platformer A and D to move to teh left and right, SPACE to jump
//You can collect the blue balls in the scene. 

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
private:
	float x;
	float y;
	float z;

public:
	Vector3() :x(0), y(0), z(0) {

	}

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
	Vector3 acceleration;
	Vector3 friction;
	Vector3 size;

	GLfloat rotation;

	const GLfloat gravity = -1;
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;

	Matrix modelMatrix;

	GLboolean isStatic = true;

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
		if (!isStatic) {
			ChangeVelocity(lerp(velocity.getX(), 0.0f, elapsed * friction.getX()),
				lerp(velocity.getY(), 0.0f, elapsed * friction.getY()),
				lerp(velocity.getZ(), 0.0f, elapsed * friction.getZ()));
			ChangeVelocity(velocity.getX() + acceleration.getX() * elapsed,
				velocity.getY() + (acceleration.getY() + gravity) * elapsed,
				velocity.getZ() + acceleration.getZ() * elapsed);

			Move(position.getX() + velocity.getX() * elapsed,
				position.getY() + velocity.getY() * elapsed,
				position.getZ() + velocity.getZ() * elapsed);
			std::cout << "Pos:" << getPos().getX() << "," << getPos().getY() << std::endl;
			std::cout << "Vel:" << getVelocity().getX() << "," << getVelocity().getY() << std::endl;
			std::cout << "Acc:" << acceleration.getX() << "," << acceleration.getY() << std::endl;
		}
		
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
		GLfloat thisWidth = size.getX(), thisHeight = size.getY();
		GLfloat otherWidth = other->size.getX(), otherHeight = other->size.getY();

		return !(
			this->position.getY() - thisHeight / 2 > other->position.getY() + otherHeight / 2 ||
			this->position.getY() + thisHeight / 2 < other->position.getY() - otherHeight / 2 ||
			this->position.getX() - thisWidth / 2 > other->position.getX() + otherWidth / 2 ||
			this->position.getX() + thisWidth / 2 < other->position.getX() - otherWidth / 2
			);
	}

	GLboolean ResolveCollision(Entity* other) {
		GLfloat thisWidth = size.getX(), thisHeight = size.getY();
		GLfloat otherWidth = other->size.getX(), otherHeight = other->size.getY();
		GLfloat penetrationY = fabs(this->position.getY() - other->position.getY())
			- thisHeight / 2 - otherHeight / 2;
		GLfloat penetrationX = fabs(this->position.getX() - other->position.getX())
			- thisWidth / 2 - otherWidth / 2;
		if (penetrationY > 0)
			penetrationY = 0;
		else
			penetrationY -= 0.001;
		if (penetrationX > 0)
			penetrationX = 0;
		else
			penetrationX -= 0.001;
		//Move up
		if (this->position.getY() > other->position.getY()) {
			penetrationY = -penetrationY;
		}
		//Move right
		if (this->position.getX() > other->position.getX()) {
			penetrationX = -penetrationX;
		}
		Move(penetrationX, penetrationY, 0);
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

class TiledMap:public TexturedEntity {
private:
	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> texCoordData;

	GLint tileCount = 0;

public:
	unsigned int levelData[LEVEL_HEIGHT][LEVEL_WIDTH] =
	{
		{ 11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11 },
		{ 0,20,4,4,4,4,4,4,0,0,0,0,0,0,4,4,4,4,4,4,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,6,6,6,6,6,6,6,6,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,6,6,6,6,6,0,0,0,0,0,0,0,0,6,6,6,6,6,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,6,6,6,6,6,6,6,6,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 0,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,0 },
		{ 2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2 },
		{ 32,33,33,34,32,33,33,34,33,35,35,35,35,32,33,32,34,32,33,32,33,33 }
	};

	TiledMap(GLfloat width, GLfloat height, GLfloat x, GLfloat y,
		GLint textureID) :
		TexturedEntity(width, height, x, y, textureID) {
		
		

		for (int y = 0; y < LEVEL_HEIGHT; y++) {
			for (int x = 0; x < LEVEL_WIDTH; x++) {
				if (levelData[y][x] != 0) {
					float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
					float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;

					float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
					float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

					vertexData.insert(vertexData.end(), {

						TILE_SIZE * x, -TILE_SIZE * y,
						TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
						(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,

						TILE_SIZE * x, -TILE_SIZE * y,
						(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
						(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
					});

					texCoordData.insert(texCoordData.end(), {
						u, v,
						u, v + (spriteHeight),
						u + spriteWidth, v + (spriteHeight),

						u, v,
						u + spriteWidth, v + (spriteHeight),
						u + spriteWidth, v
					});
					tileCount+=6;
				}

			}
		}
	}

	void Draw(ShaderProgram& program) {
		
		TexturedEntity::Draw(program, vertexData.data(), tileCount, texCoordData.data());
	}
};

class TileEntity: public UniformSpriteEntity {
private:
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

public:
	TileEntity(GLfloat tileSize, GLfloat x, GLfloat y,
		GLint textureID, GLint spriteCountX, GLint spriteCountY,
		GLint spriteId):
		UniformSpriteEntity(tileSize, x, y, textureID, 
			spriteCountX, spriteCountY, spriteId){

	}

	void Update() {
		collidedTop = false;
		collidedBottom = false;
		collidedLeft = false;
		collidedRight = false;	
	}

	void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
		*gridX = (int)(worldX / TILE_SIZE);
		*gridY = (int)(-worldY / TILE_SIZE);
	}

	void tileMapCollision(TiledMap* map) {
		GLint gridX, gridY;
		GLfloat x = getPos().getX();
		GLfloat y = getPos().getY();
		GLfloat midTopY = getPos().getY() + (getSize().getY() / 2);
		GLfloat midBottomY = getPos().getY() - (getSize().getY() / 2);
		GLfloat midLeftX = getPos().getX() - (getSize().getX() / 2);
		GLfloat midRightX = getPos().getX() + (getSize().getX() / 2);
		//Check top
		worldToTileCoordinates(x, midTopY, &gridX, &gridY);
		if (map->levelData[gridY][gridX] != 0) {
			collidedTop = true;
			Move(getPos().getX(), getPos().getY() - fabs(midTopY - (map->getPos().getY() - TILE_SIZE * gridY - TILE_SIZE)) - 0.001
				, getPos().getZ());
			ChangeVelocity(getVelocity().getX(), 0, getVelocity().getZ());
		}
			
		//Check bottom
		worldToTileCoordinates(x, midBottomY, &gridX, &gridY);
		if (map->levelData[gridY][gridX] != 0) {
			collidedBottom = true;
			Move(getPos().getX(), getPos().getY() + fabs((map->getPos().getY()-TILE_SIZE * gridY) - midBottomY) + 0.001,
				getPos().getZ());
			ChangeVelocity(getVelocity().getX(), 0, getVelocity().getZ());
		}
		std::cout << "Collisions: " << collidedBottom << gridX << "," << gridY << std::endl;
			
		//Check left
		worldToTileCoordinates(midLeftX, y, &gridX, &gridY);
		if (map->levelData[gridY][gridX] != 0) {
			collidedLeft = true;
			Move(getPos().getX() + fabs(midLeftX + (map->getPos().getX() + TILE_SIZE * gridX + TILE_SIZE)) + 0.01,
				getPos().getY(), getPos().getZ());
			ChangeVelocity(0, getVelocity().getY(), getVelocity().getZ());
		}			

		////Check right
		worldToTileCoordinates(midRightX, y, &gridX, &gridY);
		if (map->levelData[gridY][gridX] != 0) {
			collidedRight = true;
			Move(getPos().getX() + fabs((map->getPos().getX() + TILE_SIZE * gridX) - midRightX) - 0.01,
				getPos().getY(), getPos().getZ());
			ChangeVelocity(0, getVelocity().getY(), getVelocity().getZ());
		}
	}

	void virtual tileInteract(TileEntity* other) {
		GLint gridX, gridY;
		GLfloat worldTop = getPos().getY() + (getSize().getY() / 2);
		GLfloat worldBottom = getPos().getY() - (getSize().getY() / 2);
		GLfloat worldLeft = getPos().getX() - (getSize().getX() / 2);
		GLfloat worldRight = getPos().getX() + (getSize().getX() / 2);

	}

	GLboolean ifCollideTop() {
		return collidedTop;
	}

	GLboolean ifCollideBottom() {
		return collidedBottom;
	}

	GLboolean ifCollideLeft() {
		return collidedLeft;
	}

	GLboolean ifCollideRight() {
		return collidedRight;
	}
};

class Player: public TileEntity {
private:

public:
	Player(GLfloat tileSize, GLfloat x, GLfloat y,
		GLint textureID, GLint spriteCountX, GLint spriteCountY,
		GLint spriteId) :
		TileEntity(tileSize, x, y, textureID,
			spriteCountX, spriteCountY, spriteId) {

	}

	void Update(GLfloat elapsed, GLboolean ifJump) {
		
		if (ifJump && TileEntity::ifCollideBottom()) {
			Entity::ChangeVelocity(getVelocity().getX(), getVelocity().getY() + 3,
				getVelocity().getZ());
		}
			
		Entity::Update(elapsed);
		TileEntity::Update();
	}

	void Move(GLfloat amount) {
		Entity::ChangeAcc(amount * 60, 0, 0);
		if (ifCollideBottom())
			Entity::ChangeFric(1, 0, 0);
	}
};

class BlueBall : public UniformSpriteEntity {
private:
	std::vector<Entity*>* entities;
	Entity* player;

public:
	BlueBall(GLfloat tileSize, GLfloat x, GLfloat y,
		GLint textureID, GLint spriteCountX, GLint spriteCountY,
		GLint spriteId, std::vector<Entity*>* entities, Entity* player) :
		UniformSpriteEntity(tileSize, x, y, textureID,
			spriteCountX, spriteCountY, spriteId), entities(entities), player(player){

	}

	void Update(GLfloat elapsed) {
		if (player->RecCollision(this))
			Remove();
		return;

		Entity::Update(elapsed);
	}

	void Remove() {
		for (GLuint i = 0; i < entities->size(); i++) {
			if ((*entities)[i] == this) {
				Entity* temp = (*entities)[i];
				(*entities)[i] = (*entities)[entities->size() - 1];
				entities->pop_back();
			}
		}
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
const Uint8 *keys = SDL_GetKeyboardState(NULL);

SDL_Event event;
bool done = false;

GLint fontTexture;
GLint mapSheet;

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
	GLboolean moveLeft = false;
	GLboolean moveRight = false;
	GLboolean ifJump = false;
	std::vector<Entity*> entities;
	TiledMap* map;
	Player* player;

public:
	GameLevel() {
		map = new TiledMap(0, 0, 0, 0, mapSheet);
		entities.push_back(map);
		player = new Player(1, 3, -8, mapSheet, 16, 8, 98);
		player->setDynamic();
		entities.push_back(player);
		entities.push_back(new BlueBall(1, 12, -3, mapSheet, 16, 8, 21, &entities, player));
		entities.push_back(new BlueBall(1, 12, -7, mapSheet, 16, 8, 21, &entities, player));
		entities.push_back(new BlueBall(1, 6, -5, mapSheet, 16, 8, 21, &entities, player));
		entities.push_back(new BlueBall(1, 18, -5, mapSheet, 16, 8, 21, &entities, player));
		entities.push_back(new BlueBall(1, 6, -12, mapSheet, 16, 8, 21, &entities, player));
		entities.push_back(new BlueBall(1, 18, -12, mapSheet, 16, 8, 21, &entities, player));
	}

	void ProcessEvents(SDL_Event& event) {
		while (SDL_PollEvent(&event)) {

			// check quit event
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

			// check input events
			// Scancodes: https://wiki.libsdl.org/SDL_Scancode
			ifJump = false;
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
				ifJump = true;
			}
		}
	}

	void Update(GLfloat timeStep) {

		for (GLuint i = 0; i < entities.size(); i++)
		{
			if (entities[i] != player)
				entities[i]->Update(timeStep);
		}

		player->tileMapCollision(map);

		if (moveLeft) {
			player->Move(-timeStep);
		}

		else if (moveRight) {
			player->Move(timeStep);
		}

		else {
			player->Move(0);
		}

		player->Update(timeStep, ifJump);

		viewMatrix.Identity();
		viewMatrix.Translate(-(player->getPos().getX()), -(player->getPos().getY()), -(player->getPos().getZ()));

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

//Platformer

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

	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	projectionMatrix.SetOrthoProjection(-COORD_Y * ASPECT_RATIO, COORD_Y * ASPECT_RATIO, -COORD_Y, COORD_Y, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetModelMatrix(modelMatrix);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	srand(time(NULL));

	mapSheet = LoadTexture("arne_sprites.png");	
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