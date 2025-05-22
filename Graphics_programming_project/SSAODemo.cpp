#include "SSAODemo.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/shader/Material.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>
#include <random>
#include <iostream>
#include <numbers>

SSAODemo::SSAODemo()
    : Application(1024, 1024, "SSAO Scene Application")
    , m_cameraPosition(0, 30, 30)
    , m_cameraTranslationSpeed(20.0f)
    , m_cameraRotationSpeed(0.5f)
    , m_cameraEnabled(false)
    , m_cameraEnablePressed(false)
    , m_mousePosition(GetMainWindow().GetMousePosition(true))
    , m_ambientColor(0.0f)
    , m_lightColor(0.0f)
    , m_lightIntensity(0.0f)
    , m_lightPosition(0.0f)
    , m_quadVAO(0)
    , m_ssaoActive(true)
    , m_kernelSize(64)
    , m_kernelRadius(0.5f)
    , m_bias(0.025)
{
}

void SSAODemo::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    //InitializeModel();
    InitializeCamera();
    InitializeLights();
    InitializeSSAOParameters();
    InitializeFrameBuffersAndTextures();

    DeviceGL& device = GetDevice();
    device.EnableFeature(GL_DEPTH_TEST);
    device.SetVSyncEnabled(true);
}

void SSAODemo::Update()
{
    Application::Update();

    // Update camera
    UpdateCamera();
}

void SSAODemo::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    //InitializeModel();
    RenderScene();

    // Render the debug user interface
    RenderGUI();
}

void SSAODemo::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void SSAODemo::RenderScene() {
    // g-buffer pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection = m_camera.GetProjectionMatrix();
    glm::mat4 view = m_camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    m_shaderProgramGbuffer->Use();
    m_shaderProgramGbuffer->SetUniform(m_shaderProgramGbuffer->GetUniformLocation("NormalMatrix"), glm::transpose(glm::inverse(glm::mat3(view * model))));
    m_shaderProgramGbuffer->SetUniform(m_shaderProgramGbuffer->GetUniformLocation("ProjMatrix"), projection);
    m_shaderProgramGbuffer->SetUniform(m_shaderProgramGbuffer->GetUniformLocation("ViewMatrix"), view);
    m_shaderProgramGbuffer->SetUniform(m_shaderProgramGbuffer->GetUniformLocation("ModelMatrix"), model);
    m_model.Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSAO Pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    m_shaderProgramSSAO->Use();
    // Update kernel samples
    InitializeSSAOParameters();
    for (unsigned int i = 0; i < m_kernelSize; ++i) {
        auto name = "samples[" + std::to_string(i) + "]";
        m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation(name.c_str()), m_ssaoSamples[i]);
    }
    m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("ProjMatrix"), projection);
    m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("KernelSize"), m_kernelSize);
    m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("Radius"), m_kernelRadius);
    m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("Bias"), m_bias);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
    RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blur pass to remove noise
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    m_shaderProgramBlur->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBuffer);
    RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Lightning pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shaderProgramLight->Use();

    // Light uniforms
    glm::vec3 lightPosView = glm::vec3(m_camera.GetViewMatrix() * glm::vec4(m_lightPosition, 1.0));
    m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("LightPosition"), lightPosView);
    m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("LightColor"), m_lightColor * m_lightIntensity);
    m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("AmbientReflection"), 1.0f);
    m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("AmbientColor"), m_ambientColor);
    m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("DiffuseReflection"), 1.0f);
    m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("SSAOActive"), m_ssaoActive);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBufferBlur);
    RenderQuad();
}

