#include "GraphicsModule.h"

#include "..\FileLoader.h"

#include <random>
#include "Scene.h"

void Transform::SetPosition(Vec3f newPos)
{
    m_Position = newPos;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::SetScale(Vec3f newScale)
{
    m_Scale = newScale;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::SetRotation(Quaternion newRotation)
{
    m_Rotation = newRotation;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::Move(Vec3f move)
{
    m_Position += move;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::Scale(Vec3f scale)
{
    m_Scale.x *= scale.x;
    m_Scale.y *= scale.y;
    m_Scale.z *= scale.z;

    m_TransformMatrixNeedsUpdate = true;
}

void Transform::Rotate(Quaternion rotation)
{
    m_Rotation = m_Rotation * rotation;

    m_TransformMatrixNeedsUpdate = true;
}

Mat4x4f Transform::GetTransformMatrix()
{
    if (m_TransformMatrixNeedsUpdate)
    {
        UpdateTransformMatrix();
        m_TransformMatrixNeedsUpdate = false;
    }
    return m_Transform;
}

void Transform::SetTransformMatrix(Mat4x4f mat)
{
    m_Transform = mat;
}

bool Transform::WasTransformMatrixUpdated()
{
    if (m_WasTransformMatrixUpdated)
    {
        m_WasTransformMatrixUpdated = false;
        return true;
    }
    return false;
}

void Transform::UpdateTransformMatrix()
{
    m_Transform = Math::GenerateTransformMatrix(m_Position, m_Scale, m_Rotation);
    
    m_WasTransformMatrixUpdated = true;
}

GraphicsModule::GraphicsModule(Renderer& renderer)
    : m_Renderer(renderer)
    , m_Camera(nullptr)
    , m_CameraMatrixSetThisFrame(false)
    , m_IsSkyboxSet(false)
    , m_IsDebugDrawInitialized(false)
    , m_IsDebugDrawAttachedToFBuffer(false)
    , m_TexturedMeshFormat({ VertAttribute::Vec3f, VertAttribute::Vec3f, VertAttribute::Vec4f, VertAttribute::Vec2f })
    , m_DebugVertFormat({ VertAttribute::Vec3f })
    , m_RenderMode(RenderMode::DEFAULT)
{
    // ~~~~~~~~~~~~~~~~~~~~~Unlit shader code~~~~~~~~~~~~~~~~~~~~~ //

    std::string unlitVertShader = R"(
    #version 400

	uniform mat4x4 Transformation;
	uniform mat4x4 Camera;

    in vec4 VertPosition;
	in vec4 VertNormal;
	in vec4 VertColour;
	in vec2 VertUV;

    smooth out vec4 FragNormal;
    smooth out vec4 FragColour;
    smooth out vec2 FragUV;

    void main()
    {
        gl_Position = (Camera * Transformation) * VertPosition;
        
        FragNormal = VertNormal;
        FragColour = VertColour;
		FragUV = VertUV;
    }   
    )";

    std::string unlitFragShader = R"(
    #version 400

	smooth in vec4 FragNormal;
	smooth in vec4 FragColour;
	smooth in vec2 FragUV;   

	uniform sampler2D DiffuseTexture;

    out vec4 OutColour;

    void main()
    {
        vec4 textureAt = texture(DiffuseTexture, FragUV);
        //OutColour = textureAt;
        OutColour.rgb = textureAt.rgb * FragColour.rgb;
        OutColour.a = textureAt.a * FragColour.a;
    }
    )";

    m_UnlitShader = m_Renderer.LoadShader(unlitVertShader, unlitFragShader);

    // ~~~~~~~~~~~~~~~~~~~~~Basic textured mesh shader code~~~~~~~~~~~~~~~~~~~~~ //
    std::string vertShaderSource = R"(
	#version 400	
	
	uniform mat4x4 Transformation;
	uniform mat4x4 Camera;
    uniform mat4x4 LightSpaceMatrix;

	in vec4 VertPosition;
	in vec4 VertNormal;
	in vec4 VertColour;
	in vec2 VertUV;
	
    smooth out vec3 FragPosition;
	smooth out vec3 FragNormal;
	smooth out vec4 FragColour;	
	smooth out vec2 FragUV;
    out vec4 FragPosLightSpace;

	void main()
	{
		gl_Position = (Camera * Transformation) * VertPosition;
		
		FragPosition = vec3(Transformation * VertPosition);
        //TEMP(fraser): costly inverses
		FragNormal = mat3(transpose(inverse(Transformation))) * VertNormal.xyz;
		FragColour = VertColour;
		FragUV = VertUV;
        FragPosLightSpace = LightSpaceMatrix * vec4(FragPosition, 1.0);
	}

	)";

    std::string fragShaderSource = R"(
	
	#version 400

    smooth in vec3 FragPosition;
	smooth in vec3 FragNormal;
	smooth in vec4 FragColour;
	smooth in vec2 FragUV;	
    in vec4 FragPosLightSpace;

	uniform sampler2D DiffuseTexture;
    uniform samplerCube Skybox;	
    uniform sampler2D ShadowMap;

	uniform float Time;
	uniform vec3 SunDirection;
    uniform vec3 SunColour;

    uniform vec3 CameraPos;    

	out vec4 OutColour;

    float rand(vec2 co)
    {
        return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    }

    float ShadowCalculation(vec4 fragPosLightSpace)
    {
        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(ShadowMap, projCoords.xy).r; 
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // check whether current frag pos is in shadow  
        vec3 sun = vec3(-SunDirection.x, -SunDirection.y, -SunDirection.z);      
        //float bias = max(0.05 * (1.0 - dot(FragNormal, sun)), 0.005);  
        float bias = max(0.0009 * (1.0 - dot(FragNormal.xyz, sun)), 0.0002);         

    
        float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
        if (projCoords.z > 1.0)
        {
            shadow = 0.0;
        }

        return shadow;
    }

	void main()
	{
		vec3 sun = vec3(-SunDirection.x, -SunDirection.y, -SunDirection.z);

		vec3 normalizedNormal = normalize(FragNormal);
		vec4 textureAt = texture(DiffuseTexture, FragUV);
		float diffuse = ((dot(normalizedNormal.xyz, normalize(sun))) + 1.0) / 2.0;
        // Temp: reduces shadow acne
        if (diffuse < 0.53)
        {
            diffuse = 0.0;
        }
        float ambient = 0.25;        

        float shadow = ShadowCalculation(FragPosLightSpace);

    	OutColour.rgb = SunColour * (ambient + (1.0 - shadow) * diffuse) * (textureAt.rgb * FragColour.rgb);
        //OutColour = textureAt;

        float ratio = 1.00 / 1.52;
        vec3 I = normalize(FragPosition - CameraPos);
        vec3 R = reflect(I, normalizedNormal);
        vec4 reflectColour = vec4(texture(Skybox, R).rgb, 1.0);
		
        R = refract(I, normalizedNormal, ratio);
        vec4 refractColour = vec4(texture(Skybox, R).rgb, 1.0);

        //OutColour = mix(reflectColour, OutColour, 0.8);
        //OutColour = mix(OutColour, refractColour, 1.0);
		//OutColour = reflectColour * OutColour;
        //OutColour = reflectColour;
        OutColour.a = textureAt.a * FragColour.a;
	
        //OutColour = vec4(vec3(gl_FragCoord.z / (1.0 / gl_FragCoord.w)), 1.0);
    }
	)";

    m_TexturedMeshShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~Shadow shader code~~~~~~~~~~~~~~~~~~~~~ //

    std::string shadowVertShader = R"(
    #version 400
    layout (location = 0) in vec3 aPos;

    uniform mat4 LightSpaceMatrix;
    uniform mat4 Transformation;

    void main()
    {
        gl_Position = LightSpaceMatrix * Transformation * vec4(aPos, 1.0);
    }   
    )";

    std::string shadowFragShader = R"(
    #version 400
    
    void main()
    {
    }
    )";

    m_ShadowShader = m_Renderer.LoadShader(shadowVertShader, shadowFragShader);

    // ~~~~~~~~~~~~~~~~~~~~~Skybox shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
    #version 400

    in vec3 aPos;

    uniform mat4 projection;
    uniform mat4 view;

    out vec3 TexCoords;
    
    void main()
    {
        TexCoords = vec3(aPos.x, aPos.y, aPos.z);
        gl_Position = projection * view * vec4(aPos, 1.0);
    }
    )";

    fragShaderSource = R"(
    #version 400

    in vec3 TexCoords;
    
    uniform samplerCube skybox;

    out vec4 OutColour;

    void main()
    {
        OutColour = texture(skybox, TexCoords);
    }
    )";

    m_SkyboxShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~Debug line shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
	#version 400

	uniform mat4x4 Camera;	
    uniform vec3 Colour;

	in vec3 VertPosition;

	smooth out vec4 FragColour;

	void main()
	{
		gl_Position = Camera * vec4(VertPosition, 1.0);
		FragColour = vec4(Colour, 1.0);
	}
	)";

    fragShaderSource = R"(
	#version 400

	in vec4 FragColour;
	
	out vec4 OutColour;

	void main()
	{
		OutColour = FragColour;
	}
	)";

    m_DebugLineShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GUI shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
    #version 400
    
    uniform mat4x4 Projection;

    in vec3 VertPosition;
    in vec2 VertUV;

    smooth out vec2 FragUV;

    void main()
    {
        gl_Position = Projection * vec4(VertPosition, 1.0);
        FragUV = VertUV;    
    }
    )";

    fragShaderSource = R"(
    #version 400

    uniform sampler2D Texture;
    
	uniform vec2 WindowSize;
    
    smooth in vec2 FragUV;	

    out vec4 OutColour;

    void main()
    {
        vec4 textureAt = texture(Texture, FragUV);
        OutColour = textureAt;
    	
        // TEMP (fraser): Super basic crosshair 
  //      if (gl_FragCoord.x > (WindowSize.x / 2.0) - 4.0 && gl_FragCoord.x < (WindowSize.x / 2.0) + 4.0 && 
		//	gl_FragCoord.y > (WindowSize.y / 2.0) - 4.0 && gl_FragCoord.y < (WindowSize.y / 2.0) + 4.0)
		//{
		//	OutColour.r = OutColour.r > 0.5 ? 0.0 : 1.0;
		//	OutColour.g = OutColour.g > 0.5 ? 0.0 : 1.0;
		//	OutColour.b = OutColour.b > 0.5 ? 0.0 : 1.0;
		//} 
    }
    
    )";

    m_UIShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);
    
    Vec2i viewportSize = GetViewportSize();
    float sizeX = (float)viewportSize.x;
    float sizeY = (float)viewportSize.y;

    m_OrthoProjection = Math::GenerateOrthoMatrix(0.0f, sizeX, 0.0f, sizeY, 0.1f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_UIShader, "Projection", m_OrthoProjection);

    m_DebugTexture = m_Renderer.LoadTexture("textures/debugTexture.png", TextureMode::LINEAR, TextureMode::NEAREST);

    //TEMP
    //temp temp temp
    m_Renderer.SetShaderUniformVec3f(m_TexturedMeshShader, "SunDirection", Vec3f(0.0f, 0.0f, -1.0f));
    m_Renderer.SetShaderUniformVec3f(m_TexturedMeshShader, "SunColour", Vec3f(0.95f, 0.9f, 0.85f));

    m_Renderer.SetShaderUniformVec2f(m_UIShader, "WindowSize", (Vec2f)Engine::GetClientAreaSize());

    std::vector<float> skyboxVertices = {
        // positions          
        -1.0f,  -1.0f,     1.0f,
        -1.0f,  -1.0f,    -1.0f,
         1.0f,  -1.0f,    -1.0f,
         1.0f,  -1.0f,    -1.0f,
         1.0f,  -1.0f,     1.0f,
        -1.0f,  -1.0f,     1.0f,

        -1.0f,   1.0f,    -1.0f,
        -1.0f,  -1.0f,    -1.0f,
        -1.0f,  -1.0f,     1.0f,
        -1.0f,  -1.0f,     1.0f,
        -1.0f,   1.0f,     1.0f,
        -1.0f,   1.0f,    -1.0f,

         1.0f,  -1.0f,    -1.0f,
         1.0f,   1.0f,    -1.0f,
         1.0f,   1.0f,     1.0f,
         1.0f,   1.0f,     1.0f,
         1.0f,  -1.0f,     1.0f,
         1.0f,  -1.0f,    -1.0f,

        -1.0f,   1.0f,    -1.0f,
        -1.0f,   1.0f,     1.0f,
         1.0f,   1.0f,     1.0f,
         1.0f,   1.0f,     1.0f,
         1.0f,   1.0f,    -1.0f,
        -1.0f,   1.0f,    -1.0f,

        -1.0f,  -1.0f,     1.0f,
         1.0f,  -1.0f,     1.0f,
         1.0f,   1.0f,     1.0f,
         1.0f,   1.0f,     1.0f,
        -1.0f,   1.0f,     1.0f,
        -1.0f,  -1.0f,     1.0f,

        -1.0f,  -1.0f,    -1.0f,
        -1.0f,   1.0f,    -1.0f,
         1.0f,  -1.0f,    -1.0f,
         1.0f,  -1.0f,    -1.0f,
        -1.0f,   1.0f,    -1.0f,
         1.0f,   1.0f,  - 1.0f,
    };

    m_SkyboxMesh = m_Renderer.LoadMesh(VertexBufferFormat({ VertAttribute::Vec3f }), skyboxVertices);
    m_SkyboxCubemap = m_Renderer.LoadCubemap("asda");
    m_IsSkyboxSet = true;
}

