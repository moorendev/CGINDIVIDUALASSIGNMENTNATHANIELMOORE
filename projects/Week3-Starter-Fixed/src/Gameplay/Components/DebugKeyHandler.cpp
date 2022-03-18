#pragma once
#include "DebugKeyHandler.h"
#include <GLFW/glfw3.h>

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

// GetGameObject()->GetRotationEuler();

DebugKeyHandler::DebugKeyHandler() :
	IComponent(),
	_renderer(nullptr),
	diffuseRamp(false),
	specularRamp(false)
{}

void DebugKeyHandler::Awake() {
	_renderer = GetComponent<RenderComponent>();
	_coolLut = (ResourceManager::CreateAsset<Texture3D>("luts/mycool.CUBE"));
	_warmLut = (ResourceManager::CreateAsset<Texture3D>("luts/mywarm.CUBE"));
	_customLut = (ResourceManager::CreateAsset<Texture3D>("luts/mycustom.CUBE"));
	_normalLut = (ResourceManager::CreateAsset<Texture3D>("luts/mynormal.CUBE"));
}

void DebugKeyHandler::Update(float deltaTime) {

	// Lighting Keys
	if (InputEngine::GetKeyState(GLFW_KEY_1) == ButtonState::Pressed)
	{
		_renderer->GetMaterial()->Set("u_Material.Mode", 1);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_2) == ButtonState::Pressed)
	{
		_renderer->GetMaterial()->Set("u_Material.Mode", 2);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_3) == ButtonState::Pressed)
	{
		_renderer->GetMaterial()->Set("u_Material.Mode", 3);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_4) == ButtonState::Pressed)
	{
		_renderer->GetMaterial()->Set("u_Material.Mode", 4);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_5) == ButtonState::Pressed)
	{
		_renderer->GetMaterial()->Set("u_Material.Mode", 5);
	}


	// Warp/Ramp Keys
	if (InputEngine::GetKeyState(GLFW_KEY_6) == ButtonState::Pressed)
	{
		if (diffuseRamp)
		{
			_renderer->GetMaterial()->Set("u_Material.DiffuseRamp", false);
			diffuseRamp = false;
		}
		else
		{
			_renderer->GetMaterial()->Set("u_Material.DiffuseRamp", true);
			diffuseRamp = true;
		}
	}
	if (InputEngine::GetKeyState(GLFW_KEY_7) == ButtonState::Pressed)
	{
		if (specularRamp)
		{
			_renderer->GetMaterial()->Set("u_Material.SpecularRamp", false);
			specularRamp = false;
		}
		else
		{
			_renderer->GetMaterial()->Set("u_Material.SpecularRamp", true);
			specularRamp = true;
		}
	}

	// Color Grading Keys
	if (InputEngine::GetKeyState(GLFW_KEY_8) == ButtonState::Pressed)
	{
		GetGameObject()->GetScene()->SetColorLUT(_coolLut);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_9) == ButtonState::Pressed)
	{
		GetGameObject()->GetScene()->SetColorLUT(_warmLut);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_0) == ButtonState::Pressed)
	{
		GetGameObject()->GetScene()->SetColorLUT(_customLut);
	}

	//	Reset Key
	if (InputEngine::GetKeyState(GLFW_KEY_R) == ButtonState::Pressed)
	{
		_renderer->GetMaterial()->Set("u_Material.Mode", 0);
		_renderer->GetMaterial()->Set("u_Material.DiffuseRamp", false);
		diffuseRamp = false;
		_renderer->GetMaterial()->Set("u_Material.SpecularRamp", false);
		specularRamp = false;
		GetGameObject()->GetScene()->SetColorLUT(_normalLut);
	}
}

void DebugKeyHandler::RenderImGui() {

}

nlohmann::json DebugKeyHandler::ToJson() const {
	return {
		{ "ramp", diffuseRamp }
	};
}

DebugKeyHandler::Sptr DebugKeyHandler::FromJson(const nlohmann::json& data) {
	DebugKeyHandler::Sptr result = std::make_shared<DebugKeyHandler>();
	result->diffuseRamp = JsonGet(data, "ramp", result->diffuseRamp);
	return result;
}