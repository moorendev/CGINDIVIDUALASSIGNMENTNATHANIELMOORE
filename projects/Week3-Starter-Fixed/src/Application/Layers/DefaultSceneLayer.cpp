#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/LeafBehaviour.h"
#include "Gameplay/Components/BallBehaviour.h"
#include "Gameplay/Components/TrunkBehaviour.h"
#include "Gameplay/Components/DebugKeyHandler.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		// This shader handles blinn-phong lighting and all the necessary toggles involved
		ShaderProgram::Sptr basicShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});
		basicShader->SetDebugName("Blinn-phong");
		
		// Load in the meshes
		MeshResource::Sptr sandMesh = ResourceManager::CreateAsset<MeshResource>("sand.obj");
		MeshResource::Sptr binMesh = ResourceManager::CreateAsset<MeshResource>("sandbin.obj");
		MeshResource::Sptr trunkMesh = ResourceManager::CreateAsset<MeshResource>("trunk.obj");
		MeshResource::Sptr leafMesh = ResourceManager::CreateAsset<MeshResource>("leaf.obj");
		MeshResource::Sptr ballMesh = ResourceManager::CreateAsset<MeshResource>("ball.obj");
		MeshResource::Sptr waterMesh = ResourceManager::CreateAsset<MeshResource>("water.obj");

		// Load in some textures
		Texture2D::Sptr sandTex = ResourceManager::CreateAsset<Texture2D>("textures/sand.png");
		Texture2D::Sptr binTex = ResourceManager::CreateAsset<Texture2D>("textures/bin.png");
		Texture2D::Sptr trunkTex = ResourceManager::CreateAsset<Texture2D>("textures/trunk.png");
		Texture2D::Sptr leafTex = ResourceManager::CreateAsset<Texture2D>("textures/leaf.png");
		Texture2D::Sptr ballTex = ResourceManager::CreateAsset<Texture2D>("textures/ball.png");
		Texture2D::Sptr waterTex = ResourceManager::CreateAsset<Texture2D>("textures/water.png");

		Texture1D::Sptr diffRamp = ResourceManager::CreateAsset<Texture1D>("luts/diffuse-1D.png");
		diffRamp->SetWrap(WrapMode::ClampToEdge);
		diffRamp->SetMagFilter(MagFilter::Nearest);
		diffRamp->SetMinFilter(MinFilter::Nearest);

		Texture1D::Sptr specRamp = ResourceManager::CreateAsset<Texture1D>("luts/specular-1D.png");
		specRamp->SetWrap(WrapMode::ClampToEdge);
		specRamp->SetMagFilter(MagFilter::Nearest);
		diffRamp->SetMinFilter(MinFilter::Nearest);
		
		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>(); 

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading in a color lookup table
		Texture3D::Sptr lutcool = ResourceManager::CreateAsset<Texture3D>("luts/mynormal.CUBE");

		// Configure the color correction LUT
		scene->SetColorLUT(lutcool);

		// Create our materials
		// This will be our box material, with no environment reflections
		Material::Sptr sandMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			sandMaterial->Name = "Sand";
			sandMaterial->Set("u_Material.Diffuse", sandTex);
			sandMaterial->Set("u_Material.Shininess", 0.1f);
			sandMaterial->Set("u_Material.DiffRamp", diffRamp);
			sandMaterial->Set("u_Material.SpecRamp", specRamp);
		}
		
		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr binMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			binMaterial->Name = "Bin";
			binMaterial->Set("u_Material.Diffuse", binTex);
			binMaterial->Set("u_Material.Shininess", 0.1f);
			binMaterial->Set("u_Material.DiffRamp", diffRamp);
			binMaterial->Set("u_Material.SpecRamp", specRamp);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr trunkMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			trunkMaterial->Name = "Trunk";
			trunkMaterial->Set("u_Material.Diffuse", trunkTex);
			trunkMaterial->Set("u_Material.Shininess", 0.1f);
			trunkMaterial->Set("u_Material.DiffRamp", diffRamp);
			trunkMaterial->Set("u_Material.SpecRamp", specRamp);
		}

		Material::Sptr leafMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			leafMaterial->Name = "Leaf";
			leafMaterial->Set("u_Material.Diffuse", leafTex);
			leafMaterial->Set("u_Material.Shininess", 0.1f);
			leafMaterial->Set("u_Material.DiffRamp", diffRamp);
			leafMaterial->Set("u_Material.SpecRamp", specRamp);
		}
		
		// Our toon shader material
		Material::Sptr ballMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			ballMaterial->Name = "Ball";
			ballMaterial->Set("u_Material.Diffuse", ballTex);
			ballMaterial->Set("u_Material.Shininess", 0.8f);
			ballMaterial->Set("u_Material.DiffRamp", diffRamp);
			ballMaterial->Set("u_Material.SpecRamp", specRamp);
		}

		Material::Sptr waterMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			waterMaterial->Name = "Water";
			waterMaterial->Set("u_Material.Diffuse", waterTex);
			waterMaterial->Set("u_Material.Shininess", 0.9f);
			waterMaterial->Set("u_Material.DiffRamp", diffRamp);
			waterMaterial->Set("u_Material.SpecRamp", specRamp);
		}
		
		// Create some lights for our scene
		scene->Lights.resize(1);
		scene->Lights[0].Position = glm::vec3(0.5f, -1.6f, 3.0f);
		scene->Lights[0].Color = glm::vec3(0.35f, 0.35f, 0.35f);
		scene->Lights[0].Range = 100.0f;
		/*
		scene->Lights[1].Position = glm::vec3(2.0f, 0.0f, 3.5f);
		scene->Lights[1].Color = glm::vec3(0.35f, 0.35f, 0.35f);
		scene->Lights[1].Range = 100.0f;
		
		scene->Lights[2].Position = glm::vec3(-1.0f, -2.0f, 3.5f);
		scene->Lights[2].Color = glm::vec3(0.35f, 0.35f, 0.35f);
		scene->Lights[2].Range = 100.0f;
		*/
		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion({1.5f, -1.6f, 0.9f});
			camera->SetScale(glm::vec3(1.0f, 1.0f, 1.650f));
			camera->SetRotation(glm::vec3(112.0f, 0.0f, 28.0f));
			
			camera->Add<SimpleCameraControl>();

			// This is now handled by scene itself!
			//Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			//scene->MainCamera = cam;
		}
		
		GameObject::Sptr sceneParent = scene->CreateGameObject("Scene Elements");
		

		// Set up all our objects
		GameObject::Sptr bin = scene->CreateGameObject("Bin");
		{
			// Set position in the scene
			bin->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = bin->Add<RenderComponent>();
			DebugKeyHandler::Sptr handler = bin->Add<DebugKeyHandler>();
			renderer->SetMesh(binMesh);
			renderer->SetMaterial(binMaterial);

			sceneParent->AddChild(bin);
		}

		GameObject::Sptr sand = scene->CreateGameObject("Sand");
		{
			// Set position in the scene
			sand->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			
			// Add some behaviour that relies on the physics body
			sand->Add<JumpBehaviour>();
			
			// Create and attach a renderer for the sand
			RenderComponent::Sptr renderer = sand->Add<RenderComponent>();
			DebugKeyHandler::Sptr handler = sand->Add<DebugKeyHandler>();
			renderer->SetMesh(sandMesh);
			renderer->SetMaterial(sandMaterial);
			
			sceneParent->AddChild(sand);
		}

		// trunk object
		GameObject::Sptr trunk = scene->CreateGameObject("Trunk");
		{
			// Set and rotation position in the scene
			trunk->SetPostion(glm::vec3(0.0f, 0.0f, 1.0f));

			// Add the wind component
			TrunkBehaviour::Sptr sway = trunk->Add<TrunkBehaviour>();
			DebugKeyHandler::Sptr handler = trunk->Add<DebugKeyHandler>();
			sway->startRotation = trunk->GetRotationEuler();
			sway->WindSpeed = 5.0f;

			// Add a render component
			RenderComponent::Sptr renderer = trunk->Add<RenderComponent>();
			renderer->SetMesh(trunkMesh);
			renderer->SetMaterial(trunkMaterial);

			sceneParent->AddChild(trunk);
		}

		// leaf objects
		for (int i = 1; i <= 6; i++)
		{
			GameObject::Sptr leaf = scene->CreateGameObject("Leaf_" + std::to_string(i));
			{
				// Set and rotation position in the scene
				leaf->SetPostion(glm::vec3(0.0f, 0.0f, 2.65f));
				leaf->SetRotation(glm::vec3(0.0f, 0.0f, 60.0f * i));
				
				// Add the wind component
				LeafBehaviour::Sptr wind = leaf->Add<LeafBehaviour>();
				wind->startRotation = leaf->GetRotationEuler();
				wind->startPosition = leaf->GetPosition();
				wind->WindSpeed = 5.0f;

				// Add a render component
				RenderComponent::Sptr renderer = leaf->Add<RenderComponent>();
				DebugKeyHandler::Sptr handler = leaf->Add<DebugKeyHandler>();
				renderer->SetMesh(leafMesh);
				renderer->SetMaterial(leafMaterial);

				sceneParent->AddChild(leaf);
			}
		}

		// Beach ball
		GameObject::Sptr ball = scene->CreateGameObject("Ball");
		{
			// Set and rotation position in the scene
			ball->SetPostion(glm::vec3(1.0f, 0.0f, 0.8f));
			ball->SetRotation(glm::vec3(45.0f, 0.0f, 0.0f));

			// Add the wind component
			BallBehaviour::Sptr bounce = ball->Add<BallBehaviour>();
			bounce->startRotation = ball->GetRotationEuler();
			bounce->startPosition = ball->GetPosition();
			bounce->BallSpeed = 15.0f;
			
			// Add a render component
			RenderComponent::Sptr renderer = ball->Add<RenderComponent>();
			DebugKeyHandler::Sptr handler = ball->Add<DebugKeyHandler>();
			renderer->SetMesh(ballMesh);
			renderer->SetMaterial(ballMaterial);

			sceneParent->AddChild(ball);
		}

		GameObject::Sptr water = scene->CreateGameObject("Water");
		{
			// Set and rotation position in the scene
			water->SetPostion(glm::vec3(0.0f, 0.0f, 0.7f));

			// Add a render component
			RenderComponent::Sptr renderer = water->Add<RenderComponent>();
			DebugKeyHandler::Sptr handler = water->Add<DebugKeyHandler>();
			renderer->SetMesh(waterMesh);
			renderer->SetMaterial(waterMaterial);

			sceneParent->AddChild(water);
		}
		
		/////////////////////////// UI //////////////////////////////
		/*
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 16, 16 });
			transform->SetMax({ 256, 256 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();


			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 10, 10 });
				transform->SetMax({ 128, 128 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				panel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/upArrow.png"));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);

				monkey1->Get<JumpBehaviour>()->Panel = text;
			}

			canvas->AddChild(subPanel);
		}
		*/

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
