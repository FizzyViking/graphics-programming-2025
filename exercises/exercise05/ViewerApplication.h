#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/utils/DearImGui.h>

class Texture2DObject;

class ViewerApplication : public Application
{
public:
    ViewerApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeModel();
    void InitializeCamera();
    void InitializeLights();

    void UpdateCamera();

    void RenderGUI();

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

    // (todo) 05.X: Add light variables
    glm::vec3 m_ambientColor = glm::vec3(0.25f);
    glm::vec3 m_lightColor = glm::vec3(1.0f);
    glm::vec3 m_lightPosition = glm::vec3(-10.0f, 20.0f, 10.0f);
    //glm::vec3 m_lightDirection = m_lightPosition - glm::vec3(0.0f);
    float m_lightIntensity = 1.0f;
    float m_specularReflection = 1.0f;
    float m_specularExponent = 100.0f;

};
