#include "Core/Renderer/Renderer3D.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Config.h"
#include "Core/Renderer/RenderCommand.h"
#include "Core/Renderer/Shader.h"
#include "Core/Renderer/UniformBuffer.h"
#include "Core/Renderer/VertexArray.h"

namespace BEngine {

struct MeshVertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec3 Tangent;
  glm::vec2 TexCoord;
  glm::vec4 Color;

  // Editor-only
  int EntityID;
};

struct LineVertex3D {
  glm::vec3 Position;
  glm::vec4 Color;

  // Editor-only
  int EntityID;
};

struct Renderer3DData {
  static constexpr uint32_t MaxVertices = 100000;
  static constexpr uint32_t MaxIndices = 200000;
  static constexpr uint32_t MaxTextureSlots = 32;

  // Active camera for frustum culling
  const Camera3D* ActiveCamera = nullptr;

  // Entity data for culling
  struct EntityCullingData {
    glm::vec3 BoundingBoxMin;
    glm::vec3 BoundingBoxMax;
    float BoundingSphereRadius;
    bool WasVisible = true;
  };
  std::unordered_map<int, EntityCullingData> EntityCullingInfo;

  // Add tracking of visible entities for culling optimization
  uint32_t VisibleMeshCount = 0;
  uint32_t TotalMeshCount = 0;

  // Last frame statistics for monitoring
  Renderer3D::Statistics LastFrameStats;

  // Meshes
  Ref<Shader> MeshShader;
  Ref<Shader> WireframeShader;

  // Debug rendering
  Ref<VertexArray> LineVertexArray;
  Ref<VertexBuffer> LineVertexBuffer;
  Ref<Shader> LineShader;
  uint32_t LineVertexCount = 0;
  LineVertex3D* LineVertexBufferBase = nullptr;
  LineVertex3D* LineVertexBufferPtr = nullptr;
  float LineWidth = 2.0f;

  // Primitive meshes
  Ref<Mesh> CubeMesh;
  Ref<Mesh> SphereMesh;

  // Default materials
  Ref<Material> DefaultMaterial;

  // Texture slots
  std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
  uint32_t TextureSlotIndex = 1;  // 0 = white texture
  Ref<Texture2D> WhiteTexture;

  // Lighting data
  glm::vec3 PointLightPosition = {0.0f, 3.0f, 0.0f};
  glm::vec3 AmbientLightColor = {1.0f, 1.0f, 1.0f};
  float AmbientLightIntensity = 0.1f;

  // Settings
  bool WireframeMode = false;

  // Camera data
  struct CameraData {
    glm::mat4 ViewProjection;
    glm::vec3 CameraPosition;
  };
  CameraData CameraBuffer{};
  Ref<UniformBuffer> CameraUniformBuffer;

  // Lighting data
  struct LightData {
    glm::vec3 PointLightPosition;
    float PointLightIntensity = 1.0f;
    glm::vec3 AmbientLightColor;
    float AmbientLightIntensity;
  };
  LightData LightBuffer{};
  Ref<UniformBuffer> LightUniformBuffer;

  // Stats
  Renderer3D::Statistics Stats;
};

static Renderer3DData s_Data;

// Utility functions
glm::mat4 CreateTransformMatrix(const glm::vec3& position,
                                const glm::vec3& scale,
                                const glm::vec3& rotation) {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
  transform = glm::rotate(transform, glm::radians(rotation.x),
                          glm::vec3(1.0f, 0.0f, 0.0f));
  transform = glm::rotate(transform, glm::radians(rotation.y),
                          glm::vec3(0.0f, 1.0f, 0.0f));
  transform = glm::rotate(transform, glm::radians(rotation.z),
                          glm::vec3(0.0f, 0.0f, 1.0f));
  transform = glm::scale(transform, scale);
  return transform;
}

