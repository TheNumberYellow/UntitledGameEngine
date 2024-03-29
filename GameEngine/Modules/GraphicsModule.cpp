#include "GraphicsModule.h"

#include "..\FileLoader.h"

#include <random>
#include "Scene.h"

GraphicsModule* GraphicsModule::s_Instance = nullptr;

Material::Material(Texture Albedo, Texture Normal, Texture Roughness, Texture Metallic, Texture AO)
    : m_Albedo(Albedo)
    , m_Normal(Normal)
    , m_Roughness(Roughness)
    , m_Metallic(Metallic)
    , m_AO(AO)
{
}

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

void Transform::SetScale(float newScale)
{
    m_Scale = Vec3f(newScale, newScale, newScale);
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

    Math::DecomposeMatrix(m_Transform, m_Position, m_Rotation, m_Scale);

    m_WasTransformMatrixUpdated = true;
    m_TransformMatrixNeedsUpdate = false;
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

	uniform sampler2D AlbedoMap;

    out vec4 OutColour;

    void main()
    {
        vec4 textureAt = texture(AlbedoMap, FragUV);
        if (textureAt.a < 0.1)
        {
            discard;
        }
        //OutColour = vec4(1.0, 0.0, 0.0, 1.0);
        OutColour.rgb = textureAt.rgb * FragColour.rgb;
        //OutColour.a = 1.0;
        //OutColour.rgb = textureAt.rgb * FragColour.rgb;
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

	uniform sampler2D AlbedoMap;
    uniform sampler2D NormalMap;
    uniform sampler2D MetallicMap;
    uniform sampler2D RoughnessMap;
    uniform sampler2D AOMap;

    uniform samplerCube SkyBox;	
    uniform sampler2D ShadowMap;

	uniform vec3 SunDirection;
    uniform vec3 SunColour;

    uniform vec3 CameraPos;    

	out vec4 OutColour;

    const float PI = 3.14159265359;
    // ----------------------------------------------------------------------------
    float DistributionGGX(vec3 N, vec3 H, float roughness)
    {
        float a = roughness*roughness;
        float a2 = a*a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH*NdotH;

        float nom   = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r*r) / 8.0;

        float nom   = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // ----------------------------------------------------------------------------
    vec3 fresnelSchlick(float cosTheta, vec3 F0)
    {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    float rand(vec2 co)
    {
        return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    }

    mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
    {
        // get edge vectors of the pixel triangle
        vec3 dp1 = dFdx( p );
        vec3 dp2 = dFdy( p );
        vec2 duv1 = dFdx( uv );
        vec2 duv2 = dFdy( uv );
 
        // solve the linear system
        vec3 dp2perp = cross( dp2, N );
        vec3 dp1perp = cross( N, dp1 );
        vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
        vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
        // construct a scale-invariant frame 
        float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
        return mat3( T * invmax, B * invmax, N );
    }

    vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
    {
        // assume N, the interpolated vertex normal and 
        // V, the view vector (vertex to eye)
        vec3 map = texture2D( NormalMap, texcoord ).xyz;
        map = normalize(map * 2.0 - 1.0);
        mat3 TBN = cotangent_frame( N, -V, texcoord );
        return normalize( TBN * map );
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
        //vec3 Albedo = pow(texture(AlbedoMap, FragUV).rgb, vec3(2.2));
        vec4 TexAt = texture(AlbedoMap, FragUV);
        vec3 Albedo = TexAt.rgb;
        vec3 normalizedNormal = normalize(FragNormal);
        vec3 ViewVector = CameraPos - FragPosition;       
        
        vec3 Normal = perturb_normal(normalizedNormal, ViewVector, FragUV);
        float Metallic = texture(MetallicMap, FragUV).r;
        float Roughness = texture(RoughnessMap, FragUV).g;
        float AO = texture(AOMap, FragUV).r;            

        vec3 N = normalize(Normal);
        vec3 V = normalize(CameraPos - FragPosition);

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, Albedo, Metallic);

        // reflectance equation
        vec3 Lo = vec3(0.0);

        // Directional light
        vec3 L = -SunDirection;
        vec3 H = normalize(V + L);
        //float distance = 0.35;
        //float attenuation = 1.0 / (distance * distance);
        vec3 radiance = SunColour * 3.0;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, Roughness);   
        float G   = GeometrySmith(N, V, L, Roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;    

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - Metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * Albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again    

        // ambient lighting (note that the next IBL tutorial will replace 
        // this ambient lighting with environment lighting).
        
        vec3 I = normalize(FragPosition - CameraPos);
        vec3 R = reflect(I, normalize(Normal));
        vec3 reflectColour = texture(SkyBox, R).rgb * Metallic;
        //vec3 reflectColour = texture(Sky;
        vec3 ambient = max(reflectColour, vec3(0.25)) * Albedo * AO;
        
        float shadow = ShadowCalculation(FragPosLightSpace);
        vec3 color = ambient + (1.0 - shadow) * Lo;

        // HDR tonemapping
        //color = color / (color + vec3(1.0));
        // gamma correct
        //color = pow(color, vec3(1.0/2.2)); 

        OutColour = vec4(color, TexAt.a);
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
        gl_Position.z = gl_Position.w;
    }
    )";

    fragShaderSource = R"(
    #version 400

    in vec3 TexCoords;
    
    uniform samplerCube SkyBox;

    out vec4 OutColour;

    void main()
    {
        OutColour = texture(SkyBox, TexCoords);
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
    
	//uniform vec2 WindowSize;
    
    smooth in vec2 FragUV;	

    out vec4 OutColour;

    void main()
    {
        vec4 textureAt = texture(Texture, FragUV);
        OutColour = textureAt;
    	
        // TEMP (Fraser): Super basic crosshair 
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

    // GBuffer stuff

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
    #version 400

	uniform mat4x4 Transformation;
	uniform mat4x4 Camera;

	in vec4 VertPosition;
	in vec4 VertNormal;
	in vec4 VertColour;
	in vec2 VertUV;

    smooth out vec3 FragPosition;
	smooth out vec3 FragNormal;
    smooth out vec4 FragColour;
	smooth out vec2 FragUV;

    void main()
    {
        gl_Position = (Camera * Transformation) * VertPosition;

        FragPosition = vec3(Transformation * VertPosition);
        FragNormal = mat3(transpose(inverse(Transformation))) * VertNormal.xyz;
        FragColour = VertColour;
        FragUV = VertUV;
    }

    )";

    fragShaderSource = R"(
    #version 400

    //out vec4 OutColour;
    layout (location = 0) out vec4 gPosition;
    layout (location = 1) out vec4 gNormal;
    layout (location = 2) out vec4 gAlbedo;
    layout (location = 3) out vec4 gMetallic;
    layout (location = 4) out vec4 gRoughness;
    layout (location = 5) out vec4 gAO;

    smooth in vec3 FragPosition;
    smooth in vec3 FragNormal;   
    smooth in vec4 FragColour;	 
    smooth in vec2 FragUV;

    uniform sampler2D AlbedoMap;
    uniform sampler2D NormalMap;
    uniform sampler2D MetallicMap;
    uniform sampler2D RoughnessMap;
    uniform sampler2D AOMap;

    uniform vec3 CameraPos;

    mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
    {
        // get edge vectors of the pixel triangle
        vec3 dp1 = dFdx( p );
        vec3 dp2 = dFdy( p );
        vec2 duv1 = dFdx( uv );
        vec2 duv2 = dFdy( uv );
 
        // solve the linear system
        vec3 dp2perp = cross( dp2, N );
        vec3 dp1perp = cross( N, dp1 );
        vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
        vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
        // construct a scale-invariant frame 
        float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
        return mat3( T * invmax, B * invmax, N );
    }

    vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
    {
        // assume N, the interpolated vertex normal and 
        // V, the view vector (vertex to eye)
        vec3 map = texture2D( NormalMap, texcoord ).xyz;
        map = normalize(map * 2.0 - 1.0);
        mat3 TBN = cotangent_frame( N, -V, texcoord );
        return normalize( TBN * map );
    }

    void main()
    {
        vec3 ViewVector = CameraPos - FragPosition;       
       
        gPosition = vec4(FragPosition, 1.0);
        gAlbedo = texture(AlbedoMap, FragUV);
        gMetallic = texture(MetallicMap, FragUV);
        gRoughness = texture(RoughnessMap, FragUV);
        gAO = texture(AOMap, FragUV);

        
        vec3 Normal = perturb_normal(normalize(FragNormal), ViewVector, FragUV);
        gNormal = vec4(Normal, 1.0);
        //vec3 Normal = normalize(FragNormal);
        //gNormal = vec4(Normal, 1.0 + tiny);
    }   
        
    )";

    m_GBufferShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer lighting shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
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

    fragShaderSource = R"(
    #version 400

    out vec4 OutColour;

    smooth in vec2 FragUV;

    uniform sampler2D gPosition;
    uniform sampler2D gNormal;
    uniform sampler2D gAlbedo;
    uniform sampler2D gMetallic;
    uniform sampler2D gRoughness;
    uniform sampler2D gAO;

    uniform samplerCube SkyBox;	
    
	uniform vec3 SunDirection;
    uniform vec3 SunColour;

    uniform vec3 CameraPos;

    const float PI = 3.14159265359;
    // ----------------------------------------------------------------------------
    float DistributionGGX(vec3 N, vec3 H, float roughness)
    {
        float a = roughness*roughness;
        float a2 = a*a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH*NdotH;

        float nom   = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r*r) / 8.0;

        float nom   = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // ----------------------------------------------------------------------------
    vec3 fresnelSchlick(float cosTheta, vec3 F0)
    {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    void main()
    {
        vec3 Position = texture(gPosition, FragUV).xyz;
        vec3 Normal = texture(gNormal, FragUV).xyz;
        vec3 Albedo = texture(gAlbedo, FragUV).xyz;
        float Metallic = texture(gMetallic, FragUV).r;
        float Roughness = texture(gRoughness, FragUV).g;
        float AO = texture(gAO, FragUV).r;        

        vec3 N = normalize(Normal);
        vec3 V = normalize(CameraPos - Position);        

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, Albedo, Metallic);

        // reflectance equation
        vec3 Lo = vec3(0.0);

        // Directional light
        vec3 L = -SunDirection;
        vec3 H = normalize(V + L);
        //float distance = 0.35;
        //float attenuation = 1.0 / (distance * distance);
        vec3 radiance = SunColour * 3.0;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, Roughness);   
        float G   = GeometrySmith(N, V, L, Roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;    

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - Metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * Albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again    

        // ambient lighting (note that the next IBL tutorial will replace 
        // this ambient lighting with environment lighting).
        
        vec3 I = normalize(Position - CameraPos);
        vec3 R = reflect(I, normalize(Normal));
        vec3 reflectColour = texture(SkyBox, R).rgb * Metallic;
        vec3 ambient = max(reflectColour, vec3(0.25)) * Albedo * AO;

        float shadow = 0.0;//ShadowCalculation(FragPosLightSpace);
        vec3 color = ambient + (1.0 - shadow) * Lo;

        // HDR tonemapping
        //color = color / (color + vec3(1.0));
        // gamma correct
        //color = pow(color, vec3(1.0/2.2)); 


        //float randomTiny = 0.0001 * (Position.x + Normal.x + Albedo.x + Metallic + Roughness + AO + SunDirection.x + SunColour.r + CameraPos.x);
        
        //Normal = normalize(Normal * 2.0 - 1);
        //OutColour = vec4((Albedo + randomTiny) * dot(N, -SunDirection), 1.0);
        
        OutColour = vec4(color, 1.0);   
    }   
        
    )";

    m_GBufferOldLightingShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer sky shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
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

    fragShaderSource = R"(
    #version 400

    out vec4 OutColour;

    smooth in vec2 FragUV;

    uniform sampler2D gSky;
    
    void main()
    {
        vec4 Sky = texture(gSky, FragUV);

        OutColour = Sky;
    }   
        
    )";
    m_GBufferSkyShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer debug shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
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

    fragShaderSource = R"(
    #version 400

    out vec4 OutColour;

    smooth in vec2 FragUV;

    uniform sampler2D gDebug;
    
    void main()
    {
        vec4 Col = texture(gDebug, FragUV);

        OutColour = Col;
    }   
        
    )";
    m_GBufferDebugShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer directional light shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
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

    fragShaderSource = R"(
    #version 400

    out vec4 OutColour;

    smooth in vec2 FragUV;

    uniform sampler2D gPosition;
    uniform sampler2D gNormal;
    uniform sampler2D gAlbedo;
    uniform sampler2D gMetallic;
    uniform sampler2D gRoughness;
    uniform sampler2D gAO;

    uniform samplerCube SkyBox;	

    uniform sampler2D ShadowMap;    
    uniform mat4x4 LightSpaceMatrix;

	uniform vec3 SunDirection;
    uniform vec3 SunColour;

    uniform vec3 CameraPos;
    
    const float PI = 3.14159265359;
    // ----------------------------------------------------------------------------
    float DistributionGGX(vec3 N, vec3 H, float roughness)
    {
        float a = roughness*roughness;
        float a2 = a*a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH*NdotH;

        float nom   = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r*r) / 8.0;

        float nom   = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // ----------------------------------------------------------------------------
    vec3 fresnelSchlick(float cosTheta, vec3 F0)
    {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    float ShadowCalculation(vec4 fragPosLightSpace, vec3 FragNormal)
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
        vec3 Position = texture(gPosition, FragUV).xyz;
        vec3 Normal = texture(gNormal, FragUV).xyz;
        vec3 Albedo = texture(gAlbedo, FragUV).xyz;
        float Metallic = texture(gMetallic, FragUV).r;
        float Roughness = texture(gRoughness, FragUV).g;
        float AO = texture(gAO, FragUV).r;        

        vec3 N = normalize(Normal);
        vec3 V = normalize(CameraPos - Position);        

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, Albedo, Metallic);

        // reflectance equation
        vec3 Lo = vec3(0.0);

        // Directional light
        vec3 L = -SunDirection;
        vec3 H = normalize(V + L);
        //float distance = 0.35;
        //float attenuation = 1.0 / (distance * distance);
        vec3 radiance = SunColour * 3.0;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, Roughness);   
        float G   = GeometrySmith(N, V, L, Roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;    

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - Metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        vec4 FragPosLightSpace = LightSpaceMatrix * vec4(Position, 1.0);
        float shadow = ShadowCalculation(FragPosLightSpace, Normal);

        // add to outgoing radiance Lo
        Lo += (kD / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

        //float randomTiny = 0.000001 * (Position.x + Normal.x + Albedo.x + Metallic + Roughness + AO + SunDirection.x + SunColour.r + CameraPos.x);
        vec3 color = (1.0 - shadow) * Lo * AO;
        OutColour = vec4(color, 1.0); 
    }   
        
    )";
    m_GBufferDirectionalLightShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer point light shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
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

    fragShaderSource = R"(
    #version 400

    out vec4 OutColour;

    smooth in vec2 FragUV;

    uniform sampler2D gPosition;
    uniform sampler2D gNormal;
    uniform sampler2D gAlbedo;
    uniform sampler2D gMetallic;
    uniform sampler2D gRoughness;
    uniform sampler2D gAO;

    uniform samplerCube SkyBox;	
 
    uniform vec3 LightPosition;
    uniform vec3 LightColour;

    uniform vec3 CameraPos;
    
    const float PI = 3.14159265359;
    // ----------------------------------------------------------------------------
    float DistributionGGX(vec3 N, vec3 H, float roughness)
    {
        float a = roughness*roughness;
        float a2 = a*a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH*NdotH;

        float nom   = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r*r) / 8.0;

        float nom   = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
    {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // ----------------------------------------------------------------------------
    vec3 fresnelSchlick(float cosTheta, vec3 F0)
    {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    void main()
    {
        vec3 Position = texture(gPosition, FragUV).xyz;
        vec3 Normal = texture(gNormal, FragUV).xyz;
        vec3 Albedo = texture(gAlbedo, FragUV).xyz;
        float Metallic = texture(gMetallic, FragUV).r;
        float Roughness = texture(gRoughness, FragUV).g;
        float AO = texture(gAO, FragUV).r;        

        vec3 N = normalize(Normal);
        vec3 V = normalize(CameraPos - Position);        

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, Albedo, Metallic);

        // reflectance equation
        vec3 Lo = vec3(0.0);

        // calculate per-light radiance
        vec3 L = normalize(LightPosition - Position);
        vec3 H = normalize(V + L);
        float distance = length(LightPosition - Position);
        float attenuation = 1.0 / (distance * distance);
        //float linear = 0.001;
        //float quadratic = 0.02;
        //float attenuation = 1.0 / (1.0 + linear * distance + quadratic * distance * distance);
        attenuation *= 20.0;
        vec3 radiance = LightColour * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, Roughness);   
        float G   = GeometrySmith(N, V, L, Roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;    

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - Metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

        //float randomTiny = 0.000001 * (Position.x + Normal.x + Albedo.x + Metallic + Roughness + AO + SunDirection.x + SunColour.r + CameraPos.x);
        
        OutColour = vec4(Lo * AO, 1.0); 
    }   
        
    )";
    m_GBufferPointLightShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    // ~~~~~~~~~~~~~~~~~~~~~GBuffer combiner shader code~~~~~~~~~~~~~~~~~~~~~ //
    vertShaderSource = R"(
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

    fragShaderSource = R"(
    #version 400

    smooth in vec2 FragUV;

    out vec4 OutColour;

    uniform sampler2D PositionTex;
    uniform sampler2D NormalTex;
    uniform sampler2D MetallicTex;

    uniform sampler2D UnlitTex;
    uniform sampler2D LightingTex;
    
    uniform samplerCube SkyBox;	

    uniform vec3 CameraPos;

    void main()
    {
        vec3 BaseColour = texture(UnlitTex, FragUV).xyz;
        vec3 Light = texture(LightingTex, FragUV).xyz;
    
        vec3 Position = texture(PositionTex, FragUV).xyz;
        vec3 Normal = texture(NormalTex, FragUV).xyz;
        float Metallic = texture(MetallicTex, FragUV).r;

        vec3 I = normalize(Position - CameraPos);
        vec3 R = reflect(I, normalize(Normal));
        vec3 reflectColour = texture(SkyBox, R).rgb * Metallic;
        vec3 ambient = max(reflectColour, vec3(0.15)) * BaseColour;

        OutColour.xyz = ambient; // Ambient
        OutColour.xyz += BaseColour * Light; // Lighting
        //OutColour.xyz = BaseColour;
        OutColour.a = 1.0;
    }    

    )";
    m_GBufferCombinerShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    Vec2i viewportSize = GetViewportSize();
    float sizeX = (float)viewportSize.x;
    float sizeY = (float)viewportSize.y;

    m_OrthoProjection = Math::GenerateOrthoMatrix(0.0f, sizeX, 0.0f, sizeY, 0.1f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_UIShader, "Projection", m_OrthoProjection);

    // TEMP
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
         1.0f,   1.0f,  -1.0f,
    };

    m_SkyboxMesh = m_Renderer.LoadMesh(VertexBufferFormat({ VertAttribute::Vec3f }), skyboxVertices);
    m_SkyboxCubemap = m_Renderer.LoadCubemap("asda");
    m_IsSkyboxSet = true;

    m_ShadowBuffer = CreateFBuffer(Vec2i(8000, 8000), FBufferFormat::DEPTH);

    m_ShadowCamera = Camera(Projection::Orthographic);
    m_ShadowCamera.SetScreenSize(Vec2f(100.0f, 100.0f));
    m_ShadowCamera.SetNearPlane(0.0f);
    m_ShadowCamera.SetFarPlane(100.0f);
    m_ShadowCamera.SetPosition(Vec3f(0.0f, -30.0f, 6.0f));

    VertexBufferFormat quadMeshFormat = VertexBufferFormat({ VertAttribute::Vec3f, VertAttribute::Vec3f, VertAttribute::Vec4f, VertAttribute::Vec2f });
    MeshData quadMeshData = GetVertexDataFor3DQuad();

    m_BillboardQuadMesh = m_Renderer.LoadMesh(quadMeshFormat, quadMeshData.first, quadMeshData.second);

    m_LightTexture = m_Renderer.LoadTexture("images/light.png", TextureMode::LINEAR, TextureMode::NEAREST);

    s_Instance = this;
}

