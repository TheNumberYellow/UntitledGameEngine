#include "ModuleManager.h"

GraphicsModule* ModuleManager::GetGraphics()
{
    return _graphics;
}

void ModuleManager::SetGraphics(GraphicsModule* graphics)
{
    _graphics = graphics;
}

CollisionModule* ModuleManager::GetCollision()
{
    return _collision;
}

void ModuleManager::SetCollision(CollisionModule* collision)
{
    _collision = collision;
}

TextModule* ModuleManager::GetText()
{
    return _text;
}

void ModuleManager::SetText(TextModule* text)
{
    _text = text;
}

UIModule* ModuleManager::GetUI()
{
    return _ui;
}

void ModuleManager::SetUI(UIModule* ui)
{
    _ui = ui;
}

bool ModuleManager::AreAllModulesInitialized()
{
    return _graphics 
        && _collision 
        && _text 
        && _ui;
}