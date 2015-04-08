#pragma once

class Animator;

class Actor
	: public my::OctRoot
{
public:
	class Attacher
	{
	public:
		Actor * m_Owner;

		unsigned int m_SlotId;

		my::Matrix4 m_World;

	public:
		Attacher(void)
			: m_Owner(NULL)
			, m_SlotId(0)
			, m_World(my::Matrix4::Identity())
		{
		}

		void UpdateWorld(void);
	};

	my::Matrix4 m_World;

	boost::shared_ptr<Animator> m_Animator;

public:
	Actor(void)
		: OctRoot(my::AABB(FLT_MIN,FLT_MAX), 1.1f)
	{
	}

	virtual ~Actor(void)
	{
	}
};

typedef boost::shared_ptr<Actor> ActorPtr;
