#include "EnginePlatform.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

#include <gl/gl.h>

// TODO(fraser): strip out all this glm stuff
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

// TODO(fraser): Use another image loading library or something (million warnings) - or make my own!
#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 26451 6385 6011 6262 6308)
#include <stb_image.h>
#pragma warning(pop)

#include <vector>
#include <string>
#include <unordered_map>

#include <assert.h>
#include <time.h> 
#include "RendererPlatform.hpp"

#include "GUID.hpp"


struct OpenGLShader
{
	unsigned int programID;

	// Vertex locations
	unsigned int PositionLocation;
	unsigned int NormalLocation;
	unsigned int ColourLocation;
	unsigned int UVLocation;

	// Uniform locations
	unsigned int TimeLocation;
	unsigned int CameraLocation;
	unsigned int TransformationLocation;
	unsigned int SunDirectionLocation;

};

// Globals
static OpenGLShader defaultShader;

static HDC deviceContext;
static HGLRC glContext;

static Texture whiteRenderTexture;

//TODO(fraser) probably gonna want to move this elsewhere
static std::vector<DebugLineSegment> debugLineSegments;

GLuint debugLineDrawProgram;
GLuint debugLineDrawVAO;
GLuint debugLineDrawVBO;
GLuint debugLineDrawCameraTransformLocation;

struct OpenGLMesh
{
	OpenGLMesh() : texture(whiteRenderTexture)
	{}
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	int numElements;
	int numVertices;
	Texture texture;
	Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
	Vec3f scale = Vec3f(1.0f, 1.0f, 1.0f);
	Quaternion rotation;

	Mat4x4f transform;
	bool transformNeedsUpdate = true;

	//float rotationAroundXAxis = 0.0f;
	//float rotationAroundYAxis = 0.0f;
	//float rotationAroundZAxis = 0.0f;

	DrawType drawType = DrawType::Triangle;
};

std::unordered_map<Mesh_ID, OpenGLMesh> meshes;

static OpenGLMesh* GetOpenGLMeshFromID(Mesh_ID id)
{
	auto it = meshes.find(id);
	if (it != meshes.end())
	{
		return &it->second;
	}
	else {
		return nullptr;
	}
}