GraphicsModule::~GraphicsModule()
{
}

void GraphicsModule::Initialize()
{
    AssetRegistry* Registry = AssetRegistry::Get();

    m_DefaultNormalMap = *Registry->LoadTexture("textures/default_norm.png");
    m_DefaultMetallicMap = *Registry->LoadTexture("textures/default_metallic.png");
    m_DefaultRoughnessMap = *Registry->LoadTexture("textures/default_roughness.png");
    m_DefaultAOMap = *Registry->LoadTexture("textures/default_ao.png");
    m_DefaultHeightMap = *Registry->LoadTexture("textures/default_height.png");

    m_DebugMaterial = CreateMaterial(*Registry->LoadTexture("textures/debugTexture0.png"),
        *Registry->LoadTexture("textures/debugTexture0.norm.png"));
}

GBuffer GraphicsModule::CreateGBuffer(Vec2i Size)
{
    GBuffer newGBuffer;

    newGBuffer.Buffer = m_Renderer.CreateFrameBuffer(Size, FBufferFormat::EMPTY);
    newGBuffer.PositionTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.Buffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 0);
    newGBuffer.NormalTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.Buffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 1);
    newGBuffer.AlbedoTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.Buffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 2);
    newGBuffer.MetallicTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.Buffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 3);
    newGBuffer.RoughnessTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.Buffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 4);
    newGBuffer.AOTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.Buffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 5);

    newGBuffer.SkyBuffer = m_Renderer.CreateFBufferWithExistingDepthBuffer(newGBuffer.Buffer, Size, FBufferFormat::EMPTY);
    newGBuffer.SkyTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.SkyBuffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 0);

    newGBuffer.LightBuffer = m_Renderer.CreateFrameBuffer(Size, FBufferFormat::EMPTY);
    newGBuffer.LightTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.LightBuffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 0);

    newGBuffer.DebugBuffer = m_Renderer.CreateFBufferWithExistingDepthBuffer(newGBuffer.Buffer, Size, FBufferFormat::EMPTY);
    newGBuffer.DebugTex = m_Renderer.AttachColourAttachmentToFrameBuffer(newGBuffer.DebugBuffer, TextureCreateInfo(Size, ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::FLOAT), 0);

    newGBuffer.FinalOutput = m_Renderer.CreateFrameBuffer(Size, FBufferFormat::COLOUR);

    VertexBufferFormat quadMeshFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });
    MeshData quadMeshData = GetVertexDataForQuad();

    newGBuffer.QuadMesh = m_Renderer.LoadMesh(quadMeshFormat, quadMeshData.first, quadMeshData.second);

    return newGBuffer;
}