void Renderer3D::Init() {
  BENGINE_PROFILE_FUNCTION();

  // Create shaders
  s_Data.MeshShader = Shader::Create(
      "C:/_PROJECTS/block_application/block_application/assets/shaders/"
      "Renderer3D_Mesh.glsl");
  s_Data.WireframeShader = Shader::Create(
      "C:/_PROJECTS/block_application/block_application/assets/shaders/"
      "Renderer3D_Wireframe.glsl");
  s_Data.LineShader = Shader::Create(
      "C:/_PROJECTS/block_application/block_application/assets/shaders/"
      "Renderer3D_Line.glsl");

  if (s_Data.MeshShader == nullptr)
    BENGINE_CORE_CRITICAL("Mesh shader not found");

  // Create line rendering resources
  s_Data.LineVertexArray = VertexArray::Create();
  s_Data.LineVertexBuffer =
      VertexBuffer::Create(Renderer3DData::MaxVertices * sizeof(LineVertex3D));
  s_Data.LineVertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                      {ShaderDataType::Float4, "a_Color"},
                                      {ShaderDataType::Int, "a_EntityID"}});
  s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
  s_Data.LineVertexBufferBase = new LineVertex3D[Renderer3DData::MaxVertices];

  // Create white texture
  s_Data.WhiteTexture = Texture2D::Create(TextureSpecification());
  uint32_t whiteTextureData = 0xffffffff;
  s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

  // Set first texture slot to the white texture
  s_Data.TextureSlots[0] = s_Data.WhiteTexture;

  // Create primitive meshes
  PreparePrimitives();

  // Create default material
  s_Data.DefaultMaterial = CreateRef<Material>();
  s_Data.DefaultMaterial->SetAlbedoMap(s_Data.WhiteTexture);
  s_Data.DefaultMaterial->SetRoughness(0.5f);
  s_Data.DefaultMaterial->SetMetallic(0.0f);

  // Create uniform buffers
  s_Data.CameraUniformBuffer =
      UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
  s_Data.LightUniformBuffer =
      UniformBuffer::Create(sizeof(Renderer3DData::LightData), 1);

  // Set default lighting data
  s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
  s_Data.LightBuffer.PointLightIntensity = 1.0f;
  s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
  s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
  s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                     sizeof(Renderer3DData::LightData));
}

void Renderer3D::Shutdown() {
  BENGINE_PROFILE_FUNCTION();

  delete[] s_Data.LineVertexBufferBase;
}

void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform) {
  BENGINE_PROFILE_FUNCTION();

  s_Data.CameraBuffer.ViewProjection =
      camera.GetProjection() * glm::inverse(transform);
  s_Data.CameraBuffer.CameraPosition = glm::vec3(glm::inverse(transform)[3]);
  s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer,
                                      sizeof(Renderer3DData::CameraData));

  // Update lighting data
  s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
  s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
  s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
  s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                     sizeof(Renderer3DData::LightData));

  // Clear active camera since this is a generic Camera
  s_Data.ActiveCamera = nullptr;

  StartBatch();
}

void Renderer3D::BeginScene(const EditorCamera& camera) {
  BENGINE_PROFILE_FUNCTION();

  s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
  s_Data.CameraBuffer.CameraPosition = camera.GetPosition();
  s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer,
                                      sizeof(Renderer3DData::CameraData));

  // Update lighting data
  s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
  s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
  s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
  s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                     sizeof(Renderer3DData::LightData));

  // Clear active camera since this is not a Camera3D
  s_Data.ActiveCamera = nullptr;

  StartBatch();
}

void Renderer3D::BeginScene(const Camera3D& camera) {
  BENGINE_PROFILE_FUNCTION();

  s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
  s_Data.CameraBuffer.CameraPosition = camera.GetPosition();
  s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer,
                                      sizeof(Renderer3DData::CameraData));

  // Update lighting data
  s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
  s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
  s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
  s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                     sizeof(Renderer3DData::LightData));

  // Store the camera for frustum culling
  s_Data.ActiveCamera = &camera;

  StartBatch();
}

void Renderer3D::BeginScene(const Camera3DController& cameraController) {
  BeginScene(cameraController.GetCamera());
}

bool Renderer3D::IsPointVisible(const glm::vec3& point) {
  // Use the currently active camera's frustum culling
  if (s_Data.ActiveCamera) {
    return s_Data.ActiveCamera->PointInFrustum(point);
  }
  return true;  // If no camera is active, consider everything visible
}

bool Renderer3D::IsSphereVisible(const glm::vec3& center, float radius) {
  if (s_Data.ActiveCamera) {
    return s_Data.ActiveCamera->SphereInFrustum(center, radius);
  }
  return true;
}

bool Renderer3D::IsAABBVisible(const glm::vec3& min, const glm::vec3& max) {
  if (s_Data.ActiveCamera) {
    return s_Data.ActiveCamera->AABBInFrustum(min, max);
  }
  return true;
}

bool Renderer3D::IsEntityVisible(int entityID, const glm::mat4& transform,
                                 float boundingSphereRadius) {
  // Extract position from transform matrix
  glm::vec3 position(transform[3][0], transform[3][1], transform[3][2]);

  // Use sphere test for quick culling
  return IsSphereVisible(position, boundingSphereRadius);
}