GraphicsModule::~GraphicsModule()
{
}

Framebuffer_ID GraphicsModule::CreateFBuffer(Vec2i size, FBufferFormat format)
{
    return m_Renderer.CreateFrameBuffer(size, format);
}

Texture_ID GraphicsModule::CreateTexture(Vec2i size)
{
    return m_Renderer.CreateEmptyTexture(size);
}

Texture_ID GraphicsModule::LoadTexture(std::string filePath, TextureMode minFilter, TextureMode magFilter)
{
    return m_Renderer.LoadTexture(filePath, minFilter, magFilter);
}

Mesh_ID GraphicsModule::LoadMesh(std::string filePath)
{
    // todo(Fraser): Switch here on different file types? (If I ever want something that's not .obj)
    return FileLoader::LoadOBJFile(filePath, m_Renderer);
}

Mesh_ID GraphicsModule::CreatePlane(float width)
{



    return Mesh_ID();
}

void GraphicsModule::AttachTextureToFBuffer(Texture_ID textureID, Framebuffer_ID fBufferID)
{ 
    m_Renderer.AttachTextureToFramebuffer(textureID, fBufferID);
}

void GraphicsModule::SetActiveFrameBuffer(Framebuffer_ID fBufferID)
{
    m_Renderer.SetActiveFBuffer(fBufferID);
    m_Renderer.ClearScreenAndDepthBuffer();
    m_ActiveFrameBuffer = fBufferID;

    if (m_IsSkyboxSet && fBufferID == m_DebugFBuffer)
    {
        // TEMP: skybox code blech
        m_Renderer.DisableDepthTesting();

        m_Renderer.SetActiveShader(m_TexturedMeshShader);
        m_Renderer.SetActiveCubemap(m_SkyboxCubemap, 1);

        m_Renderer.SetActiveShader(m_SkyboxShader);

        m_Renderer.SetShaderUniformMat4x4f(m_SkyboxShader, "projection", m_Camera->GetProjectionMatrix());

        Mat4x4f newView = m_Camera->GetViewMatrix();

        newView[3][0] = 0.0f;
        newView[3][1] = 0.0f;
        newView[3][2] = 0.0f;
        newView[3][3] = 1.0f;

        newView[0][3] = 0.0f;
        newView[1][3] = 0.0f;
        newView[2][3] = 0.0f;

        m_Renderer.SetShaderUniformMat4x4f(m_SkyboxShader, "view", newView);

        m_Renderer.SetActiveCubemap(m_SkyboxCubemap);
        m_Renderer.DrawMesh(m_SkyboxMesh);

        m_Renderer.EnableDepthTesting();
    }
}