void GraphicsModule::DeleteGBuffer(GBuffer& GBuf)
{
    DeleteFBuffer(GBuf.Buffer);


    DeleteFBuffer(GBuf.SkyBuffer);


    DeleteFBuffer(GBuf.LightBuffer);


    DeleteFBuffer(GBuf.DebugBuffer);


    DeleteFBuffer(GBuf.FinalOutput);
}

void GraphicsModule::AddRenderCommand(StaticMeshRenderCommand Command)
{
    m_StaticMeshRenderCommands.push_back(Command);
}

void GraphicsModule::AddRenderCommand(BillboardRenderCommand Command)
{
    m_BillboardRenderCommands.push_back(Command);
}

void GraphicsModule::AddRenderCommand(PointLightRenderCommand Command)
{
    m_PointLightRenderCommands.push_back(Command);
}

void GraphicsModule::Render(GBuffer Buffer, Camera Cam, DirectionalLight DirLight)
{
    m_Renderer.SetActiveFBuffer(Buffer.Buffer);

    m_Renderer.SetActiveShader(m_GBufferShader);
    m_Renderer.ClearScreenAndDepthBuffer();

    m_Renderer.SetShaderUniformMat4x4f(m_GBufferShader, "Camera", Cam.GetCamMatrix());
    m_Renderer.SetShaderUniformVec3f(m_GBufferShader, "CameraPos", Cam.GetPosition());

    for (StaticMeshRenderCommand& Command : m_StaticMeshRenderCommands)
    {
        m_Renderer.SetShaderUniformMat4x4f(m_GBufferShader, "Transformation", Command.m_Transform.GetTransformMatrix());

        m_Renderer.SetActiveTexture(Command.m_Material.m_Albedo.Id, "AlbedoMap");
        m_Renderer.SetActiveTexture(Command.m_Material.m_Normal.Id, "NormalMap");
        m_Renderer.SetActiveTexture(Command.m_Material.m_Metallic.Id, "MetallicMap");
        m_Renderer.SetActiveTexture(Command.m_Material.m_Roughness.Id, "RoughnessMap");
        m_Renderer.SetActiveTexture(Command.m_Material.m_AO.Id, "AOMap");

        m_Renderer.DrawMesh(Command.m_Mesh);
    }

    // TEMP: skybox code blech

    m_Renderer.SetActiveFBuffer(Buffer.SkyBuffer);
    m_Renderer.SetActiveShader(m_SkyboxShader);
    m_Renderer.ClearColourBuffer();

    m_Renderer.SetShaderUniformMat4x4f(m_SkyboxShader, "projection", Cam.GetProjectionMatrix());

    Mat4x4f newView = Cam.GetViewMatrix();

    newView[3][0] = 0.0f;
    newView[3][1] = 0.0f;
    newView[3][2] = 0.0f;
    newView[3][3] = 1.0f;

    newView[0][3] = 0.0f;
    newView[1][3] = 0.0f;
    newView[2][3] = 0.0f;

    m_Renderer.SetShaderUniformMat4x4f(m_SkyboxShader, "view", newView);

    m_Renderer.SetActiveCubemap(m_SkyboxCubemap, "SkyBox");
    
    m_Renderer.DrawMesh(m_SkyboxMesh);

    // Temp skybox code end

    // Debug draw
    m_Renderer.SetActiveFBuffer(Buffer.DebugBuffer);
    m_Renderer.ClearColourBuffer();
    DrawDebugDrawMesh(Cam);

    // Billboards draw (also to debug buffer)

    m_Renderer.SetActiveShader(m_UnlitShader);
    m_Renderer.SetShaderUniformMat4x4f(m_UnlitShader, "Camera", Cam.GetCamMatrix());

    for (BillboardRenderCommand& Command : m_BillboardRenderCommands)
    {
        Mat4x4f BillboardMatrix;

        Vec3f CamToBill = Math::normalize(Cam.GetPosition() - Command.m_Position);

        Vec3f CrossBill = Cam.GetUp();

        Vec3f BillDir = CamToBill;
        Vec3f BillUp = Cam.GetUp();

        //Vec3f BillUp = Math::cross(BillDir, Cam.GetPerpVector());

        Vec3f xAxis = Math::cross(BillUp, BillDir);
        xAxis = Math::normalize(xAxis);

        Vec3f yAxis = Math::cross(BillDir, xAxis);
        yAxis = Math::normalize(yAxis);

        BillboardMatrix[0][0] = xAxis.x;
        BillboardMatrix[1][0] = yAxis.x;
        BillboardMatrix[2][0] = BillDir.x;

        BillboardMatrix[0][1] = xAxis.y;
        BillboardMatrix[1][1] = yAxis.y;
        BillboardMatrix[2][1] = BillDir.y;

        BillboardMatrix[0][2] = xAxis.z;
        BillboardMatrix[1][2] = yAxis.z;
        BillboardMatrix[2][2] = BillDir.z;

        Transform BillboardTransform;
        BillboardTransform.SetTransformMatrix(BillboardMatrix);
        BillboardTransform.SetPosition(Command.m_Position);
        BillboardTransform.SetScale(Vec3f(Command.m_Size, Command.m_Size, Command.m_Size));

        m_Renderer.SetShaderUniformMat4x4f(m_UnlitShader, "Transformation", BillboardTransform.GetTransformMatrix());
        m_Renderer.SetActiveTexture(Command.m_Texture, "AlbedoMap");

        m_Renderer.SetMeshColour(m_BillboardQuadMesh, Vec4f(Command.m_Colour.x, Command.m_Colour.y, Command.m_Colour.z, 1.0f));
        m_Renderer.DrawMesh(m_BillboardQuadMesh);
    }

    // ~~~~~~~ Render lights ~~~~~~~

    m_Renderer.SetBlendFunction(BlendFunc::ADDITIVE);
    {
        // Directional lights

        // Loop through directional lights here
        
        // Create directional light shadow map
        m_ShadowCamera.SetPosition(Cam.GetPosition() + (-m_ShadowCamera.GetDirection() * 40.0f));
        m_ShadowCamera.SetDirection(DirLight.direction);
        
        m_Renderer.SetActiveShader(m_ShadowShader);
        SetActiveFrameBuffer(m_ShadowBuffer);
        //m_Renderer.ClearScreenAndDepthBuffer();
        {
            m_Renderer.SetShaderUniformMat4x4f(m_ShadowShader, "LightSpaceMatrix", m_ShadowCamera.GetCamMatrix());

            for (StaticMeshRenderCommand& Command : m_StaticMeshRenderCommands)
            {
                // Set mesh-specific transform uniform
                m_Renderer.SetShaderUniformMat4x4f(m_ShadowShader, "Transformation", Command.m_Transform.GetTransformMatrix());

                // Draw mesh to shadow map
                m_Renderer.DrawMesh(Command.m_Mesh);
            }
        }

        m_Renderer.SetActiveShader(m_GBufferDirectionalLightShader);
        m_Renderer.SetActiveFBuffer(Buffer.LightBuffer);
        m_Renderer.ClearScreenAndDepthBuffer();

        m_Renderer.SetShaderUniformVec3f(m_GBufferDirectionalLightShader, "CameraPos", Cam.GetPosition());

        m_Renderer.SetActiveTexture(Buffer.PositionTex, "gPosition");
        m_Renderer.SetActiveTexture(Buffer.NormalTex, "gNormal");
        m_Renderer.SetActiveTexture(Buffer.AlbedoTex, "gAlbedo");
        m_Renderer.SetActiveTexture(Buffer.MetallicTex, "gMetallic");
        m_Renderer.SetActiveTexture(Buffer.RoughnessTex, "gRoughness");
        m_Renderer.SetActiveTexture(Buffer.AOTex, "gAO");

        m_Renderer.SetActiveFBufferTexture(m_ShadowBuffer, "ShadowMap");
        m_Renderer.SetShaderUniformMat4x4f(m_GBufferDirectionalLightShader, "LightSpaceMatrix", m_ShadowCamera.GetCamMatrix());

        m_Renderer.SetShaderUniformVec3f(m_GBufferDirectionalLightShader, "SunDirection", DirLight.direction);
        m_Renderer.SetShaderUniformVec3f(m_GBufferDirectionalLightShader, "SunColour", DirLight.colour);

        m_Renderer.DrawMesh(Buffer.QuadMesh);

        // Point lights
        m_Renderer.SetActiveShader(m_GBufferPointLightShader);

        m_Renderer.SetShaderUniformVec3f(m_GBufferPointLightShader, "CameraPos", Cam.GetPosition());

        // Needed?
        m_Renderer.SetActiveTexture(Buffer.PositionTex, "gPosition");
        m_Renderer.SetActiveTexture(Buffer.NormalTex, "gNormal");
        m_Renderer.SetActiveTexture(Buffer.AlbedoTex, "gAlbedo");
        m_Renderer.SetActiveTexture(Buffer.MetallicTex, "gMetallic");
        m_Renderer.SetActiveTexture(Buffer.RoughnessTex, "gRoughness");
        m_Renderer.SetActiveTexture(Buffer.AOTex, "gAO");

        // Loop through point lights here

        for (PointLightRenderCommand& Command : m_PointLightRenderCommands)
        {
            m_Renderer.SetShaderUniformVec3f(m_GBufferPointLightShader, "LightPosition", Command.m_Position);
            m_Renderer.SetShaderUniformVec3f(m_GBufferPointLightShader, "LightColour", Command.m_Colour * Command.m_Intensity);

            m_Renderer.DrawMesh(Buffer.QuadMesh);
        }

    }
    m_Renderer.SetBlendFunction(BlendFunc::TRANS);

    // Combine everything
    m_Renderer.SetActiveFBuffer(Buffer.FinalOutput);
    {
        m_Renderer.SetActiveShader(m_GBufferCombinerShader);
        m_Renderer.ClearScreenAndDepthBuffer();

        m_Renderer.SetShaderUniformVec3f(m_GBufferCombinerShader, "CameraPos", Cam.GetPosition());
        
        m_Renderer.SetActiveTexture(Buffer.PositionTex, "PositionTex");
        m_Renderer.SetActiveTexture(Buffer.NormalTex, "NormalTex");
        m_Renderer.SetActiveTexture(Buffer.MetallicTex, "MetallicTex");
        
        m_Renderer.SetActiveTexture(Buffer.AlbedoTex, "UnlitTex");
        m_Renderer.SetActiveTexture(Buffer.LightTex, "LightingTex");

        m_Renderer.SetActiveCubemap(m_SkyboxCubemap, "SkyBox");

        m_Renderer.DrawMesh(Buffer.QuadMesh);

        // Skybox stuff
        m_Renderer.DisableDepthTesting();
        m_Renderer.SetActiveShader(m_GBufferSkyShader);

        m_Renderer.SetActiveTexture(Buffer.SkyTex, "gSky");
        m_Renderer.DrawMesh(Buffer.QuadMesh);
        
        // Debug stuff
        m_Renderer.SetActiveShader(m_GBufferDebugShader);

        m_Renderer.SetActiveTexture(Buffer.DebugTex, "gDebug");
        m_Renderer.DrawMesh(Buffer.QuadMesh);
        
        m_Renderer.EnableDepthTesting();

    }

    m_StaticMeshRenderCommands.clear();
    m_BillboardRenderCommands.clear();
    m_PointLightRenderCommands.clear();
}

