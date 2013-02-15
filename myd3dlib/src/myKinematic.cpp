#include "StdAfx.h"
#include "myKinematic.h"

using namespace my;

void Location::integrate(float duration, const Vector3 & velocity, float rotation)
{
	position += velocity * duration;

	orientation = fmod(orientation + rotation * duration, D3DX_PI * 2);
}

void Location::integrate(const SteeringOutput & steer, float duration)
{
	integrate(duration, steer.linear, steer.angular);
}

void Location::setOrientationFromVelocityLH(const Vector3 & velocity)
{
	if(velocity.magnitudeSq() > EPSILON_E12)
	{
		orientation = atan2(velocity.x, velocity.z);
	}
}

Vector3 Location::getOrientationAsVector(void) const
{
	return Vector3(sin(orientation), 0, cos(orientation));
}

void Kinematic::integrate(float duration)
{
	Location::integrate(duration, velocity, rotation);
}

void Kinematic::integrate(const SteeringOutput & steer, float duration)
{
	Location::integrate(duration, velocity, rotation);

	velocity += steer.linear * duration;

	rotation += steer.angular * duration;
}

void Kinematic::integrate(
	const SteeringOutput & steer,
	float drag,
	float duration)
{
	Location::integrate(duration, velocity, rotation);

	float integration_drag = pow(drag, duration);
	velocity *= integration_drag;
	rotation *= integration_drag * integration_drag;

	velocity += steer.linear * duration;

	rotation += steer.angular * duration;
}

void Kinematic::integrate(
	const SteeringOutput & steer,
	const SteeringOutput & drag,
	float duration)
{
	Location::integrate(duration, velocity, rotation);

	velocity.x *= pow(drag.linear.x, duration);
	velocity.y *= pow(drag.linear.y, duration);
	velocity.z *= pow(drag.linear.z, duration);

	rotation *= pow(drag.angular, duration);

	velocity += steer.linear * duration;

	rotation += steer.angular * duration;
}

void Seek::getSteering(SteeringOutput * output)
{
	output->linear = *target - character->position;

	if(output->linear.magnitudeSq() > EPSILON_E12)
	{
		output->linear.normalizeSelf();
		output->linear *= maxAcceleration;
	}
}

void Flee::getSteering(SteeringOutput * output)
{
	output->linear = character->position - *target;

	if(output->linear.magnitudeSq() > EPSILON_E12)
	{
		output->linear.normalizeSelf();
		output->linear *= maxAcceleration;
	}
}

void Wander::getSteering(SteeringOutput * output)
{
	if(target->magnitudeSq() < EPSILON_E12)
	{
		internal_target = character->position;
		internal_target.x += volatility;
	}

	Vector3 offset = *target - character->position;
	float angle;
	if(offset.x * offset.x + offset.z * offset.z > EPSILON_E12)
	{
		angle = atan2(offset.z, offset.x);
	}
	else
	{
		angle = 0;
	}

	internal_target = character->position;
	internal_target.x += volatility * cos(angle);
	internal_target.z += volatility * sin(angle);

	internal_target.x += Random(turnSpeed) - Random(turnSpeed);
	internal_target.z += Random(turnSpeed) - Random(turnSpeed);

	Seek::getSteering(output);
}