void GraphicsModule::ResizeFrameBuffer(Framebuffer_ID fBufferID, Vec2i size)
{
    m_Renderer.ResizeFBuffer(fBufferID, size);
}

void GraphicsModule::ResetFrameBuffer()
{
    if (m_IsDebugDrawAttachedToFBuffer && m_ActiveFrameBuffer == m_DebugFBuffer)
    {
        DrawDebugDrawMesh();
    }
    m_Renderer.ResetToScreenBuffer();
}

Model GraphicsModule::CreateModel(TexturedMesh texturedMesh)
{
    return Model(texturedMesh);
}

Model GraphicsModule::CloneModel(const Model& original)
{
    return Model(original.m_TexturedMeshes[0]);
}

Model GraphicsModule::CreateBoxModel(AABB box, Texture_ID texture)
{
    if (texture == 0)
    {
        texture = this->m_DebugTexture;
    }

    Mesh_ID boxMesh = CreateBoxMesh(box);

    return Model(TexturedMesh(boxMesh, m_DebugTexture));
}

Brush GraphicsModule::CreateBrush(AABB box, Texture_ID texture)
{
    Mesh_ID boxMesh = CreateBoxMesh(box);

    Brush boxBrush(boxMesh);
    boxBrush.SetTexture(texture);

    return boxBrush;
}

