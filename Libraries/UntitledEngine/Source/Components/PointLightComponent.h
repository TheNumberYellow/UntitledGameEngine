#include "Component.h"
#include "Graphics/PointLight.h"

class PointLightComponent : public Component
{
public:
   
    PointLightComponent(PointLight* pointLight);
    ~PointLightComponent();
    
    void DrawEditorInspector() override;

    PointLight& GetPointLight() { return *m_PointLight; }

private:
    PointLight* m_PointLight = nullptr;
};