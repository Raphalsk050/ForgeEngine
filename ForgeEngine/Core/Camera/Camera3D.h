#pragma once

// Enable GLM experimental features before including GLM headers
#define GLM_ENABLE_EXPERIMENTAL
#include "Core/Camera/camera.h"
#include <array>

namespace ForgeEngine {

// Frustum planes enumeration for clarity
enum class FrustumPlane {
    Left = 0,
    Right,
    Bottom,
    Top,
    Near,
    Far
};

class Camera3D : public Camera {
public:
    Camera3D() = default;
    Camera3D(float fov, float aspectRatio, float nearClip, float farClip);

    void SetPerspective(float fov, float aspectRatio, float nearClip, float farClip);
    void SetViewport(float width, float height);
    
    // Updates the view matrix and frustum planes
    void RecalculateViewMatrix();
    void RecalculateFrustum();
    
    // Frustum culling methods
    bool PointInFrustum(const glm::vec3& point) const;
    bool SphereInFrustum(const glm::vec3& center, float radius) const;
    bool AABBInFrustum(const glm::vec3& min, const glm::vec3& max) const;
    
    // Position and orientation control
    void SetPosition(const glm::vec3& position);
    const glm::vec3& GetPosition() const { return m_Position; }
    
    void SetRotation(const glm::vec3& rotation);
    const glm::vec3& GetRotation() const { return m_Rotation; }
    
    void SetFocalPoint(const glm::vec3& focalPoint);
    const glm::vec3& GetFocalPoint() const { return m_FocalPoint; }
    
    // Direction vectors
    glm::vec3 GetForwardDirection() const;
    glm::vec3 GetRightDirection() const;
    glm::vec3 GetUpDirection() const;
    
    // Matrices access
    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
    glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
    
    // Frustum info
    const std::array<glm::vec4, 6>& GetFrustumPlanes() const { return m_FrustumPlanes; }
    
    // Camera properties
    float GetNearClip() const { return m_NearClip; }
    float GetFarClip() const { return m_FarClip; }
    float GetAspectRatio() const { return m_AspectRatio; }
    float GetFOV() const { return m_FOV; }
    
private:
    // Camera view properties
    glm::vec3 m_Position = { 0.0f, 0.0f, 5.0f };
    glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f }; // Pitch, Yaw, Roll (in radians)
    glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
    
    // Camera projection properties
    float m_FOV = 45.0f;
    float m_AspectRatio = 1.778f;
    float m_NearClip = 0.1f;
    float m_FarClip = 1000.0f;
    
    // View and projection matrices
    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    
    // Frustum culling
    std::array<glm::vec4, 6> m_FrustumPlanes; // Left, Right, Bottom, Top, Near, Far
};

} // namespace BEngine