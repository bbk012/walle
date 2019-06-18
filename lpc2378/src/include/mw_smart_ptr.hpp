/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        smart_ptr.hpp
* Description: Defines smart pointer class to be capable to deal with Notifiers dynamically allocated
*              on the heap without taking care about memory management (delete) which is done
*              automatically when data are not any longer needed.
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
*********************************************************************************************************
*/


#ifndef SMART_PTR_HPP_
#define SMART_PTR_HPP_
#include "type.h"
/*
*********************************************************************************************************
* Name:                            cMemMgrBase Class 
* 
* Description: This is base clase used by objects allocated on the heap for which we want to count
*              number of referencies. This is inteneded to be used by Notifiers.
*              All objects for which we would like to count referencies and auto delate from the heap need to
*              inherit from this base class.
*       
* IMPORTANT!   Because there may be number of referencies to same memory object from different threads
*              access to Inc and to Dec need to be protected to assure consistency.
*              Formally every cMemMgrBase object should have its own protection but to make it simple
*              I have decided to use one static Mutext for all the accesses protection.
*              That means there is one global mutext to protect any object of that type I hope this will
*              not lower performance because operations are short.
* *********************************************************************************************************
*/
class cMemMgrBase
{
private:
	int m_ClassRefs;//counts referencies for the class
public:
	cMemMgrBase(void);//on construction initialize reference to zero but derivative class uses Inc() to control final value for initialization
	~cMemMgrBase(){};//do nothing destructor
	void Inc(void);//called when another reference to object on the heap arrise
	void Dec(void);//called when reference to object on the heap disappears
};//cMemMgrBase

/*
*********************************************************************************************************
* Name:                            cSmartPtrBase Class 
* 
* Description: This is base for all types of Smart Pointer classes. 
* 			   Smart pointer works only when applied to classes which inherits from cMemMgrBase class
* 		       i.e. when cSmartPtrBase can rely on reference counting of cMemMgrBase.
* 				
*              Smart pointer class counts referencies to objects and deletes them when there
*              is not more referencies for the object itself.
*
* IMPORTANT!   cSmartPtrBase works as common base for all types of smart pointer this allows for
* 			   initialization and assignament operation between different types of smart pointers
*              and this is the only purpose of the cSmartPtrBase class.
* *********************************************************************************************************
*/

class cSmartPtrBase
{
protected:
	cMemMgrBase* m_pClass;//pointer to what smart pointer points to or NULL if not initialized correctly

public:
	cSmartPtrBase(){m_pClass=static_cast<cMemMgrBase*>(NULL);}//gives chance to create empty smart pointer which points to nothing
	cSmartPtrBase(cMemMgrBase* pClass):m_pClass(pClass){if(m_pClass)m_pClass->Inc();}//initialize smart pointer let it points to object capable to count referencies
	cSmartPtrBase(const cSmartPtrBase& rClass){m_pClass=rClass.m_pClass; if(m_pClass)m_pClass->Inc();}//copy contructor
	~cSmartPtrBase(){if(m_pClass)m_pClass->Dec();}

	int isValid(){return  m_pClass!=NULL;};//check is smart pointer assigned to point to any memory block
	operator int() {return isValid();};//operator conversion to int to check if smart pointer is correctly initialized
	
	cMemMgrBase& operator*(void) {return *m_pClass;};//get reference (de-reference operator) to memory pointed by smart pointer
	cMemMgrBase* operator->(void) {return m_pClass;};//get access to memory pointed by smart pointer
	cSmartPtrBase& operator=(const cSmartPtrBase& rClass){return operator=(rClass.m_pClass);}
	cSmartPtrBase& operator=(cMemMgrBase* pClass);//assign standard pointer to managed memory to the smart pointer
	friend class cBasePublisher;//let publisher operates directly on referencies pointed by m_pClass
	friend class cBaseSubscriber;//let subscriber operates directly on referencies pointed by m_pClass
};

/*
*********************************************************************************************************
* Name:                           template <class T> class cSmartPtr 
* 
* Description: With this template class smart pointers to different notifier types can be created
*              for example to cDataNotifier or to cTypeNotifier.
* 			   Because the base for template is same cSmartPtrBase pointers to different types of notifiers
* 			   can be interchanged during initialization and assignament.
*              The basic purpose of the template is to provide correct casting to proper notifier
*              type when dereference operators * and -> are used.
* *********************************************************************************************************
*/
// T need to be any managed memory type i.e. derived from cMemMgrBase
// this is smart pointer to any managed memory type T for example T can be cNotifier or cDataNotifier or cTypeNotifier
template <class T> 
class cSmartPtr:public cSmartPtrBase 
{
public:
	cSmartPtr():cSmartPtrBase(){};//initialize NULL cSmartPtr - smart pointer which is not pointing to anything
	cSmartPtr(T* pClass):cSmartPtrBase(static_cast<cMemMgrBase*>(pClass)){}//initialization with any type of notifier
	cSmartPtr(const cSmartPtrBase& refClass):cSmartPtrBase(refClass){}//copy contructor for any smart pointer derived from cSmartPtrBase
	~cSmartPtr(){}
	operator T*(void) {return static_cast<T*>(m_pClass);}//cast to Type T* (will work as long as notifier derives from cMemMgrBase) 
	T& operator*(void) {return static_cast<T&>(*m_pClass);}//dereference operator from cMemMgrBase type to any derived one type T
	T* operator->(void) {return static_cast<T*>(m_pClass);}
	//assign operator for operastions like smart_p1 = smart_p2 where p1 is cSmartPtr<T> type and p2 is cSmartPtrBase or derived
	cSmartPtr& operator=(const cSmartPtrBase& rClass){return static_cast<cSmartPtr&>(cSmartPtrBase::operator=(rClass));}
	
};//class cSmartPtr


#endif /*SMART_PTR_HPP_*/
