#include "TexturedTerrainApplication.h"

#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/texture/Texture2DObject.h>

#include <glm/gtx/transform.hpp>  // for matrix transformations

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <iostream>
#include <numbers>  // for PI constant

TexturedTerrainApplication::TexturedTerrainApplication()
    : Application(1024, 1024, "Textures demo")
    , m_gridX(128), m_gridY(128)
    , m_vertexShaderLoader(Shader::Type::VertexShader)
    , m_fragmentShaderLoader(Shader::Type::FragmentShader)
{
}

void TexturedTerrainApplication::Initialize()
{
    Application::Initialize();

    // Build textures and keep them in a list
    InitializeTextures();

    // Build materials and keep them in a list
    InitializeMaterials();

    // Build meshes and keep them in a list
    InitializeMeshes();

    //Enable depth test
    GetDevice().EnableFeature(GL_DEPTH_TEST);

    //Enable wireframe
    //GetDevice().SetWireframeEnabled(true);

    GetDevice().EnableFeature(GL_BLEND);
}

void TexturedTerrainApplication::Update()
{
    Application::Update();

    const Window& window = GetMainWindow();

    glm::vec2 mousePosition = window.GetMousePosition(true);
    m_camera.SetViewMatrix(glm::vec3(0.0f, 15.0f, 15.0f), glm::vec3(mousePosition, 0.0f));

    int width, height;
    window.GetDimensions(width, height);
    float aspectRatio = static_cast<float>(width) / height;
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 100.0f);
}

void TexturedTerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Terrain patches
    DrawObject(m_terrainPatch, *m_terrainMaterial0, glm::scale(glm::vec3(10.0f)));

    // (todo) 04.2: Add more patches here
    DrawObject(m_terrainPatch, *m_terrainMaterial1, glm::translate(glm::vec3(-10.0f, 0, 0)) * glm::scale(glm::vec3(10.0f)));
    DrawObject(m_terrainPatch, *m_terrainMaterial2, glm::translate(glm::vec3(0, 0, -10.0f)) * glm::scale(glm::vec3(10.0f)));
    DrawObject(m_terrainPatch, *m_terrainMaterial3, glm::translate(glm::vec3(-10.0f, 0, -10.0f)) * glm::scale(glm::vec3(10.0f)));

    // Water patches
    // (todo) 04.5: Add water planes
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::translate(glm::vec3(0, -1, 0)) * glm::scale(glm::vec3(10.0f)));
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::translate(glm::vec3(0, -1, -10.0f)) * glm::scale(glm::vec3(10.0f)));
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::translate(glm::vec3(0, -1, 0)) * glm::scale(glm::vec3(10.0f)));
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::translate(glm::vec3(-10.0f, -1, -10.0f)) * glm::scale(glm::vec3(10.0f)));
}

void TexturedTerrainApplication::InitializeTextures()
{
    m_defaultTexture = CreateDefaultTexture();

    // (todo) 04.3: Load terrain textures here
    m_heightmapTexture0 = CreateHeightMap(m_gridX, m_gridY, glm::ivec2(0,0));
    m_heightmapTexture1 = CreateHeightMap(m_gridX, m_gridY, glm::ivec2(-1, 0));
    m_heightmapTexture2 = CreateHeightMap(m_gridX, m_gridY, glm::ivec2(0, -1));
    m_heightmapTexture3 = CreateHeightMap(m_gridX, m_gridY, glm::ivec2(-1, -1));

    // Grass
    m_grassTexture = LoadTexture("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/textures/grass.jpg");
    m_dirtTexture = LoadTexture("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/textures/dirt.png");
    m_rockTexture = LoadTexture("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/textures/rock.jpg");
    m_snowTexture = LoadTexture("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/textures/snow.jpg");

    // (todo) 04.5: Load water texture here
    m_waterTexture = LoadTexture("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/textures/water.png");
}

void TexturedTerrainApplication::InitializeMaterials()
{
    // Default shader program
    Shader defaultVS = m_vertexShaderLoader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/shaders/default.vert");
    Shader defaultFS = m_fragmentShaderLoader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/shaders/default.frag");
    std::shared_ptr<ShaderProgram> defaultShaderProgram = std::make_shared<ShaderProgram>();
    defaultShaderProgram->Build(defaultVS, defaultFS);

    // Default material
    m_defaultMaterial = std::make_shared<Material>(defaultShaderProgram);
    m_defaultMaterial->SetUniformValue("Color", glm::vec4(1.0f));

    // (todo) 04.1: Add terrain shader and material here
    Shader terrainVS = m_vertexShaderLoader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/shaders/terrain.vert");
    Shader terrainFS = m_fragmentShaderLoader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/shaders/terrain.frag");
    std::shared_ptr<ShaderProgram> terrainShaderProgram = std::make_shared<ShaderProgram>();
    terrainShaderProgram->Build(terrainVS, terrainFS);

    m_terrainMaterial0 = std::make_shared<Material>(terrainShaderProgram);
    m_terrainMaterial0->SetUniformValue("Color", glm::vec4(1.0f));
    m_terrainMaterial0->SetUniformValue("ColorTextureScale", glm::vec2(0.1f));
    m_terrainMaterial0->SetUniformValue("Heightmap", m_heightmapTexture0);
    m_terrainMaterial0->SetUniformValue("HeightRange1", glm::vec2(-0.1f, 0.2f));
    m_terrainMaterial0->SetUniformValue("HeightRange2", glm::vec2(0.15f, 0.2f));
    m_terrainMaterial0->SetUniformValue("HeightRange3", glm::vec2(0.2f, 0.5f));

    m_terrainMaterial0->SetUniformValue("ColorTextureGrass", m_grassTexture);
    m_terrainMaterial0->SetUniformValue("ColorTextureDirt", m_dirtTexture);
    m_terrainMaterial0->SetUniformValue("ColorTextureRock", m_rockTexture);
    m_terrainMaterial0->SetUniformValue("ColorTextureSnow", m_snowTexture);

    m_terrainMaterial1 = std::make_shared<Material>(*m_terrainMaterial0);
    m_terrainMaterial1->SetUniformValue("Heightmap", m_heightmapTexture1);

    m_terrainMaterial2 = std::make_shared<Material>(*m_terrainMaterial0);
    m_terrainMaterial2->SetUniformValue("Heightmap", m_heightmapTexture2);

    m_terrainMaterial3 = std::make_shared<Material>(*m_terrainMaterial0);
    m_terrainMaterial3->SetUniformValue("Heightmap", m_heightmapTexture3);
    

    // (todo) 04.5: Add water shader and material here
    Shader waterVS = m_vertexShaderLoader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/shaders/water.vert");
    Shader waterFS = m_fragmentShaderLoader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise04/shaders/water.frag");
    std::shared_ptr<ShaderProgram> waterShaderProgram = std::make_shared<ShaderProgram>();
    waterShaderProgram->Build(waterVS, waterFS);

    m_waterMaterial = std::make_shared<Material>(waterShaderProgram);
    m_waterMaterial->SetUniformValue("Color", glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
    m_waterMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.1f));
    m_waterMaterial->SetUniformValue("ColorTextureWater", m_waterTexture);

    m_waterMaterial->SetBlendEquation(Material::BlendEquation::Add);
    m_waterMaterial->SetBlendParams(Material::BlendParam::ConstantAlpha, Material::BlendParam::OneMinusSourceAlpha);

}

