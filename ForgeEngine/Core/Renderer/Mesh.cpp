#include "Core/Renderer/Mesh.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include "VertexArray.h"

namespace ForgeEngine {

Mesh::Mesh() {
  m_VertexArray = VertexArray::Create();
  if (!m_VertexArray) {
    FENGINE_ASSERT(false, "Vertex Array Creation Failed");
    FENGINE_CORE_CRITICAL("Vertex Array Creation Failed");
  }
}

  void Mesh::SetVertices(std::vector<float>& vertices, uint32_t vertexCount) {
  m_VertexCount = vertexCount;

  m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));

  // ✅ Adicionar layout padrão ou exigir que seja definido externamente
  if (!m_VertexBuffer->GetLayout().GetElements().empty()) {
    m_VertexArray->AddVertexBuffer(m_VertexBuffer);
  } else {
    FENGINE_ASSERT(false, "Layout is not defined!")
  }
}

void Mesh::SetIndices(std::vector<uint32_t>& indices) {
  m_IndexCount = indices.size();

  m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());
  m_VertexArray->SetIndexBuffer(m_IndexBuffer);
}

void Mesh::SetVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {
  m_VertexBuffer = vertexBuffer;
  m_VertexArray->AddVertexBuffer(vertexBuffer);
}

void Mesh::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {
  m_IndexBuffer = indexBuffer;
  m_IndexCount = indexBuffer->GetCount();
  m_VertexArray->SetIndexBuffer(indexBuffer);
}

Ref<Mesh> Mesh::CreateCube(float size) {
  Ref<Mesh> mesh = CreateRef<Mesh>();

  // Define vertex layout
  BufferLayout layout = {
    { ShaderDataType::Float3, "a_Position" },
    { ShaderDataType::Float3, "a_Normal" },
    { ShaderDataType::Float3, "a_Tangent" },
    { ShaderDataType::Float2, "a_TexCoord" }
  };

  float halfSize = size * 0.5f;

  // 8 unique corner positions
  std::vector<glm::vec3> positions = {
      {-halfSize, -halfSize, halfSize},  // 0
      {halfSize, -halfSize, halfSize},   // 1
      {halfSize, halfSize, halfSize},    // 2
      {-halfSize, halfSize, halfSize},   // 3
      {-halfSize, -halfSize, -halfSize}, // 4
      {halfSize, -halfSize, -halfSize},  // 5
      {halfSize, halfSize, -halfSize},   // 6
      {-halfSize, halfSize, -halfSize}   // 7
  };

  // Build vertex attributes from positions
  std::vector<float> vertexData;
  vertexData.reserve(positions.size() * 11);

  auto computeTangent = [](const glm::vec3& normal) {
    glm::vec3 up = glm::abs(normal.y) > 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f)
                                              : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
    return tangent;
  };

  for (const auto& pos : positions) {
    glm::vec3 normal = glm::normalize(pos);
    glm::vec3 tangent = computeTangent(normal);
    glm::vec2 uv{pos.x / size + 0.5f, pos.y / size + 0.5f};

    vertexData.push_back(pos.x);
    vertexData.push_back(pos.y);
    vertexData.push_back(pos.z);
    vertexData.push_back(normal.x);
    vertexData.push_back(normal.y);
    vertexData.push_back(normal.z);
    vertexData.push_back(tangent.x);
    vertexData.push_back(tangent.y);
    vertexData.push_back(tangent.z);
    vertexData.push_back(uv.x);
    vertexData.push_back(uv.y);
  }

  // Indices referencing the 8 vertices
  std::vector<uint32_t> cubeIndices = {
      0, 1, 2, 2, 3, 0,       // Front
      1, 5, 6, 6, 2, 1,       // Right
      5, 4, 7, 7, 6, 5,       // Back
      4, 0, 3, 3, 7, 4,       // Left
      4, 5, 1, 1, 0, 4,       // Bottom
      3, 2, 6, 6, 7, 3        // Top
  };

  Ref<VertexBuffer> vertexBuffer =
      VertexBuffer::Create(vertexData.data(), vertexData.size() * sizeof(float));
  vertexBuffer->SetLayout(layout);
  mesh->SetVertexBuffer(vertexBuffer);
  mesh->m_VertexCount = positions.size();

  mesh->SetIndices(cubeIndices);

  return mesh;
}