Renderer::Renderer()
{
	HWND window = GetActiveWindow();

	deviceContext = GetDC(window);

	PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
	desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	desiredPixelFormat.nVersion = 1;
	desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	desiredPixelFormat.cColorBits = 32;
	desiredPixelFormat.cDepthBits = 24;
	desiredPixelFormat.cAlphaBits = 8;
	desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	int pixelFormatIndex = ChoosePixelFormat(deviceContext, &desiredPixelFormat);
	
	if (pixelFormatIndex == 0)
	{
		Engine::DEBUGPrint("Unable to pick pixel format.\n");
		// TODO: Error handling
	}

	SetPixelFormat(deviceContext, pixelFormatIndex, &desiredPixelFormat);

	glContext = wglCreateContext(deviceContext);
	wglMakeCurrent(deviceContext, glContext);

	GLenum err = glewInit();
	if (err == GLEW_OK)
	{
		Engine::DEBUGPrint("Glew was initialized.\n");
	}
	else {
		Engine::DEBUGPrint("Glew was not initialized.\n");
	}

	// Enable various OpenGL features
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// I prefer clockwise winding
	glFrontFace(GL_CW);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// TODO(fraser): come up with a way for the .lib to store textures I want to be included in the engine (ie. white texture)
	// Will probably write something to load an image into a header file which can be included wherever it's needed
	whiteRenderTexture = LoadTexture("images/white.png");

	// TEMP(fraser)
	glLineWidth(2.0f);

	// Set up the default shader
	GLuint defaultVertShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint defaultFragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// TODO: decide on glsl version
	const char* vertShaderSource = R"(
	#version 400	
	
	uniform mat4x4 Transformation;
	uniform mat4x4 Camera;

	in vec4 VertPosition;
	in vec4 VertNormal;
	in vec4 VertColour;
	in vec2 VertUV;
	
	smooth out vec3 FragNormal;
	smooth out vec4 FragColour;	
	smooth out vec2 FragUV;

	void main()
	{
		gl_Position = (Camera * Transformation) * VertPosition;
		
		//TEMP(fraser): costly inverses
		FragNormal = mat3(transpose(inverse(Transformation))) * VertNormal.xyz;
		FragColour = VertColour;
		FragUV = VertUV;
	}

	)";

	const char* fragShaderSource = R"(
	
	#version 400

	smooth in vec3 FragNormal;
	smooth in vec4 FragColour;
	smooth in vec2 FragUV;	

	uniform sampler2D DiffuseTexture;
	
	uniform vec2 WindowSize;
	uniform float Time;
	uniform vec3 SunDirection;

	out vec4 OutColour;

	void main()
	{
		vec3 sun = vec3(SunDirection.x, SunDirection.y, -SunDirection.z);

		vec3 normalizedNormal = normalize(FragNormal);
		vec4 textureAt = texture(DiffuseTexture, FragUV);
		float diffuse = ((dot(normalizedNormal.xyz, normalize(sun))) + 1.0) / 2.0;
		
		OutColour.rgb = diffuse * (textureAt.rgb * FragColour.rgb);
		//OutColour.rgb = (textureAt.rgb * FragColour.rgb);
				
		OutColour.a = textureAt.a * FragColour.a;
		
		//TEMP (fraser) 
 		if (gl_FragCoord.x > (WindowSize.x / 2.0) - 5.0 && gl_FragCoord.x < (WindowSize.x / 2.0) + 5.0 && 
			gl_FragCoord.y > (WindowSize.y / 2.0) - 5.0 && gl_FragCoord.y < (WindowSize.y / 2.0) + 5.0)
		{
			OutColour.r = OutColour.r > 0.5 ? 0.0 : 1.0;
			OutColour.g = OutColour.g > 0.5 ? 0.0 : 1.0;
			OutColour.b = OutColour.b > 0.5 ? 0.0 : 1.0;
		} 
	}

	)";
	
	glShaderSource(defaultVertShaderID, 1, &vertShaderSource, nullptr);
	glShaderSource(defaultFragShaderID, 1, &fragShaderSource, nullptr);

	glCompileShader(defaultVertShaderID);
	glCompileShader(defaultFragShaderID);

	CheckShaderCompilation(defaultVertShaderID);
	CheckShaderCompilation(defaultFragShaderID);

	defaultShader.programID = glCreateProgram();
	glAttachShader(defaultShader.programID, defaultVertShaderID);
	glAttachShader(defaultShader.programID, defaultFragShaderID);
	
	glLinkProgram(defaultShader.programID);

	glValidateProgram(defaultShader.programID);
	
	GLint linked = false;
	glGetProgramiv(defaultShader.programID, GL_LINK_STATUS, &linked);

	//TODO: maybe make my own assert
	assert(linked && "Shader program failed to link.");

	glDetachShader(defaultShader.programID, defaultVertShaderID);
	glDetachShader(defaultShader.programID, defaultFragShaderID);

	glDeleteShader(defaultVertShaderID);
	glDeleteShader(defaultFragShaderID);

	//Link default shader program's attributes and uniforms
	defaultShader.TimeLocation = glGetUniformLocation(defaultShader.programID, "Time");
	defaultShader.CameraLocation = glGetUniformLocation(defaultShader.programID, "Camera");
	defaultShader.TransformationLocation = glGetUniformLocation(defaultShader.programID, "Transformation");
	defaultShader.SunDirectionLocation = glGetUniformLocation(defaultShader.programID, "SunDirection");

	defaultShader.PositionLocation = glGetAttribLocation(defaultShader.programID, "VertPosition");
	defaultShader.NormalLocation = glGetAttribLocation(defaultShader.programID, "VertNormal");
	defaultShader.ColourLocation = glGetAttribLocation(defaultShader.programID, "VertColour");
	defaultShader.UVLocation = glGetAttribLocation(defaultShader.programID, "VertUV");

	glUseProgram(defaultShader.programID);
	glUniform1i(glGetUniformLocation(defaultShader.programID, "DiffuseTexture"), GL_TEXTURE0);
	
	RECT screenRect;
	GetClientRect(window, &screenRect);
	screenSize = Vec2i(screenRect.right - screenRect.left, screenRect.bottom - screenRect.top);

	// Set uniform vec2 to screen size
	glUniform2f(glGetUniformLocation(defaultShader.programID, "WindowSize"), (GLfloat)screenSize.x, (GLfloat)screenSize.y);

	InitializeDebugDraw();

}

