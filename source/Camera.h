#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <algorithm>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		float aspectRatio{ 1.f };

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float _aspectRatio = 1.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			aspectRatio = _aspectRatio;
		}

		void CalculateViewMatrix()
		{
			right	= Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up		= Vector3::Cross(forward, right);

			invViewMatrix =
			{
				right,
				up,
				forward,
				origin			
			};

			viewMatrix = Matrix::Inverse(invViewMatrix);
		}
		const Matrix& GetViewMatrix() const
		{
			return viewMatrix;
		}
		const Matrix& GetInvViewMatrix() const
		{
			return invViewMatrix;
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
		}
		const Matrix& GetProjectionMatrix() const
		{
			return projectionMatrix;
		}

		Matrix GetWorldViewProjection() const
		{
			return GetViewMatrix() * GetProjectionMatrix();
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const float movementSpeed{ 5.f * deltaTime };
			const float minFov{ 30.f };
			const float maxFov{ 170.f };
			const float mouseSpeed{ 20.0f * deltaTime };
			const float rotateSpeed{ 360.f * TO_RADIANS * deltaTime};

			Vector3 directionVector{};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_Z] || pKeyboardState[SDL_SCANCODE_W])
			{
				directionVector += forward * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				directionVector -= forward * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_A])
			{
				directionVector -= right * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				directionVector += right * movementSpeed;
			}

			//Mouse Input
			int mouseX{}, mouseY{};

			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if ((mouseState & SDL_BUTTON_LMASK) != 0)
			{
				directionVector -= forward * (mouseY * mouseSpeed);
				totalYaw += mouseX * rotateSpeed;
			}
			else if ((mouseState & SDL_BUTTON_RMASK) != 0)
			{
				totalYaw += mouseX * rotateSpeed;
				totalPitch -= mouseY * rotateSpeed;
			}
			totalPitch = std::clamp(totalPitch, -89.f * TO_RADIANS, 89.0f * TO_RADIANS);

			const float shiftSpeed{ 4.0f };
			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				directionVector *= shiftSpeed;
			}

			origin += directionVector;

			Matrix rotationMatrix = Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
