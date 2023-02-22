// Include standard headers
#define STB_IMAGE_IMPLEMENTATION  

#include"stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <time.h>       
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <windows.h>
#include <math.h>    


GLFWwindow* window;
using namespace glm;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser  Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}

int main(void) {
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(800, 800, u8"Ηλιακό Σύστημα", NULL, NULL);

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_FALSE, GL_TRUE);

	//background COLOR
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//Load our shaders.
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");



	//------ LOAD MY TEXTURES ---------------------------------------------
	int sunWidth, sunHeight, sunnrChannels;
	unsigned char* sunData = stbi_load("sun.jpg", &sunWidth, &sunHeight, &sunnrChannels, 0);

	if (sunData)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint sunTexture;
	glGenTextures(1, &sunTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, sunTexture);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sunWidth, sunHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, sunData);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint sunID = glGetUniformLocation(programID, "myTextureSampler");


	//For planet texture.
	int planetWidth, planetHeight, planetnrChannels;
	unsigned char* planetData = stbi_load("planet.jpg", &planetWidth, &planetHeight, &planetnrChannels, 0);

	if (planetData)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint planetTexture;
	glGenTextures(1, &planetTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, planetTexture);

	// Get a handle for our "myTextureSampler" uniform
	GLuint planetID = glGetUniformLocation(programID, "myTextureSampler");

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, planetWidth, planetHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, planetData);
	glGenerateMipmap(GL_TEXTURE_2D);
	//--------END OF TEXTURE LOADING -------



	//---------- OBJECT LOADING-----------------
	std::vector<glm::vec3> sunVertices;
	std::vector<glm::vec3> sunNormals;
	std::vector<glm::vec2> sunUvs;
	bool res = loadOBJ("sun.obj", sunVertices, sunUvs, sunNormals);

	// Load it into a VBO

	GLuint sunVertexbuffer;
	glGenBuffers(1, &sunVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sunVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sunVertices.size() * sizeof(glm::vec3), &sunVertices[0], GL_STATIC_DRAW);

	GLuint sunUvbuffer;
	glGenBuffers(1, &sunUvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sunUvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sunUvs.size() * sizeof(glm::vec2), &sunUvs[0], GL_STATIC_DRAW);
	//------------------end of sun obj------------------------------

	std::vector<glm::vec3> planetVertices;
	std::vector<glm::vec3> planetNormals;
	std::vector<glm::vec2> planetUvs;
	bool planetRes = loadOBJ("planet.obj", planetVertices, planetUvs, planetNormals);

	// Load it into a VBO

	GLuint planetVertexbuffer;
	glGenBuffers(1, &planetVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, planetVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, planetVertices.size() * sizeof(glm::vec3), &planetVertices[0], GL_STATIC_DRAW);

	GLuint planetUvbuffer;
	glGenBuffers(1, &planetUvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, planetUvbuffer);
	glBufferData(GL_ARRAY_BUFFER, planetUvs.size() * sizeof(glm::vec2), &planetUvs[0], GL_STATIC_DRAW);
	//------------------END OF OBJECT LOADING---------------------------	



