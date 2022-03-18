#pragma once
#include "Gameplay/Components/IComponent.h"
#include <math.h>

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class TrunkBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<TrunkBehaviour> Sptr;

	TrunkBehaviour() = default;
	float time = 0;
	float WindSpeed = 5;
	glm::vec3 startRotation;
	glm::vec3 startPosition;

	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static TrunkBehaviour::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(TrunkBehaviour);
};