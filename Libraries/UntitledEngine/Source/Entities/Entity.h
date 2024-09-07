#pragma once

#include "Scene.h"

class Entity
{
public:

    virtual void OnCreate();

    virtual void Update(float dt);

    virtual void OnDestroy();


    static void SetScene(Scene* scene);


protected:

    static Scene* GetScene();

private:

    static Scene* m_Scene;
};