//-----Meteor Texture Load-----
	int meteorWidth, meteorHeight, meteornrChannels;
	unsigned char* meteorData = stbi_load("meteor.jpg", &meteorWidth, &meteorHeight, &meteornrChannels, 0);

	if (meteorData)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint meteorTexture;
	glGenTextures(1, &meteorTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, meteorTexture);

	// Get a handle for our "myTextureSampler" uniform
	GLuint meteorID = glGetUniformLocation(programID, "myTextureSampler");

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, meteorWidth, meteorHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, meteorData);
	glGenerateMipmap(GL_TEXTURE_2D);
	//--------END OF METEOR TEXTURE LOADING -------

	//Load Meteor Object
	std::vector<glm::vec3> meteorVertices;
	std::vector<glm::vec3> meteorNormals;
	std::vector<glm::vec2> meteorUvs;
	bool meteorRes = loadOBJ("planet.obj", meteorVertices, meteorUvs, meteorNormals);

	// Load it into a VBO

	GLuint meteorVertexbuffer;
	glGenBuffers(1, &meteorVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meteorVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, meteorVertices.size() * sizeof(glm::vec3), &meteorVertices[0], GL_STATIC_DRAW);

	GLuint meteorUvbuffer;
	glGenBuffers(1, &meteorUvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meteorUvbuffer);
	glBufferData(GL_ARRAY_BUFFER, meteorUvs.size() * sizeof(glm::vec2), &meteorUvs[0], GL_STATIC_DRAW);
	//---end of meteor object loading-----

	//Some variables we need...
	glm::vec3 position = glm::vec3(50.0f, 50.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 meteorPosition = position;//Meteor's position initialized.

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 sunModel = glm::mat4(1.0f);
	glm::mat4 planetModel = glm::mat4(1.0f);
	glm::mat4 meteorModel = glm::mat4(1.0f);
	glm::mat4 rotate = glm::mat4(1.0f);
	glm::mat4 spin = glm::mat4(1.0f);
	glm::mat4 translate = glm::mat4(1.0f);

	glm::mat4 View = glm::lookAt(
		position,
		direction,
		up
	);
	glm::mat4 planetMVP, sunMVP, meteorMVP;

	float scaleFactor1 = 1.0f;
	float scaleFactor2 = 1.0f;
	float xangle = 1.0f;
	float yangle = 1.0f;
	float in = 0.1f;
	float out = 0.0f;
	float rot_angle = 0.0f;
	float spin_angle = 0.0f;
	int flag = 0;
	int meteorDraw = 1;
	int meteorFlag = 0;
	float speed = 25.0f;
	float MeteorScale = +0.5f;

	glEnable(GL_DEPTH_TEST);
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	glm::mat4 scaleMatrix = glm::mat4(1.0f);
	glm::mat4 translationMatrix = glm::mat4(1.0f);

	planetModel = glm::translate(sunModel, glm::vec3(25.0f, 0.0f, 0.0f)); //The planet will spawn at 25,0,0.

	


	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);
		// code 20 is for caps lock when is on and code 16 is fro shift when is on 
		if (((GetKeyState(20) & 0x0001) == 1)) {
			//std::cout << "caps lock pressed";

			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
				glfwTerminate();
				exit(0);
			}
		}

		//------- DRAW OUR SUN ------------------
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sunTexture);

		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(sunID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sunVertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, sunUvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		sunMVP = Projection * View * sunModel;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &sunMVP[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, sunVertices.size());

		if (meteorDraw == 1) {
			//--------------Draw planet-----------------------------------

				// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, planetTexture);

			// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(planetID, 0);

			// 3rd attribute buffer : planetVertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, planetVertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 4th attribute buffer : planetUVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, planetUvbuffer);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);


			rot_angle += glm::radians(100.0f) / 100.0f; //Orbit speed.
			spin_angle += glm::radians(100.0f) / 100.0f; //How quickly the planet orbits around itself.

			glm::vec3 tvec = glm::vec3(20.0f, -10.0f, 0.0f);//Distance from (0,0,0).
			glm::vec3 axis = glm::vec3(0.0f, 0.0f, 1.0f);
			glm::vec3 spinAxis = glm::vec3(0.0f, 1.0f, 0.0f);


			translate = glm::translate(glm::mat4(1.0f), tvec);
			rotate = glm::rotate(glm::mat4(1.0f), rot_angle, axis);

			spin = glm::rotate(glm::mat4(1.0f), spin_angle, spinAxis);

			planetModel = rotate * translate * spin;

			planetMVP = Projection * View * planetModel;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &planetMVP[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, planetVertices.size());
			//END---OF---DRAWING---PLANET
		}
		//--------------DRAW METEOR-----------------------------------
			// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, meteorTexture);

		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(meteorID, 0);

		// 3rd attribute buffer : planetVertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, meteorVertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 4th attribute buffer : planetUVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, meteorUvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);



		//std::cout << meteorModel[3][0] << " \t" << meteorModel[3][1] << " \t" << meteorModel[3][2] << " \t\n---";	


		meteorModel = glm::scale(meteorModel, glm::vec3(0.4f));

		meteorMVP = Projection * View * meteorModel;

		if (meteorFlag==1) {
			meteorPosition = position;
		}

		if (flag == 1) {
			meteorFlag = 0;//This means meteor is travelling towaards the sun.

			glm::vec3 P = glm::vec3(0, 0, 0); //Where we want to move.
			glm::vec3 BP = P - meteorPosition;
			meteorPosition = meteorPosition + 0.01f * BP;

			meteorModel = glm::translate(glm::mat4(1.0f), meteorPosition);
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &meteorMVP[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, meteorVertices.size());

			//Calculate distance between meteor's center and planet's center.
			float xd = pow(meteorModel[3][0] - sunModel[3][0], 2);
			float yd = pow(meteorModel[3][1] - sunModel[3][1], 2);
			float zd = pow(meteorModel[3][2] - sunModel[3][2], 2);
			float s = xd + yd + zd;

			if ( pow(s,0.5) <= 17.0f  ) {
				meteorFlag = 1;
				meteorModel = glm::translate(glm::mat4(1.0f), position);
				flag = 0;
			}

		}

		//-----END----OF----DRAWING------METEOR
		//Calculate distance between meteor's center and planet's center.
		float xd = pow(meteorModel[3][0] - planetModel[3][0], 2);
		float yd = pow(meteorModel[3][1] - planetModel[3][1], 2);
		float zd = pow(meteorModel[3][2] - planetModel[3][2], 2);
		float s = xd + yd + zd;
		

		//Check for collision.
		if (pow(s, 0.5) <= 7.0f) {
			meteorDraw = 0;
			flag = 0;
		}

		//Keyboards inputs.
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			flag = 1;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			float y = position[1] * position[1];// position.y^2
			float z = position[2] * position[2];// position.z^2
			float s =  y + z;
			

			
			float distance = glm::sqrt(s);//Distance between sphere's center and 0,0.
			float tan = position[2] / position[1]; 
			float angle = atan(tan); //Calculate the angle.Result is in radians.Parameter of asin always in degrees.

			angle = glm::degrees(angle);//Convert radians to degrees.
			angle += 1.0f;
			angle = glm::radians(angle);//Convert back to degrees.
			std::cout << "\nAngle:" << angle;
			//Calculate  new positions.
			position[2] = distance * glm::sin(angle);
			position[1] = distance * glm::cos(angle);
			
			//Update our ViewMatrix.
			View = glm::lookAt(
				position,
				direction,
				up
			);
		}

		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			float y = position[1] * position[1];
			float z = position[2] * position[2];
			float s = y + z;



			float distance = glm::sqrt(s);//Distance between sphere's center and 0,0,0.
			float tan = position[2] / position[1];
			float angle = atan(tan); //Calculate the angle.Result is in radians.Parameter of asin always in degrees.

			angle = glm::degrees(angle);//Convert radians to degrees.
			angle -= 1.0f;
			angle = glm::radians(angle);//Convert back to degrees.

			//Calculate  new positions.
			position[1] = distance * glm::cos(angle);
			position[2] = distance * glm::sin(angle);

			//Update our ViewMatrix.
			View = glm::lookAt(
				position,
				direction,
				up
			);
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {

			float x = position[0] * position[0];
			float z = position[2] * position[2];
			float s = x + z;



			float distance = glm::sqrt(s);//Distance between sphere's center and 0,0,0.
			float tan = position[2] / position[0];
			float angle = atan(tan); //Calculate the angle.Result is in radians.Parameter of asin always in degrees.

			angle = glm::degrees(angle);//Convert radians to degrees.
			angle += 1.0f;
			angle = glm::radians(angle);//Convert back to degrees.

			//Calculate  new positions.
			position[0] = distance * glm::cos(angle);
			position[2] = distance * glm::sin(angle);

			//Update our ViewMatrix.
			View = glm::lookAt(
				position,
				direction,
				up
			);
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			float x = position[0] * position[0];
			float z = position[2] * position[2];
			float s = x + z;



			float distance = glm::sqrt(s);//Distance between sphere's center and 0,0,0.
			float tan = position[2] / position[0];
			float angle = atan(tan); //Calculate the angle.Result is in radians.Parameter of asin always in degrees.

			angle = glm::degrees(angle);//Convert radians to degrees.
			angle -= 1.0f;
			angle = glm::radians(angle);//Convert back to degrees.

			//Calculate  new positions.
			position[0] = distance * glm::cos(angle);
			position[2] = distance * glm::sin(angle);

			//Update our ViewMatrix.
			View = glm::lookAt(
				position,
				direction,
				up
			);
		}
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
			
			glm::vec3 P = glm::vec3(0,0,0); //Where we want to move.
			glm::vec3 BP = P - position;

			position = position + 0.01f * BP;


			View = glm::lookAt(
				position,
				direction,
				up
			);
		}

		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
			glm::vec3 P = glm::vec3(0, 0, 0); //Where we want to move.
			glm::vec3 BP = P - position;

			position = position - 0.01f * BP;


			View = glm::lookAt(
				position,
				direction,
				up
			);
		}


		//Disable our buffers.
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}


	// Check if the ESC key was pressed or the window was closed
	while ((glfwWindowShouldClose(window) == 0));


	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	// Close OpenGL window and terminate GLFW


	return 0;
}