#include "TerrainApplication.h"

// (todo) 01.1: Include the libraries you need
#include <ituGL/core/DeviceGL.h>
#include <ituGL/application/Window.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/geometry/VertexAttribute.h>
#include <ituGL/geometry/ElementBufferObject.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <cmath>
#include <iostream>
#include <vector>

// Helper structures. Declared here only for this exercise
struct Vector2
{
    Vector2() : Vector2(0.f, 0.f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct Vector3
{
    Vector3() : Vector3(0.f,0.f,0.f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;

    Vector3 Normalize() const
    {
        float length = std::sqrt(1 + x * x + y * y);
        return Vector3(x / length, y / length, z / length);
    }
};

// (todo) 01.8: Declare an struct with the vertex format
struct Vertex
{
	Vector3 position;
	Vector2 texCoord;
	Vector3 color;
	Vector3 normal;
};

static Vector3 getColorFromHeight(float z) {
    if (z > 0.3f) { // Mountain top, white
        return Vector3(1.0f, 1.0f, 1.0f);
    }
    if (z > 0.15) { // Mountain, dark gray
        return Vector3(0.1f, 0.1f, 0.1f);
    }
    if (z > -0.1f) { // Forest, green
        return Vector3(0.0f, 0.1f, 0.0f);
    }
    return Vector3(0.0f, 0.5f, 1.0f);
}

TerrainApplication::TerrainApplication()
    : Application(1024, 1024, "Terrain demo"), m_gridX(128), m_gridY(128), m_shaderProgram(0)
{
}

void TerrainApplication::Initialize()
{
    Application::Initialize();

    // Build shaders and store in m_shaderProgram
    BuildShaders();
    
    // (todo) 01.1: Create containers for the vertex position
    std::vector<Vertex> vertices;

    std::vector<unsigned int> indices;

    // Create a scale for the vertices
	Vector2 scale(1.0f / m_gridX, 1.0f / m_gridY);
    unsigned int columns = m_gridX + 1;
    unsigned int rows = m_gridY + 1;
    
    // (todo) 01.1: Fill in vertex data
    for (unsigned int i = 0u; i < columns; i++)
    {
        for (unsigned int j = 0u; j < rows; j++) {

			// Create the vertex position
            // Calculate the bottom-left corner of the quad
            Vector3 position = Vector3(i * scale.x - 0.5f, j * scale.y - 0.5f, 0.0f);

            // Define the two triangles for the quad
            Vertex vertex;
            float z = stb_perlin_fbm_noise3(position.x*2, position.y*2, 0.0f, 2.0f, 0.5f, 6);
			vertex.position = Vector3(position.x, position.y, z);
			vertex.texCoord = Vector2(static_cast<float>(i), static_cast<float>(j));
			vertex.color = getColorFromHeight(z);
			vertex.normal = Vector3(0.0f, 0.0f, 1.0f);
            vertices.push_back(vertex);
			
            if (i > 0 && j > 0) {
                // Calculate the indices for the two triangles
                unsigned int topRight = j * columns + i;
                unsigned int topLeft = topRight - 1;
                unsigned int bottomLeft = topLeft - columns;
                unsigned int bottomRight = bottomLeft + 1;

                // First triangle
                indices.push_back(topLeft);          
                indices.push_back(topRight);              
                indices.push_back(bottomRight);
                // Second triangle
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);                        
                indices.push_back(bottomRight);
                
            }
            
        }
    }

    
    for (unsigned int i = 0u; i < rows; i++)
    {
        for (unsigned int j = 0u; j < columns; j++) {
            unsigned int current = i * columns + j;

            Vertex& vertex = vertices[current]; // Current vertex
            Vertex& vertex_right = (j < m_gridX) ? vertices[current + 1] : vertices[current];
            Vertex& vertex_left = (j > 0) ? vertices[current - 1] : vertices[current];

            Vertex& vertex_up = (i < m_gridY) ? vertices[current + columns] : vertices[current];
            Vertex& vertex_down = (i > 0) ? vertices[current - columns] : vertices[current];

            float delta_x = (vertex_right.position.z - vertex_left.position.z) / (vertex_right.position.x - vertex_left.position.x);
            float delta_y = (vertex_up.position.z - vertex_down.position.z) / (vertex_up.position.y - vertex_down.position.y);
            vertex.normal = Vector3(delta_x, delta_y, 1.0f).Normalize();
        }
    }

    // (todo) 01.1: Initialize VAO, and VBO
	m_vbo.Bind();
    m_vbo.AllocateData<Vertex>(std::span(vertices));

	VertexAttribute positionAttribute(Data::Type::Float, 3);
	VertexAttribute texCoordAttribute(Data::Type::Float, 2);
	VertexAttribute colorAttribute(Data::Type::Float, 3);
	VertexAttribute normalAttribute(Data::Type::Float, 3);

    // Calculate stride
	GLsizei stride = sizeof(Vector3) + sizeof(Vector2) + sizeof(Vector3) + sizeof(Vector3);

    m_vao.Bind();
	m_vao.SetAttribute(0, positionAttribute, 0u, stride); // Position attribute
	m_vao.SetAttribute(1, texCoordAttribute, (0u + positionAttribute.GetSize()), stride); // Texture coordinate attribute
	m_vao.SetAttribute(2, colorAttribute, (0u + positionAttribute.GetSize()) + texCoordAttribute.GetSize(), stride); // Color attribute
	m_vao.SetAttribute(3, normalAttribute, ((0u + positionAttribute.GetSize()) + texCoordAttribute.GetSize()) + colorAttribute.GetSize(), stride); // Normal attribute


    // (todo) 01.5: Initialize EBO
    m_ebo.Bind();
    m_ebo.AllocateData(std::span(indices));

    // (todo) 01.1: Unbind VAO, and VBO
    //m_vbo.Unbind();
	VertexBufferObject::Unbind();
	VertexArrayObject::Unbind();
	//m_vao.Unbind();


    // (todo) 01.5: Unbind EBO
	ElementBufferObject::Unbind();

    glEnable(GL_DEPTH_TEST);
}

void TerrainApplication::Update()
{
    Application::Update();

    UpdateOutputMode();
}

void TerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Set shader to be used
    glUseProgram(m_shaderProgram);

    // (todo) 01.1: Draw the grid
    m_vao.Bind();
    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, m_gridX * m_gridY * 6, GL_UNSIGNED_INT, nullptr);
}