Shader_ID GraphicsModule::CreateShader(std::string vertShaderSource, std::string fragShaderSource)
{
    return m_Renderer.LoadShader(vertShaderSource, fragShaderSource);
}

Framebuffer_ID GraphicsModule::CreateFBuffer(Vec2i size, FBufferFormat format)
{
    return m_Renderer.CreateFrameBuffer(size, format);
}

void GraphicsModule::DeleteFBuffer(Framebuffer_ID Buffer)
{
    // TODO: MEMORY LEAK!!! Delete additional colour attachments
    // I'm so lazy
    m_Renderer.DeleteFrameBuffer(Buffer);
}

Texture GraphicsModule::CreateTexture(Vec2i size)
{
    Texture_ID Id = m_Renderer.CreateEmptyTexture(size);

    Texture Result;
    Result.Id = Id;
    Result.LoadedFromFile = false;

    return Result;
}

Texture GraphicsModule::LoadTexture(std::string filePath, TextureMode minFilter, TextureMode magFilter)
{
    Texture_ID Id = m_Renderer.LoadTexture(filePath, minFilter, magFilter);

    Texture Result;
    Result.Id = Id;
    Result.LoadedFromFile = true;
    Result.Path = filePath;

    return Result;
}

StaticMesh GraphicsModule::LoadMesh(std::string filePath)
{
    // todo(Fraser): Switch here on different file types? (If I ever want something that's not .obj)
    StaticMesh_ID Id = FileLoader::LoadOBJFile(filePath, m_Renderer);

    StaticMesh Result;
    Result.Id = Id;
    Result.LoadedFromFile = true;
    Result.Path = filePath;

    return Result;
}

