#pragma once

namespace TEN::Entities::Player::Context
{
	struct SetupGroundMovement
	{
		short HeadingAngle	  = 0;
		float LowerFloorBound = 0.0f;
		float UpperFloorBound = 0.0f;
		bool  CheckSlopeDown  = true;
		bool  CheckSlopeUp	  = true;
		bool  CheckDeathFloor = true;
	};

	struct SetupMonkeyMovement
	{
		short HeadingAngle		= 0;
		float LowerCeilingBound = 0.0f;
		float UpperCeilingBound = 0.0f;
	};

	struct SetupJump
	{
		short HeadingAngle	  = 0;
		float Distance		  = 0.0f;
		bool  CheckWadeStatus = true;
	};

}