Ref<Mesh> Mesh::CreateSphere(float radius, uint32_t segmentsX, uint32_t segmentsY) {
  Ref<Mesh> mesh = CreateRef<Mesh>();

  // Define vertex layout
  BufferLayout layout = {
    { ShaderDataType::Float3, "a_Position" },
    { ShaderDataType::Float3, "a_Normal" },
    { ShaderDataType::Float3, "a_Tangent" },
    { ShaderDataType::Float2, "a_TexCoord" }
  };

  std::vector<float> vertices;
  std::vector<uint32_t> indices;

  const float PI = static_cast<float>(M_PI);

  // Calculate vertices
  for (uint32_t y = 0; y <= segmentsY; ++y) {
    for (uint32_t x = 0; x <= segmentsX; ++x) {
      // Calculate position
      float xSegment = (float)x / (float)segmentsX;
      float ySegment = (float)y / (float)segmentsY;
      float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
      float yPos = std::cos(ySegment * PI);
      float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

      // Calculate normal (same as position for a sphere)
      float nx = xPos;
      float ny = yPos;
      float nz = zPos;

      // Calculate tangent
      float tx = -std::sin(xSegment * 2.0f * PI);
      float ty = 0.0f;
      float tz = std::cos(xSegment * 2.0f * PI);

      // Normalize tangent
      float length = std::sqrt(tx * tx + ty * ty + tz * tz);
      if (length > 0.0f) {
        tx /= length;
        ty /= length;
        tz /= length;
      }

      // Add to vertices
      vertices.push_back(xPos * radius);    // Position X
      vertices.push_back(yPos * radius);    // Position Y
      vertices.push_back(zPos * radius);    // Position Z
      vertices.push_back(nx);               // Normal X
      vertices.push_back(ny);               // Normal Y
      vertices.push_back(nz);               // Normal Z
      vertices.push_back(tx);               // Tangent X
      vertices.push_back(ty);               // Tangent Y
      vertices.push_back(tz);               // Tangent Z
      vertices.push_back(xSegment);         // Texture U
      vertices.push_back(ySegment);         // Texture V
    }
  }

  // Calculate indices
  for (uint32_t y = 0; y < segmentsY; ++y) {
    for (uint32_t x = 0; x < segmentsX; ++x) {
      uint32_t first = (y * (segmentsX + 1)) + x;
      uint32_t second = first + 1;
      uint32_t third = first + (segmentsX + 1);
      uint32_t fourth = third + 1;

      indices.push_back(first);
      indices.push_back(third);
      indices.push_back(second);

      indices.push_back(second);
      indices.push_back(third);
      indices.push_back(fourth);
    }
  }

  // Set up the mesh
  Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));
  vertexBuffer->SetLayout(layout);
  mesh->SetVertexBuffer(vertexBuffer);
  mesh->m_VertexCount = (segmentsX + 1) * (segmentsY + 1);

  mesh->SetIndices(indices);

  return mesh;
}