void TexturedTerrainApplication::InitializeMeshes()
{
    CreateTerrainMesh(m_terrainPatch, m_gridX, m_gridY);
}

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::CreateDefaultTexture()
{
    std::shared_ptr<Texture2DObject> texture = std::make_shared<Texture2DObject>();

    int width = 4;
    int height = 4;
    std::vector<float> pixels;
    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            pixels.push_back(1.0f);
            pixels.push_back(0.0f);
            pixels.push_back(1.0f);
            pixels.push_back(1.0f);
        }
    }

    texture->Bind();
    texture->SetImage<float>(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, pixels);
    texture->GenerateMipmap();

    return texture;
}

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::LoadTexture(const char* path)
{
    std::shared_ptr<Texture2DObject> texture = std::make_shared<Texture2DObject>();

    int width = 0;
    int height = 0;
    int components = 0;
    
    
    // (todo) 04.3: Load the texture data here
    unsigned char* data = stbi_load(path, &width, &height, &components, 4);

    texture->Bind();
    texture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, std::span<const unsigned char>(data, width * height * 4));

    // (todo) 04.3: Generate mipmaps
    texture->GenerateMipmap();

    // (todo) 04.3: Release texture data
    stbi_image_free(data);

    return texture;
}

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords)
{
    std::shared_ptr<Texture2DObject> heightmap = std::make_shared<Texture2DObject>();

    std::vector<float> pixels;
    for (unsigned int j = 0; j < height; ++j)
    {
        for (unsigned int i = 0; i < width; ++i)
        {
            // (todo) 04.1: Add pixel data
            float x = (static_cast<float>(i) / width - 1) + coords.x;
            float y = (static_cast<float>(j) / height - 1) + coords.y;
            pixels.push_back(stb_perlin_fbm_noise3(x, y, 0.0f, 2.0f, 0.5f, 8) * 0.5f);
        }
    }

    heightmap->Bind();
    heightmap->SetImage<float>(0, width, height, TextureObject::FormatR, TextureObject::InternalFormatR16F, pixels);
    heightmap->GenerateMipmap();

    return heightmap;
}

void TexturedTerrainApplication::DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix)
{
    material.Use();

    ShaderProgram& shaderProgram = *material.GetShaderProgram();
    ShaderProgram::Location locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
    material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
    ShaderProgram::Location locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");
    material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

    mesh.DrawSubmesh(0);
}

void TexturedTerrainApplication::CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY)
{
    // Define the vertex structure
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2 texCoord)
            : position(position), normal(normal), texCoord(texCoord) {}
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    // List of vertices (VBO)
    std::vector<Vertex> vertices;

    // List of indices (EBO)
    std::vector<unsigned int> indices;

    // Grid scale to convert the entire grid to size 1x1
    glm::vec2 scale(1.0f / (gridX - 1), 1.0f / (gridY - 1));

    // Number of columns and rows
    unsigned int columnCount = gridX;
    unsigned int rowCount = gridY;

    // Iterate over each VERTEX
    for (unsigned int j = 0; j < rowCount; ++j)
    {
        for (unsigned int i = 0; i < columnCount; ++i)
        {
            // Vertex data for this vertex only
            glm::vec3 position(i * scale.x, 0.0f, j * scale.y);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec2 texCoord(i, j);
            vertices.emplace_back(position, normal, texCoord);

            // Index data for quad formed by previous vertices and current
            if (i > 0 && j > 0)
            {
                unsigned int top_right = j * columnCount + i; // Current vertex
                unsigned int top_left = top_right - 1;
                unsigned int bottom_right = top_right - columnCount;
                unsigned int bottom_left = bottom_right - 1;

                //Triangle 1
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
                indices.push_back(top_left);

                //Triangle 2
                indices.push_back(bottom_right);
                indices.push_back(top_left);
                indices.push_back(top_right);
            }
        }
    }

    mesh.AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
}
