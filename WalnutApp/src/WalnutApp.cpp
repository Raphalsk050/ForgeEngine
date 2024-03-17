#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "Walnut/Image.h"
#include <stdio.h>
#include "../PhysicsTest.h"


class ExampleLayer : public Walnut::Layer
{
public:
	
	virtual void OnUIRender() override
	{
		ImGui::Begin("Hello");
		if(ImGui::Button("Button"))
		{
			if(!physicsTest)
			{
				physicsTest = new PhysicsTest();
				physicsTest->InitPhysics();
			}
		}

		if(ImGui::Button("CleanUp"))
		{
			if(physicsTest)
			{
				physicsTest->CleanUp();
			}
		}
		ImGui::End();
	}

	void OnUpdate(float ts) override
	{
		if(physicsTest == nullptr) return;
		
		physicsTest->Update();
	}
private:
	PhysicsTest* physicsTest;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";
	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}