Renderer::~Renderer()
{
	wglMakeCurrent(deviceContext, nullptr);
	wglDeleteContext(glContext);
}

Texture Renderer::LoadTexture(std::string filePath)
{
	GLuint glTextureID;
	glGenTextures(1, &glTextureID);

	glBindTexture(GL_TEXTURE_2D, glTextureID);

	int width, height, channels;
	unsigned char* textureData = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	float anisotropic = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic);

	switch (channels)
	{
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
		break;
	case 4:
	default:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
		break;

	}

	glGenerateMipmap(GL_TEXTURE_2D);


	stbi_image_free(textureData);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	return Texture(glTextureID);
}

Mesh_ID Renderer::LoadMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
	// TODO: Bunch of options for VAOs, VBOs, etc. Could use one VBO for all meshes using the same shader, 
	// could use same VAO with multiple VBOs with the functions in new opengl versions etc.
	OpenGLMesh newMesh;
	newMesh.texture = whiteRenderTexture;

	glGenVertexArrays(1, &newMesh.VAO);
	glBindVertexArray(newMesh.VAO);

	glGenBuffers(1, &newMesh.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, newMesh.VBO);

	glGenBuffers(1, &newMesh.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.EBO);
	
	// TODO: option for dynamic vs static draw (profile difference too)
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(defaultShader.PositionLocation);
	glVertexAttribPointer(defaultShader.PositionLocation, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)0);

	glEnableVertexAttribArray(defaultShader.NormalLocation);
	glVertexAttribPointer(defaultShader.NormalLocation, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	glEnableVertexAttribArray(defaultShader.ColourLocation);
	glVertexAttribPointer(defaultShader.ColourLocation, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

	glEnableVertexAttribArray(defaultShader.UVLocation);
	glVertexAttribPointer(defaultShader.UVLocation, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(10 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	newMesh.numElements = (int)indices.size();
	newMesh.numVertices = (int)vertices.size();

	Mesh_ID newID;
	newID = GUIDGen::Generate();
	meshes.insert(std::pair<Mesh_ID, OpenGLMesh>(newID, newMesh));
	return newID;
}


void Renderer::DrawMesh(Mesh_ID meshID)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);

	// TODO(fraser): allow non default shaders, decided on a per-mesh basis
	glUseProgram(defaultShader.programID);

	// TODO(fraser): probably want to do this somewhere else (ie a camera module updates every frame in the engine)
	// Probably I want only the specific mesh's transform to be updated in the shader here
	if (defaultCamera.camMatrixNeedsUpdate)
	{
		UpdateCamTransform();
	}
	// TODO(fraser): stop using glm here

	glm::mat4 transformMatrix(1.0);
	transformMatrix = glm::translate(transformMatrix, glm::vec3(mesh.position.x, mesh.position.y, mesh.position.z));
	transformMatrix = glm::scale(transformMatrix, glm::vec3(mesh.scale.x, mesh.scale.y, mesh.scale.z));
	glm::quat quat = glm::quat(mesh.rotation.w, mesh.rotation.x, mesh.rotation.y, mesh.rotation.z);
	glm::mat4 rotMat = glm::toMat4(quat);
	transformMatrix = transformMatrix * rotMat;

	OpenGLMesh* meshPtr = GetOpenGLMeshFromID(meshID);

	meshPtr->transform.m_Rows[0] = { transformMatrix[0][0], transformMatrix[1][0], transformMatrix[2][0], transformMatrix[3][0] };
	meshPtr->transform.m_Rows[1] = { transformMatrix[0][1], transformMatrix[1][1], transformMatrix[2][1], transformMatrix[3][1] };
	meshPtr->transform.m_Rows[2] = { transformMatrix[0][2], transformMatrix[1][2], transformMatrix[2][2], transformMatrix[3][2] };
	meshPtr->transform.m_Rows[3] = { transformMatrix[0][3], transformMatrix[1][3], transformMatrix[2][3], transformMatrix[3][3] };

	glUniformMatrix4fv(defaultShader.TransformationLocation, 1, GL_FALSE, glm::value_ptr(transformMatrix));

	glBindTexture(GL_TEXTURE_2D, mesh.texture.m_TextureId);
	glBindVertexArray(mesh.VAO);
	
	if (mesh.drawType == DrawType::Triangle)
	{
		glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
	}
	if (mesh.drawType == DrawType::Line)
	{
		glDrawElements(GL_LINES, mesh.numElements, GL_UNSIGNED_INT, 0);
	}

	// TODO(fraser): look into overhead of binding/unbinding VAOs and shader programs
	glBindVertexArray(0);
	glUseProgram(0);
}

void Renderer::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetTime(float time)
{
	glUseProgram(defaultShader.programID);

	glUniform1f(defaultShader.TimeLocation, time);

	//glUseProgram(0);
}

void Renderer::SetCamTransform(Vec3f position, Vec3f direction, Vec3f up)
{
	
	defaultCamera.position = position;
	defaultCamera.direction = direction;
	defaultCamera.up = up;
	
	defaultCamera.camMatrixNeedsUpdate = true;

}

void Renderer::SetSunDirection(Vec3f newDirection)
{
	glUseProgram(defaultShader.programID);
	glm::vec3 glmVec = { newDirection.x, newDirection.y, newDirection.z };
	glUniform3fv(defaultShader.SunDirectionLocation, 1, glm::value_ptr(glmVec));
	//glUseProgram(0);
}

void Renderer::SetVertexPosition(Mesh_ID meshID, unsigned int vertexIndex, Vec3f newPosition)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);

	static bool bound = false;
	if (!bound)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
		bound = true;
	}
	glBufferSubData(GL_ARRAY_BUFFER, vertexIndex * sizeof(Vertex), sizeof(Vec3f), &newPosition);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::vector<Vertex*> Renderer::MapMeshVertices(Mesh_ID meshID)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);

	Vertex* vertexBuffer = (Vertex*)glMapNamedBuffer(mesh.VBO, GL_READ_WRITE);
	std::vector<Vertex*> vertices;
	for (int i = 0; i < mesh.numVertices; ++i)
	{
		vertices.push_back(&vertexBuffer[i]);
	}
	return vertices;
}

