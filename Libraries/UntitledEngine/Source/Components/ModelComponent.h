#include "Component.h"
#include "Graphics/Model.h"

class ModelComponent : public Component
{
public:
    
    ModelComponent(Model* model);
    ~ModelComponent();
    
    void DrawEditorInspector() override;

    Model& GetModel() { return *m_Model; }

private:

    Model* m_Model = nullptr;
};