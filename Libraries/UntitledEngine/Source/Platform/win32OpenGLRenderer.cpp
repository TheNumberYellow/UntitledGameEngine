#include "EnginePlatform.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

#include <gl/gl.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include <assert.h>
#include <time.h> 
#include "RendererPlatform.h"

// TODO(fraser): Use another image loading library or something (million warnings) - or make my own!
#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 26451 6385 6011 6262 6308)
#include <stb_image.h>
#pragma warning(pop)

#define MAX_SHADER_VARIABLE_NAME_SIZE 40

VertexBufferFormat::VertexBufferFormat(std::initializer_list<VertAttribute> vertAttributes)
    : _attributes(vertAttributes)
    , _vertexStride(0)
{
    for (auto it = vertAttributes.begin(); it != vertAttributes.end(); ++it)
    {
        _vertexStride += GetSize(*it);
    }
}

void VertexBufferFormat::EnableVertexAttributes() const
{
    size_t offset = 0;
    for (int i = 0; i < _attributes.size(); ++i)
    {
        glEnableVertexAttribArray(i);

        GLint type;
        switch (_attributes[i])
        {
        case VertAttribute::Int:
            type = GL_INT;
            break;
        case VertAttribute::UInt:
            type = GL_UNSIGNED_INT;
            break;
        case VertAttribute::Float:
        case VertAttribute::Vec2f:
        case VertAttribute::Vec3f:
        case VertAttribute::Vec4f:
            type = GL_FLOAT;
            break;
        default:
            type = GL_FLOAT;
            break;
        }

        glVertexAttribPointer(i, GetCount(_attributes[i]), type, GL_FALSE, _vertexStride, (void*)offset);
        offset += GetSize(_attributes[i]);
    }
}

const std::vector<VertAttribute>& VertexBufferFormat::GetAttributes() const
{
    return _attributes;
}

unsigned int VertexBufferFormat::GetVertexStride() const
{
    return _vertexStride;
}

unsigned int VertexBufferFormat::GetCount(VertAttribute vertAttribute) const
{
    switch (vertAttribute)
    {
    case VertAttribute::Int:
    case VertAttribute::UInt:
    case VertAttribute::Float:
        return 1;
    case VertAttribute::Vec2f:
        return 2;
    case VertAttribute::Vec3f:
        return 3;
    case VertAttribute::Vec4f:
        return 4;
    default:
        return 0;
    }
}

unsigned int VertexBufferFormat::GetSize(VertAttribute vertAttribute) const
{
    switch (vertAttribute)
    {
    case VertAttribute::Int:
        return sizeof(int);
    case VertAttribute::UInt:
        return sizeof(unsigned int);
    case VertAttribute::Float:
    case VertAttribute::Vec2f:
    case VertAttribute::Vec3f:
    case VertAttribute::Vec4f:
        return sizeof(float) * GetCount(vertAttribute);
    default:
        return 0;
    }
}

void PosVertex::ActivateVertexAttributes()
{
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);
}

void Vertex::ActivateVertexAttributes()
{
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2); // Colour
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glEnableVertexAttribArray(3); // UV
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(10 * sizeof(GLfloat)));
}

void UVVertex::ActivateVertexAttributes()
{
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1); // UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

namespace
{
    template <typename Output, typename Input>
    std::vector<Output> FlattenVerticesToType(std::vector<typename Input> vertices)
    {
        size_t inputSize = sizeof(Input);
        size_t outputSize = sizeof(Output);

        assert(inputSize >= outputSize && "Input vertex data has vertices smaller than the size of given output");
        assert(inputSize % outputSize == 0 && "Input vertex data does not cleanly divide into given output size");
     
        std::vector<Output> result;

        int inputToOutputRatio = (int)inputSize / (int)outputSize;

        Input* inputPtr = vertices.data();

        for (int i = 0; i < vertices.size(); ++i)
        {
            Output* outputPtr = (Output*)inputPtr;

            for (int j = 0; j < inputToOutputRatio; ++j)
            {
                result.push_back(*outputPtr);
                outputPtr++;
            }

            inputPtr++;
        }

        return result;
    }

    bool CheckShaderCompilation(GLuint shader)
    {

        GLint compilationSuccess = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compilationSuccess);

