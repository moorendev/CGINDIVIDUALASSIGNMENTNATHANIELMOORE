#pragma once
#include "Gameplay/Components/IComponent.h"
#include <math.h>

#include "Gameplay/GameObject.h"
#include "RenderComponent.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Light.h"
#include "Gameplay/InputEngine.h"
#include "Gameplay/Material.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class DebugKeyHandler : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<DebugKeyHandler> Sptr;
	
	RenderComponent::Sptr _renderer;
	Texture3D::Sptr _coolLut;
	Texture3D::Sptr _warmLut;
	Texture3D::Sptr _customLut;
	Texture3D::Sptr _normalLut;
	DebugKeyHandler();

	bool diffuseRamp;
	bool specularRamp;

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static DebugKeyHandler::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(DebugKeyHandler);
};