void GraphicsModule::Draw(Model& model)
{
    if (m_RenderMode == RenderMode::DEFAULT)
    {
        if (!m_CameraMatrixSetThisFrame && m_Camera != nullptr)
        {
            m_Renderer.SetShaderUniformMat4x4f(m_TexturedMeshShader, "Camera", m_Camera->GetCamMatrix());
            m_Renderer.SetShaderUniformVec3f(m_TexturedMeshShader, "CameraPos", m_Camera->GetPosition());
            m_CameraMatrixSetThisFrame = true;
        }

        m_Renderer.SetActiveShader(m_TexturedMeshShader);
        m_Renderer.SetShaderUniformMat4x4f(m_TexturedMeshShader, "Transformation", model.GetTransform().GetTransformMatrix());

        for (int i = 0; i < model.m_TexturedMeshes.size(); ++i)
        {
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Texture, "DiffuseTexture");
            m_Renderer.DrawMesh(model.m_TexturedMeshes[i].m_Mesh);
        }

    }
    else if (m_RenderMode == RenderMode::FULLBRIGHT)
    {
        // Temp(fraser): Matrix set this frame PER render mode
        //if (!m_CameraMatrixSetThisFrame && m_Camera != nullptr)
        //{
            m_Renderer.SetShaderUniformMat4x4f(m_UnlitShader, "Camera", m_Camera->GetCamMatrix());
            m_CameraMatrixSetThisFrame = true;
        //}

        m_Renderer.SetActiveShader(m_UnlitShader);
        m_Renderer.SetShaderUniformMat4x4f(m_UnlitShader, "Transformation", model.GetTransform().GetTransformMatrix());

        for (int i = 0; i < model.m_TexturedMeshes.size(); ++i)
        {
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Texture, "DiffuseTexture");
            m_Renderer.DrawMesh(model.m_TexturedMeshes[i].m_Mesh);
        }
    }
}

