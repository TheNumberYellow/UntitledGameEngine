#include "GameEngine.hpp"

#include "EditorScene.h"

#include <cmath>
#include <sstream>

Texture_ID cursorToolTexture;
Texture_ID boxToolTexture;
Texture_ID vertexToolTexture;

Font testFont;

Camera cam;

Model previewPSpawn;

Framebuffer_ID viewportBuffer;

Framebuffer_ID shadowBuffer;

Mesh_ID quadMesh;

Camera shadowCam;

Shader_ID shadowShader;
Shader_ID testDepthShader;

bool holdingAlt = false;
bool cursorLocked = false;

//Rect viewportRect;

bool draggingNewModel = false;

bool draggingNewBox = false;
Vec3f boxStartPoint;
float boxHeight = 5.0f;
AABB aabbBox;
std::vector<CollisionMesh> boxColMeshes;

enum class ToolMode
{
    NORMAL,
    BOX,
    VERTEX
};

std::vector<Model> boxes;

ToolMode toolMode = ToolMode::NORMAL;

Rect GetViewportSizeFromScreenSize(Vec2i screenSize)
{
    Rect newViewport = Rect(Vec2f(100.0f, 200.0f), screenSize - Vec2f(100.0f, 0.0f));

    return newViewport;
}

void DrawEditorGrid(GraphicsModule& graphics)
{
    static float EDITOR_GRID_SIZE = 500.0f;
    static int EDITOR_GRID_NUM = 100;

    static Vec3f gridColour = Vec3f(0.4f, 0.4f, 0.4f);
    static Vec3f gridSecondaryColour = Vec3f(167.0f / 255.0f, 120.0f / 255.0f, 38.0f / 255.0f);

    for (int i = 0; i < EDITOR_GRID_NUM + 1; ++i)
    {
        Vec3f thisGridColour = i % 5 == 0 ? gridSecondaryColour : gridColour;
        graphics.DebugDrawLine(
            Vec3f(EDITOR_GRID_SIZE / 2.0f, (i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, 0.0f),
            Vec3f(-EDITOR_GRID_SIZE / 2.0f, (i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, 0.0f),
            thisGridColour
        );

        graphics.DebugDrawLine(
            Vec3f((i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, EDITOR_GRID_SIZE / 2.0f, 0.0f),
            Vec3f((i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, -EDITOR_GRID_SIZE / 2.0f, 0.0f),
            thisGridColour
        );
    }
}

Ray GetMouseRay(Camera& cam, Vec2i mousePosition, Rect viewPort)
{
    Mat4x4f invCamMatrix = cam.GetInvCamMatrix();

    Vec3f a, b;

    Vec2i screenSize = viewPort.size;

    Vec2i mousePos;
    mousePos.x = mousePosition.x - viewPort.location.x;
    mousePos.y = mousePosition.y;

    float x = ((2.0f * (float)mousePos.x) / (float)screenSize.x) - 1.0f;
    float y = 1.0f - ((2.0f * (float)mousePos.y) / (float)screenSize.y);
    float z = 1.0f;

    Vec3f ray_nds = Vec3f(x, y, z);

    Vec4f ray_clip = Vec4f(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

    Vec4f ray_eye = ray_clip * Math::inv(cam.GetProjectionMatrix());

    ray_eye = Vec4f(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    Vec4f ray_world = ray_eye * Math::inv(cam.GetViewMatrix());

    Vec3f ray = Vec3f(ray_world.x, ray_world.y, ray_world.z);
    ray = Math::normalize(ray);

    return Ray(cam.GetPosition(), ray);
}

void MoveCamera(ControlInputs& inputs, GraphicsModule& graphics, Camera& cam, float pixelToRadians)
{
    float speed = 0.075f;

    if (inputs.keysDown.shift)
    {
        speed *= 5.0f;
    }

    if (inputs.keysDown.w)
    {
        cam.Move(cam.GetDirection() * speed);
    }
    if (inputs.keysDown.s)
    {
        cam.Move(-cam.GetDirection() * speed);
    }
    if (inputs.keysDown.d)
    {
        cam.Move(Math::normalize(cam.GetPerpVector()) * speed);
    }
    if (inputs.keysDown.a)
    {
        cam.Move(-Math::normalize(cam.GetPerpVector()) * speed);
    }

    if (inputs.keysDown.space)
    {
        cam.Move(Vec3f(0.0f, 0.0f, speed));
    }
    if (inputs.keysDown.ctrl)
    {
        cam.Move(Vec3f(0.0f, 0.0f, -speed));
    }

    cam.RotateCamBasedOnDeltaMouse(inputs.DeltaMouse, pixelToRadians);

    if (inputs.keysDown.q)
    {
        //shadowCam.SetCamMatrix(cam.GetCamMatrix());
        shadowCam.SetPosition(cam.GetPosition());
        shadowCam.SetDirection(cam.GetDirection());
        graphics.m_Renderer.SetShaderUniformVec3f(graphics.m_TexturedMeshShader, "SunDirection", cam.GetDirection());
        //shadowCam.RotateCamBasedOnDeltaMouse(inputs.DeltaMouse, pixelToRadians);
    }
}

void UpdateEditor(ModuleManager& modules, ControlInputs& inputs)
{
    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();
    UIModule& ui = *modules.GetUI();

    if (inputs.keysDown.esc)
    {
        Engine::StopGame();
        return;
    }

    if (cursorLocked)
    {
        MoveCamera(inputs, graphics, cam, 0.001f);
    }

    Vec2i mousePos = Engine::GetMousePosition();
    mousePos.y = Engine::GetClientAreaSize().y - mousePos.y;

    Rect newViewport = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    if (inputs.mouse.leftMouseButton && newViewport.contains(mousePos))
    {
        if (!draggingNewModel)
        {
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), newViewport);

            if (toolMode == ToolMode::NORMAL)
            {
                RayCastHit rayHit;
                for (int i = 0; i < boxColMeshes.size(); ++i)
                {
                    RayCastHit placedHit = collisions.RayCast(mouseRay, boxColMeshes[i]);

                    if (placedHit.hitDistance < rayHit.hitDistance)
                    {
                        rayHit = placedHit;
                    }
                }

                if (rayHit.hit)
                {
                    graphics.DebugDrawLine(rayHit.hitPoint, rayHit.hitPoint + rayHit.hitNormal, Vec3f(1.0f, 1.0f, 0.0f));
                }
                else {
                    graphics.DebugDrawLine(mouseRay.point + mouseRay.direction, mouseRay.point + mouseRay.direction + Vec3f(0.0f, 0.0f, 0.1f), Vec3f(1.0f, 1.0f, 0.0f));
                }
            }

            if (toolMode == ToolMode::BOX)
            {
                if (!draggingNewBox)
                {
                    // Find intersect with horizontal plane
                    RayCastHit hit = collisions.RayCast(mouseRay, Plane{ Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f) });

                    // Test against existing brushes
                    for (int i = 0; i < boxColMeshes.size(); ++i)
                    {
                        RayCastHit placedHit = collisions.RayCast(mouseRay, boxColMeshes[i]);

                        if (placedHit.hitDistance < hit.hitDistance)
                        {
                            hit = placedHit;
                        }
                    }

                    if (hit.hit)
                    {
                        hit.hitPoint.x = Math::Round(hit.hitPoint.x, 2.5f);
                        hit.hitPoint.y = Math::Round(hit.hitPoint.y, 2.5f);
                        hit.hitPoint.z = Math::Round(hit.hitPoint.z, 2.5f);


                        boxStartPoint = hit.hitPoint;
                        draggingNewBox = true;
                    }
                }
                else
                {
                    if (inputs.keysDown.three)
                    {
                        boxHeight -= 0.025f;
                    }
                    if (inputs.keysDown.four)
                    {
                        boxHeight += 0.025f;
                    }
                    RayCastHit hit = collisions.RayCast(mouseRay, Plane{ boxStartPoint, Vec3f(0.0f, 0.0f, 1.0f) });

                    hit.hitPoint.x = Math::Round(hit.hitPoint.x, 2.5);
                    hit.hitPoint.y = Math::Round(hit.hitPoint.y, 2.5f);
                    hit.hitPoint.z = Math::Round(hit.hitPoint.z, 2.5f);

                    float minX = std::min(hit.hitPoint.x, boxStartPoint.x);
                    float minY = std::min(hit.hitPoint.y, boxStartPoint.y);

                    float maxX = std::max(hit.hitPoint.x, boxStartPoint.x);
                    float maxY = std::max(hit.hitPoint.y, boxStartPoint.y);

                    aabbBox.min = Vec3f(minX, minY, hit.hitPoint.z);
                    aabbBox.max = Vec3f(maxX, maxY, hit.hitPoint.z + boxHeight);

                    graphics.DebugDrawAABB(aabbBox, Vec3f(0.1f, 1.0f, 0.3f));
                }

            }

        }
    }
    else
    {
        if (draggingNewBox)
        {
            Model newBox = graphics.CreateBoxModel(aabbBox);
            boxColMeshes.push_back(collisions.GenerateCollisionMeshFromMesh(newBox.m_TexturedMeshes[0].m_Mesh));
            boxes.push_back(newBox);
            boxHeight = 5.0f;
            draggingNewBox = false;

        }
    }

    if (inputs.keysDown.alt)
    {
        if (!holdingAlt)
        {
            if (cursorLocked)
            {
                Engine::UnlockCursor();
                Engine::ShowCursor();
                cursorLocked = false;
            }
            else {
                Engine::LockCursor();
                Engine::HideCursor();
                cursorLocked = true;
            }
            holdingAlt = true;
        }
    }
    else {
        holdingAlt = false;
    }

    // BEGIN DRAW

    DrawEditorGrid(graphics);

    graphics.SetActiveFrameBuffer(shadowBuffer);
    graphics.m_Renderer.SetActiveShader(shadowShader);
    {
        graphics.m_Renderer.SetShaderUniformMat4x4f(shadowShader, "lightSpaceMatrix", shadowCam.GetCamMatrix());
        for (int i = 0; i < boxes.size(); ++i)
        {
            graphics.m_Renderer.SetShaderUniformMat4x4f(shadowShader, "Transformation", boxes[i].GetTransform().GetTransformMatrix());
            graphics.m_Renderer.DrawMesh(boxes[i].m_TexturedMeshes[0].m_Mesh);
            //graphics.Draw(boxes[i]);
        }
        
        graphics.m_Renderer.SetShaderUniformMat4x4f(shadowShader, "Transformation", previewPSpawn.GetTransform().GetTransformMatrix());
        graphics.m_Renderer.DrawMesh(previewPSpawn.m_TexturedMeshes[0].m_Mesh);
        //graphics.Draw(previewPSpawn);
    }
    graphics.ResetFrameBuffer();
        
    // THIS IS ALL TEMPORARY WHILE I WORK ON A SHADOW SYSTEM IN THE GRAPHICS MODULE :^) 
    graphics.SetCamera(&cam);
    graphics.SetActiveFrameBuffer(viewportBuffer);
    {
        graphics.m_Renderer.SetActiveFBufferTexture(shadowBuffer, 1);
        graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_TexturedMeshShader, "Camera", graphics.m_Camera->GetCamMatrix());
        graphics.m_Renderer.SetShaderUniformVec3f(graphics.m_TexturedMeshShader, "CameraPos", graphics.m_Camera->GetPosition());
        graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_TexturedMeshShader, "LightSpaceMatrix", shadowCam.GetCamMatrix());
        for (int i = 0; i < boxes.size(); ++i)
        {
            graphics.m_Renderer.SetActiveTexture(boxes[i].m_TexturedMeshes[0].m_Texture, 0);
            graphics.m_Renderer.DrawMesh(boxes[i].m_TexturedMeshes[0].m_Mesh);
        }
        graphics.Draw(previewPSpawn);
    }
    graphics.ResetFrameBuffer();

    ui.BufferPanel(viewportBuffer, newViewport);

    //for (int i = 0; i < 10000; ++i)
    //{
    //    ui.ImgPanel(vertexToolTexture, Rect(Vec2f(200, 200), Vec2f(300, 300)));
    //}

    graphics.m_Renderer.SetActiveShader(testDepthShader);
    graphics.m_Renderer.SetActiveFBufferTexture(shadowBuffer);
    graphics.m_Renderer.DrawMesh(quadMesh);

    Vec2i screen = Engine::GetClientAreaSize();

    //ui.StartFrame(Rect(Vec2f(100.0f, 0.0f), Vec2f(screen.x - 100.0f, 200.0f)), 20.0f);

    if (ui.ImgButton(cursorToolTexture, Rect(Vec2f(0.0f, screen.y - 200.0f), Vec2f(100.0f, screen.y - 100.0f)), 20.0f))
    {
        toolMode = ToolMode::NORMAL;
    }
    if (ui.ImgButton(boxToolTexture, Rect(Vec2f(0.0f, screen.y - 300.0f), Vec2f(100.0f, screen.y - 200.0f)), 20.0f))
    {
        toolMode = ToolMode::BOX;
    }

    if (ui.ImgButton(vertexToolTexture, Rect(Vec2f(0.0f, screen.y - 400.0f), Vec2f(100, screen.y - 300.0f)), 20.0f))
    {
        toolMode = ToolMode::VERTEX;
    }

    if (inputs.keysDown.one)
    {
        toolMode = ToolMode::NORMAL;
    }
    if (inputs.keysDown.two)
    {
        toolMode = ToolMode::BOX;
    }
    if (inputs.keysDown.three)
    {
        toolMode = ToolMode::VERTEX;
    }

    std::string modeString;
    switch (toolMode)
    {
    case ToolMode::NORMAL:
        modeString = "Normal mode";
        break;
    case ToolMode::BOX:
        modeString = "Box mode";
        break;
    case ToolMode::VERTEX:
        modeString = "Vertex Edit Mode";
        break;
    }
    text.DrawText(modeString, &testFont, Vec2f(100.0f, Engine::GetClientAreaSize().y - 30), Vec3f(0.0f, 0.0f, 0.0f));

    // END DRAW
}

void Initialize(ModuleManager& modules)
{

    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    //Engine::LockCursor();
    //Engine::HideCursor();
    //Engine::SetCursorCenter(Vec2i(viewportRect.location.x + viewportRect.size.x / 2, Engine::GetClientAreaSize().y - (viewportRect.size.y / 2) - viewportRect.location.y));

    previewPSpawn = graphics.CreateModel(TexturedMesh(
        graphics.LoadMesh("models/PlayerSpawn.obj"),
        graphics.LoadTexture("textures/PlayerSpawn.png")
    ));

    cursorToolTexture = graphics.LoadTexture("images/cursorTool.png");
    boxToolTexture = graphics.LoadTexture("images/boxTool.png");
    vertexToolTexture = graphics.LoadTexture("images/vertexTool.png");

    Vec2i screenSizeI = Engine::GetClientAreaSize();
    cam = Camera(Projection::Perspective);
    cam.SetScreenSize(screenSizeI);
    
    cam.SetPosition(Vec3f(0.0f, -8.0f, 6.0f));
    cam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), 1.3f));

    shadowCam = Camera(Projection::Orthographic);
    shadowCam.SetScreenSize(Vec2f(100.0f, 100.0f));
    shadowCam.SetNearPlane(0.0f);
    shadowCam.SetFarPlane(100.0f);
    //shadowCam.SetPosition(Vec3f(0.0f, 0.0f, 5.0f));
    //shadowCam.SetDirection(Vec3f(0.0f, 0.0f, -1.0f));
    shadowCam.SetPosition(Vec3f(0.0f, -30.0f, 6.0f));
    shadowCam.Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.9f));
    shadowCam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), 1.3f));

    viewportBuffer = graphics.CreateFBuffer(Vec2i(viewportRect.size), FBufferFormat::COLOUR);

    shadowBuffer = graphics.CreateFBuffer(Vec2i(4000, 4000), FBufferFormat::DEPTH);

    std::string shadowVertShader = R"(
    #version 400
    layout (location = 0) in vec3 aPos;

    uniform mat4 lightSpaceMatrix;
    uniform mat4 Transformation;

    void main()
    {
        gl_Position = lightSpaceMatrix * Transformation * vec4(aPos, 1.0);
    }   
    )";

    std::string shadowFragShader = R"(
    #version 400
    
    void main()
    {
    }
    )";

    shadowShader = graphics.m_Renderer.LoadShader(shadowVertShader, shadowFragShader);

    std::string depthVertShader = R"(
    #version 400

    in vec2 VertPosition;
    in vec2 VertUV;

    smooth out vec2 FragUV;

    void main()
    {
        gl_Position = vec4(VertPosition, 0.0, 1.0);
        FragUV = VertUV;    
    }

    )";

    std::string depthFragShader = R"(
    #version 400

    uniform sampler2D DepthMap;
    
    smooth in vec2 FragUV;
    
    out vec4 FragColour;

    void main()
    {             
        float depthValue = texture(DepthMap, FragUV).r;
        FragColour = vec4(vec3(depthValue), 1.0);
        //FragColour = vec4(1.0, 0.0, 0.0, 1.0);
    }  
    )";

    testDepthShader = graphics.m_Renderer.LoadShader(depthVertShader, depthFragShader);

    std::vector<float> quadVertices =
    {
        -1.0f, -1.0f,       0.0f, 0.0f,
        -1.0f, -0.6f,       0.0f, 1.0f,
        -0.6f, -0.6f,       1.0f, 1.0f,
        -0.6f, -1.0f,       1.0f, 0.0f,
    };

    std::vector<ElementIndex> quadIndices =
    {
        0, 1, 2, 0, 2, 3
    };

    quadMesh = graphics.m_Renderer.LoadMesh(VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f }), quadVertices, quadIndices);

    graphics.InitializeDebugDraw(viewportBuffer);

    graphics.SetCamera(&cam);



    testFont = text.LoadFont("fonts/ARLRDBD.TTF", 30);
}

