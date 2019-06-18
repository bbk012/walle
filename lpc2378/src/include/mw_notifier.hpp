/*
*********************************************************************************************************
*                                            LPC 2378
*
*                        (c) Copyright 2008, Bogdan Kowalczyk, POLAND
*                                           All Rights Reserved
*
*
* File:        mw_notifier.hpp
* Description: Defines class to handle information communicated between publishers and subscribers.
* Author:      Bogdan Kowalczyk
* Date:        13-Dec-2008
* History:
* 13-Dec-2008 - Initial version created
* 24-Sep-2013 - Added m_Info cNotifier member instead of original BYTE m_Handling to pass more info through a notifier
*********************************************************************************************************
*/

#ifndef NOTIFIER_HPP_
#define NOTIFIER_HPP_

#include "type.h"
#include "mw_smart_ptr.hpp"
#include "mw_notifier_data.hpp" //to get access to various notifier data structures
#include "lib_std.h" //to get access memcpy

//NOTIFIERS IDs
//Only 32 Notifiers ID are allowed - means only 32 Notifiers are allowed in the system
#define  NT_NONE 0x00000000
#define  NT_ID01 BIT0 // this is special notifier type named request notifier issued by subscriber when they want to receive most up to date Notifiers
#define  NT_ID02 BIT1      
#define  NT_ID03 BIT2         
#define  NT_ID04 BIT3        
#define  NT_ID05 BIT4        
#define  NT_ID06 BIT5        
#define  NT_ID07 BIT6        
#define  NT_ID08 BIT7        
#define  NT_ID09 BIT8        
#define  NT_ID10 BIT9        
#define  NT_ID11 BIT10       
#define  NT_ID12 BIT11       
#define  NT_ID13 BIT12       
#define  NT_ID14 BIT13       
#define  NT_ID15 BIT14       
#define  NT_ID16 BIT15       
#define  NT_ID17 BIT16       
#define  NT_ID18 BIT17       
#define  NT_ID19 BIT18       
#define  NT_ID20 BIT19       
#define  NT_ID21 BIT20       
#define  NT_ID22 BIT21       
#define  NT_ID23 BIT22       
#define  NT_ID24 BIT23       
#define  NT_ID25 BIT24       
#define  NT_ID26 BIT25       
#define  NT_ID27 BIT26       
#define  NT_ID28 BIT27       
#define  NT_ID29 BIT28       
#define  NT_ID30 BIT29       
#define  NT_ID31 BIT30       
#define  NT_ID32 BIT31
#define  NT_ALL  0xFFFFFFFF

// There are 3 types of Notifiers
// CMD_ - commands always have corresponding RSP_ as an evidence of command execution
// this gives capability to create synchronous CMD execution
// EVT_ are Notifiers for event brodcast - never contains coresponding responses
// That assume we have 32 different Notifiers.
// Notifier 0x00000001 - named REQ_NOTIFIER is reserved for special cases see above so we have 31 places
// for CMD, RSP and EVT notifiers as long as we use DWORD to handle NetID

#define NONE_NOTIFIER			NT_NONE
//Rerquest notifier issued to request most up to date notifiers from all subscribers - usuualy to get
//current state on stratup - if required
#define REQ_NOTIFIER 			NT_ID01


// MNG_MOTION - Wall-e movement commands and related head movement and scan ones
#define CMD_MOVE 				NT_ID02 
#define RSP_MOVE				NT_ID03 

#define CMD_SCAN				NT_ID04 
#define RSP_SCAN				NT_ID05 

#define CMD_CHECK				NT_ID06 
#define RSP_CHECK				NT_ID07 

#define CMD_TURN_HEAD			NT_ID08 
#define RSP_TURN_HEAD			NT_ID09 

//MNG_ARM - Wall-e left and right arm control command
#define CMD_MOVE_ARM			NT_ID10 
#define RSP_MOVE_ARM			NT_ID11 

//MNG_INDICATOR - Wall-e basic indicators control
#define CMD_INDICATOR			NT_ID12
#define RSP_INDICATOR			NT_ID13

//MNG_RTC - Wall-e RTC management commands
#define CMD_RTC					NT_ID14 
#define RSP_RTC					NT_ID15 

#define EVT_TIME				NT_ID16 
#define EVT_ALARM				NT_ID17 

//MNG_MONITOR - Wall-e basic resources state
#define EVT_BATTERY				NT_ID18 
#define EVT_DAY_NIGHT			NT_ID19 
#define EVT_SYS_RES				NT_ID20 
#define EVT_SYS_ALIVE			NT_ID21 

//MNG_KEYPAD
#define EVT_KEY					NT_ID22

//MNG_DISPLAY - request to display information
#define EVT_DISPLAY_INFO		NT_ID23

//MNG_EXE - external commands directed to execution manager to execute some actions
#define CMD_EXE_CMD				NT_ID24
#define RSP_EXE_CMD				NT_ID25 


//NOTIFIER HANDLING FLAGS
//So far only two possible handling defined
#define NT_HND_NORMAL_PRT 0x00 //normal notifier priority
#define NT_HND_HIGH_PRT 0x01   //high priority notifier when disptach it is put on front of subscriber's receive queue

