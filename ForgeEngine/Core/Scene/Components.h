#pragma once

#include "Core/Application/UUID.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

#include "Core/Scene/SceneCamera.h"
#include "Core/Renderer/Mesh.h"

namespace ForgeEngine
{
    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;

        TagComponent(const std::string& tag)
            : Tag(tag)
        {
        }
    };

    struct TransformComponent
    {
        glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;

        TransformComponent(const glm::vec3& translation)
            : Translation(translation)
        {
        }

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct ModelRendererComponent
    {
        Ref<Model> Model;
        Ref<Material> OverrideMaterial;

        ModelRendererComponent() = default;
        ModelRendererComponent(const ModelRendererComponent&) = default;

        ModelRendererComponent(const Ref<ForgeEngine::Model>& model)
            : Model(model)
        {
        }
    };

    struct SpriteRendererComponent
    {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        Ref<Texture2D> Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;

        SpriteRendererComponent(const glm::vec4& color)
            : Color(color)
        {
        }
    };

    struct CircleRendererComponent
    {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        float Thickness = 1.0f;
        float Fade = 0.005f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct ScriptComponent
    {
        std::string ClassName;

        ScriptComponent() = default;
        ScriptComponent(const ScriptComponent&) = default;
    };

    // Forward declaration
    class ScriptableEntity;

    struct NativeScriptComponent
    {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity*(*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);

        template <typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc)
            {
                delete nsc->Instance;
                nsc->Instance = nullptr;
            };
        }
    };

    // Physics

    struct Rigidbody2DComponent
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };

        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        // Storage for runtime
        void* RuntimeBody = nullptr;

        Rigidbody2DComponent() = default;
        Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
    };

    struct BoxCollider2DComponent
    {
        glm::vec2 Offset = {0.0f, 0.0f};
        glm::vec2 Size = {0.5f, 0.5f};

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        // Storage for runtime
        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

    struct CircleCollider2DComponent
    {
        glm::vec2 Offset = {0.0f, 0.0f};
        float Radius = 0.5f;

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        // Storage for runtime
        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    /*struct TextComponent
    {
        std::string TextString;
        //Ref<Font> FontAsset = Font::GetDefault();
        glm::vec4 Color{ 1.0f };
        float Kerning = 0.0f;
        float LineSpacing = 0.0f;
    };*/

    template <typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents =
    ComponentGroup<TransformComponent, SpriteRendererComponent,
                   CircleRendererComponent, CameraComponent, ScriptComponent,
                   NativeScriptComponent, Rigidbody2DComponent, BoxCollider2DComponent,
                   CircleCollider2DComponent/*, TextComponent*/>;
}