void Update(ModuleManager& modules, ControlInputs& inputs)
{

    UpdateEditor(modules, inputs);
    //if (inputs.mouse.leftMouseButton)
    //{
    //	if (holdingMouseLeft)
    //	{
    //		if (translatingX)
    //		{
    //			Line mouseLine;
    //			mouseLine.point = cam.position - xAxisOffset;
    //			mouseLine.direction = cam.direction;

    //			Line axisLine;
    //			axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			axisLine.direction = Vec3f(1.0f, 0.0f, 0.0f);

    //			Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

    //			editingMesh->SetPosition(pointAlongAxis, &renderer);

    //		}
    //		if (translatingY)
    //		{
    //			Line mouseLine;
    //			mouseLine.point = cam.position - yAxisOffset;
    //			mouseLine.direction = cam.direction;

    //			Line axisLine;
    //			axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			axisLine.direction = Vec3f(0.0f, 1.0f, 0.0f);

    //			Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

    //			editingMesh->SetPosition(pointAlongAxis, &renderer);
    //		}
    //		if (translatingZ)
    //		{
    //			Line mouseLine;
    //			mouseLine.point = cam.position - zAxisOffset;
    //			mouseLine.direction = cam.direction;

    //			Line axisLine;
    //			axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			axisLine.direction = Vec3f(0.0f, 0.0f, 1.0f);

    //			Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

    //			editingMesh->SetPosition(pointAlongAxis, &renderer);
    //		}
    //		if (rotatingX)
    //		{
    //			// Get intersection between mouse ray and x plane
    //			Plane xPlane;
    //			xPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			xPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);

    //			RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), xPlane);

    //			Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			if (intersection.hit)
    //			{
    //				Vec3f point = intersection.hitPoint - objectCenter;
    //				Vec3f unitVec = Vec3f(0.0f, 0.0f, 1.0f);
    //				float dot = Math::dot(point, unitVec);
    //				float det = Math::dot(xPlane.normal, (Math::cross(point, unitVec)));
    //				float angle = atan2(det, dot);
    //				
    //				float deltaAngle = xAngleLast - angle;

    //				xAngleLast = angle;

    //				editingMesh->RotateMeshAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), deltaAngle, &renderer);
    //				
    //				DebugLineSegment debugLine;
    //				debugLine.a = point + objectCenter;
    //				debugLine.b = objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);

    //				debugLine.a = unitVec + objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);
    //			}
    //		}
    //		if (rotatingY)
    //		{
    //			// Get intersection between mouse ray and y plane
    //			Plane yPlane;
    //			yPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			yPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);

    //			RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), yPlane);

    //			Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			if (intersection.hit)
    //			{
    //				Vec3f point = intersection.hitPoint - objectCenter;
    //				Vec3f unitVec = Vec3f(1.0f, 0.0f, 0.0f);
    //				float dot = Math::dot(point, unitVec);
    //				float det = Math::dot(yPlane.normal, (Math::cross(point, unitVec)));
    //				float angle = atan2(det, dot);

    //				float deltaAngle = yAngleLast - angle;

    //				yAngleLast = angle;

    //				editingMesh->RotateMeshAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), deltaAngle, &renderer);

    //				DebugLineSegment debugLine;
    //				debugLine.a = point + objectCenter;
    //				debugLine.b = objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);

    //				debugLine.a = unitVec + objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);
    //			}
    //		}
    //		if (rotatingZ)
    //		{
    //			// Get intersection between mouse ray and z plane
    //			Plane zPlane;
    //			zPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			zPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);

    //			RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), zPlane);

    //			Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			if (intersection.hit)
    //			{
    //				Vec3f point = intersection.hitPoint - objectCenter;
    //				Vec3f unitVec = Vec3f(0.0f, 1.0f, 0.0f);
    //				float dot = Math::dot(point, unitVec);
    //				float det = Math::dot(zPlane.normal, (Math::cross(point, unitVec)));
    //				float angle = atan2(det, dot);

    //				float deltaAngle = zAngleLast - angle;

    //				zAngleLast = angle;

    //				editingMesh->RotateMeshAroundAxis(Vec3f(0.0f, 0.0f, 1.0f), deltaAngle, &renderer);
    //			
    //				DebugLineSegment debugLine;
    //				debugLine.a = point + objectCenter;
    //				debugLine.b = objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);

    //				debugLine.a = unitVec + objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);
    //			}
    //		}
    //	}
    //	else {
    //		holdingMouseLeft = true;

    //		Ray testRay;
    //		testRay.origin = cam.position;
    //		testRay.direction = cam.direction;

    //		EditorRayCastInfo rayCastInfo = editorScene.RayCast(testRay);

    //		if (rayCastInfo.hitMesh)
    //		{
    //			if (rayCastInfo.hitType == HitType::MESH)
    //			{
    //				editorScene.UnselectSelectedEditableMesh();
    //				editorScene.SetEditableMeshSelected(rayCastInfo.hitMesh);

    //				editingMesh = rayCastInfo.hitMesh;
    //			}
    //			if (rayCastInfo.hitType == HitType::TOOL)
    //			{
    //				if (rayCastInfo.toolType == ToolType::X_ARROW)
    //				{
    //					xAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
    //					translatingX = true;
    //				}
    //				if (rayCastInfo.toolType == ToolType::Y_ARROW)
    //				{
    //					yAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
    //					translatingY = true;
    //				}
    //				if (rayCastInfo.toolType == ToolType::Z_ARROW)
    //				{
    //					zAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
    //					translatingZ = true;
    //				}
    //				if (rayCastInfo.toolType == ToolType::X_RING)
    //				{
    //					// Get intersection between mouse ray and x plane
    //					Plane xPlane;
    //					xPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					xPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
    //					
    //					RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), xPlane);
    //					
    //					Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					if (intersection.hit)
    //					{
    //						Vec3f point = intersection.hitPoint - objectCenter;
    //						Vec3f unitVec = Vec3f(0.0f, 0.0f, 1.0f);
    //						float dot = Math::dot(point, unitVec);
    //						float det = Math::dot(xPlane.normal, (Math::cross(point, unitVec)));
    //						float angle = atan2(det, dot);
    //						xAngleLast = angle;
    //						rotatingX = true;
    //					}
    //				}

    //				if (rayCastInfo.toolType == ToolType::Y_RING)
    //				{
    //					// Get intersection between mouse ray and y plane
    //					Plane yPlane;
    //					yPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					yPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);

    //					RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), yPlane);

    //					Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					if (intersection.hit)
    //					{
    //						Vec3f point = intersection.hitPoint - objectCenter;
    //						Vec3f unitVec = Vec3f(1.0f, 0.0f, 0.0f);
    //						float dot = Math::dot(point, unitVec);
    //						float det = Math::dot(yPlane.normal, (Math::cross(point, unitVec)));
    //						float angle = atan2(det, dot);
    //						yAngleLast = angle;
    //						rotatingY = true;
    //					}
    //				}
    //				if (rayCastInfo.toolType == ToolType::Z_RING)
    //				{
    //					// Get intersection between mouse ray and z plane
    //					Plane zPlane;
    //					zPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					zPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);

    //					RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), zPlane);

    //					Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					if (intersection.hit)
    //					{
    //						Vec3f point = intersection.hitPoint - objectCenter;
    //						Vec3f unitVec = Vec3f(0.0f, 1.0f, 0.0f);
    //						float dot = Math::dot(point, unitVec);
    //						float det = Math::dot(zPlane.normal, (Math::cross(point, unitVec)));
    //						float angle = atan2(det, dot);
    //						zAngleLast = angle;
    //						rotatingZ = true;
    //					}
    //				}
    //				
    //			}

    //		}
    //	}
    //}
    //else {
    //	translatingX = false;
    //	translatingY = false;
    //	translatingZ = false;

    //	rotatingX = false;
    //	rotatingY = false;
    //	rotatingZ = false;

    //	holdingMouseLeft = false;
    //}

    //if (inputs.keysDown.alt && !holdingAlt)
    //{
    //	Engine::UnlockCursor();
    //	Engine::ShowCursor();
    //	cameraControlEnabled = false;

    //	holdingAlt = true;
    //}
    //else if (!inputs.keysDown.alt && holdingAlt)
    //{
    //	holdingAlt = false;
    //}
    //if (inputs.keysDown.space && !holdingSpace)
    //{
    //	Engine::LockCursor();
    //	Engine::HideCursor();
    //	cameraControlEnabled = true;
    //	
    //	holdingSpace = true;
    //}
    //else if (!inputs.keysDown.space && holdingSpace)
    //{
    //	holdingSpace = false;
    //}

    //if (inputs.keysDown.tab && !holdingTab)
    //{
    //	holdingTab = true;
    //}
    //else if (!inputs.keysDown.tab && holdingTab)
    //{
    //	holdingTab = false;
    //	//editorScene.CycleToolType();
    //}

    //renderer.ClearScreen();

    //editorScene.DrawScene();

    //renderer.SwapBuffer();
}

void Resize(ModuleManager& modules, Vec2i newSize)
{
    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
    cam.SetScreenSize(viewportRect.size);
    GraphicsModule* graphics = modules.GetGraphics();

    graphics->ResizeFrameBuffer(viewportBuffer, viewportRect.size);
    Engine::SetCursorCenter(Vec2i(viewportRect.location.x + viewportRect.size.x / 2, Engine::GetClientAreaSize().y - (viewportRect.size.y / 2) - viewportRect.location.y));
}
