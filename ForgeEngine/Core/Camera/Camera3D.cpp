#include "Core/Camera/Camera3D.h"
#include "FEPCH.h"

#include <gtc/matrix_transform.hpp>

// The header needs to be included here after defining GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

namespace ForgeEngine {

Camera3D::Camera3D(float fov, float aspectRatio, float nearClip, float farClip)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip),
      Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
{
    RecalculateViewMatrix();
    RecalculateFrustum();
}

void Camera3D::SetPerspective(float fov, float aspectRatio, float nearClip, float farClip)
{
    m_FOV = fov;
    m_AspectRatio = aspectRatio;
    m_NearClip = nearClip;
    m_FarClip = farClip;
    m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
    RecalculateFrustum();
}

void Camera3D::SetViewport(float width, float height)
{
    m_AspectRatio = width / height;
    m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    RecalculateFrustum();
}

void Camera3D::RecalculateViewMatrix()
{
    FENGINE_PROFILE_FUNCTION();

    // Create rotation matrix from Euler angles (pitch, yaw, roll)
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll

    // Create view matrix: inverse of the camera's transformation matrix
    m_ViewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f), m_Position) * rotationMatrix);

    // Recalculate frustum planes after view matrix changes
    RecalculateFrustum();
}

void Camera3D::RecalculateFrustum()
{
    FENGINE_PROFILE_FUNCTION();

    // Get the view-projection matrix
    glm::mat4 viewProj = m_Projection * m_ViewMatrix;

    // Extract frustum planes from the view-projection matrix
    // Left plane
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Left)].x = viewProj[0][3] + viewProj[0][0];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Left)].y = viewProj[1][3] + viewProj[1][0];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Left)].z = viewProj[2][3] + viewProj[2][0];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Left)].w = viewProj[3][3] + viewProj[3][0];

    // Right plane
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Right)].x = viewProj[0][3] - viewProj[0][0];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Right)].y = viewProj[1][3] - viewProj[1][0];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Right)].z = viewProj[2][3] - viewProj[2][0];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Right)].w = viewProj[3][3] - viewProj[3][0];

    // Bottom plane
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Bottom)].x = viewProj[0][3] + viewProj[0][1];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Bottom)].y = viewProj[1][3] + viewProj[1][1];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Bottom)].z = viewProj[2][3] + viewProj[2][1];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Bottom)].w = viewProj[3][3] + viewProj[3][1];

    // Top plane
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Top)].x = viewProj[0][3] - viewProj[0][1];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Top)].y = viewProj[1][3] - viewProj[1][1];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Top)].z = viewProj[2][3] - viewProj[2][1];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Top)].w = viewProj[3][3] - viewProj[3][1];

    // Near plane
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Near)].x = viewProj[0][3] + viewProj[0][2];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Near)].y = viewProj[1][3] + viewProj[1][2];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Near)].z = viewProj[2][3] + viewProj[2][2];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Near)].w = viewProj[3][3] + viewProj[3][2];

    // Far plane
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Far)].x = viewProj[0][3] - viewProj[0][2];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Far)].y = viewProj[1][3] - viewProj[1][2];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Far)].z = viewProj[2][3] - viewProj[2][2];
    m_FrustumPlanes[static_cast<int>(FrustumPlane::Far)].w = viewProj[3][3] - viewProj[3][2];

    // Normalize all planes
    for (auto& plane : m_FrustumPlanes)
    {
        float length = glm::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        plane /= length;
    }
}

bool Camera3D::PointInFrustum(const glm::vec3& point) const
{
    // // Check if the point is inside all frustum planes
    // for (const auto& plane : m_FrustumPlanes)
    // {
    //     if (glm::dot(glm::vec3(plane), point) + plane.w < 0.0f)
    //         return false;
    // }

    return true;
}

bool Camera3D::SphereInFrustum(const glm::vec3& center, float radius) const
{
    // // Check if the sphere is completely outside any frustum plane
    // for (const auto& plane : m_FrustumPlanes)
    // {
    //     float distance = glm::dot(glm::vec3(plane), center) + plane.w;
    //     if (distance < -radius)
    //         return false;
    // }

    return true;
}

bool Camera3D::AABBInFrustum(const glm::vec3& min, const glm::vec3& max) const
{
    // // Check if the AABB is completely outside any frustum plane
    // for (const auto& plane : m_FrustumPlanes)
    // {
    //     glm::vec3 p;
    //     glm::vec3 n = glm::vec3(plane);
    //
    //     // Find the positive vertex (P-vertex)
    //     p.x = (n.x >= 0.0f) ? max.x : min.x;
    //     p.y = (n.y >= 0.0f) ? max.y : min.y;
    //     p.z = (n.z >= 0.0f) ? max.z : min.z;
    //
    //     // If the positive vertex is outside, the entire AABB is outside
    //     if (glm::dot(n, p) + plane.w < 0.0f)
    //         return false;
    // }

    return true;
}

void Camera3D::SetPosition(const glm::vec3& position)
{
    m_Position = position;
    //FENGINE_CORE_INFO("Camera position: ({},{},{})",position.x,position.y,position.z);
    RecalculateViewMatrix();
}

void Camera3D::SetRotation(const glm::vec3& rotation)
{
    m_Rotation = rotation;
    RecalculateViewMatrix();
}

void Camera3D::SetFocalPoint(const glm::vec3& focalPoint)
{
    m_FocalPoint = focalPoint;
    // If using focal point, you would adjust position or rotation here
    // For now, we just store it
}

glm::vec3 Camera3D::GetForwardDirection() const
{
    // Forward direction is -Z in view space
    glm::vec3 forward(0.0f, 0.0f, -1.0f);

    // Create rotation matrix from Euler angles
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Transform the forward vector
    return glm::normalize(glm::vec3(rotationMatrix * glm::vec4(forward, 0.0f)));
}

glm::vec3 Camera3D::GetRightDirection() const
{
    // Right direction is +X in view space
    glm::vec3 right(1.0f, 0.0f, 0.0f);

    // Create rotation matrix from Euler angles
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Transform the right vector
    return glm::normalize(glm::vec3(rotationMatrix * glm::vec4(right, 0.0f)));
}

glm::vec3 Camera3D::GetUpDirection() const
{
    // Up direction is +Y in view space
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    // Create rotation matrix from Euler angles
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Transform the up vector
    return glm::normalize(glm::vec3(rotationMatrix * glm::vec4(up, 0.0f)));
}

} // namespace BEngine