void Renderer3D::EndScene() {
  BENGINE_PROFILE_FUNCTION();

  // Update culling statistics before flushing
  s_Data.Stats.VisibleMeshCount = s_Data.VisibleMeshCount;
  s_Data.Stats.MeshCount = s_Data.TotalMeshCount;
  s_Data.Stats.CulledMeshCount = s_Data.TotalMeshCount - s_Data.VisibleMeshCount;

  Flush();
}

void Renderer3D::StartBatch() {
  s_Data.LineVertexCount = 0;
  s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

  s_Data.TextureSlotIndex = 1;
}

void Renderer3D::Flush() {
  BENGINE_PROFILE_FUNCTION();

  // Flush lines if we have any
  if (s_Data.LineVertexCount) {
    uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr -
                                   (uint8_t*)s_Data.LineVertexBufferBase);
    s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

    s_Data.LineShader->Bind();
    RenderCommand::SetLineWidth(s_Data.LineWidth);
    RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
    s_Data.Stats.DrawCalls++;
  }
}

void Renderer3D::NextBatch() {
  Flush();
  StartBatch();
}

void Renderer3D::PreparePrimitives() {
  // Create cube mesh
  // Cube vertices and indices would be created here
  // This is a simplified version - in a real engine, you would create proper
  // mesh data
  s_Data.CubeMesh = CreateRef<Mesh>();

  // Same for sphere
  s_Data.SphereMesh = CreateRef<Mesh>();
}

void Renderer3D::DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                          const glm::vec4& color, int entityID) {
  BENGINE_PROFILE_FUNCTION();

  // If we have an active camera and entityID is valid, check visibility
  if (s_Data.ActiveCamera && entityID >= 0) {
    // Update or create entity culling data
    auto& cullingData = s_Data.EntityCullingInfo[entityID];

    // If this is the first time we're seeing this entity, calculate its bounds
    if (cullingData.BoundingSphereRadius == 0.0f) {
      // In a real implementation, you would compute actual bounds from the mesh
      // This is a simplified version using a fixed radius
      cullingData.BoundingSphereRadius = 1.0f;
    }

    s_Data.TotalMeshCount++;

    // Perform frustum culling
    if (!IsEntityVisible(entityID, transform,
                         cullingData.BoundingSphereRadius)) {
      cullingData.WasVisible = false;
      return;  // Skip rendering this mesh
    }

    cullingData.WasVisible = true;
    s_Data.VisibleMeshCount++;
  }

  // Configure default material with color
  s_Data.DefaultMaterial->SetAlbedoColor(color);

  DrawMesh(transform, mesh, s_Data.DefaultMaterial, entityID);
}

void Renderer3D::DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                          Ref<Material> material, int entityID) {
  BENGINE_PROFILE_FUNCTION();

  // If we have an active camera and entityID is valid, check visibility
  if (s_Data.ActiveCamera && entityID >= 0) {
    // Update or create entity culling data
    auto& cullingData = s_Data.EntityCullingInfo[entityID];

    // If this is the first time we're seeing this entity, calculate its bounds
    if (cullingData.BoundingSphereRadius == 0.0f) {
      // In a real implementation, you would compute actual bounds from the mesh
      // This is a simplified version using a fixed radius
      cullingData.BoundingSphereRadius = 1.0f;
    }

    s_Data.TotalMeshCount++;

    // Perform frustum culling
    if (!IsEntityVisible(entityID, transform,
                         cullingData.BoundingSphereRadius)) {
      cullingData.WasVisible = false;
      return;  // Skip rendering this mesh
    }

    cullingData.WasVisible = true;
    s_Data.VisibleMeshCount++;
  }

  // Bind material textures
  if (material->GetAlbedoMap())
    material->GetAlbedoMap()->Bind(0);
  else
    s_Data.WhiteTexture->Bind(0);

  if (material->GetNormalMap())
    material->GetNormalMap()->Bind(1);
  else
    s_Data.WhiteTexture->Bind(1);

  if (material->GetMetallicMap())
    material->GetMetallicMap()->Bind(2);
  else
    s_Data.WhiteTexture->Bind(2);

  if (material->GetRoughnessMap())
    material->GetRoughnessMap()->Bind(3);
  else
    s_Data.WhiteTexture->Bind(3);

  // Use wireframe or standard shader based on the setting
  if (s_Data.WireframeMode) {
    s_Data.WireframeShader->Bind();
    s_Data.WireframeShader->SetMat4("u_Transform", transform);
    s_Data.WireframeShader->SetFloat4("u_Color", material->GetAlbedoColor());
  } else {
    s_Data.MeshShader->Bind();
    s_Data.MeshShader->SetMat4("u_Transform", transform);
    s_Data.MeshShader->SetFloat4("u_MaterialAlbedoColor",
                                 material->GetAlbedoColor());
    s_Data.MeshShader->SetFloat("u_MaterialMetallic", material->GetMetallic());
    s_Data.MeshShader->SetFloat("u_MaterialRoughness",
                                material->GetRoughness());
    s_Data.MeshShader->SetInt("u_AlbedoMap", 0);
    s_Data.MeshShader->SetInt("u_NormalMap", 1);
    s_Data.MeshShader->SetInt("u_MetallicMap", 2);
    s_Data.MeshShader->SetInt("u_RoughnessMap", 3);
    s_Data.MeshShader->SetInt("u_EntityID", entityID);
  }

  // Draw the mesh
  mesh->GetVertexArray()->Bind();
  RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetIndexCount());

  s_Data.Stats.DrawCalls++;
  s_Data.Stats.MeshCount++;
  s_Data.Stats.VertexCount += mesh->GetVertexCount();
  s_Data.Stats.IndexCount += mesh->GetIndexCount();
}