        if (compilationSuccess == GL_FALSE) {
            GLint logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            std::vector<char> errorLog(logLength);
            glGetShaderInfoLog(shader, logLength, &logLength, &(errorLog[0]));

            glDeleteShader(shader);
            std::string log = &(errorLog[0]);

            Engine::DEBUGPrint(log.c_str());

        }
        return compilationSuccess;
    }

    struct GLAttribSize
    {
        GLAttribSize() : count(0), size(0) {};
        GLAttribSize(GLint count, GLsizei size) : count(count), size(size) {};
        GLint count;
        GLsizei size;
    };

    GLAttribSize GetSizeFromGLTypeEnum(GLenum e)
    {
        switch (e)
        {
        case GL_FLOAT: return GLAttribSize(1, sizeof(GLfloat));
        case GL_FLOAT_VEC2: return GLAttribSize(2, sizeof(GLfloat) * 2);
        case GL_FLOAT_VEC3: return GLAttribSize(3, sizeof(GLfloat) * 3);
        case GL_FLOAT_VEC4: return GLAttribSize(4, sizeof(GLfloat) * 4);
        default: return GLAttribSize(0, 0);
        }
    }

    int GetSizeFromVertexType(VertType type)
    {
        int result = 0;
        switch (type)
        {
        case VertType::Normal:
            result = sizeof(Vertex);
            break;
        case VertType::Pos:
            result = sizeof(PosVertex);
            break;
        case VertType::UV:
            result = sizeof(UVVertex);
            break;
        default:
            break;
        }
        return result;
    }

    GLenum ColourFormatToGLFormat(ColourFormat format)
    {
        switch (format)
        {
        case ColourFormat::RGB:
            return GL_RGB;
        case ColourFormat::RGBA:
            return GL_RGBA;
        case ColourFormat::Red:
            return GL_RED;
        case ColourFormat::DEPTH:
            return GL_DEPTH_COMPONENT;
        default:
            return GL_RGBA;
        }
    }

    GLenum DataFormatToGLFormat(DataFormat format)
    {
        switch (format)
        {
        case DataFormat::FLOAT:
            return GL_FLOAT;
        case DataFormat::UNSIGNED_BYTE:
            return GL_UNSIGNED_BYTE;
        default:
            return GL_FLOAT;
        }
    }

    GLint GetUniformLocation(std::unordered_map<std::string, GLint>& uniformMap, std::string key)
    {
        auto it = uniformMap.find(key);
        if (it != uniformMap.end())
        {
            return it->second;
        }
        else
        {
            Engine::DEBUGPrint("ERROR: Could not find uniform <" + key + ">. It may have been optimized out by OpenGL.");
            return -1;
        }
    }

    GLint GetSamplerLocation(std::unordered_map<std::string, GLint>& samplerMap, std::string key)
    {
        auto it = samplerMap.find(key);
        if (it != samplerMap.end())
        {
            return it->second;
        }
        else
        {
            Engine::DEBUGPrint("ERROR: Could not find sampler <" + key + ">. It may have been optimized out by OpenGL.");
            return -1;
        }
    }

    struct GLAttribInfo
    {
        GLAttribInfo() {};


    };

    struct OpenGLFBuffer
    {
        OpenGLFBuffer(Vec2i size, FBufferFormat format) 
            : size(size)
            , format(format)
        {
            glGenFramebuffers(1, &fbo);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);



            if (format == FBufferFormat::DEPTH)
            {
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                // Temp(fraser): this is specifically for shadow maps, technically this should be a construction parameter
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);

            }
            else if (format == FBufferFormat::COLOUR)
            {
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                GLfloat max_anisotropy, value = 16.0f; /* don't exceed this value...*/
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);

                value = (value > max_anisotropy) ? max_anisotropy : value;
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            
                // For colour buffers, we also need to create a render buffer to hold depth and stencil information (these could be attached
                glGenRenderbuffers(1, &rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            }
            else if (format == FBufferFormat::EMPTY)
            {
                // We'll assume that empty fbuffers will have color attachments added later, so just add the render buffer to allow depth/stencil info now
                // FOUND BUG: TEXTURE IS NOT SET SO WHEN I GO TO DELETE IT LATER IT DELETES texture WHICH IS UNSET - WHICH MIGHT BE ANOTHER TEXTURE!!!
                glGenRenderbuffers(1, &rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);

                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            }
           
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        OpenGLFBuffer(Vec2i size, FBufferFormat format, GLuint renderBuffer)
            : size(size)
            , format(format)
            , rbo(renderBuffer)
        {
            glGenFramebuffers(1, &fbo);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        std::vector<unsigned int> attachments;
        GLuint texture;
        GLuint fbo;
        GLuint rbo;
        Vec2i size;
        FBufferFormat format;
    };

    struct OpenGLTexture
    {
        OpenGLTexture(Vec2i size, std::vector<unsigned char> textureData, ColourFormat format, TextureMode minTexMode, TextureMode magTexMode)
        {
            GLenum glMinTextureMode = 0;
            GLenum glMagTextureMode = 0;

            switch (minTexMode)
            {
            case TextureMode::LINEAR:
                glMinTextureMode = GL_LINEAR_MIPMAP_NEAREST;
                break;
            case TextureMode::NEAREST:
                glMinTextureMode = GL_NEAREST;
                break;
            }

            switch (magTexMode)
            {
            case TextureMode::LINEAR:
                glMagTextureMode = GL_LINEAR;
                break;
            case TextureMode::NEAREST:
                glMagTextureMode = GL_NEAREST;
                break;
            }

            glBindTexture(GL_TEXTURE_2D, 0);

            glGenTextures(1, &texture);

            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMinTextureMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagTextureMode);

            float anisotropic = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic);

            switch (format)
            {
            case ColourFormat::RGB:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
                createInfo = TextureCreateInfo(size, format, format, DataFormat::UNSIGNED_BYTE);
                break;
            case ColourFormat::RGBA:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
                createInfo = TextureCreateInfo(size, format, format, DataFormat::UNSIGNED_BYTE);
                break;
            case ColourFormat::Red:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size.x, size.y, 0, GL_RED, GL_UNSIGNED_BYTE, textureData.data());
                createInfo = TextureCreateInfo(size, format, format, DataFormat::UNSIGNED_BYTE);
                break;
            case ColourFormat::DEPTH:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, textureData.data());
                createInfo = TextureCreateInfo(size, format, format, DataFormat::FLOAT);
                break;
            }

            if (!glIsTexture(texture))
            {
                Engine::Error("Failed to create texture");
            }

            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);


        }
        
        OpenGLTexture(std::string filePath, TextureMode minTexMode, TextureMode magTexMode)
        {
            GLenum glMinTextureMode = 0;
            GLenum glMagTextureMode = 0;

            switch (minTexMode)
            {
            case TextureMode::LINEAR:
                glMinTextureMode = GL_LINEAR_MIPMAP_LINEAR;
                break;
            case TextureMode::NEAREST:
                glMinTextureMode = GL_NEAREST;
                break;
            }

            switch (magTexMode)
            {
            case TextureMode::LINEAR:
                glMagTextureMode = GL_LINEAR;
                break;
            case TextureMode::NEAREST:
                glMagTextureMode = GL_NEAREST;
                break;
            }

            glBindTexture(GL_TEXTURE_2D, 0);

            glGenTextures(1, &texture);

            glBindTexture(GL_TEXTURE_2D, texture);

            stbi_set_flip_vertically_on_load(true);

            int width, height, channels;
            unsigned char* textureData = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMinTextureMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMagTextureMode);

            float anisotropic = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic);

            switch (channels)
            {
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, textureData);
                createInfo = TextureCreateInfo(Vec2i(width, height), ColourFormat::Red, ColourFormat::Red, DataFormat::UNSIGNED_BYTE);
                break;
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
                createInfo = TextureCreateInfo(Vec2i(width, height), ColourFormat::RGB, ColourFormat::RGB, DataFormat::UNSIGNED_BYTE);
                break;
            case 4:
            default:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
                createInfo = TextureCreateInfo(Vec2i(width, height), ColourFormat::RGBA, ColourFormat::RGBA, DataFormat::UNSIGNED_BYTE);
                break;

            }
            stbi_image_free(textureData);

            glGenerateMipmap(GL_TEXTURE_2D);

            if (!glIsTexture(texture))
            {
                Engine::FatalError("Failed to create texture");
                return;
            }

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        OpenGLTexture(Vec2i size, ColourFormat format)
        {
            glBindTexture(GL_TEXTURE_2D, 0);

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            GLenum glFormat = 0;

            switch (format)
            {
            case ColourFormat::RGB:
                glFormat = GL_RGB;
                break;
            case ColourFormat::RGBA:
                glFormat = GL_RGBA;
                break;
            case ColourFormat::Red:
                glFormat = GL_RED;
                break;
            case ColourFormat::DEPTH:
                glFormat = GL_DEPTH_COMPONENT;
                break;
            }


            // For now I'll be using a float for depth component textures - 
            // likely will want to give more options to the caller if more of these cases emerge
            if (format == ColourFormat::DEPTH)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, glFormat, size.x, size.y, 0, glFormat, GL_FLOAT, 0);
                createInfo = TextureCreateInfo(size, format, format, DataFormat::FLOAT);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, glFormat, size.x, size.y, 0, glFormat, GL_UNSIGNED_BYTE, 0);
                createInfo = TextureCreateInfo(size, format, format, DataFormat::UNSIGNED_BYTE);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            float anisotropic = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic);

            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        OpenGLTexture(TextureCreateInfo& CreateInfo) :
            createInfo(CreateInfo)
        {
            glBindTexture(GL_TEXTURE_2D, 0);

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            Vec2i Size = CreateInfo.Size;
            GLenum InternalFormat = ColourFormatToGLFormat(CreateInfo.InternalFormat);
            GLenum ExternalFormat = ColourFormatToGLFormat(CreateInfo.ExternalFormat);
            GLenum DataFormat = DataFormatToGLFormat(CreateInfo.DataFormat);

            // TEMP: FORMAT
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Size.x, Size.y, 0, GL_RGBA, DataFormat, 0);

            // TODO(fraser) Specify these parameters via the TextureCreateInfo struct as well
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            float anisotropic = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic);

            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        GLuint texture;
        TextureCreateInfo createInfo;
    };

    struct OpenGLCubemap
    {

        OpenGLCubemap(std::string filepath)
        {
            //temp(fraser) obviously
            std::vector<std::string> faces
            {
                "Assets/textures/right.jpg",
                "Assets/textures/left.jpg",
                
                "Assets/textures/front.jpg",
                "Assets/textures/back.jpg",

                "Assets/textures/top.jpg",
                "Assets/textures/bottom.jpg",
            };

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

            int width, height, nrChannels;

            //stbi_set_flip_vertically_on_load(true);

            for (unsigned int i = 0; i < faces.size(); i++)
            {
                unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
                if (data)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                    );
                    stbi_image_free(data);
                }
                else
                {
                    Engine::FatalError("Cubemap tex failed to load at path : " + faces[i] + "\n");
                    stbi_image_free(data);
                }
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        GLuint texture;
    };

    struct OpenGLShader
    {
        GLuint m_ProgramId;

        std::unordered_map<std::string, GLint> m_UniformLocations;
        std::unordered_map<std::string, GLint> m_AttributeLocations;
        std::unordered_map<std::string, GLint> m_SamplerLocations;

        OpenGLShader(std::string vertShaderSource, std::string fragShaderSource)
        {
            const char* vertCStr = vertShaderSource.c_str();
            const char* fragCStr = fragShaderSource.c_str();

            GLuint vertShaderId = glCreateShader(GL_VERTEX_SHADER);
            GLuint fragShaderId = glCreateShader(GL_FRAGMENT_SHADER);

            glShaderSource(vertShaderId, 1, &vertCStr, nullptr);
            glShaderSource(fragShaderId, 1, &fragCStr, nullptr);

            glCompileShader(vertShaderId);
            glCompileShader(fragShaderId);

            CheckShaderCompilation(vertShaderId);
            CheckShaderCompilation(fragShaderId);

            m_ProgramId = glCreateProgram();
            glAttachShader(m_ProgramId, vertShaderId);
            glAttachShader(m_ProgramId, fragShaderId);

            glLinkProgram(m_ProgramId);

            glValidateProgram(m_ProgramId);

            GLint linked = false;
            glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &linked);

            assert(linked && "Shader program failed to link.");

            glDetachShader(m_ProgramId, vertShaderId);
            glDetachShader(m_ProgramId, fragShaderId);

            glDeleteShader(vertShaderId);
            glDeleteShader(fragShaderId);

            glUseProgram(m_ProgramId);

            // TODO(fraser): THIS WHOLE THING IS PROBABLY UNNCESSARY AND DOOMED TO FAIL
            // Loop through shader attributes and save their locations, and enable each attribute
            {
                GLint numAttributes;
                GLsizei totalStride = 0;
                glGetProgramiv(m_ProgramId, GL_ACTIVE_ATTRIBUTES, &numAttributes);
                std::vector<GLAttribSize> attribInfos;
                attribInfos.resize(numAttributes);

                for (GLint i = 0; i < numAttributes; ++i)
                {
                    GLchar name[MAX_SHADER_VARIABLE_NAME_SIZE + 1];
                    GLint size;
                    GLenum typeEnum;

                    glGetActiveAttrib(m_ProgramId, i, MAX_SHADER_VARIABLE_NAME_SIZE, nullptr, &size, &typeEnum, name);

                    std::string nameStr = name;
                    m_AttributeLocations[nameStr] = i;

                    attribInfos[i] = GetSizeFromGLTypeEnum(typeEnum);
                    attribInfos[i].size *= size;
                    totalStride += attribInfos[i].size;
                }
                GLsizei currentOffset = 0;
                for (GLint i = 0; i < numAttributes; ++i)
                {
                    if (i != 0) currentOffset += attribInfos[i].size;
                }
            }

            // Loop through shader uniforms and save their locations
            {
                GLint numUniforms;
                glGetProgramiv(m_ProgramId, GL_ACTIVE_UNIFORMS, &numUniforms);

                int texIndex = 0;

                for (GLint i = 0; i < numUniforms; ++i)
                {
                    GLchar name[MAX_SHADER_VARIABLE_NAME_SIZE + 1];
                    GLint size;
                    GLenum typeEnum;

                    glGetActiveUniform(m_ProgramId, i, MAX_SHADER_VARIABLE_NAME_SIZE, nullptr, &size, &typeEnum, name);

                    GLint uniformLoc = glGetUniformLocation(m_ProgramId, name);

                    m_UniformLocations[name] = uniformLoc;


                    //if (i != testLoc)
                    //{ 
                    //    Engine::Error("What");
                    //}

                    // ASSUMPTION: the order that textures are defined in shader code is the same as the order they're gotten in glGetActiveUniform
                    // This might not be true, so I look forward to a fun bug
                    if (typeEnum == GL_SAMPLER_2D || typeEnum == GL_SAMPLER_CUBE)
                    {
                        m_SamplerLocations[name] = texIndex;
                        glUniform1i(uniformLoc, texIndex++);
                    }
                }
            }

            glUseProgram(0);
        }
    };

    struct OpenGLMesh
    {

        OpenGLMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData)
        {
            // TODO: Bunch of options for VAOs, VBOs, etc. Could use one VBO for all meshes using the same shader, 
            // could use same VAO with multiple VBOs with the functions in new opengl versions etc.
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            bufferSize = (int)vertexData.size() * sizeof(float);
            useElementArray = false;

            // TODO: option for dynamic vs static draw (profile difference too)
            glBufferData(GL_ARRAY_BUFFER, bufferSize, vertexData.data(), GL_DYNAMIC_DRAW);

            vertBufFormat.EnableVertexAttributes();

            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            numVertices = (int)vertexData.size() / (vertBufFormat.GetVertexStride() / sizeof(float));
        }

        OpenGLMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices)
        {
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            bufferSize = (int)vertexData.size() * sizeof(float);
            useElementArray = true;

            // TODO: option for dynamic vs static draw (profile difference too)
            glNamedBufferData(VBO, bufferSize, vertexData.data(), GL_DYNAMIC_DRAW);
            glNamedBufferData(EBO, indices.size() * sizeof(ElementIndex), indices.data(), GL_DYNAMIC_DRAW);

            vertBufFormat.EnableVertexAttributes();

            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            numElements = (int)indices.size();
            numVertices = (int)vertexData.size() / (vertBufFormat.GetVertexStride() / sizeof(float));

            //Engine::DEBUGPrint("Created mesh with " + std::to_string(numElements) + " elements and " + std::to_string(numVertices) + " vertices.");
            //Engine::DEBUGPrint("VBO: " + std::to_string(VBO) + ", EBO: " + std::to_string(EBO) + ", VAO: " + std::to_string(VAO));

        }

        OpenGLMesh(const VertexBufferFormat& vertBufFormat, bool useElementArray)
        {
            // TODO: Bunch of options for VAOs, VBOs, etc. Could use one VBO for all meshes using the same shader, 
            // could use same VAO with multiple VBOs with the functions in new opengl versions etc.
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            // Preallocate vertex buffer
            glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

            vertBufFormat.EnableVertexAttributes();

            // Unbind stuff
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            this->numVertices = 0;
            this->numElements = 0;
            this->bufferSize = 0;
            this->useElementArray = useElementArray;
        }

        ~OpenGLMesh()
        {

        }

        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        int numElements;
        int numVertices;
        int bufferSize;
        bool useElementArray;

        DrawType drawType = DrawType::Triangle;
    };

    std::unordered_map<Framebuffer_ID, OpenGLFBuffer> fBufferMap;
    std::unordered_map<Texture_ID, OpenGLTexture> textureMap;
    std::unordered_map<Cubemap_ID, OpenGLCubemap> cubemapMap;
    std::unordered_map<Shader_ID, OpenGLShader> shaderMap;
    std::unordered_map<StaticMesh_ID, OpenGLMesh> meshMap;
    Shader_ID currentlyBoundShader;
    OpenGLShader* currentlyBoundShaderPtr = nullptr;

    HDC deviceContext;
    HGLRC glContext;

    //TEMP (fraser)
    Texture_ID whiteRenderTexture;
    bool renderDebugMesh = false;

    OpenGLFBuffer* GetGLFBufferFromFBufferID(Framebuffer_ID fBufferID)
    {
        auto it = fBufferMap.find(fBufferID);
        if (it != fBufferMap.end())
        {
            return &it->second;
        }
        else
        {
            return nullptr;
        }
    }

    OpenGLTexture* GetGLTextureFromTextureID(Texture_ID textureID)
    {
        auto it = textureMap.find(textureID);
        if (it != textureMap.end())
        {
            return &it->second;
        }
        else 
        {
            return nullptr;
        }
    }
    
    OpenGLCubemap* GetGLCubemapFromCubemapID(Cubemap_ID cubemapID)
    {
        auto it = cubemapMap.find(cubemapID);
        if (it != cubemapMap.end())
        {
            return &it->second;
        }
        else
        {
            return nullptr;
        }
    }

    OpenGLShader* GetGLShaderFromShaderID(Shader_ID shaderID)
    {
        auto it = shaderMap.find(shaderID);
        if (it != shaderMap.end())
        {
            return &it->second;
        }
        else {
            return nullptr;
        }
    }

    OpenGLMesh* GetGLMeshFromMeshID(StaticMesh_ID meshID)
    {
        auto it = meshMap.find(meshID);
        if (it != meshMap.end())
        {
            return &it->second;
        }
        else {
            return nullptr;
        }
    }
}