/*
*********************************************************************************************************
*                                       Notifiers
*  Notifiers are used to hold copy of data spreaded out from publishers.
*  They are managed through counting reference and should always be instantinated on the heap.
*
*  Notifiers are referenced and managed by pointers to cBaseNotifier class which works like a header
*  for the storage area defined with help of template as shown below.
* 
*                                      |                |
* 									   |                |
*      +-----------------------------> |----------------|  
*      |             cMemMgrBase       | m_pData        |-----+
*      |                               |                |     |
*      |                               |----------------|     |
* 	cNotifier                          |                |<----+
*      |                               |                |  
*      |                               | m_Data         |
*      |                               |                |                                     
*      |                               |                |
* 	   +-----------------------------> |----------------|
*                                      |                |  
*                                      |                |
*                                      |                |
*
* *********************************************************************************************************
*/


/*
*********************************************************************************************************
* Name:                            cNotifier Class 
* 
* Description: Base clase for notifier contains common notifier interface used next by Notifier templates
*              to create certain types of notifier storage area
*       
* Note:
*      m_Info - 4 additional bytes to handle more information through the notifier
*      <Not Used>< Not Used><Manager ID><Handling>
*      Handling - handling byte 1 means high priority notifier placed in front of the queue, 0 means standard
*      Manager ID - ID of the Manager (thread priority) 
* *********************************************************************************************************
*/
class cNotifier:public cMemMgrBase //cMemMgrBas is used to count referencies to notifier
{
private:
	DWORD m_Id; //id of the notification every id identify specific type of information encapsulated by notifier
	DWORD m_Info;//additional notifier information see cNotifier description above
	void* m_pData;//pointer to the area with the data encapsulated by notifier
public:
	cNotifier(DWORD Id,BYTE MngId,BYTE Handling, void* pData=NULL):cMemMgrBase(){m_Id=Id;SetManagerId(MngId);SetHandling(Handling);m_pData=pData;};
	void SetNotifierId(DWORD Id){m_Id=Id;};
	DWORD GetNotifierId(){return m_Id;};
	void SetHandling(BYTE Handling){m_Info=(m_Info & 0xFFFFFF00)|((DWORD)Handling);};//clear handling part of m_Info and next set it up with new value
	BYTE GetHandling(){return (BYTE)(m_Info & 0x000000FF);};
	void SetManagerId(BYTE MngrId) {m_Info=(m_Info & 0xFFFF00FF)|(((DWORD)MngrId)<<8);};//clear manager ID part of m_Info and next set it up with new value
	BYTE GetManagerId() {return (BYTE)(m_Info>>8);}; //extracT Manger ID info and return as byte value
	void* GetDataPtr(){return m_pData;}; 

};//class cNotifier

/*
*********************************************************************************************************
* Name:                            cDataNotifier Class 
* 
* Description: DataNotifier is used to hold specific number of bytes should always be instantinated dynamically
*              on the heap.
*       
* Example:
* 	cDataNotifier *pNotifier;
*   pNotifier = new cDataNotifier<sizeof(int)> //notifier to hold integer data
*          
* *********************************************************************************************************
*/
template <BYTE size=1> class cDataNotifier:public cNotifier
{
private:
	BYTE m_Data[size];
public:
	//creates DataNotifier and copy pInData into notifier's data space
	cDataNotifier(DWORD Id,BYTE MngId,BYTE Handling,BYTE *pInData):cNotifier(Id,MngId,Handling,static_cast<void*>(m_Data)){SetData(pInData);};
	//copy data from pInData source into Notifier storage buffer
	void SetData(BYTE *pInData){memcpy(static_cast<void*>(m_Data),static_cast<void*>(pInData),size);};
	
	//copy data out of Notifier storage bufer into pOutData pointed buffer
	void GetData(BYTE *pOutData){memcpy(static_cast<void*>(pOutData),static_cast<void*>(m_Data),size);};
	BYTE GetDataSize(){return size;};
};//cNotifier

/*
*********************************************************************************************************
* Name:                            cTypeNotifier Class 
* 
* Description: TypeNotifier is used to hold specific type of data should always be instantinated dynamically
*              on the heap.
*       
* Example:
* 	cTypeNotifier *pNotifier;
*   pNotifier = new cTypeNotifier<int> //notifier to hold integer data
*          
* *********************************************************************************************************
*/
template <class T> class cTypeNotifier:public cNotifier
{
private:
	T m_Data;
public:
	//constructor to initialize Notifier with external provided InData
	cTypeNotifier(DWORD Id,BYTE MngId,BYTE Handling,T& InData):cNotifier(Id,MngId,Handling,static_cast<void*>(&m_Data)){SetData(InData);};
	//constructor without Notifier m_Data initialized by external source
	//after creation Notifier m_Data must be initialized through member functions call
	cTypeNotifier(DWORD Id,BYTE MngId,BYTE Handling):cNotifier(Id,MngId,Handling,static_cast<void*>(&m_Data)){};
	void SetData(T& rInData){m_Data=rInData;};
	T& GetData(){return m_Data;};
	BYTE GetDataSize(){return static_cast<BYTE>(sizeof(T));};
};//cNotifier


#endif /*NOTIFIER_HPP_*/