void GraphicsModule::SetCamera(Camera* camera)
{
    assert(camera);
    m_Camera = camera;
    m_CameraMatrixSetThisFrame = false;
}

void GraphicsModule::SetDirectionalLight(DirectionalLight dirLight)
{
    m_Renderer.SetShaderUniformVec3f(m_TexturedMeshShader, "SunDirection", dirLight.direction);
    m_Renderer.SetShaderUniformVec3f(m_TexturedMeshShader, "SunColour", dirLight.colour);
}

void GraphicsModule::OnFrameStart()
{
    m_CameraMatrixSetThisFrame = false;
    m_Renderer.ClearScreenAndDepthBuffer();

    if (m_IsDebugDrawInitialized)
    {
        m_DebugLineMap.clear();
    }
}

void GraphicsModule::OnFrameEnd()
{
    if (m_IsDebugDrawInitialized && !m_IsDebugDrawAttachedToFBuffer)
    {
        DrawDebugDrawMesh();
    }

    m_Renderer.SwapBuffer();
}

void GraphicsModule::InitializeDebugDraw()
{
    m_DebugDrawMesh = m_Renderer.CreateEmptyMesh(m_DebugVertFormat, false);
    m_Renderer.SetMeshDrawType(m_DebugDrawMesh, DrawType::Line);
    m_IsDebugDrawInitialized = true;
}

void GraphicsModule::InitializeDebugDraw(Framebuffer_ID fBuffer)
{
    InitializeDebugDraw();
    m_IsDebugDrawAttachedToFBuffer = true;
    m_DebugFBuffer = fBuffer;
}

