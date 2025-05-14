#pragma once

struct CameraFollowComponent
{
	// Add at least one data member to prevent Flecs crash with empty components
	bool Follow;
	
	CameraFollowComponent(bool Follow = true)
	{
		this->Follow = Follow;
	}
};