void GraphicsModule::AttachTextureToFBuffer(Texture texture, Framebuffer_ID fBufferID)
{
    m_Renderer.AttachTextureToFramebuffer(texture.Id, fBufferID);
}

void GraphicsModule::SetActiveFrameBuffer(Framebuffer_ID fBufferID)
{
    m_Renderer.SetActiveFBuffer(fBufferID);
    m_Renderer.ClearScreenAndDepthBuffer();
    m_ActiveFrameBuffer = fBufferID;

    if (m_IsSkyboxSet && fBufferID == m_DebugFBuffer)
    {
        //// TEMP: skybox code blech
        //m_Renderer.DisableDepthTesting();

        //m_Renderer.SetActiveShader(m_TexturedMeshShader);
        //m_Renderer.SetActiveCubemap(m_SkyboxCubemap, "SkyBox");

        //m_Renderer.SetActiveShader(m_SkyboxShader);

        //m_Renderer.SetShaderUniformMat4x4f(m_SkyboxShader, "projection", m_Camera->GetProjectionMatrix());

        //Mat4x4f newView = m_Camera->GetViewMatrix();

        //newView[3][0] = 0.0f;
        //newView[3][1] = 0.0f;
        //newView[3][2] = 0.0f;
        //newView[3][3] = 1.0f;

        //newView[0][3] = 0.0f;
        //newView[1][3] = 0.0f;
        //newView[2][3] = 0.0f;

        //m_Renderer.SetShaderUniformMat4x4f(m_SkyboxShader, "view", newView);

        //m_Renderer.SetActiveCubemap(m_SkyboxCubemap, "SkyBox");
        //m_Renderer.DrawMesh(m_SkyboxMesh);

        //m_Renderer.EnableDepthTesting();
    }
}