void GraphicsModule::DebugDrawLine(Vec3f a, Vec3f b, Vec3f colour)
{
    assert(m_IsDebugDrawInitialized);

    if (m_DebugLineMap.find(colour) == m_DebugLineMap.end())
    {
        m_DebugLineMap[colour] = std::vector<float>();
    }

    m_DebugLineMap[colour].insert(m_DebugLineMap[colour].end(), { a.x, a.y, a.z, b.x, b.y, b.z });
}

void GraphicsModule::DebugDrawLine(LineSegment line, Vec3f colour)
{
    DebugDrawLine(line.a, line.b, colour);
}

void GraphicsModule::DebugDrawModelMesh(Model model, Vec3f colour)
{
    assert(m_IsDebugDrawInitialized);

    Mat4x4f modelTransform = model.GetTransform().GetTransformMatrix();

    for (int i = 0; i < model.m_TexturedMeshes.size(); ++i)
    {
        std::vector<Vertex*> vertices = m_Renderer.MapMeshVertices(model.m_TexturedMeshes[i].m_Mesh);
        std::vector<unsigned int*> indices = m_Renderer.MapMeshElements(model.m_TexturedMeshes[i].m_Mesh);
        for (int j = 0; j < indices.size(); j += 3)
        {
            //TODO(fraser): So much duplicated data, if this is actually going to be done in this way I'll want to use an index buffer
            //m_DebugLineVec.push_back(PosVertex(vertices[*indices[j]]->position * modelTransform));
            //m_DebugLineVec.push_back(PosVertex(vertices[*indices[(size_t)j + 1]]->position * modelTransform));

            //m_DebugLineVec.push_back(PosVertex(vertices[*indices[(size_t)j + 1]]->position * modelTransform));
            //m_DebugLineVec.push_back(PosVertex(vertices[*indices[(size_t)j + 2]]->position * modelTransform));

            //m_DebugLineVec.push_back(PosVertex(vertices[*indices[(size_t)j + 2]]->position * modelTransform));
            //m_DebugLineVec.push_back(PosVertex(vertices[*indices[j]]->position * modelTransform));
        }
        m_Renderer.UnmapMeshVertices(model.m_TexturedMeshes[i].m_Mesh);
        m_Renderer.UnmapMeshElements(model.m_TexturedMeshes[i].m_Mesh);
    }
}

void GraphicsModule::DebugDrawAABB(AABB box, Vec3f colour, Mat4x4f transform)
{
    assert(m_IsDebugDrawInitialized);
    Vec3f points[] =
    {
        box.min,
        box.min + Vec3f(0.0f, box.max.y - box.min.y, 0.0f),
        box.min + Vec3f(box.max.x - box.min.x, box.max.y - box.min.y, 0.0f),
        box.min + Vec3f(box.max.x - box.min.x, 0.0f, 0.0f),

        box.max - Vec3f(box.max.x - box.min.x, box.max.y - box.min.y, 0.0f),
        box.max - Vec3f(box.max.x - box.min.x, 0.0f, 0.0f),
        box.max,
        box.max - Vec3f(0.0f, box.max.y - box.min.y, 0.0f)
    };

    for (int i = 0; i < 8; ++i)
    {
        points[i] = points[i] * transform;
    }

    DebugDrawLine(points[0], points[1], colour);
    DebugDrawLine(points[1], points[2], colour);
    DebugDrawLine(points[2], points[3], colour);
    DebugDrawLine(points[3], points[0], colour);

    DebugDrawLine(points[4], points[5], colour);
    DebugDrawLine(points[5], points[6], colour);
    DebugDrawLine(points[6], points[7], colour);
    DebugDrawLine(points[7], points[4], colour);

    DebugDrawLine(points[0], points[4], colour);
    DebugDrawLine(points[1], points[5], colour);
    DebugDrawLine(points[2], points[6], colour);
    DebugDrawLine(points[3], points[7], colour);
}