void TerrainApplication::Cleanup()
{
    Application::Cleanup();
}


void TerrainApplication::BuildShaders()
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "layout (location = 2) in vec3 aColor;\n"
        "layout (location = 3) in vec3 aNormal;\n"
        "uniform mat4 Matrix = mat4(1);\n"
        "out vec2 texCoord;\n"
        "out vec3 color;\n"
        "out vec3 normal;\n"
        "void main()\n"
        "{\n"
        "   texCoord = aTexCoord;\n"
        "   color = aColor;\n"
        "   normal = aNormal;\n"
        "   gl_Position = Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "uniform uint Mode = 0u;\n"
        "in vec2 texCoord;\n"
        "in vec3 color;\n"
        "in vec3 normal;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   switch (Mode)\n"
        "   {\n"
        "   default:\n"
        "   case 0u:\n"
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "       break;\n"
        "   case 1u:\n"
        "       FragColor = vec4(fract(texCoord), 0.0f, 1.0f);\n"
        "       break;\n"
        "   case 2u:\n"
        "       FragColor = vec4(color, 1.0f);\n"
        "       break;\n"
        "   case 3u:\n"
        "       FragColor = vec4(normalize(normal), 1.0f);\n"
        "       break;\n"
        "   case 4u:\n"
        "       FragColor = vec4(color * max(dot(normalize(normal), normalize(vec3(1,0,1))), 0.2f), 1.0f);\n"
        "       break;\n"
        "   }\n"
        "}\n\0";

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    m_shaderProgram = shaderProgram;
}

void TerrainApplication::UpdateOutputMode()
{
    for (int i = 0; i <= 4; ++i)
    {
        if (GetMainWindow().IsKeyPressed(GLFW_KEY_0 + i))
        {
            int modeLocation = glGetUniformLocation(m_shaderProgram, "Mode");
            glUseProgram(m_shaderProgram);
            glUniform1ui(modeLocation, i);
            break;
        }
    }
    if (GetMainWindow().IsKeyPressed(GLFW_KEY_TAB))
    {
        const float projMatrix[16] = { 0, -1.294f, -0.721f, -0.707f, 1.83f, 0, 0, 0, 0, 1.294f, -0.721f, -0.707f, 0, 0, 1.24f, 1.414f };
        int matrixLocation = glGetUniformLocation(m_shaderProgram, "Matrix");
        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(matrixLocation, 1, false, projMatrix);
    }
}
