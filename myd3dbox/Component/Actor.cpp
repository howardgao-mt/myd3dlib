#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"

using namespace my;

void Actor::Attacher::UpdateWorld(void)
{
	_ASSERT(m_Owner);
	if (m_Owner->m_Animator && m_SlotId < m_Owner->m_Animator->m_DualQuats.size())
	{
		my::Matrix4 Slot = BoneList::UDQtoRM(m_Owner->m_Animator->m_DualQuats[m_SlotId]);
		m_World = Slot * m_Owner->m_World;
	}
	else
	{
		m_World = m_Owner->m_World;
	}
}
