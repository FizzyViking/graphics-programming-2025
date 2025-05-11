#include "ViewerApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

ViewerApplication::ViewerApplication()
    : Application(1024, 1024, "Viewer demo")
    , m_cameraPosition(0, 30, 30)
    , m_cameraTranslationSpeed(20.0f)
    , m_cameraRotationSpeed(0.5f)
    , m_cameraEnabled(false)
    , m_cameraEnablePressed(false)
    , m_mousePosition(GetMainWindow().GetMousePosition(true))
{
}

void ViewerApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());
    
    InitializeModel();
    InitializeCamera();
    InitializeLights();

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    device.SetVSyncEnabled(true);
}

void ViewerApplication::Update()
{
    Application::Update();

    // Update camera controller
    UpdateCamera();
}

void ViewerApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    m_model.Draw();
    RenderGUI();
}

void ViewerApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void ViewerApplication::InitializeModel()
{
    Shader vertexShader = ShaderLoader::Load(Shader::Type::VertexShader, "D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise05/shaders/blinn-phong.vert");
    Shader fragmentShader = ShaderLoader::Load(Shader::Type::FragmentShader, "D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise05/shaders/blinn-phong.frag");
    std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->Build(vertexShader, fragmentShader);

    // filtered uniform
    ShaderUniformCollection::NameSet uniformFilter;
    uniformFilter.insert("WorldMatrix");
    uniformFilter.insert("ViewProjMatrix");
    uniformFilter.insert("AmbientColor");
    uniformFilter.insert("LightColor");
    uniformFilter.insert("LightPosition");
    uniformFilter.insert("CameraPosition");
    //uniformFilter.insert("LightConstant");
    //uniformFilter.insert("LightLinear");
    //uniformFilter.insert("LightQuadratic");
    //uniformFilter.insert("LightDirection");

    // create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, uniformFilter);
    material->SetUniformValue("Color", glm::vec4(1.0f));
    material->SetUniformValue("AmbientReflection", 1.0f);
    material->SetUniformValue("DiffuseReflection", 1.0f);
    material->SetUniformValue("LightConstant", 1.0f);
    material->SetUniformValue("LightLinear", 0.35f);
    material->SetUniformValue("LightQuadratic", 0.44f);

    ShaderProgram::Location worldMatrixLocation = material->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjLocation = material->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location ambientColorLocation = material->GetUniformLocation("AmbientColor");
    ShaderProgram::Location lightColorLocation = material->GetUniformLocation("LightColor");
    ShaderProgram::Location lightPositionLocation = material->GetUniformLocation("LightPosition");
    ShaderProgram::Location cameraPositionLocation = material->GetUniformLocation("CameraPosition");
    ShaderProgram::Location specularReflectionLocation = material->GetUniformLocation("SpecularReflection");
    ShaderProgram::Location specularExponentLocation = material->GetUniformLocation("SpecularExponent");
    //ShaderProgram::Location lightDirectionLocation = material->GetUniformLocation("LightDireciton");
    material->SetShaderSetupFunction([=](ShaderProgram& shaderProgram) 
    {
        shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(0.1f)));
        shaderProgram.SetUniform(viewProjLocation, m_camera.GetViewProjectionMatrix());
        shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
        shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
        shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
        shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
        shaderProgram.SetUniform(specularReflectionLocation, m_specularReflection);
        shaderProgram.SetUniform(specularExponentLocation, m_specularExponent);
        //shaderProgram.SetUniform(lightDirectionLocation, m_lightDirection);
    });

    // model loader
    ModelLoader loader(material);
    loader.SetCreateMaterials(true);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "WorldPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");
    
    m_model = loader.Load("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise05/models/mill/Mill.obj");
    Texture2DLoader textureLoader(TextureObject::Format::FormatRGBA, TextureObject::InternalFormat::InternalFormatRGBA8);
    textureLoader.SetFlipVertical(true);
    
    m_model.GetMaterial(0).SetUniformValue("ColorTexture", textureLoader.LoadShared("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise05/models/mill/Ground_color.jpg"));
    m_model.GetMaterial(1).SetUniformValue("ColorTexture", textureLoader.LoadShared("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise05/models/mill/Ground_shadow.jpg"));
    m_model.GetMaterial(2).SetUniformValue("ColorTexture", textureLoader.LoadShared("D:/ITU/Graphics_programming_2025/graphics-programming-2025/exercises/exercise05/models/mill/MillCat_color.jpg"));
    
    m_model.GetMaterial(0).SetUniformValue("AmbientReflection", 0.2f);
    m_model.GetMaterial(0).SetUniformValue("AmbientReflection", 0.7f);
    m_model.GetMaterial(0).SetUniformValue("AmbientReflection", 0.5f);
}

void ViewerApplication::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}

void ViewerApplication::InitializeLights()
{
    // (todo) 05.X: Initialize light variables

}

void ViewerApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // (todo) 05.4: Add debug controls for light properties
    //ImGui::DragFloat3("LightDirection", &m_lightDirection.x);
    //ImGui::NewLine();
    ImGui::DragFloat("SpecularReflectionStrength", &m_specularReflection);
    ImGui::DragFloat("SpecularExponent", &m_specularExponent);
    ImGui::NewLine();
    ImGui::ColorEdit3("AmbientColor", &m_ambientColor.x);
    ImGui::NewLine();
    ImGui::DragFloat3("LightPosition", &m_lightPosition.x);
    ImGui::ColorEdit3("LightColor", &m_lightColor.x);
    ImGui::DragFloat("LightIntensity", &m_lightIntensity);
    m_imGui.EndFrame();
}

void ViewerApplication::UpdateCamera()
{
    Window& window = GetMainWindow();

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_cameraEnablePressed)
        {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_cameraEnablePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A))
            inputTranslation.x = -1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_D))
            inputTranslation.x = 1.0f;

        if (window.IsKeyPressed(GLFW_KEY_W))
            inputTranslation.y = 1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_S))
            inputTranslation.y = -1.0f;

        inputTranslation *= m_cameraTranslationSpeed;
        inputTranslation *= GetDeltaTime();

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

    // Update camera rotation
   {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_cameraRotationSpeed;

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0,1,0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

   // Update view matrix
   m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}
