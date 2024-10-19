#pragma once
#include "Asset/AssetRegistry.h"

struct Material
{
    Material() {}
    Material(Texture Albedo, Texture Normal, Texture Roughness, Texture Metallic, Texture AO);

    Texture m_Albedo;
    Texture m_Normal;
    Texture m_Metallic;
    Texture m_Roughness;
    Texture m_AO;
    Texture m_Height;

    friend bool operator<(const Material& lhs, const Material& rhs)
    {
        return lhs.m_Albedo < rhs.m_Albedo
            || lhs.m_Normal < rhs.m_Normal;
    }

    friend bool operator==(const Material& lhs, const Material& rhs)
    {
        return lhs.m_Albedo == rhs.m_Albedo
            && lhs.m_Normal == rhs.m_Normal
            && lhs.m_Metallic == rhs.m_Metallic
            && lhs.m_Roughness == rhs.m_Roughness
            && lhs.m_AO == rhs.m_AO;
    }
};