void SSAODemo::InitializeFrameBuffersAndTextures() {

    // G-buffer 
    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);

    int width, height;
    GetMainWindow().GetDimensions(width, height);

    // Position
    glGenTextures(1, &m_gPosition);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);

    // Normal
    glGenTextures(1, &m_gNormal);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);

    // Albedo
    glGenTextures(1, &m_gAlbedo);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedo, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Depth Framebuffer in form of a renderbuffer
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSAO Framebuffers
    glGenFramebuffers(1, &m_ssaoFBO);  
    glGenFramebuffers(1, &m_ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoFBO);

    // SSAO color buffer to occlusion result
    glGenTextures(1, &m_ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;

    // Blur pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssaoBlurFBO);
    glGenTextures(1, &m_ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, m_ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Texture to hold the rotation vectors
    glGenTextures(1, &m_noiseTexture);
    glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &m_ssaoRotationVectors[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Build and load the shaders
    {
        Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/gBuffer/g_buffer.vert");
        Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/gBuffer/g_buffer.frag");
        m_shaderProgramGbuffer = std::make_shared<ShaderProgram>();
        m_shaderProgramGbuffer->Build(vertexShader, fragmentShader);
        m_shaderProgramGbuffer->Use();
        m_shaderProgramGbuffer->SetUniform(m_shaderProgramGbuffer->GetUniformLocation("Color"), glm::vec3(1.0f));
    }

    {
        Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/gBuffer/ssao.vert");
        Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/gBuffer/lightning.frag");
        m_shaderProgramLight = std::make_shared<ShaderProgram>();
        m_shaderProgramLight->Build(vertexShader, fragmentShader);
        m_shaderProgramLight->Use();
        m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("Position"), 0);
        m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("Normal"), 1);
        m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("Albedo"), 2);
        m_shaderProgramLight->SetUniform(m_shaderProgramLight->GetUniformLocation("ssao"), 3);
    }
    
    {
        Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/gBuffer/ssao.vert");
        Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/postfx/ssao.frag");
        m_shaderProgramSSAO = std::make_shared<ShaderProgram>();
        m_shaderProgramSSAO->Build(vertexShader, fragmentShader);
        m_shaderProgramSSAO->Use();
        m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("Position"), 0);
        m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("Normal"), 1);
        m_shaderProgramSSAO->SetUniform(m_shaderProgramSSAO->GetUniformLocation("texNoise"), 2);
    }
    {
        Shader vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/gBuffer/ssao.vert");
        Shader fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/postfx/blur.frag");
        m_shaderProgramBlur = std::make_shared<ShaderProgram>();
        m_shaderProgramBlur->Build(vertexShader, fragmentShader);
        m_shaderProgramBlur->Use();
        m_shaderProgramBlur->SetUniform(m_shaderProgramBlur->GetUniformLocation("ssaoInput"), 0);
    }

    
    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewMatrix");
    filteredUniforms.insert("ProjMatrix");
    filteredUniforms.insert("NormalMatrix");
    filteredUniforms.insert("ModelMatrix");
    filteredUniforms.insert("InvertedNormals");

    // Load model
    std::shared_ptr<Material> material = std::make_shared<Material>(m_shaderProgramGbuffer, filteredUniforms);
    ModelLoader loader(material);
    loader.SetCreateMaterials(true);
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Position, "VertexPosition");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::Normal, "VertexNormal");
    loader.SetMaterialProperty(ModelLoader::MaterialProperty::DiffuseTexture, "ColorTexture");
    loader.SetMaterialAttribute(VertexAttribute::Semantic::TexCoord0, "VertexTexCoord");
    m_model = loader.Load("models/mill/Mill.obj");

    // Load and set textures
    Texture2DLoader textureLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA8);
    textureLoader.SetFlipVertical(true);
    m_model.GetMaterial(0).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/mill/Ground_shadow.jpg"));
    m_model.GetMaterial(1).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/mill/Ground_color.jpg"));
    m_model.GetMaterial(2).SetUniformValue("ColorTexture", textureLoader.LoadShared("models/mill/MillCat_color.jpg"));
}

void SSAODemo::InitializeSSAOParameters() {
    // Create samples for unit hemisphere
    // Generator for float between 0.0 and 1.0
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    std::default_random_engine generator;
    
    for (int i = 0; i < m_kernelSize; ++i) {
        glm::vec3 sample(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f, distribution(generator));
        sample = glm::normalize(sample);
        sample *= distribution(generator); // Scale the sample to a random radius
        float scale = static_cast<float>(i) / static_cast<float>(m_kernelSize);
        scale = glm::mix(0.1f, 1.0f, scale * scale);
        sample *= scale;
        m_ssaoSamples.push_back(sample);
    }

    // Create the random kernel rotation vectors
    for (int i = 0; i < 16; ++i) {
        // Generate random 3D vector
        glm::vec3 rotation(distribution(generator) * 2.0f - 1.0f, distribution(generator) * 2.0f - 1.0f, 0.0f);
        m_ssaoRotationVectors.push_back(rotation);
    }
}

void SSAODemo::InitializeCamera()
{
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}

void SSAODemo::InitializeLights()
{
    // Initialize light variables
    m_ambientColor = glm::vec3(0.25f);
    m_lightColor = glm::vec3(1.0f);
    m_lightIntensity = 1.0f;
    m_lightPosition = glm::vec3(-10, 20, 10);
}

void SSAODemo::RenderGUI()
{
    m_imGui.BeginFrame();

    // Debug controls
    ImGui::ColorEdit3("Ambient color", &m_ambientColor[0]);
    ImGui::Separator();
    ImGui::DragFloat3("Light position", &m_lightPosition[0], 0.1f);
    ImGui::ColorEdit3("Light color", &m_lightColor[0]);
    ImGui::DragFloat("Light intensity", &m_lightIntensity, 0.05f, 0.0f, 100.0f);
    ImGui::Separator();
    ImGui::DragInt("SSAOActive", &m_ssaoActive, 0, 1);
    ImGui::DragFloat("Kernel Radius", &m_kernelRadius, 0.5, 0.1f, 10.0f);
    ImGui::DragInt("Kernel Size", &m_kernelSize, 1, 8, 256);
    ImGui::DragFloat("Kernel Bias", &m_bias, 0.025, 0.025f, 1.0f);

    m_imGui.EndFrame();
}

// Render a full screen quad in NDC coordinates
void SSAODemo::RenderQuad()
{
    if (m_quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void SSAODemo::UpdateCamera()
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

        viewForward = glm::rotate(inputRotation.x, glm::vec3(0, 1, 0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) * glm::vec4(viewForward, 0);
    }

    // Update view matrix
    m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);    
    m_camera.SetPerspectiveProjectionMatrix(1.0f, GetMainWindow().GetAspectRatio(), 0.1f, 1000.0f);
}