// Error handling
void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_LOW)
    {
        Engine::DEBUGPrint("Low severity: " + std::string(message) + "\n");
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        Engine::DEBUGPrint("Medium severity: " + std::string(message) + "\n");
    }
    else if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        Engine::DEBUGPrint("High severity: " + std::string(message) + "\n");
    }
    //else
    //{
    //    Engine::DEBUGPrint("Low severity: " + std::string(message) + "\n");
    //}
}

Renderer::Renderer()
{
    HWND window = GetActiveWindow();

    deviceContext = GetDC(window);

    PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
    desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    desiredPixelFormat.nVersion = 1;
    desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    desiredPixelFormat.cColorBits = 32;
    desiredPixelFormat.cDepthBits = 24;
    desiredPixelFormat.cStencilBits = 8;
    desiredPixelFormat.cAlphaBits = 8;
    desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int pixelFormatIndex = ChoosePixelFormat(deviceContext, &desiredPixelFormat);

    if (pixelFormatIndex == 0)
    {
        Engine::DEBUGPrint("Unable to pick pixel format.\n");
        // TODO: Error handling
    }

    SetPixelFormat(deviceContext, pixelFormatIndex, &desiredPixelFormat);

    HGLRC tempGlContext = wglCreateContext(deviceContext);
    wglMakeCurrent(deviceContext, tempGlContext);

    GLenum err = glewInit();
    if (err == GLEW_OK)
    {
        Engine::DEBUGPrint("Glew was initialized.\n");
    }
    else {
        Engine::DEBUGPrint("Glew was not initialized.\n");
    }

    int gl34_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0,
    };

    if (wglewIsSupported("WGL_ARB_create_context") == 1)
    {
        glContext = wglCreateContextAttribsARB(deviceContext, 0, gl34_attribs);
        wglMakeCurrent(deviceContext, 0);
        wglDeleteContext(tempGlContext);
        wglMakeCurrent(deviceContext, glContext);   
    }
    else
    {
        glContext = tempGlContext;
    }

    if (wglewIsSupported("WGL_EXT_swap_control") == 1)
    {
        // VSYNC
        wglSwapIntervalEXT(1);
    }

    // Enable various OpenGL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glStencilFunc(GL_EQUAL, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    // I prefer clockwise winding
    glFrontFace(GL_CW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, Engine::GetClientAreaSize().x, Engine::GetClientAreaSize().y);

    // TODO(fraser): come up with a way for the .lib to store textures I want to be included in the engine (ie. white texture)
    // Will probably write something to load an image into a header file which can be included wherever it's needed
    whiteRenderTexture = LoadTexture("images/white.png");

    // TEMP(fraser)
    glLineWidth(1.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDepthFunc(GL_LEQUAL);


    // Print out opengl version
    GLint major;
    GLint minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::string debugStr = "OpenGL version: " + std::to_string(major) + "." + std::to_string(minor);
    //Engine::Alert(debugStr);
}

