/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_smart_ptr.cpp
* Description: Defines smart pointer class to be capable to deal with Notifiers dynamically allocated
*              on the heap without taking care about memory management (delete) which is done
*              automatically when data are not any longer needed.
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/
#include "mw_smart_ptr.hpp"
#include "wrp_kernel.hpp"

//on construction noone is using referenced object so counter is set to zero
cMemMgrBase::cMemMgrBase(void)
{
	Kernel.MemMgrMutex.Acquire();
	m_ClassRefs=0;
	Kernel.MemMgrMutex.Release();
}//cMemMgrBase::cMemMgrBase(void)

//called when another reference to objecton the heap arrise
void cMemMgrBase::Inc(void)
{
	Kernel.MemMgrMutex.Acquire();
	++m_ClassRefs;
	Kernel.MemMgrMutex.Release();
}//void cMemMgrBase::Inc(void)

//called when reference to object on the heap disappears
void cMemMgrBase::Dec(void)
{
	Kernel.MemMgrMutex.Acquire();
	if(--m_ClassRefs == 0)delete this;
	Kernel.MemMgrMutex.Release();
}//cMemMgrBase::Dec(void)

cSmartPtrBase& cSmartPtrBase::operator=(cMemMgrBase* pClass)
{
	if(!pClass)//assign called for NULL pointer so only copy it
		{
		m_pClass=pClass;
		}
	else//pClass point to managed memory area
		{
		if(m_pClass!= pClass)//if we do not assign existing pointer to itself i.e. p1=p1
			{
			//if left smart pointer points to something lower reference counter as pointer will be substituted with new value
			//received from the right pointer
			if(m_pClass)m_pClass->Dec();
			m_pClass=pClass;//left pointer receives same value as right pointer both will point to the same memory
			if(m_pClass)m_pClass->Inc();//increase reference counter as now both left and right pointers points the same cMemMgrBase
			}
		}
	return *this;
}//cSmartPtrBase::operator=