void Renderer::UnmapMeshVertices(Mesh_ID meshID)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);
	glUnmapNamedBuffer(mesh.VBO);
}

std::vector<unsigned int*> Renderer::MapMeshElements(Mesh_ID meshID)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);

	unsigned int* elementBuffer = (unsigned int*)glMapNamedBuffer(mesh.EBO, GL_READ_WRITE);
	std::vector<unsigned int*> elements;
	for (int i = 0; i < mesh.numElements; ++i)
	{
		elements.push_back(&elementBuffer[i]);
	}
	return elements;
}

void Renderer::UnmapMeshElements(Mesh_ID meshID)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);
	glUnmapNamedBuffer(mesh.EBO);
}

void* Renderer::MapMeshRange(Mesh_ID meshID, unsigned int offset, unsigned int range)
{
	OpenGLMesh mesh = *GetOpenGLMeshFromID(meshID);
	return glMapNamedBufferRange(mesh.VBO, offset, range, GL_MAP_WRITE_BIT);
}

void Renderer::SetMeshTexture(Mesh_ID meshID, Texture texture)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->texture = texture;
}

void Renderer::SetMeshDrawType(Mesh_ID meshID, DrawType type)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->drawType = type;
}

void Renderer::MoveMesh(Mesh_ID meshID, Vec3f move)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->position += move;
}

void Renderer::ScaleMesh(Mesh_ID meshID, Vec3f scaleFactor)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->scale += scaleFactor;
}

Vec3f Renderer::GetMeshPosition(Mesh_ID meshID)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	return mesh->position;
}

void Renderer::SetMeshPosition(Mesh_ID meshID, Vec3f newPos)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->position = newPos;
}

Vec3f Renderer::GetMeshScale(Mesh_ID meshID)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	return mesh->scale;
}