Renderer::~Renderer()
{
    wglMakeCurrent(deviceContext, nullptr);
    wglDeleteContext(glContext);
}

Framebuffer_ID Renderer::CreateFrameBuffer(Vec2i size, FBufferFormat format)
{
    OpenGLFBuffer newBuffer = OpenGLFBuffer(size, format);

    Framebuffer_ID newID = frameBufferIDGenerator.Generate();

    fBufferMap.insert(std::pair<Framebuffer_ID, OpenGLFBuffer>(newID, newBuffer));
    return newID;
}

Framebuffer_ID Renderer::CreateFBufferWithExistingDepthBuffer(Framebuffer_ID existingFBuffer, Vec2i size, FBufferFormat format)
{
    OpenGLFBuffer fBuffer = *GetGLFBufferFromFBufferID(existingFBuffer);

    OpenGLFBuffer newBuffer = OpenGLFBuffer(size, format, fBuffer.rbo);
    
    Framebuffer_ID newID = frameBufferIDGenerator.Generate();

    fBufferMap.insert(std::pair<Framebuffer_ID, OpenGLFBuffer>(newID, newBuffer));
    return newID;
}

void Renderer::AttachTextureToFramebuffer(Texture_ID textureID, Framebuffer_ID fBufferID)
{
    OpenGLFBuffer fBuffer = *GetGLFBufferFromFBufferID(fBufferID);
    OpenGLTexture texture = *GetGLTextureFromTextureID(textureID);

    glBindFramebuffer(GL_FRAMEBUFFER, fBuffer.fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Texture_ID Renderer::AttachColourAttachmentToFrameBuffer(Framebuffer_ID buffer, TextureCreateInfo createInfo, int attachmentIndex)
{
    OpenGLFBuffer* fBuffer = GetGLFBufferFromFBufferID(buffer);

    glBindFramebuffer(GL_FRAMEBUFFER, fBuffer->fbo);

    if (attachmentIndex > GL_MAX_COLOR_ATTACHMENTS)
    {
        Engine::FatalError("Unsupported color attachment index");
        return 0;
    }

    OpenGLTexture newTexture = OpenGLTexture(createInfo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_TEXTURE_2D, newTexture.texture, 0);

    fBuffer->attachments.push_back(GL_COLOR_ATTACHMENT0 + attachmentIndex);

    glDrawBuffers((GLsizei)fBuffer->attachments.size(), fBuffer->attachments.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Texture_ID newID = textureIDGenerator.Generate();
    textureMap.insert(std::pair<Texture_ID, OpenGLTexture>(newID, newTexture));
    
    return newID;
}

Texture_ID Renderer::LoadTexture(Vec2i size, std::vector<unsigned char> textureData, ColourFormat format, TextureMode minTexMode, TextureMode magTexMode)
{
    OpenGLTexture newTexture = OpenGLTexture(size, textureData, format, minTexMode, magTexMode);

    Texture_ID newID = textureIDGenerator.Generate();

    textureMap.insert(std::pair<Texture_ID, OpenGLTexture>(newID, newTexture));
    return newID;
}

Texture_ID Renderer::LoadTexture(std::string filePath, TextureMode minTexMode, TextureMode magTexMode)
{
    OpenGLTexture newTexture = OpenGLTexture(filePath, minTexMode, magTexMode);

    Texture_ID newID = textureIDGenerator.Generate();

    textureMap.insert(std::pair<Texture_ID, OpenGLTexture>(newID, newTexture));
    return newID;
}

bool Renderer::IsTextureValid(Texture_ID texID)
{
    OpenGLTexture* tex = GetGLTextureFromTextureID(texID);
    return glIsTexture(tex->texture);
}

#pragma optimize("", off)
bool Renderer::IsFBufferTextureValid(Framebuffer_ID bufID)
{
    OpenGLFBuffer* fBuffer = GetGLFBufferFromFBufferID(bufID);
    return glIsTexture(fBuffer->texture);
}
#pragma optimize("", on)

Cubemap_ID Renderer::LoadCubemap(std::string filepath)
{
    OpenGLCubemap newCubemap = OpenGLCubemap(filepath);

    Cubemap_ID newID = cubemapIDGenerator.Generate();

    cubemapMap.insert(std::pair<Cubemap_ID, OpenGLCubemap>(newID, newCubemap));
    return newID;
}

Shader_ID Renderer::LoadShader(std::string vertShaderSource, std::string fragShaderSource)
{
    OpenGLShader newShader = OpenGLShader(vertShaderSource, fragShaderSource);

    Shader_ID newID = shaderIDGenerator.Generate();

    shaderMap.insert(std::pair<Shader_ID, OpenGLShader>(newID, newShader));
    return newID;
}

StaticMesh_ID Renderer::LoadMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData)
{
    OpenGLMesh newMesh = OpenGLMesh(vertBufFormat, vertexData);

    StaticMesh_ID newID = staticMeshIDGenerator.Generate();

    meshMap.insert(std::pair<StaticMesh_ID, OpenGLMesh>(newID, std::move(newMesh)));
    return newID;
}

StaticMesh_ID Renderer::LoadMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices)
{
    OpenGLMesh newMesh = OpenGLMesh(vertBufFormat, vertexData, indices);

    StaticMesh_ID newID = staticMeshIDGenerator.Generate();

    meshMap.insert(std::pair<StaticMesh_ID, OpenGLMesh>(newID, std::move(newMesh)));
        
    return newID;
}

#pragma optimize("", off)
void Renderer::DeleteFrameBuffer(Framebuffer_ID fBufferID)
{
    OpenGLFBuffer* fBuffer = GetGLFBufferFromFBufferID(fBufferID);

    if (fBuffer)
    {
        glDeleteFramebuffers(1, &(fBuffer->fbo));
        glDeleteRenderbuffers(1, &(fBuffer->rbo));
        if (fBuffer->format != FBufferFormat::EMPTY)
        {
            glDeleteTextures(1, &(fBuffer->texture));
        }

        fBufferMap.erase(fBufferID);
    }

    frameBufferIDGenerator.FreeID(fBufferID);
}
#pragma optimize("", on)

void Renderer::DeleteTexture(Texture_ID textureID)
{
    OpenGLTexture* texture = GetGLTextureFromTextureID(textureID);

    if (texture)
    {
        glDeleteTextures(1, &(texture->texture));

        textureMap.erase(textureID);
    }

    textureIDGenerator.FreeID(textureID);
}

void Renderer::DeleteMesh(StaticMesh_ID meshID)
{
    OpenGLMesh* mesh = GetGLMeshFromMeshID(meshID);

    if (mesh)
    {
        glDeleteVertexArrays(1, &mesh->VAO);
        glDeleteBuffers(1, &mesh->VBO);
        glDeleteBuffers(1, &mesh->EBO);

        meshMap.erase(meshID);
    }

    staticMeshIDGenerator.FreeID(meshID);
}

Texture_ID Renderer::CreateEmptyTexture(Vec2i size, ColourFormat format)
{
    OpenGLTexture newTexture = OpenGLTexture(size, format);

    Texture_ID newID = textureIDGenerator.Generate();

    textureMap.insert(std::pair<Texture_ID, OpenGLTexture>(newID, newTexture));
    return newID;
}

StaticMesh_ID Renderer::CreateEmptyMesh(const VertexBufferFormat& vertBufFormat, bool useElementArray)
{
    StaticMesh_ID newID = staticMeshIDGenerator.Generate();
    
    OpenGLMesh newMesh = OpenGLMesh(vertBufFormat, useElementArray);

    meshMap.insert(std::pair<StaticMesh_ID, OpenGLMesh>(newID, newMesh));
    return newID;
}

void Renderer::ClearMesh(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    glInvalidateBufferData(mesh.VBO);
    glInvalidateBufferData(mesh.EBO);
    
    mesh.numVertices = 0;
    mesh.numElements = 0;
}

void Renderer::UpdateTextureData(Texture_ID textureID, Recti region, std::vector<unsigned char> textureData, ColourFormat format)
{
    OpenGLTexture texture = *GetGLTextureFromTextureID(textureID);

    GLenum glFormat = 0;

    switch (format)
    {
    case ColourFormat::RGB:
        glFormat = GL_RGB;
        break;
    case ColourFormat::RGBA:
        glFormat = GL_RGBA;
        break;
    case ColourFormat::Red:
        glFormat = GL_RED;
        break;
    }

    glTextureSubImage2D(texture.texture, 0, region.location.x, region.location.y, region.size.x, region.size.y, glFormat, GL_UNSIGNED_BYTE, textureData.data());
}

void Renderer::UpdateMeshData(StaticMesh_ID meshID, const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData)
{
    OpenGLMesh* mesh = GetGLMeshFromMeshID(meshID);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    mesh->bufferSize = (int)vertexData.size() * sizeof(float);
    
    glBufferData(GL_ARRAY_BUFFER, mesh->bufferSize, vertexData.data(), GL_STATIC_DRAW);

    vertBufFormat.EnableVertexAttributes();

    glBindVertexArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->numVertices = (int)vertexData.size() / (vertBufFormat.GetVertexStride() / sizeof(float));
}

void Renderer::UpdateMeshData(StaticMesh_ID meshID, const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices)
{
    OpenGLMesh* mesh = GetGLMeshFromMeshID(meshID);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);

    mesh->bufferSize = (int)vertexData.size() * sizeof(float);

    glBufferData(GL_ARRAY_BUFFER, mesh->bufferSize, vertexData.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)indices.size() * sizeof(ElementIndex), indices.data(), GL_STATIC_DRAW);

    vertBufFormat.EnableVertexAttributes();

    glBindVertexArray(0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    mesh->numElements = (int)indices.size();
    mesh->numVertices = (int)vertexData.size() / (vertBufFormat.GetVertexStride() / sizeof(float));

    // Test: Memory leak?
    //float* bufferData = new float[mesh->bufferSize];
    //glGetNamedBufferSubData(mesh->VBO, 0, mesh->bufferSize, (void*)bufferData);
    //delete[] bufferData;
}