Ref<Mesh> Mesh::CreateCylinder(float radius, float height, uint32_t segments) {
  Ref<Mesh> mesh = CreateRef<Mesh>();

  // Define vertex layout
  BufferLayout layout = {
    { ShaderDataType::Float3, "a_Position" },
    { ShaderDataType::Float3, "a_Normal" },
    { ShaderDataType::Float3, "a_Tangent" },
    { ShaderDataType::Float2, "a_TexCoord" }
  };

  std::vector<float> vertices;
  std::vector<uint32_t> indices;

  const float PI = static_cast<float>(M_PI);
  float halfHeight = height * 0.5f;

  // Create top and bottom center vertices
  // Top center vertex (center top)
  vertices.push_back(0.0f);            // Position X
  vertices.push_back(halfHeight);      // Position Y
  vertices.push_back(0.0f);            // Position Z
  vertices.push_back(0.0f);            // Normal X
  vertices.push_back(1.0f);            // Normal Y
  vertices.push_back(0.0f);            // Normal Z
  vertices.push_back(1.0f);            // Tangent X
  vertices.push_back(0.0f);            // Tangent Y
  vertices.push_back(0.0f);            // Tangent Z
  vertices.push_back(0.5f);            // Texture U
  vertices.push_back(0.5f);            // Texture V

  // Bottom center vertex (center bottom)
  vertices.push_back(0.0f);            // Position X
  vertices.push_back(-halfHeight);     // Position Y
  vertices.push_back(0.0f);            // Position Z
  vertices.push_back(0.0f);            // Normal X
  vertices.push_back(-1.0f);           // Normal Y
  vertices.push_back(0.0f);            // Normal Z
  vertices.push_back(1.0f);            // Tangent X
  vertices.push_back(0.0f);            // Tangent Y
  vertices.push_back(0.0f);            // Tangent Z
  vertices.push_back(0.5f);            // Texture U
  vertices.push_back(0.5f);            // Texture V

  // Create vertices for the cylinder sides, top, and bottom
  for (uint32_t i = 0; i <= segments; i++) {
    float angle = (float)i / (float)segments * 2.0f * PI;
    float x = std::cos(angle);
    float z = std::sin(angle);

    // Top rim vertex
    vertices.push_back(x * radius);     // Position X
    vertices.push_back(halfHeight);     // Position Y
    vertices.push_back(z * radius);     // Position Z
    vertices.push_back(0.0f);           // Normal X
    vertices.push_back(1.0f);           // Normal Y
    vertices.push_back(0.0f);           // Normal Z
    vertices.push_back(1.0f);           // Tangent X
    vertices.push_back(0.0f);           // Tangent Y
    vertices.push_back(0.0f);           // Tangent Z
    vertices.push_back((x + 1.0f) * 0.5f); // Texture U
    vertices.push_back((z + 1.0f) * 0.5f); // Texture V

    // Bottom rim vertex
    vertices.push_back(x * radius);     // Position X
    vertices.push_back(-halfHeight);    // Position Y
    vertices.push_back(z * radius);     // Position Z
    vertices.push_back(0.0f);           // Normal X
    vertices.push_back(-1.0f);          // Normal Y
    vertices.push_back(0.0f);           // Normal Z
    vertices.push_back(1.0f);           // Tangent X
    vertices.push_back(0.0f);           // Tangent Y
    vertices.push_back(0.0f);           // Tangent Z
    vertices.push_back((x + 1.0f) * 0.5f); // Texture U
    vertices.push_back((z + 1.0f) * 0.5f); // Texture V

    // Side vertex top
    vertices.push_back(x * radius);     // Position X
    vertices.push_back(halfHeight);     // Position Y
    vertices.push_back(z * radius);     // Position Z
    vertices.push_back(x);              // Normal X
    vertices.push_back(0.0f);           // Normal Y
    vertices.push_back(z);              // Normal Z
    vertices.push_back(-z);             // Tangent X
    vertices.push_back(0.0f);           // Tangent Y
    vertices.push_back(x);              // Tangent Z
    vertices.push_back(float(i) / float(segments)); // Texture U
    vertices.push_back(1.0f);           // Texture V

    // Side vertex bottom
    vertices.push_back(x * radius);     // Position X
    vertices.push_back(-halfHeight);    // Position Y
    vertices.push_back(z * radius);     // Position Z
    vertices.push_back(x);              // Normal X
    vertices.push_back(0.0f);           // Normal Y
    vertices.push_back(z);              // Normal Z
    vertices.push_back(-z);             // Tangent X
    vertices.push_back(0.0f);           // Tangent Y
    vertices.push_back(x);              // Tangent Z
    vertices.push_back(float(i) / float(segments)); // Texture U
    vertices.push_back(0.0f);           // Texture V
  }

  // Create indices
  uint32_t topCenterIndex = 0;
  uint32_t bottomCenterIndex = 1;
  uint32_t topRimStartIndex = 2;
  uint32_t bottomRimStartIndex = 3;
  uint32_t sideTopStartIndex = 4;
  uint32_t sideBottomStartIndex = 5;

  // Top face indices
  for (uint32_t i = 0; i < segments; i++) {
    indices.push_back(topCenterIndex);
    indices.push_back(topRimStartIndex + i * 4);
    indices.push_back(topRimStartIndex + ((i + 1) % segments) * 4);
  }

  // Bottom face indices
  for (uint32_t i = 0; i < segments; i++) {
    indices.push_back(bottomCenterIndex);
    indices.push_back(bottomRimStartIndex + ((i + 1) % segments) * 4);
    indices.push_back(bottomRimStartIndex + i * 4);
  }

  // Side face indices
  for (uint32_t i = 0; i < segments; i++) {
    uint32_t nextI = (i + 1) % segments;

    indices.push_back(sideTopStartIndex + i * 4);
    indices.push_back(sideBottomStartIndex + i * 4);
    indices.push_back(sideBottomStartIndex + nextI * 4);

    indices.push_back(sideTopStartIndex + i * 4);
    indices.push_back(sideBottomStartIndex + nextI * 4);
    indices.push_back(sideTopStartIndex + nextI * 4);
  }

  // Set up the mesh
  Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));
  vertexBuffer->SetLayout(layout);
  mesh->SetVertexBuffer(vertexBuffer);
  mesh->m_VertexCount = 2 + 4 * (segments + 1); // Centers + 4 vertices per segment

  mesh->SetIndices(indices);

  return mesh;
}

Ref<Mesh> Mesh::CreatePlane(float width, float height) {
  Ref<Mesh> mesh = CreateRef<Mesh>();

  // Define vertex layout
  BufferLayout layout = {
    { ShaderDataType::Float3, "a_Position" },
    { ShaderDataType::Float3, "a_Normal" },
    { ShaderDataType::Float3, "a_Tangent" },
    { ShaderDataType::Float2, "a_TexCoord" }
  };

  float halfWidth = width * 0.5f;
  float halfHeight = height * 0.5f;

  // Create plane vertices (facing up in Y direction)
  std::vector<float> vertices = {
    // Position (3), Normal (3), Tangent (3), TexCoord (2)
    -halfWidth, 0.0f, -halfHeight,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     halfWidth, 0.0f, -halfHeight,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     halfWidth, 0.0f,  halfHeight,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -halfWidth, 0.0f,  halfHeight,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f
  };

  // Create plane indices
  std::vector<uint32_t> indices = {
    1, 0, 2,
    2, 0, 3
  };

  // Set up the mesh
  Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));
  vertexBuffer->SetLayout(layout);
  mesh->SetVertexBuffer(vertexBuffer);
  mesh->m_VertexCount = 4;

  mesh->SetIndices(indices);

  return mesh;
}

// Model implementation

Ref<Model> Model::Load(const std::string& filepath) {
  // This is a placeholder - in a real engine, this would use a model loading library
  // like Assimp to load models from various formats (OBJ, FBX, etc.)

  Ref<Model> model = CreateRef<Model>();

  // For now, we'll just return a simple cube model as an example
  model->AddMesh(Mesh::CreateCube());

  return model;
}

} // namespace BEngine