#include "WeirdOne.h"

#include "Core/GameTimer.h"

#include <iostream>

namespace solar
{
	void WeirdOne::OnUpdate(const Blainn::GameTimer& gt)
	{
		auto currPos = transform->GetLocalPosition();
		currPos.y += 3.f * gt.DeltaTime();
		auto currRot = transform->GetLocalYawPitchRoll();
		currRot.z += 1.f * gt.DeltaTime();
		//transform->SetLocalPositionYawPitchRoll(currPos, currRot);
		//transform->SetLocalPosition(currPos);
	}
}