std::vector<float> Renderer::GetMeshVertexData(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    float* vertexBuffer = new float[mesh.bufferSize / sizeof(float)];
    glGetNamedBufferSubData(mesh.VBO, 0, mesh.bufferSize, (void*)vertexBuffer);
    std::vector<float> vertices;
    for (int i = 0; i < mesh.bufferSize / sizeof(float); ++i)
    {
        vertices.push_back(vertexBuffer[i]);
    }

    delete[] vertexBuffer;

    return vertices;
}

std::vector<unsigned int> Renderer::GetMeshIndexData(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    unsigned int* elementBuffer = new unsigned int[mesh.numElements];
    glGetNamedBufferSubData(mesh.EBO, 0, mesh.numElements * sizeof(unsigned int), (void*)elementBuffer);
    std::vector<unsigned int> elements;
    for (int i = 0; i < mesh.numElements; ++i)
    {
        elements.push_back(elementBuffer[i]);
    }

    delete[] elementBuffer;

    return elements;
}

void Renderer::SetActiveFBuffer(Framebuffer_ID fBufferID)
{
    OpenGLFBuffer* bufferPtr = GetGLFBufferFromFBufferID(fBufferID);

    glBindFramebuffer(GL_FRAMEBUFFER, bufferPtr->fbo);
    glViewport(0, 0, bufferPtr->size.x, bufferPtr->size.y);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Engine::DEBUGPrint("WTF\n");
    }
}

