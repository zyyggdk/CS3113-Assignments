#ifdef _WINDOWS
	#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "ShaderProgram.h"
#include "Matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

GLuint LoadTexture(const char *filePath);

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	//Setup
	glViewport(0, 0, 640, 360);

	ShaderProgram program, programTextured;

	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	
	programTextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint emojiTexture1 = LoadTexture(RESOURCE_FOLDER"emoji1.png");
	GLuint emojiTexture2 = LoadTexture(RESOURCE_FOLDER"emoji2.png");
	GLuint emojiTexture3 = LoadTexture(RESOURCE_FOLDER"emoji3.png");

	Matrix projectionMatrix;
	Matrix modelMatrix1;
	Matrix modelMatrix2;
	Matrix viewMatrix;
	projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float lastFrameTicks = 0.0f;

	SDL_Event event;
	bool done = false;

	//Game Loop
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		//Get time
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//CLear the screen
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);		

		//Draw the emoji
		glUseProgram(programTextured.programID);
		float vertices2[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

		//emoji1
		modelMatrix1.Identity();
		programTextured.SetModelMatrix(modelMatrix1);
		programTextured.SetProjectionMatrix(projectionMatrix);
		programTextured.SetViewMatrix(viewMatrix);
		
		glBindTexture(GL_TEXTURE_2D, emojiTexture1);
		glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(programTextured.positionAttribute);
		glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(programTextured.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(programTextured.positionAttribute);
		glDisableVertexAttribArray(programTextured.texCoordAttribute);

		//emoji2
		modelMatrix1.Translate(3.0f, 0.0f, 0.0f);
		programTextured.SetModelMatrix(modelMatrix1);
		programTextured.SetProjectionMatrix(projectionMatrix);
		programTextured.SetViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, emojiTexture2);
		glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(programTextured.positionAttribute);
		glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(programTextured.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(programTextured.positionAttribute);
		glDisableVertexAttribArray(programTextured.texCoordAttribute);

		//emoji3
		modelMatrix2.Translate(elapsed, 0.0f, 0.0f);
		programTextured.SetModelMatrix(modelMatrix2);
		programTextured.SetProjectionMatrix(projectionMatrix);
		programTextured.SetViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, emojiTexture3);
		glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(programTextured.positionAttribute);
		glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(programTextured.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(programTextured.positionAttribute);
		glDisableVertexAttribArray(programTextured.texCoordAttribute);

		//Draw the triangle
		/*glUseProgram(program.programID);

		modelMatrix2.Translate(elapsed, 0.0f, 0.0f);
		program.SetModelMatrix(modelMatrix2);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);
		program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);

		float vertices[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(program.positionAttribute);*/

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

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