void Renderer::SetMeshScale(Mesh_ID meshID, Vec3f newScale)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->scale = newScale;
}

void Renderer::RotateMeshAroundAxis(Mesh_ID meshID, Vec3f axis, float rotationAmount)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	Quaternion quat(axis, rotationAmount);
	mesh->rotation = quat * mesh->rotation;
}

void Renderer::SetMeshRotation(Mesh_ID meshID, Quaternion rotation)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	mesh->rotation = rotation;
}

Quaternion Renderer::GetMeshRotation(Mesh_ID meshID)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);
	return mesh->rotation;
}

void Renderer::InitializeDebugDraw()
{
	GLuint debugDrawVertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint debugDrawFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	const char* vertShaderSource = R"(
	#version 400

	uniform mat4x4 Camera;	

	in vec3 VertPosition;

	smooth out vec4 FragColour;

	void main()
	{
		gl_Position = Camera * vec4(VertPosition, 1.0);
		FragColour = vec4(1.0, 1.0, 1.0, 1.0);
	}
	)";

	const char* fragShaderSource = R"(
	#version 400

	in vec4 FragColour;
	
	out vec4 OutColour;

	void main()
	{
		OutColour = FragColour;
	}
	)";

	glShaderSource(debugDrawVertShader, 1, &vertShaderSource, nullptr);
	glShaderSource(debugDrawFragShader, 1, &fragShaderSource, nullptr);

	glCompileShader(debugDrawVertShader);
	glCompileShader(debugDrawFragShader);

	CheckShaderCompilation(debugDrawVertShader);
	CheckShaderCompilation(debugDrawFragShader);

	debugLineDrawProgram = glCreateProgram();
	glAttachShader(debugLineDrawProgram, debugDrawVertShader);
	glAttachShader(debugLineDrawProgram, debugDrawFragShader);

	glLinkProgram(debugLineDrawProgram);

	glValidateProgram(debugLineDrawProgram);

	GLint linked = false;
	glGetProgramiv(debugLineDrawProgram, GL_LINK_STATUS, &linked);

	//TODO: maybe make my own assert
	assert(linked && "Shader program failed to link.");

	glDetachShader(debugLineDrawProgram, debugDrawVertShader);
	glDetachShader(debugLineDrawProgram, debugDrawFragShader);

	glDeleteShader(debugDrawVertShader);
	glDeleteShader(debugDrawFragShader);

	// Bind program inputs
	glBindAttribLocation(debugLineDrawProgram, 0, "VertPosition");
	debugLineDrawCameraTransformLocation = glGetUniformLocation(debugLineDrawProgram, "Camera");

	// Generate Array/Buffer objects
	glGenVertexArrays(1, &debugLineDrawVAO);
	glGenBuffers(1, &debugLineDrawVBO);

	glBindVertexArray(debugLineDrawVAO);
	glBindBuffer(GL_ARRAY_BUFFER, debugLineDrawVBO);

	//TODO(fraser): decide on maximum size of this buffer (also potentially use some kind of bin allocate method  
	// but that's probably overkill for what I want to do with debug drawing at the moment).
	// Preallocate vertex buffer
	glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(DebugLineSegment), nullptr, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0); // VertPosition
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);

	// Unbind stuff
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Renderer::DrawDebugLines()
{

	//DisableDepthTesting();

	glBindVertexArray(debugLineDrawVAO);
	glBindBuffer(GL_ARRAY_BUFFER, debugLineDrawVBO);
	glUseProgram(debugLineDrawProgram);

	glm::mat4 viewMatrix;

	glm::vec3 glmPosition = glm::vec3(defaultCamera.position.x, defaultCamera.position.y, defaultCamera.position.z);
	glm::vec3 glmDirection = glm::vec3(defaultCamera.direction.x, defaultCamera.direction.y, defaultCamera.direction.z);
	glm::vec3 glmUp = glm::vec3(defaultCamera.up.x, defaultCamera.up.y, defaultCamera.up.z);

	viewMatrix = glm::lookAt(glmPosition, glmPosition + glmDirection, glmUp);

	glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), (float)screenSize.x / (float)screenSize.y, 0.01f, 400.0f);

	glm::mat4 camMatrix = projectionMatrix * viewMatrix;

	glUniformMatrix4fv(debugLineDrawCameraTransformLocation, 1, GL_FALSE, glm::value_ptr(camMatrix));

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(DebugLineSegment) * debugLineSegments.size(), debugLineSegments.data());

	glDrawArrays(GL_LINES, 0, (int)debugLineSegments.size() * 2);

	debugLineSegments.clear();

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//EnableDepthTesting();
}