void Renderer3D::DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                          const glm::vec3& rotation, Ref<Mesh> mesh,
                          const glm::vec4& color, int entityID) {
  glm::mat4 transform = CreateTransformMatrix(position, scale, rotation);
  DrawMesh(transform, mesh, color, entityID);
}

void Renderer3D::DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                          const glm::vec3& rotation, Ref<Mesh> mesh,
                          Ref<Material> material, int entityID) {
  glm::mat4 transform = CreateTransformMatrix(position, scale, rotation);
  DrawMesh(transform, mesh, material, entityID);
}

void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size,
                          const glm::vec4& color, int entityID) {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), size);

  DrawCube(transform, color, entityID);
}

void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size,
                          Ref<Material> material, int entityID) {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), size);

  DrawCube(transform, material, entityID);
}

void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec4& color,
                          int entityID) {
  DrawMesh(transform, s_Data.CubeMesh, color, entityID);
}

void Renderer3D::DrawCube(const glm::mat4& transform, Ref<Material> material,
                          int entityID) {
  DrawMesh(transform, s_Data.CubeMesh, material, entityID);
}

void Renderer3D::DrawSphere(const glm::vec3& position, float radius,
                            const glm::vec4& color, int entityID) {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(radius));

  DrawSphere(transform, color, entityID);
}

void Renderer3D::DrawSphere(const glm::vec3& position, float radius,
                            Ref<Material> material, int entityID) {
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(radius));

  DrawSphere(transform, material, entityID);
}

void Renderer3D::DrawSphere(const glm::mat4& transform, const glm::vec4& color,
                            int entityID) {
  DrawMesh(transform, s_Data.SphereMesh, color, entityID);
}

void Renderer3D::DrawSphere(const glm::mat4& transform, Ref<Material> material,
                            int entityID) {
  DrawMesh(transform, s_Data.SphereMesh, material, entityID);
}

void Renderer3D::DrawLine3D(const glm::vec3& p0, const glm::vec3& p1,
                            const glm::vec4& color, int entityID) {
  s_Data.LineVertexBufferPtr->Position = p0;
  s_Data.LineVertexBufferPtr->Color = color;
  s_Data.LineVertexBufferPtr->EntityID = entityID;
  s_Data.LineVertexBufferPtr++;

  s_Data.LineVertexBufferPtr->Position = p1;
  s_Data.LineVertexBufferPtr->Color = color;
  s_Data.LineVertexBufferPtr->EntityID = entityID;
  s_Data.LineVertexBufferPtr++;

  s_Data.LineVertexCount += 2;
}

void Renderer3D::DrawBox(const glm::vec3& position, const glm::vec3& size,
                         const glm::vec4& color, int entityID) {
  glm::vec3 halfSize = size * 0.5f;

  // Calculate the 8 corners of the box
  glm::vec3 p0 = position + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z);
  glm::vec3 p1 = position + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z);
  glm::vec3 p2 = position + glm::vec3(halfSize.x, -halfSize.y, halfSize.z);
  glm::vec3 p3 = position + glm::vec3(-halfSize.x, -halfSize.y, halfSize.z);
  glm::vec3 p4 = position + glm::vec3(-halfSize.x, halfSize.y, -halfSize.z);
  glm::vec3 p5 = position + glm::vec3(halfSize.x, halfSize.y, -halfSize.z);
  glm::vec3 p6 = position + glm::vec3(halfSize.x, halfSize.y, halfSize.z);
  glm::vec3 p7 = position + glm::vec3(-halfSize.x, halfSize.y, halfSize.z);

  // Bottom face
  DrawLine3D(p0, p1, color, entityID);
  DrawLine3D(p1, p2, color, entityID);
  DrawLine3D(p2, p3, color, entityID);
  DrawLine3D(p3, p0, color, entityID);

  // Top face
  DrawLine3D(p4, p5, color, entityID);
  DrawLine3D(p5, p6, color, entityID);
  DrawLine3D(p6, p7, color, entityID);
  DrawLine3D(p7, p4, color, entityID);

  // Connecting edges
  DrawLine3D(p0, p4, color, entityID);
  DrawLine3D(p1, p5, color, entityID);
  DrawLine3D(p2, p6, color, entityID);
  DrawLine3D(p3, p7, color, entityID);
}