void GraphicsModule::DebugDrawPoint(Vec3f p, Vec3f colour)
{
    assert(m_IsDebugDrawInitialized);

    if (m_DebugLineMap.find(colour) == m_DebugLineMap.end())
    {
        m_DebugLineMap[colour] = std::vector<float>();
    }

    m_DebugLineMap[colour].insert(m_DebugLineMap[colour].end(), { 
        p.x, p.y - 0.5f, p.z, p.x, p.y + 0.5f, p.z,
        p.x - 0.5f, p.y, p.z, p.x + 0.5f, p.y, p.z,
        p.x, p.y, p.z - 0.5f, p.x, p.y, p.z + 0.5f,
        
        });
}

Vec2i GraphicsModule::GetViewportSize()
{
    return m_Renderer.GetViewportSize();
}

void GraphicsModule::SetRenderMode(RenderMode mode)
{
    m_RenderMode = mode;
}

Mesh_ID GraphicsModule::CreateBoxMesh(AABB box)
{
    Vec3f min = box.min;
    Vec3f max = box.max;

    Vec3f size = (box.max - box.min) / 5.0f;

    Vec3f points[] =
    {
        box.min,
        box.min + Vec3f(0.0f, box.max.y - box.min.y, 0.0f),
        box.min + Vec3f(box.max.x - box.min.x, box.max.y - box.min.y, 0.0f),
        box.min + Vec3f(box.max.x - box.min.x, 0.0f, 0.0f),

        box.max - Vec3f(box.max.x - box.min.x, box.max.y - box.min.y, 0.0f),
        box.max - Vec3f(box.max.x - box.min.x, 0.0f, 0.0f),
        box.max,
        box.max - Vec3f(0.0f, box.max.y - box.min.y, 0.0f)
    };

    std::vector<float> vertices =
    {
        // Positions                                // Normals              // Colours                  // Texcoords
        points[0].x, points[0].y, points[0].z,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        points[1].x, points[1].y, points[1].z,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f, size.y,
        points[2].x, points[2].y, points[2].z,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.x, size.y,
        points[3].x, points[3].y, points[3].z,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.x, 0.0f,

        points[4].x, points[4].y, points[4].z,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        points[5].x, points[5].y, points[5].z,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f, size.y,
        points[6].x, points[6].y, points[6].z,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.x, size.y,
        points[7].x, points[7].y, points[7].z,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.x, 0.0f,

        points[0].x, points[0].y, points[0].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        points[1].x, points[1].y, points[1].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f, size.y,
        points[5].x, points[5].y, points[5].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.z, size.y,
        points[4].x, points[4].y, points[4].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.z, 0.0f,

        points[2].x, points[2].y, points[2].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        points[3].x, points[3].y, points[3].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f, size.y,
        points[7].x, points[7].y, points[7].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.z, size.y,
        points[6].x, points[6].y, points[6].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.z, 0.0f,

        points[0].x, points[0].y, points[0].z,      0.0f, -1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        points[4].x, points[4].y, points[4].z,      0.0f, -1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f, size.z,
        points[7].x, points[7].y, points[7].z,      0.0f, -1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.x, size.z,
        points[3].x, points[3].y, points[3].z,      0.0f, -1.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.x, 0.0f,

        points[1].x, points[1].y, points[1].z,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        points[5].x, points[5].y, points[5].z,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f, size.z,
        points[6].x, points[6].y, points[6].z,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.x, size.z,
        points[2].x, points[2].y, points[2].z,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.x, 0.0f,
    };

    std::vector<ElementIndex> indices =
    {
        0, 2, 1, 0, 3, 2,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 22, 21, 20, 23, 22
    };

    Mesh_ID boxMesh = m_Renderer.LoadMesh(m_TexturedMeshFormat, vertices, indices);

    return boxMesh;
}

void GraphicsModule::DrawDebugDrawMesh()
{
    m_Renderer.ClearMesh(m_DebugDrawMesh);
    if (m_DebugLineMap.size() > 0)
    {
        m_Renderer.SetShaderUniformMat4x4f(m_DebugLineShader, "Camera", m_Camera->GetCamMatrix());
        for (auto& it : m_DebugLineMap)
        {
            m_Renderer.UpdateMeshData(m_DebugDrawMesh, m_DebugVertFormat, it.second);
            m_Renderer.SetShaderUniformVec3f(m_DebugLineShader, "Colour", it.first);
            m_Renderer.DrawMesh(m_DebugDrawMesh);
        }
    }
}

void GraphicsModule::Resize(Vec2i newSize)
{
    m_Renderer.ResetToScreenBuffer();
}