void Renderer::DebugDrawLineSegment(DebugLineSegment lineSegment)
{
	debugLineSegments.push_back(lineSegment);
}

Mat4x4f Renderer::GetMeshTransform(Mesh_ID meshID)
{
	OpenGLMesh* mesh = GetOpenGLMeshFromID(meshID);

	return mesh->transform;
}

void Renderer::EnableDepthTesting()
{
	glEnable(GL_DEPTH_TEST);
}

void Renderer::DisableDepthTesting()
{
	glDisable(GL_DEPTH_TEST);
}

void Renderer::ClearDepthBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetMeshColour(Mesh_ID meshID, Vec4f colour)
{
	std::vector<Vertex*> meshVertices = MapMeshVertices(meshID);

	for (int i = 0; i < meshVertices.size(); ++i)
	{
		meshVertices[i]->colour = colour;
	}

	UnmapMeshVertices(meshID);

}

Mesh_ID Renderer::GenerateLineMeshFromMesh(Mesh_ID mesh, Vec4f colour)
{
	Mesh_ID newMesh;

	std::vector<unsigned int*> meshElements = MapMeshElements(mesh);
	std::vector<Vertex*> meshVertices = MapMeshVertices(mesh);


	std::vector<unsigned int> lineElements;
	std::vector<Vertex> lineVertices;

	for (int i = 0; i < meshElements.size(); i += 3)
	{
		lineElements.push_back(*meshElements[i]);
		lineElements.push_back(*meshElements[(size_t)i + 1]);
		lineElements.push_back(*meshElements[(size_t)i + 1]);
		lineElements.push_back(*meshElements[(size_t)i + 2]);
		lineElements.push_back(*meshElements[(size_t)i + 2]);
		lineElements.push_back(*meshElements[i]);
	}
	for (int i = 0; i < meshVertices.size(); ++i)
	{
		Vertex vert = Vertex(*meshVertices[i]);
		vert.position += vert.normal * 0.005f;
		vert.colour = colour;
		lineVertices.push_back(vert);
	}

	newMesh = LoadMesh(lineVertices, lineElements);

	SetMeshPosition(newMesh, GetMeshPosition(mesh));
	SetMeshScale(newMesh, GetMeshScale(mesh));

	SetMeshDrawType(newMesh, DrawType::Line);

	UnmapMeshElements(mesh);
	UnmapMeshVertices(mesh);

	return newMesh;
}

void Renderer::SwapBuffer()
{
	DrawDebugLines();
	SwapBuffers(deviceContext);
}

bool Renderer::CheckShaderCompilation(unsigned int shader)
{

	GLint compilationSuccess = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compilationSuccess);

	if (compilationSuccess == GL_FALSE) {
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<char> errorLog(logLength);
		glGetShaderInfoLog(shader, logLength, &logLength, &(errorLog[0]));

		glDeleteShader(shader);
		std::string log = &(errorLog[0]);

		Engine::DEBUGPrint(log.c_str());

	}
	return compilationSuccess;
}

void Renderer::UpdateCamTransform()
{
	Mat4x4f viewMatrix = Math::GenerateViewMatrix(defaultCamera.position, defaultCamera.direction, defaultCamera.up);

	Mat4x4f projectionMatrix = Math::GenerateProjectionMatrix(glm::radians(70.0f), (float)screenSize.x / (float)screenSize.y, 0.01f, 400.0f);

	Mat4x4f camMatrix = projectionMatrix * viewMatrix;

	glUniformMatrix4fv(defaultShader.CameraLocation, 1, GL_FALSE, &camMatrix[0][0]);


	defaultCamera.camMatrixNeedsUpdate = false;
}
