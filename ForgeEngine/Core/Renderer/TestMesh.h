#pragma once

#include "Core/Renderer/Buffer.h"
#include "Core/Renderer/VertexArray.h"
#include "Config.h"

#include <vector>
#include <string>

namespace ForgeEngine
{
    // Forward declaration da estrutura Vertex
    struct Vertex;

    class TestMesh
    {
    public:
        TestMesh();
        ~TestMesh() = default;

        // Métodos básicos
        void SetVertices(std::vector<float>& vertices, uint32_t vertexCount);
        void SetIndices(std::vector<uint32_t>& indices);
        void SetVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
        void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

        // Getters
        const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
        uint32_t GetVertexCount() const { return m_VertexCount; }
        uint32_t GetIndexCount() const { return m_IndexCount; }

        static Ref<Mesh> CreateTriangles(const std::vector<Vertex>& vertices,
                                         const std::vector<uint32_t>& indices = {});


        // Primitiva fundamental
        static Ref<Mesh> CreateTriangle(float size = 1.0f);

        // Usa 2 triângulos
        static Ref<Mesh> CreatePlane(float width = 1.0f, float height = 1.0f);

        // Usa 12 triângulos (6 faces × 2 triângulos por face)
        static Ref<Mesh> CreateCube(float size = 1.0f);

        // Usa segmentsX × segmentsY × 2 triângulos
        static Ref<Mesh> CreateSphere(float radius = 0.5f,
                                      uint32_t segmentsX = 32,
                                      uint32_t segmentsY = 16);

        // Usa segments × 4 triângulos (2 tampas + laterais)
        static Ref<Mesh> CreateCylinder(float radius = 0.5f,
                                        float height = 1.0f,
                                        uint32_t segments = 32);

    private:
        Ref<VertexArray> m_VertexArray;
        Ref<VertexBuffer> m_VertexBuffer;
        Ref<IndexBuffer> m_IndexBuffer;

        uint32_t m_VertexCount = 0;
        uint32_t m_IndexCount = 0;
    };
} // namespace ForgeEngine