void GraphicsModule::ResizeFrameBuffer(Framebuffer_ID fBufferID, Vec2i size)
{
    m_Renderer.ResizeFBuffer(fBufferID, size);
}

void GraphicsModule::ResizeGBuffer(GBuffer Buffer, Vec2i Size)
{
    m_Renderer.ResizeFBuffer(Buffer.Buffer, Size);
    m_Renderer.ResizeFBuffer(Buffer.FinalOutput, Size);
    m_Renderer.ResizeFBuffer(Buffer.SkyBuffer, Size);
    m_Renderer.ResizeFBuffer(Buffer.LightBuffer, Size);
    m_Renderer.ResizeFBuffer(Buffer.DebugBuffer, Size);

    m_Renderer.ResizeTexture(Buffer.PositionTex, Size);
    m_Renderer.ResizeTexture(Buffer.NormalTex, Size);
    m_Renderer.ResizeTexture(Buffer.AlbedoTex, Size);
    m_Renderer.ResizeTexture(Buffer.MetallicTex, Size);
    m_Renderer.ResizeTexture(Buffer.RoughnessTex, Size);
    m_Renderer.ResizeTexture(Buffer.AOTex, Size);
    m_Renderer.ResizeTexture(Buffer.DebugTex, Size);

    m_Renderer.ResizeTexture(Buffer.SkyTex, Size);
    m_Renderer.ResizeTexture(Buffer.LightTex, Size);
}

void GraphicsModule::ResetFrameBuffer()
{
    if (m_IsDebugDrawAttachedToFBuffer && m_ActiveFrameBuffer == m_DebugFBuffer)
    {
        DrawDebugDrawMesh(*m_Camera);
    }
    m_Renderer.ResetToScreenBuffer();
}

//Material GraphicsModule::CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap, Texture MetallicMap, Texture AOMap, Texture HeightMap)
//{
//    Material Result = Material(AlbedoMap, NormalMap, RoughnessMap, MetallicMap, AOMap, HeightMap);
//    return Result;
//}

Material GraphicsModule::CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap, Texture MetallicMap, Texture AOMap)
{
    Material Result = Material(AlbedoMap, NormalMap, RoughnessMap, MetallicMap, AOMap);
    return Result;
}

Material GraphicsModule::CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap, Texture MetallicMap)
{
    Material Result = Material(AlbedoMap, NormalMap, RoughnessMap, MetallicMap, m_DefaultAOMap);
    return Result;
}

