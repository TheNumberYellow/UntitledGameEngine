#pragma once

#include "GraphicsModule.h"
#include "CollisionModule.h"
#include "TextModule.h"
#include "UIModule.h"
#include "InputModule.h"

class ModuleManager
{
public:
    GraphicsModule* GetGraphics();
    void SetGraphics(GraphicsModule* graphics);

    CollisionModule* GetCollision();
    void SetCollision(CollisionModule* collision);

    TextModule* GetText();
    void SetText(TextModule* text);

    UIModule* GetUI();
    void SetUI(UIModule* ui);

    InputModule* GetInput();
    void SetInput(InputModule* input);

    bool AreAllModulesInitialized();

private:

    GraphicsModule* _graphics = nullptr;
    CollisionModule* _collision = nullptr;
    TextModule* _text = nullptr;
    UIModule* _ui = nullptr;
    InputModule* _input = nullptr;
};