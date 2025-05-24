#include "Core/Renderer/TestMesh.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include "VertexArray.h"

namespace ForgeEngine
{
    // Estrutura para representar um vértice completo
    struct Vertex
    {
        float position[3];
        float normal[3];
        float tangent[3];
        float texCoord[2];

        Vertex(float px, float py, float pz,
               float nx, float ny, float nz,
               float tx, float ty, float tz,
               float u, float v)
        {
            position[0] = px;
            position[1] = py;
            position[2] = pz;
            normal[0] = nx;
            normal[1] = ny;
            normal[2] = nz;
            tangent[0] = tx;
            tangent[1] = ty;
            tangent[2] = tz;
            texCoord[0] = u;
            texCoord[1] = v;
        }
    };

    TestMesh::TestMesh()
    {
        m_VertexArray = VertexArray::Create();
        if (!m_VertexArray)
        {
            FENGINE_ASSERT(false, "Vertex Array Creation Failed");
            FENGINE_CORE_CRITICAL("Vertex Array Creation Failed");
        }
    }

    void TestMesh::SetVertices(std::vector<float>& vertices, uint32_t vertexCount)
    {
        m_VertexCount = vertexCount;

        m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(float));

        if (!m_VertexBuffer->GetLayout().GetElements().empty())
        {
            m_VertexArray->AddVertexBuffer(m_VertexBuffer);
        }
        else
        {
            FENGINE_ASSERT(false, "Layout is not defined!")
        }
    }

    void TestMesh::SetIndices(std::vector<uint32_t>& indices)
    {
        m_IndexCount = indices.size();

        m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size());
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);
    }

    void TestMesh::SetVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
    {
        m_VertexBuffer = vertexBuffer;
        m_VertexArray->AddVertexBuffer(vertexBuffer);
    }

    void TestMesh::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        m_IndexBuffer = indexBuffer;
        m_IndexCount = indexBuffer->GetCount();
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    // ============================================================================
    // FUNÇÃO BASE: CRIAÇÃO DE TRIÂNGULOS
    // ============================================================================

    Ref<Mesh> TestMesh::CreateTriangles(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        Ref<Mesh> mesh = CreateRef<Mesh>();

        // Define vertex layout padrão
        BufferLayout layout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float2, "a_TexCoord"}
        };

        // Converter estrutura Vertex para array de floats
        std::vector<float> vertexData;
        vertexData.reserve(vertices.size() * 11); // 11 floats per vertex

        for (const auto& vertex : vertices)
        {
            // Position (3)
            vertexData.push_back(vertex.position[0]);
            vertexData.push_back(vertex.position[1]);
            vertexData.push_back(vertex.position[2]);

            // Normal (3)
            vertexData.push_back(vertex.normal[0]);
            vertexData.push_back(vertex.normal[1]);
            vertexData.push_back(vertex.normal[2]);

            // Tangent (3)
            vertexData.push_back(vertex.tangent[0]);
            vertexData.push_back(vertex.tangent[1]);
            vertexData.push_back(vertex.tangent[2]);

            // TexCoord (2)
            vertexData.push_back(vertex.texCoord[0]);
            vertexData.push_back(vertex.texCoord[1]);
        }

        // Criar vertex buffer
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertexData.data(), vertexData.size() * sizeof(float));
        vertexBuffer->SetLayout(layout);
        mesh->SetVertexBuffer(vertexBuffer);
        mesh->m_VertexCount = vertices.size();

        // Criar index buffer se fornecido
        if (!indices.empty())
        {
            std::vector<uint32_t> indexCopy = indices; // CreateRef precisa de non-const
            mesh->SetIndices(indexCopy);
        }

        return mesh;
    }

    // ============================================================================
    // FUNÇÃO DERIVADA: TRIÂNGULO ÚNICO
    // ============================================================================

    Ref<Mesh> TestMesh::CreateTriangle(float size)
    {
        float halfSize = size * 0.5f;
        float height = size * 0.866f; // altura de triângulo equilátero

        std::vector<Vertex> vertices = {
            Vertex(0.0f, height * 0.5f, 0.0f, // top center
                   0.0f, 0.0f, 1.0f, // normal facing forward
                   1.0f, 0.0f, 0.0f, // tangent
                   0.5f, 1.0f), // texcoord

            Vertex(-halfSize, -height * 0.5f, 0.0f, // bottom left
                   0.0f, 0.0f, 1.0f, // normal facing forward
                   1.0f, 0.0f, 0.0f, // tangent
                   0.0f, 0.0f), // texcoord

            Vertex(halfSize, -height * 0.5f, 0.0f, // bottom right
                   0.0f, 0.0f, 1.0f, // normal facing forward
                   1.0f, 0.0f, 0.0f, // tangent
                   1.0f, 0.0f) // texcoord
        };

        std::vector<uint32_t> indices = {0, 1, 2};

        return CreateTriangles(vertices, indices);
    }

    // ============================================================================
    // FUNÇÃO DERIVADA: QUAD (USA TRIÂNGULOS)
    // ============================================================================

    Ref<Mesh> TestMesh::CreatePlane(float width, float height)
    {
        float halfWidth = width * 0.5f;
        float halfHeight = height * 0.5f;

        std::vector<Vertex> vertices = {
            Vertex(-halfWidth, 0.0f, -halfHeight, // bottom-left
                   0.0f, 1.0f, 0.0f, // normal up
                   1.0f, 0.0f, 0.0f, // tangent
                   0.0f, 0.0f), // texcoord

            Vertex(halfWidth, 0.0f, -halfHeight, // bottom-right
                   0.0f, 1.0f, 0.0f, // normal up
                   1.0f, 0.0f, 0.0f, // tangent
                   1.0f, 0.0f), // texcoord

            Vertex(halfWidth, 0.0f, halfHeight, // top-right
                   0.0f, 1.0f, 0.0f, // normal up
                   1.0f, 0.0f, 0.0f, // tangent
                   1.0f, 1.0f), // texcoord

            Vertex(-halfWidth, 0.0f, halfHeight, // top-left
                   0.0f, 1.0f, 0.0f, // normal up
                   1.0f, 0.0f, 0.0f, // tangent
                   0.0f, 1.0f) // texcoord
        };

        // Dois triângulos formam um quad
        std::vector<uint32_t> indices = {
            0, 1, 2, // primeiro triângulo
            2, 3, 0 // segundo triângulo
        };

        return CreateTriangles(vertices, indices);
    }

    // ============================================================================
    // FUNÇÃO DERIVADA: CUBO (USA QUADS -> TRIÂNGULOS)
    // ============================================================================

    Ref<Mesh> TestMesh::CreateCube(float size)
    {
        float halfSize = size * 0.5f;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // Helper para adicionar uma face (quad) como dois triângulos
        auto addQuadFace = [&](const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3)
        {
            uint32_t baseIndex = vertices.size();

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);

            // Dois triângulos por face
            indices.insert(indices.end(), {
                               baseIndex, baseIndex + 1, baseIndex + 2,
                               baseIndex + 2, baseIndex + 3, baseIndex
                           });
        };

        // Front face (Z+)
        addQuadFace(
            Vertex(-halfSize, -halfSize, halfSize, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
            Vertex(halfSize, -halfSize, halfSize, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
            Vertex(halfSize, halfSize, halfSize, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
            Vertex(-halfSize, halfSize, halfSize, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)
        );

        // Back face (Z-)
        addQuadFace(
            Vertex(-halfSize, -halfSize, -halfSize, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
            Vertex(-halfSize, halfSize, -halfSize, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
            Vertex(halfSize, halfSize, -halfSize, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
            Vertex(halfSize, -halfSize, -halfSize, 0.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f)
        );

        // Left face (X-)
        addQuadFace(
            Vertex(-halfSize, -halfSize, -halfSize, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
            Vertex(-halfSize, -halfSize, halfSize, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
            Vertex(-halfSize, halfSize, halfSize, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
            Vertex(-halfSize, halfSize, -halfSize, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f)
        );

        // Right face (X+)
        addQuadFace(
            Vertex(halfSize, -halfSize, halfSize, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
            Vertex(halfSize, -halfSize, -halfSize, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f),
            Vertex(halfSize, halfSize, -halfSize, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
            Vertex(halfSize, halfSize, halfSize, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f)
        );

        // Bottom face (Y-)
        addQuadFace(
            Vertex(-halfSize, -halfSize, -halfSize, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
            Vertex(halfSize, -halfSize, -halfSize, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
            Vertex(halfSize, -halfSize, halfSize, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
            Vertex(-halfSize, -halfSize, halfSize, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)
        );

        // Top face (Y+)
        addQuadFace(
            Vertex(-halfSize, halfSize, halfSize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
            Vertex(halfSize, halfSize, halfSize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
            Vertex(halfSize, halfSize, -halfSize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
            Vertex(-halfSize, halfSize, -halfSize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)
        );

        return CreateTriangles(vertices, indices);
    }

    // ============================================================================
    // FUNÇÃO DERIVADA: ESFERA (USA TRIÂNGULOS DIRETAMENTE)
    // ============================================================================

    Ref<Mesh> TestMesh::CreateSphere(float radius, uint32_t segmentsX, uint32_t segmentsY)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        const float PI = static_cast<float>(M_PI);

        // Gerar vértices
        for (uint32_t y = 0; y <= segmentsY; ++y)
        {
            for (uint32_t x = 0; x <= segmentsX; ++x)
            {
                float xSegment = (float)x / (float)segmentsX;
                float ySegment = (float)y / (float)segmentsY;

                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                // Normal = posição normalizada para esfera
                float nx = xPos, ny = yPos, nz = zPos;

                // Calcular tangente
                float tx = -std::sin(xSegment * 2.0f * PI);
                float ty = 0.0f;
                float tz = std::cos(xSegment * 2.0f * PI);

                // Normalizar tangente
                float length = std::sqrt(tx * tx + ty * ty + tz * tz);
                if (length > 0.0f)
                {
                    tx /= length;
                    ty /= length;
                    tz /= length;
                }

                vertices.emplace_back(
                    xPos * radius, yPos * radius, zPos * radius, // position
                    nx, ny, nz, // normal
                    tx, ty, tz, // tangent
                    xSegment, ySegment // texcoord
                );
            }
        }

        // Gerar índices (cada quad = 2 triângulos)
        for (uint32_t y = 0; y < segmentsY; ++y)
        {
            for (uint32_t x = 0; x < segmentsX; ++x)
            {
                uint32_t first = (y * (segmentsX + 1)) + x;
                uint32_t second = first + 1;
                uint32_t third = first + (segmentsX + 1);
                uint32_t fourth = third + 1;

                // Primeiro triângulo
                indices.push_back(first);
                indices.push_back(third);
                indices.push_back(second);

                // Segundo triângulo
                indices.push_back(second);
                indices.push_back(third);
                indices.push_back(fourth);
            }
        }

        return CreateTriangles(vertices, indices);
    }

    // ============================================================================
    // FUNÇÃO DERIVADA: CILINDRO (USA TRIÂNGULOS DIRETAMENTE)
    // ============================================================================

    Ref<Mesh> TestMesh::CreateCylinder(float radius, float height, uint32_t segments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        const float PI = static_cast<float>(M_PI);
        float halfHeight = height * 0.5f;

        // Centro do topo e base
        vertices.emplace_back(0.0f, halfHeight, 0.0f, // pos
                              0.0f, 1.0f, 0.0f, // normal up
                              1.0f, 0.0f, 0.0f, // tangent
                              0.5f, 0.5f); // texcoord center

        vertices.emplace_back(0.0f, -halfHeight, 0.0f, // pos
                              0.0f, -1.0f, 0.0f, // normal down
                              1.0f, 0.0f, 0.0f, // tangent
                              0.5f, 0.5f); // texcoord center

        // Gerar vértices do perímetro
        for (uint32_t i = 0; i <= segments; i++)
        {
            float angle = (float)i / (float)segments * 2.0f * PI;
            float x = std::cos(angle);
            float z = std::sin(angle);

            // Vértice do topo
            vertices.emplace_back(x * radius, halfHeight, z * radius,
                                  0.0f, 1.0f, 0.0f, // normal up
                                  1.0f, 0.0f, 0.0f, // tangent
                                  (x + 1.0f) * 0.5f, (z + 1.0f) * 0.5f);

            // Vértice da base
            vertices.emplace_back(x * radius, -halfHeight, z * radius,
                                  0.0f, -1.0f, 0.0f, // normal down
                                  1.0f, 0.0f, 0.0f, // tangent
                                  (x + 1.0f) * 0.5f, (z + 1.0f) * 0.5f);

            // Vértices laterais (topo)
            vertices.emplace_back(x * radius, halfHeight, z * radius,
                                  x, 0.0f, z, // normal lateral
                                  -z, 0.0f, x, // tangent
                                  float(i) / float(segments), 1.0f);

            // Vértices laterais (base)
            vertices.emplace_back(x * radius, -halfHeight, z * radius,
                                  x, 0.0f, z, // normal lateral
                                  -z, 0.0f, x, // tangent
                                  float(i) / float(segments), 0.0f);
        }

        // Índices para tampas e lados
        uint32_t topCenter = 0, bottomCenter = 1;

        for (uint32_t i = 0; i < segments; i++)
        {
            uint32_t nextI = (i + 1) % segments;

            // Tampa superior (triângulos)
            indices.insert(indices.end(), {
                               topCenter,
                               2 + i * 4, // top rim vertex
                               2 + nextI * 4 // next top rim vertex
                           });

            // Tampa inferior (triângulos - ordem inversa)
            indices.insert(indices.end(), {
                               bottomCenter,
                               3 + nextI * 4, // next bottom rim vertex
                               3 + i * 4 // bottom rim vertex
                           });

            // Lados (2 triângulos por segmento)
            uint32_t sideTop = 4 + i * 4;
            uint32_t sideBottom = 5 + i * 4;
            uint32_t nextSideTop = 4 + nextI * 4;
            uint32_t nextSideBottom = 5 + nextI * 4;

            // Primeiro triângulo do lado
            indices.insert(indices.end(), {sideTop, sideBottom, nextSideBottom});
            // Segundo triângulo do lado
            indices.insert(indices.end(), {sideTop, nextSideBottom, nextSideTop});
        }

        return CreateTriangles(vertices, indices);
    }
} // namespace ForgeEngine
