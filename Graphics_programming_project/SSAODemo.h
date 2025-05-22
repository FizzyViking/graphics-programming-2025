#pragma once
#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>

class Texture2DObject;

class SSAODemo : public Application
{
public:
    SSAODemo();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeCamera();
    void InitializeLights();

    void UpdateCamera();

    void RenderGUI();

    void InitializeFrameBuffersAndTextures();
    void InitializeSSAOParameters();
    void RenderScene();
    void RenderQuad();

private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Mouse position for camera controller
    glm::vec2 m_mousePosition;

    // Camera controller parameters
    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    // Loaded model
    Model m_model;

    // Light variables
    glm::vec3 m_ambientColor;
    glm::vec3 m_lightColor;
    float m_lightIntensity;
    glm::vec3 m_lightPosition;

    std::vector<glm::vec3> m_ssaoRotationVectors;
    std::vector<glm::vec3> m_ssaoSamples;
    int m_kernelSize;
    float m_kernelRadius;
    float m_bias;
    int m_ssaoActive;

    unsigned int m_gPosition, m_gNormal, m_gAlbedo, m_gBuffer;
    unsigned int m_ssaoColorBuffer, m_ssaoColorBufferBlur;
    unsigned int m_ssaoFBO, m_ssaoBlurFBO;
    unsigned int m_noiseTexture;
    unsigned int m_quadVAO;
    unsigned int m_quadVBO;

    std::shared_ptr<ShaderProgram> m_shaderProgramLight;
    std::shared_ptr<ShaderProgram> m_shaderProgramSSAO;
    std::shared_ptr<ShaderProgram> m_shaderProgramBlur;
    std::shared_ptr<ShaderProgram> m_shaderProgramGbuffer;
};