void Renderer3D::DrawBox(const glm::mat4& transform, const glm::vec4& color,
                         int entityID) {
  // Get the 8 corners of a unit cube
  glm::vec3 p0 = glm::vec3(-0.5f, -0.5f, -0.5f);
  glm::vec3 p1 = glm::vec3(0.5f, -0.5f, -0.5f);
  glm::vec3 p2 = glm::vec3(0.5f, -0.5f, 0.5f);
  glm::vec3 p3 = glm::vec3(-0.5f, -0.5f, 0.5f);
  glm::vec3 p4 = glm::vec3(-0.5f, 0.5f, -0.5f);
  glm::vec3 p5 = glm::vec3(0.5f, 0.5f, -0.5f);
  glm::vec3 p6 = glm::vec3(0.5f, 0.5f, 0.5f);
  glm::vec3 p7 = glm::vec3(-0.5f, 0.5f, 0.5f);

  // Transform the corners
  p0 = transform * glm::vec4(p0, 1.0f);
  p1 = transform * glm::vec4(p1, 1.0f);
  p2 = transform * glm::vec4(p2, 1.0f);
  p3 = transform * glm::vec4(p3, 1.0f);
  p4 = transform * glm::vec4(p4, 1.0f);
  p5 = transform * glm::vec4(p5, 1.0f);
  p6 = transform * glm::vec4(p6, 1.0f);
  p7 = transform * glm::vec4(p7, 1.0f);

  // Bottom face
  DrawLine3D(p0, p1, color, entityID);
  DrawLine3D(p1, p2, color, entityID);
  DrawLine3D(p2, p3, color, entityID);
  DrawLine3D(p3, p0, color, entityID);

  // Top face
  DrawLine3D(p4, p5, color, entityID);
  DrawLine3D(p5, p6, color, entityID);
  DrawLine3D(p6, p7, color, entityID);
  DrawLine3D(p7, p4, color, entityID);

  // Connecting edges
  DrawLine3D(p0, p4, color, entityID);
  DrawLine3D(p1, p5, color, entityID);
  DrawLine3D(p2, p6, color, entityID);
  DrawLine3D(p3, p7, color, entityID);
}

void Renderer3D::DrawModel(const glm::mat4& transform,
                           ModelRendererComponent& src, int entityID) {
  if (!src.Model) return;

  // Draw meshes in the model
  for (const auto& mesh : src.Model->GetMeshes()) {
    if (src.OverrideMaterial)
      DrawMesh(transform, mesh, src.OverrideMaterial, entityID);
    else if (mesh->GetMaterial())
      DrawMesh(transform, mesh, mesh->GetMaterial(), entityID);
    else
      DrawMesh(transform, mesh, s_Data.DefaultMaterial, entityID);
  }
}

void Renderer3D::SetPointLightPosition(const glm::vec3& position) {
  s_Data.PointLightPosition = position;
}

void Renderer3D::SetAmbientLight(const glm::vec3& color, float intensity) {
  s_Data.AmbientLightColor = color;
  s_Data.AmbientLightIntensity = intensity;
}

void Renderer3D::EnableWireframe(bool enable) { s_Data.WireframeMode = enable; }

bool Renderer3D::IsWireframeEnabled() { return s_Data.WireframeMode; }

void Renderer3D::ResetStats() {
  // Save last frame stats before resetting
  s_Data.LastFrameStats = s_Data.Stats;

  s_Data.Stats.DrawCalls = 0;
  s_Data.Stats.MeshCount = 0;
  s_Data.Stats.VisibleMeshCount = 0;
  s_Data.Stats.CulledMeshCount = 0;
  s_Data.Stats.VertexCount = 0;
  s_Data.Stats.IndexCount = 0;

  s_Data.VisibleMeshCount = 0;
  s_Data.TotalMeshCount = 0;
}

Renderer3D::Statistics Renderer3D::GetStats() { return s_Data.Stats; }

}  // namespace BEngine