Material GraphicsModule::CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap)
{
    Material Result = Material(AlbedoMap, NormalMap, RoughnessMap, m_DefaultMetallicMap, m_DefaultAOMap);
    return Result;
}

Material GraphicsModule::CreateMaterial(Texture AlbedoMap, Texture NormalMap)
{
    Material Result = Material(AlbedoMap, NormalMap, m_DefaultRoughnessMap, m_DefaultMetallicMap, m_DefaultAOMap);
    return Result;
}

Material GraphicsModule::CreateMaterial(Texture AlbedoMap)
{
    Material Result = Material(AlbedoMap, m_DefaultNormalMap, m_DefaultRoughnessMap, m_DefaultMetallicMap, m_DefaultAOMap);
    return Result;
}

Model GraphicsModule::CreateModel(TexturedMesh texturedMesh)
{
    return Model(texturedMesh);
}

Model GraphicsModule::CloneModel(const Model& original)
{
    return Model(original.m_TexturedMeshes[0]);
}

Model GraphicsModule::CreateBoxModel(AABB box)
{
    return CreateBoxModel(box, m_DebugMaterial);
}

Model GraphicsModule::CreateBoxModel(AABB box, Material material)
{
    Vec3f min = box.min;
    Vec3f max = box.max;

    Vec3f size = (box.max - box.min) / 2.0f;

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

    Vec3f averagePoint = Vec3f();
    for (int i = 0; i < 8; ++i)
    {
        averagePoint += points[i];
    }
    averagePoint = averagePoint / 8.0f;

    for (int i = 0; i < 8; ++i)
    {
        points[i] -= averagePoint;
    }

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

        points[0].x, points[0].y, points[0].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.y, 0.0f,
        points[1].x, points[1].y, points[1].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f,   0.0f,
        points[5].x, points[5].y, points[5].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     0.0f,   size.z,
        points[4].x, points[4].y, points[4].z,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,     size.y, size.z,

        points[2].x, points[2].y, points[2].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.y, 0.0f,
        points[3].x, points[3].y, points[3].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f,   0.0f,
        points[7].x, points[7].y, points[7].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     0.0f,   size.z,
        points[6].x, points[6].y, points[6].z,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f, 1.0f, 1.0f,     size.y, size.z,

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

    StaticMesh_ID boxMeshId = m_Renderer.LoadMesh(m_TexturedMeshFormat, vertices, indices);

    StaticMesh boxMesh;
    boxMesh.Id = boxMeshId;
    boxMesh.LoadedFromFile = false;

    Model result = Model(TexturedMesh(boxMesh, material));
    result.GetTransform().SetPosition(averagePoint);
    result.Type = ModelType::BLOCK;

    return result;
}

Model GraphicsModule::CreatePlaneModel(Vec2f min, Vec2f max, float elevation, int subsections)
{
    return CreatePlaneModel(min, max, m_DebugMaterial, elevation, subsections);
}

Model GraphicsModule::CreatePlaneModel(Vec2f min, Vec2f max, Material material, float elevation, int subsections)
{
    int Rows = subsections;
    int Columns = subsections;

    float Width = max.x - min.x;
    float Height = max.y - min.y;

    Vec3f SouthWest = Vec3f(min.x, min.y, elevation);
    Vec3f NorthWest = Vec3f(min.x, max.y, elevation);
    Vec3f NorthEast = Vec3f(max.x, max.y, elevation);
    Vec3f SouthEast = Vec3f(max.x, min.y, elevation);

    Vec3f AveragePoint = Vec3f((min.x + max.x) / 2.0f, (min.y + max.y) / 2.0f, elevation);

    std::vector<float> Vertices;
    for (int y = 0; y < Rows + 1; ++y)
    {
        for (int x = 0; x < Columns + 1; ++x)
        {
            float NewX = min.x + (x * Width / Rows);
            float NewY = min.y + (y * Height / Columns);
            float NewZ = elevation;

            float NormX = 0.0f;
            float NormY = 0.0f;
            float NormZ = 1.0f;

            float ColR = 1.0f;
            float ColG = 1.0f;
            float ColB = 1.0f;
            float ColA = 1.0f;

            float TexU = (x * Width / Rows) / 2.0f;
            float TexV = (y * Height / Columns) / 2.0f;

            Vec3f NewPoint = Vec3f(NewX, NewY, NewZ) - AveragePoint;
            Vertices.insert(Vertices.end(), { NewPoint.x, NewPoint.y, NewPoint.z, NormX, NormY, NormZ, ColR, ColG, ColB, ColA, TexU, TexV});
        }
    }

    std::vector<ElementIndex> Indices;

    for (int y = 0; y < Rows; ++y)
    {
        for (int x = 0; x < Columns; ++x)
        {
            unsigned int FlatIndex = (y * (Columns + 1)) + x;


            Indices.insert(Indices.end(), { FlatIndex, FlatIndex + (Columns + 1), FlatIndex + (Columns + 2) });
            Indices.insert(Indices.end(), { FlatIndex, FlatIndex + (Columns + 2), FlatIndex + 1 });
        }
    }

    StaticMesh_ID planeMeshId = m_Renderer.LoadMesh(m_TexturedMeshFormat, Vertices, Indices);

    StaticMesh planeMesh;
    planeMesh.Id = planeMeshId;
    planeMesh.LoadedFromFile = false;

    Model result = Model(TexturedMesh(planeMesh, material));
    result.GetTransform().SetPosition(AveragePoint);
    result.Type = ModelType::PLANE;

    return result;
}

