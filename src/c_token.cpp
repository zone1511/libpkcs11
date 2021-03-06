/*
 * ------------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <ramo -at- goodvikings -dot- com> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day, and
 * you think this stuff is worth it, you can buy me a beer in return - Ramo
 * ------------------------------------------------------------------------------
 */

#include <vector>
#include <string.h>
#include "log.h"
#include "mechanisms.h"
#include "p11.h"
#include "slot.h"
using namespace std;

extern bool cryptokiInitialized;
extern std::vector<slot*>* slots;
extern mechanisms* mechs;

extern int getSlotBySession(CK_SESSION_HANDLE hSession);

CK_RV C_GetSlotList(CK_BBOOL tokenPresent, CK_SLOT_ID_PTR pSlotList, CK_ULONG_PTR pulCount)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;
	unsigned long slotCount = 0;

	// check if initialized
	if (!cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;

	// count slots
	if (!rv)
		slotCount = slots->size();

	// if we only want slots with tokens present
	if (!rv && tokenPresent)
		for (unsigned int i = 0; i < slots->size(); i++)
			if (!(*slots)[i]->isTokenPresent())
				slotCount--;

	// if slot list is null
	if (!rv && !pSlotList)
	{
		*pulCount = slotCount;
	} else if (!rv && *pulCount < slotCount) // if slot list is not big enough
	{
		*pulCount = slotCount;
		rv = CKR_BUFFER_TOO_SMALL;
	} else if (!rv)
	{
		//Buffer size of pslotlist is big enough, copy the slot details acorss
		int j = 0;
		for (unsigned int i = 0; i < slots->size(); i++)
		{
			if (tokenPresent)
			{
				if ((*slots)[i]->isTokenPresent())
				{
					pSlotList[j++] = i;
				}
			} else
			{
				pSlotList[j++] = i;
			}
		}
	}

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_GetSlotInfo(CK_SLOT_ID slotID, CK_SLOT_INFO_PTR pInfo)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;

	if (!cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && !pInfo)
		rv = CKR_ARGUMENTS_BAD;
	if (!rv && slotID >= slots->size())
		rv = CKR_SLOT_ID_INVALID;

	if (!rv)
	{
		memcpy(pInfo->slotDescription, SLOTDESC, SLOTDESLEN);
		memcpy(pInfo->manufacturerID, SLOTMANID, SLOTMANIDLEN);
		pInfo->flags = (*slots)[slotID]->isTokenPresent() ? CKF_TOKEN_PRESENT : 0;
	}

	pInfo->hardwareVersion.major = SLOTVERSIONMAJOR;
	pInfo->hardwareVersion.minor = SLOTVERSIONMINOR;
	pInfo->firmwareVersion.major = SLOTVERSIONMAJOR;
	pInfo->firmwareVersion.minor = SLOTVERSIONMINOR;

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_GetTokenInfo(CK_SLOT_ID slotID, CK_TOKEN_INFO_PTR pTokenInfo)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;

	if (!cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && !pTokenInfo)
		rv = CKR_ARGUMENTS_BAD;
	if (!rv && slotID >= slots->size())
		rv = CKR_SLOT_ID_INVALID;
	if (!rv && !(*slots)[slotID]->isTokenPresent())
		rv = CKR_TOKEN_NOT_PRESENT;

	if (!rv)
		rv = (*slots)[slotID]->getTokenInfo(pTokenInfo);

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_WaitForSlotEvent(CK_FLAGS flags, CK_SLOT_ID_PTR pSlot, CK_VOID_PTR pRserved)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;

	rv = CKR_FUNCTION_NOT_SUPPORTED;

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_GetMechanismList(CK_SLOT_ID slotID, CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;

	if (!cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && slotID >= slots->size())
		rv = CKR_SLOT_ID_INVALID;
	if (!rv && !(*slots)[slotID]->isTokenPresent())
		rv = CKR_TOKEN_NOT_PRESENT;

	if (!rv)
	{
		if (!pMechanismList) // we only want the count
		{
			*pulCount = mechs->getSize();
		} else
		{
			rv = mechs->getMachanismList(pMechanismList, pulCount);
		}
	}

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_GetMechanismInfo(CK_SLOT_ID slotID, CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;

	if (!rv && !cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && slotID >= slots->size())
		rv = CKR_SLOT_ID_INVALID;
	if (!rv && !(*slots)[slotID]->isTokenPresent())
		rv = CKR_TOKEN_NOT_PRESENT;
	if (!rv && !pInfo)
		rv = CKR_ARGUMENTS_BAD;

	if (!rv)
	{
		rv = mechs->getMechanismInfo(type, pInfo);
	}

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_InitToken(CK_SLOT_ID slotID, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen, CK_UTF8CHAR_PTR pLabel)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;

	if (!rv && !cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && slotID >= slots->size())
		rv = CKR_SLOT_ID_INVALID;
	if (!rv && !(*slots)[slotID]->isTokenPresent())
		rv = CKR_TOKEN_NOT_PRESENT;
	if (!rv && !pPin)
		rv = CKR_ARGUMENTS_BAD;
	if (!rv && !pLabel)
		rv = CKR_ARGUMENTS_BAD;

	if (!rv)
		rv = (*slots)[slotID]->initToken(pPin, ulPinLen, pLabel);

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_InitPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;
	int slot = getSlotBySession(hSession);

	if (!rv && !cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && slot == -1)
		rv = CKR_SESSION_HANDLE_INVALID;
	if (!rv && !pPin)
		rv = CKR_ARGUMENTS_BAD;
	if (!rv && !((*slots)[slot]->getTokenState() & CKS_RW_SO_FUNCTIONS))
		rv = CKR_USER_NOT_LOGGED_IN;

	if (!rv)
		rv = (*slots)[slot]->initTokenUserPin(pPin, ulPinLen);

	LOG_RETURNCODE(rv);

	return rv;
}

CK_RV C_SetPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pOldPin, CK_ULONG ulOldLen, CK_UTF8CHAR_PTR pNewPin, CK_ULONG ulNewLen)
{
	LOG_INSTANCE(NULL);
	LOG_FUNCTIONCALL();

	CK_RV rv = CKR_OK;
	int slot = getSlotBySession(hSession);
	CK_STATE state = (*slots)[slot]->getTokenState();

	if (!rv && !cryptokiInitialized)
		rv = CKR_CRYPTOKI_NOT_INITIALIZED;
	if (!rv && slot == -1)
		rv = CKR_SESSION_HANDLE_INVALID;
	if (!rv && (!pOldPin || !pNewPin))
		rv = CKR_ARGUMENTS_BAD;
	if (!rv && (state == CKS_RO_PUBLIC_SESSION || state == CKS_RO_USER_FUNCTIONS))
		rv = CKR_SESSION_READ_ONLY;

	if (!rv)
		rv = (*slots)[slot]->setTokenPin(pOldPin, ulOldLen, pNewPin, ulNewLen);

	LOG_RETURNCODE(rv);

	return rv;
}
