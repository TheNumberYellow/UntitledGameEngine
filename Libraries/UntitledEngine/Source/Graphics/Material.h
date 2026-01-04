#pragma once
#include "Asset/AssetRegistry.h"

struct Material
{
    Material() {}
    Material(Texture* Albedo, Texture* Normal, Texture* Roughness, Texture* Metallic, Texture* AO);

    Texture* m_Albedo;
    Texture* m_Normal;
    Texture* m_Metallic;
    Texture* m_Roughness;
    Texture* m_AO;
    Texture* m_Height;

    friend bool operator<(const Material& lhs, const Material& rhs)
    {

        size_t lhsTotal = 0;
        size_t rhsTotal = 0;

        lhsTotal = lhs.m_Albedo->GetID() +
            lhs.m_Normal->GetID() +
            lhs.m_Metallic->GetID() +
            lhs.m_Roughness->GetID() + 
            lhs.m_AO->GetID();

        rhsTotal = rhs.m_Albedo->GetID() +
            rhs.m_Normal->GetID() +
            rhs.m_Metallic->GetID() +
            rhs.m_Roughness->GetID() +
            rhs.m_AO->GetID();

        return lhsTotal < rhsTotal;
    }

    friend bool operator==(const Material& lhs, const Material& rhs)
    {
        return lhs.m_Albedo->GetID() == rhs.m_Albedo->GetID()
            && lhs.m_Normal->GetID() == rhs.m_Normal->GetID()
            && lhs.m_Metallic->GetID() == rhs.m_Metallic->GetID()
            && lhs.m_Roughness->GetID() == rhs.m_Roughness->GetID()
            && lhs.m_AO->GetID() == rhs.m_AO->GetID();
    }
};