void GraphicsModule::RecalculateTerrainModelNormals(Model& model)
{
    if (model.Type != ModelType::PLANE)
    {
        Engine::Error("Tried to recalculate normals on a non-terrain model");
        return;
    }

    StaticMesh_ID MeshId = model.m_TexturedMeshes[0].m_Mesh.Id;

    std::vector<Vertex*> Vertices = m_Renderer.MapMeshVertices(MeshId);

    // TODO(fraser): Assumption: plane meshes have same vertex width/height. Will need to store width/height info in model
    int Width = (int)sqrt(Vertices.size());
    int Height = (int)sqrt(Vertices.size());

    for (int i = 0; i < Vertices.size(); ++i)
    {
        int x = i % Width;
        int y = i / Width;

        Vec3f North = Vertices[i]->position + Vec3f(0.0f, 1.0f, 0.0f);
        Vec3f South = Vertices[i]->position + Vec3f(0.0f, -1.0f, 0.0f);
        Vec3f East = Vertices[i]->position + Vec3f(1.0f, 0.0f, 0.0f);
        Vec3f West = Vertices[i]->position + Vec3f(-1.0f, 0.0f, 0.0f);

        if (x != 0)
        {
            West = Vertices[i - 1]->position;
        }
        if (x != Width - 1)
        {
            East = Vertices[i + 1]->position;
        }
        if (y != 0)
        {
            South = Vertices[i - Width]->position;
        }
        if (y != Height - 1)
        {
            North = Vertices[i + Width]->position;
        }

        Vec3f PlaneNorm0 = -Math::cross(North - East, North - West);
        PlaneNorm0 = Math::normalize(PlaneNorm0);

        Vec3f PlaneNorm1 = -Math::cross(South - West, South - East);
        PlaneNorm1 = Math::normalize(PlaneNorm1);

        Vertices[i]->normal = Math::normalize((PlaneNorm0 + PlaneNorm1) / 2.0f);

    }

    m_Renderer.UnmapMeshVertices(MeshId);
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
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Material.m_Normal.Id, "NormalMap");
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Material.m_Albedo.Id, "AlbedoMap");
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Material.m_Metallic.Id, "MetallicMap");
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Material.m_Roughness.Id, "RoughnessMap");
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Material.m_AO.Id, "AOMap");
            m_Renderer.DrawMesh(model.m_TexturedMeshes[i].m_Mesh.Id);
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
            m_Renderer.SetActiveTexture(model.m_TexturedMeshes[i].m_Material.m_Albedo.Id, "AlbedoMap");
            m_Renderer.DrawMesh(model.m_TexturedMeshes[i].m_Mesh.Id);
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
        DrawDebugDrawMesh(*m_Camera);
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
        std::vector<Vertex*> vertices = m_Renderer.MapMeshVertices(model.m_TexturedMeshes[i].m_Mesh.Id);
        std::vector<unsigned int*> indices = m_Renderer.MapMeshElements(model.m_TexturedMeshes[i].m_Mesh.Id);
        for (int j = 0; j < indices.size(); j += 3)
        {
            DebugDrawLine(vertices[*indices[j]]->position * modelTransform, vertices[*indices[(size_t)j + 1]]->position * modelTransform, colour);

            DebugDrawLine(vertices[*indices[(size_t)j + 1]]->position * modelTransform, vertices[*indices[(size_t)j + 2]]->position * modelTransform, colour);

            DebugDrawLine(vertices[*indices[(size_t)j + 2]]->position * modelTransform, vertices[*indices[j]]->position * modelTransform, colour);
        }
        m_Renderer.UnmapMeshVertices(model.m_TexturedMeshes[i].m_Mesh.Id);
        m_Renderer.UnmapMeshElements(model.m_TexturedMeshes[i].m_Mesh.Id);
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

void GraphicsModule::DebugDrawSphere(Vec3f p, float radius /*= 1.0f*/, Vec3f colour /*= Vec3f(1.0f, 1.0f, 1.0f)*/)
{
    Vec3f Top = p + Vec3f(0.0f, 0.0f, 1.0f) * radius;
    Vec3f Bot = p + Vec3f(0.0f, 0.0f, -1.0f) * radius;
    Vec3f Left = p + Vec3f(1.0f, 0.0f, 0.0f) * radius;
    Vec3f Right = p + Vec3f(-1.0f, 0.0f, 0.0f) * radius;
    Vec3f Back = p + Vec3f(0.0f, -1.0f, 0.0f) * radius;
    Vec3f Front = p + Vec3f(0.0f, 1.0f, 0.0f) * radius;

    Vec3f CenterPoints[10];

    for (int i = 0; i < 10; ++i)
    {
        CenterPoints[i] = p + Math::rotate(Vec3f(radius, 0.0f, 0.0f), i * (3.1415f * 2.0f / 10.0f), Vec3f(0.0f, 0.0f, 1.0f));
    }

    for (int i = 0; i < 10; ++i)
    {
        if (i == 9)
        {
            DebugDrawLine(CenterPoints[i], CenterPoints[0], colour);
        }
        else
        {
            DebugDrawLine(CenterPoints[i], CenterPoints[i + 1], colour);
        }
    }

    //DebugDrawPoint(Top, colour);
    //DebugDrawPoint(Bot, colour);
    //DebugDrawPoint(Left, colour);
    //DebugDrawPoint(Right, colour);
    //DebugDrawPoint(Back, colour);
    //DebugDrawPoint(Front, colour);

    //DebugDrawLine(Top, Left, colour);
    //DebugDrawLine(Top, Right, colour);
    //DebugDrawLine(Top, Back, colour);
    //DebugDrawLine(Top, Front, colour);

    //DebugDrawLine(Bot, Left, colour);
    //DebugDrawLine(Bot, Right, colour);
    //DebugDrawLine(Bot, Back, colour);
    //DebugDrawLine(Bot, Front, colour);

    //DebugDrawLine(Back, Left, colour);
    //DebugDrawLine(Left, Front, colour);
    //DebugDrawLine(Front, Right, colour);
    //DebugDrawLine(Right, Back, colour);

}

Vec2i GraphicsModule::GetViewportSize()
{
    return m_Renderer.GetViewportSize();
}

void GraphicsModule::SetRenderMode(RenderMode mode)
{
    m_RenderMode = mode;
}

void GraphicsModule::DrawDebugDrawMesh(Camera cam)
{
    m_Renderer.ClearMesh(m_DebugDrawMesh);
    m_Renderer.SetActiveShader(m_DebugLineShader);
    if (m_DebugLineMap.size() > 0)
    {
        m_Renderer.SetShaderUniformMat4x4f(m_DebugLineShader, "Camera", cam.GetCamMatrix());
        for (auto& it : m_DebugLineMap)
        {
            m_Renderer.UpdateMeshData(m_DebugDrawMesh, m_DebugVertFormat, it.second);
            m_Renderer.SetShaderUniformVec3f(m_DebugLineShader, "Colour", it.first);
            m_Renderer.DrawMesh(m_DebugDrawMesh);
        }
    }
}

MeshData GraphicsModule::GetVertexDataForQuad()
{
    std::vector<float> vertices =
    {
        // Position                 // UV
        -1.0f, -1.0f,               0.0f, 0.0f,
        -1.0f, 1.0f,                0.0f, 1.0f,
        1.0f, 1.0f,                 1.0f, 1.0f,
        1.0f, -1.0f,                1.0f, 0.0f,
    };

    std::vector<ElementIndex> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    return MeshData(vertices, indices);
}

MeshData GraphicsModule::GetVertexDataFor3DQuad()
{
    std::vector<float> vertices =
    {
        // Position             // Normals          // Colours                  // UV
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,      0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,     0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,       0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,      0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 1.0f,     1.0f, 0.0f,
    };

    std::vector<ElementIndex> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    return MeshData(vertices, indices);
}

void GraphicsModule::Resize(Vec2i newSize)
{
    m_Renderer.ResetToScreenBuffer();
}

Texture_ID GraphicsModule::GetLightTexture()
{
    return m_LightTexture;
}
