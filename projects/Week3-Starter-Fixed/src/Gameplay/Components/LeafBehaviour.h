#pragma once
#include "Gameplay/Components/IComponent.h"
#include <math.h>

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class LeafBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<LeafBehaviour> Sptr;

	LeafBehaviour() = default;
	float time = 0;
	float WindSpeed = 5;
	glm::vec3 startRotation;
	glm::vec3 startPosition;

	virtual void Update(float deltaTime) override;
	
	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static LeafBehaviour::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(LeafBehaviour);
};