void Renderer::ResizeFBuffer(Framebuffer_ID fBufferID, Vec2i newSize)
{
    OpenGLFBuffer* bufferPtr = GetGLFBufferFromFBufferID(fBufferID);

    // I'm not actually deleting the old framebuffer, texture, or renderbuffer...
    // I've read this can cause issues but it hasn't for me so '_>'
    if (bufferPtr)
    {
        bufferPtr->size = newSize;

        glBindFramebuffer(GL_FRAMEBUFFER, bufferPtr->fbo);

        if (bufferPtr->format == FBufferFormat::DEPTH)
        {
            glBindTexture(GL_TEXTURE_2D, bufferPtr->texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, newSize.x, newSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        if (bufferPtr->format == FBufferFormat::COLOUR)
        {
            glBindTexture(GL_TEXTURE_2D, bufferPtr->texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newSize.x, newSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glBindRenderbuffer(GL_RENDERBUFFER, bufferPtr->rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, newSize.x, newSize.y);
        }
        if (bufferPtr->format == FBufferFormat::EMPTY)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, bufferPtr->rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, newSize.x, newSize.y);
        }
    }
}

void Renderer::ResetToScreenBuffer()
{
    glViewport(0, 0, Engine::GetClientAreaSize().x, Engine::GetClientAreaSize().y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetActiveTexture(Texture_ID texture, unsigned int textureSlot)
{
    OpenGLTexture* texturePtr = GetGLTextureFromTextureID(texture);

    if (!glIsTexture(texturePtr->texture))
    {
        Engine::DEBUGPrint("Invalid texture");
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_2D, texturePtr->texture);
}

void Renderer::SetActiveTexture(Texture_ID textureID, std::string textureName)
{
    OpenGLShader* shaderPtr = GetGLShaderFromShaderID(currentlyBoundShader);

    GLint sampler = GetSamplerLocation(shaderPtr->m_SamplerLocations, textureName);

    if (sampler >= 0)
    {
        SetActiveTexture(textureID, shaderPtr->m_SamplerLocations[textureName]);
    }
}

void Renderer::ResizeTexture(Texture_ID textureID, Vec2i newSize)
{
    OpenGLTexture* texturePtr = GetGLTextureFromTextureID(textureID);

    TextureCreateInfo CreateInfo = texturePtr->createInfo;

    GLenum InternalFormat = ColourFormatToGLFormat(CreateInfo.InternalFormat);
    GLenum ExternalFormat = ColourFormatToGLFormat(CreateInfo.ExternalFormat);
    GLenum DataFormat = DataFormatToGLFormat(CreateInfo.DataFormat);

    glBindTexture(GL_TEXTURE_2D, texturePtr->texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, newSize.x, newSize.y, 0, GL_RGBA, DataFormat, nullptr);
}

void Renderer::SetActiveFBufferTexture(Framebuffer_ID frameBufferID, unsigned int textureSlot)
{
    OpenGLFBuffer* bufferPtr = GetGLFBufferFromFBufferID(frameBufferID);

    if (!glIsTexture(bufferPtr->texture))
    {
        Engine::DEBUGPrint("Invalid buffer texture");
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_2D, bufferPtr->texture);
}

void Renderer::SetActiveFBufferTexture(Framebuffer_ID frameBufferID, std::string textureName)
{
    OpenGLShader* shaderPtr = GetGLShaderFromShaderID(currentlyBoundShader);
    SetActiveFBufferTexture(frameBufferID, shaderPtr->m_SamplerLocations[textureName]);
}

void Renderer::SetActiveCubemap(Cubemap_ID cubemapID, unsigned int textureSlot)
{
    OpenGLCubemap* cubemapPtr = GetGLCubemapFromCubemapID(cubemapID);

    glActiveTexture(GL_TEXTURE0 + textureSlot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapPtr->texture);
}

void Renderer::SetActiveCubemap(Cubemap_ID cubemapID, std::string textureName)
{
    OpenGLShader* shaderPtr = GetGLShaderFromShaderID(currentlyBoundShader);
    
    GLint sampler = GetSamplerLocation(shaderPtr->m_SamplerLocations, textureName);

    if (sampler >= 0)
    {
        SetActiveCubemap(cubemapID, sampler);
    }
}

void Renderer::SetActiveShader(Shader_ID shader)
{
    if (currentlyBoundShader == shader)
        return;

    OpenGLShader* shaderPtr = GetGLShaderFromShaderID(shader);
    glUseProgram(shaderPtr->m_ProgramId);

    currentlyBoundShader = shader;
    currentlyBoundShaderPtr = shaderPtr;
}

void Renderer::DrawMesh(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    glBindVertexArray(mesh.VAO);

    if (mesh.drawType == DrawType::Triangle)
    {
        if (mesh.useElementArray)
        {   
            glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, mesh.numVertices);
        }
    }
    else if (mesh.drawType == DrawType::TriangleFan)
    {
        if (mesh.useElementArray)
        {
            glDrawElements(GL_TRIANGLE_FAN, mesh.numElements, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, mesh.numVertices);
        }
    }
    else if (mesh.drawType == DrawType::Line)
    {
        if (mesh.useElementArray)
        {
            glDrawElements(GL_LINES, mesh.numElements, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_LINES, 0, mesh.numVertices);
        }
    }

    // TODO(fraser): look into overhead of binding/unbinding VAOs
    glBindVertexArray(0);
}

void Renderer::SetShaderUniformVec2f(Shader_ID shaderID, std::string uniformName, Vec2f vec)
{
    SetActiveShader(shaderID);

    OpenGLShader shader = *GetGLShaderFromShaderID(shaderID);
    GLfloat arr[] = { vec.x, vec.y };

    GLint uniform = GetUniformLocation(shader.m_UniformLocations, uniformName);

    if (uniform >= 0)
    {
        glUniform2fv(uniform, 1, arr);
    }
}

void Renderer::SetShaderUniformVec3f(Shader_ID shaderID, std::string uniformName, Vec3f vec)
{
    SetActiveShader(shaderID);

    OpenGLShader shader = *GetGLShaderFromShaderID(shaderID);
    GLfloat arr[] = { vec.x, vec.y, vec.z };

    GLint uniform = GetUniformLocation(shader.m_UniformLocations, uniformName);

    if (uniform >= 0)
    {
        glUniform3fv(uniform, 1, arr);
    }
}

void Renderer::SetShaderUniformMat4x4f(Shader_ID shaderID, std::string uniformName, Mat4x4f mat)
{
    SetActiveShader(shaderID);

    //OpenGLShader shader = *GetGLShaderFromShaderID(shaderID);
    
    GLint uniform = GetUniformLocation(currentlyBoundShaderPtr->m_UniformLocations, uniformName);

    if (uniform >= 0)
    {
        glUniformMatrix4fv(uniform, 1, GL_FALSE, &mat[0][0]);
    }
}

void Renderer::SetShaderUniformFloat(Shader_ID shaderID, std::string uniformName, float f)
{
    SetActiveShader(shaderID);

    OpenGLShader shader = *GetGLShaderFromShaderID(shaderID);
    
    GLint uniform = GetUniformLocation(shader.m_UniformLocations, uniformName);
    
    if (uniform >= 0)
    {
        glUniform1f(uniform, f);
    }
}

void Renderer::SetShaderUniformInt(Shader_ID shaderID, std::string uniformName, int i)
{
    SetActiveShader(shaderID);

    OpenGLShader shader = *GetGLShaderFromShaderID(shaderID);
    
    GLint uniform = GetUniformLocation(shader.m_UniformLocations, uniformName);

    if (uniform >= 0)
    {
        glUniform1i(uniform, i);
    }
}

void Renderer::SetShaderUniformBool(Shader_ID shaderID, std::string uniformName, bool b)
{
    SetActiveShader(shaderID);

    OpenGLShader shader = *GetGLShaderFromShaderID(shaderID);
    
    GLint uniform = GetUniformLocation(shader.m_UniformLocations, uniformName);

    if (uniform >= 0)
    {
        glUniform1i(uniform, b);
    }
}

void Renderer::SetMeshDrawType(StaticMesh_ID meshID, DrawType type)
{
    OpenGLMesh* mesh = GetGLMeshFromMeshID(meshID);
    mesh->drawType = type;
}

void Renderer::SetMeshColour(StaticMesh_ID meshID, Vec4f colour)
{
    std::vector<Vertex*> meshVertices = MapMeshVertices(meshID);

    for (int i = 0; i < meshVertices.size(); ++i)
    {
        meshVertices[i]->colour = colour;
    }

    UnmapMeshVertices(meshID);
}

std::vector<Vertex*> Renderer::MapMeshVertices(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    Vertex* vertexBuffer = (Vertex*)glMapNamedBuffer(mesh.VBO, GL_READ_WRITE);
    std::vector<Vertex*> vertices;
    for (int i = 0; i < mesh.numVertices; ++i)
    {
        vertices.push_back(&vertexBuffer[i]);
    }

    glFinish();
    return vertices;
}

void Renderer::UnmapMeshVertices(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);
    glUnmapNamedBuffer(mesh.VBO);
    glFinish();
}

std::vector<unsigned int*> Renderer::MapMeshElements(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    GLint size = 0;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    size /= sizeof(ElementIndex);

    unsigned int* elementBuffer = (unsigned int*)glMapNamedBufferRange(mesh.EBO, 0, mesh.numElements, GL_MAP_READ_BIT);

    std::vector<unsigned int*> elements;
    for (int i = 0; i < size; ++i)
    {
        elements.push_back(&elementBuffer[i]);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return elements;
}

void Renderer::UnmapMeshElements(StaticMesh_ID meshID)
{
    OpenGLMesh mesh = *GetGLMeshFromMeshID(meshID);

    glUnmapNamedBuffer(mesh.EBO);
}

void Renderer::ClearScreenAndDepthBuffer()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SwapBuffer()
{
    SwapBuffers(deviceContext);
}

void Renderer::ClearColourBuffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::ClearDepthBuffer()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::ClearStencilBuffer()
{
    glClear(GL_STENCIL_BUFFER_BIT);
}

void Renderer::EnableDepthTesting()
{
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DisableDepthTesting()
{
    glDisable(GL_DEPTH_TEST);
}

void Renderer::SetDepthFunction(DepthFunc func)
{
    GLenum glDepth = GL_LEQUAL;
    switch (func)
    {
    case DepthFunc::LESS:
        glDepth = GL_LEQUAL;
        break;
    case DepthFunc::GREATER:
        glDepth = GL_GREATER;
        break;
    }

    glDepthFunc(glDepth);
}

void Renderer::SetBlendFunction(BlendFunc func)
{
    GLenum glBlendSource = GL_SRC_ALPHA;
    GLenum glBlendDest = GL_ONE_MINUS_SRC_ALPHA;

    switch (func)
    {
    case BlendFunc::TRANS:
        glBlendSource = GL_SRC_ALPHA;
        glBlendDest = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case BlendFunc::ADDITIVE:
        glBlendSource = GL_ONE;
        glBlendDest = GL_ONE;
        break;
    }

    glBlendFunc(glBlendSource, glBlendDest);
}

void Renderer::EnableStencilTesting()
{
    glEnable(GL_STENCIL_TEST);
}

void Renderer::DisableStencilTesting()
{
    glDisable(GL_STENCIL_TEST);
}

void Renderer::StartStencilDrawing(StencilCompareFunc compareFunc, StencilOperationFunc opFunc, int value)
{
    if (compareFunc == StencilCompareFunc::ALWAYS)
    {
        glStencilFunc(GL_ALWAYS, value, 0xFF);
    }
    else if (compareFunc == StencilCompareFunc::EQUAL)
    {
        glStencilFunc(GL_EQUAL, value, 0xFF);
    }
    else if (compareFunc == StencilCompareFunc::GREATER)
    {
        glStencilFunc(GL_GEQUAL, value, 0xFF);
    }
    else if (compareFunc == StencilCompareFunc::LESS)
    {
        glStencilFunc(GL_LEQUAL, value, 0xFF);
    }

    if (opFunc == StencilOperationFunc::KEEP)
    {
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }
    else if (opFunc == StencilOperationFunc::REPLACE)
    {
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    else if (opFunc == StencilOperationFunc::INCREMENT)
    {
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    }
    else if (opFunc == StencilOperationFunc::DECREMENT)
    {
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    }

    //glStencilMask(0xFF);
}

void Renderer::EndStencilDrawing()
{
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    //glStencilMask(0x00);
}

void Renderer::StartStencilTesting(StencilCompareFunc func, int value)
{
    if (func == StencilCompareFunc::ALWAYS)
    {
        glStencilFunc(GL_ALWAYS, value, 0xFF);
    }
    else if (func == StencilCompareFunc::EQUAL)
    {
        glStencilFunc(GL_EQUAL, value, 0xFF);
    }
    else if (func == StencilCompareFunc::GREATER)
    {
        glStencilFunc(GL_GEQUAL, value, 0xFF);
    }
    else if (func == StencilCompareFunc::LESS)
    {
        glStencilFunc(GL_LEQUAL, value, 0xFF);
    }
    
    //glStencilMask(0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void Renderer::EndStencilTesting()
{
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void Renderer::SetCulling(Cull c)
{
    switch (c)
    {
    case Cull::Back:
        glCullFace(GL_BACK);
        break;
    case Cull::Front:
        glCullFace(GL_FRONT);
        break;
    }
}

Vec2i Renderer::GetViewportSize()
{
    GLint m_viewport[4];

    glGetIntegerv(GL_VIEWPORT, m_viewport);
    return Vec2i(m_viewport[2] - m_viewport[0], m_viewport[3] - m_viewport[1]);
}
