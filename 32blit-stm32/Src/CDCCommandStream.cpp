/*
 * CDCCommandStream.cpp
 *
 *  Created on: 17 Jan 2020
 *      Author: andrewcapon
 */
#include "32blit.h"
#include "usbd_cdc_if.h"
extern USBD_HandleTypeDef hUsbDeviceHS;

#include "CDCCommandStream.h"

void CDCCommandStream::Init(void)
{
	m_state = stDetect;
	m_uHeaderScanPos = 0;
	m_bNeedsUSBResume = false;
}

void CDCCommandStream::AddCommandHandler(CDCCommandHandler::CDCFourCC uCommand, CDCCommandHandler *pCommandHandler)
{
	m_commandHandlers[uCommand] = pCommandHandler;
}


void CDCCommandStream::LogTimeTaken(CDCCommandHandler::StreamResult result, uint32_t uBytesHandled)
{
	// can be used to log time taken
}


void CDCCommandStream::Stream(void)
{
  static uint32_t uLastResumeTime = HAL_GetTick();

  if(m_bNeedsUSBResume) // FIFO Full, so empty and resume USB
  {
    while(CDCFifoElement *pElement = GetFifoReadElement())
    {
      if(pElement->m_uLen)
        Stream(pElement->m_data, pElement->m_uLen);
      ReleaseFifoReadElement();
    }
		m_bNeedsUSBResume = false;
	  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, GetFifoWriteBuffer());
		USBD_CDC_ReceivePacket(&hUsbDeviceHS);
    uLastResumeTime = HAL_GetTick();
  }
  else
  {
    if(HAL_GetTick() > uLastResumeTime + 100) // USB Stalled, empty FIFO
    {
      // empty fifo
      while(CDCFifoElement *pElement = GetFifoReadElement())
      {
        if(pElement->m_uLen)
          Stream(pElement->m_data, pElement->m_uLen);
        ReleaseFifoReadElement();
      }
    }
  }
	// while(CDCFifoElement *pElement = GetFifoReadElement())
	// {
	// 	if(pElement->m_uLen)
	// 		Stream(pElement->m_data, pElement->m_uLen);
	// 	ReleaseFifoReadElement();
	// }

	// // restart USB CDC if fifo was full
	// if(m_bNeedsUSBResume && m_uFifoUsedCount == 0)
	// {
	// 	m_bNeedsUSBResume = false;
	//   USBD_CDC_SetRxBuffer(&hUsbDeviceHS, GetFifoWriteBuffer());
	// 	USBD_CDC_ReceivePacket(&hUsbDeviceHS);
	// }
}

uint8_t CDCCommandStream::Stream(uint8_t *data, uint32_t len)
{
	blit_disable_ADC();
	uint8_t   *pScanPos  = data;

	CDCCommandHandler::CDCFourCC uCommand   = 0;

	if(m_state == stDetect)
	{
		bool bHeaderFound = false;
		while ((!bHeaderFound) && (pScanPos < (data+len)))
		{
			if(*pScanPos == m_header[m_uHeaderScanPos])
			{
				if(m_uHeaderScanPos == 3)
				{
					// header found
					bHeaderFound = true;
					m_uHeaderScanPos = 0;
					m_uCommandScanPos = 0;
				}
				else
					m_uHeaderScanPos++;
			}
			else
			{
				m_uHeaderScanPos = 0;
				m_uCommandScanPos = 0;
			}
			pScanPos++;
		}

		if(bHeaderFound)
		{
			// next command byte could be in next call
			while((m_state == stDetect) && (m_uCommandScanPos < 4))
			{
				uint8_t  *pCommandByte = (uint8_t *)(&uCommand)+m_uCommandScanPos;
				if(pScanPos > (data + len)) // rest in next packet
					m_state = stDetectCommandWord;
				else
				{
					*pCommandByte = *pScanPos++;
					if(m_uCommandScanPos == 3)
					{
						m_state = stDispatch;
					}
					m_uCommandScanPos++;
				}
			}
		}
	}
	else
	{
		if(m_state == stDetectCommandWord)
		{
			while(m_uCommandScanPos < 4)
			{
				uint8_t *pCommandByte = (uint8_t *)(&uCommand)+m_uCommandScanPos;
				*pCommandByte = *pScanPos++;
				m_uCommandScanPos++;
			}
			m_state = stDispatch;
		}
	}

	if(m_state == stDispatch)
	{
		m_uRetryCount = 0;
		m_uDispatchTime = HAL_GetTick();

		m_pCurrentCommandHandler = m_commandHandlers[uCommand];
		if(m_pCurrentCommandHandler)
		{
			if(m_pCurrentCommandHandler->StreamInit(uCommand))
				m_state = stProcessing;
			else
			{
				m_state = stDetect;
				LogTimeTaken(CDCCommandHandler::srFinish, 0);
			}
		}
		else
			m_state = stDetect; // No handler go back to detect state
	}

	if(m_state == stProcessing)
	{
		m_dataStream.AddData(pScanPos, len - (pScanPos - data));
		CDCCommandHandler::StreamResult result = m_pCurrentCommandHandler->StreamData(m_dataStream);

		switch(result)
		{
			case CDCCommandHandler::srError:
			case CDCCommandHandler::srFinish:
			{
				// handler has finished or failed go back to detect state
				m_state = stDetect;
				LogTimeTaken(result, m_pCurrentCommandHandler->GetBytesHandled());
				blit_enable_ADC();
			}
			break;

			case CDCCommandHandler::srNeedData:
				m_uRetryCount++;
				if(m_uRetryCount > 1)
				{
					m_state = stDetect;
					LogTimeTaken(result, m_pCurrentCommandHandler->GetBytesHandled());
					blit_enable_ADC();
				}
			break;

			case CDCCommandHandler::srContinue:
			break;
		}
	}


	return m_state != stDetect;
}

uint32_t CDCCommandStream::GetTimeTaken(void)
{
	return HAL_GetTick() - m_uDispatchTime;
}


uint8_t	*CDCCommandStream::GetFifoWriteBuffer(void)
{
	uint8_t *pData = NULL;
	if(m_uFifoUsedCount < CDC_FIFO_BUFFERS-1)
	{
		pData = m_fifoElements[m_uFifoWritePos].m_data;
#ifdef FIFO_DEBUG
    m_uWritePacketsGot++;
#endif  
	}
	else
		m_bNeedsUSBResume = true;

	return pData;
}

void	CDCCommandStream::ReleaseFifoWriteBuffer(uint8_t uLen)
{
#ifdef FIFO_DEBUG
  m_uWritePacketsReleased++;
  m_uWriteBytesReceived += uLen;
#endif  
	m_fifoElements[m_uFifoWritePos].m_uLen = uLen;
	m_uFifoWritePos++;
	if(m_uFifoWritePos == CDC_FIFO_BUFFERS)
		m_uFifoWritePos = 0;
	m_uFifoUsedCount++;
}

CDCFifoElement  *CDCCommandStream::GetFifoReadElement(void)
{
	CDCFifoElement *pElement = NULL;

	if(m_uFifoUsedCount)
  {
		pElement = &m_fifoElements[m_uFifoReadPos];
#ifdef FIFO_DEBUG
    m_uReadPacketsGot++;
#endif  
  }
	return pElement;
}

void CDCCommandStream::ReleaseFifoReadElement(void)
{
	m_uFifoUsedCount--;
#ifdef FIFO_DEBUG
  m_uReadPacketsReleased++;
#endif  
	m_uFifoReadPos++;
	if(m_uFifoReadPos == CDC_FIFO_BUFFERS)
		m_uFifoReadPos = 0;
}

#ifdef NDEBUG
  int a = 1;
#else
  int a = 2;
#endif

#ifdef FIFO_DEBUG
void CDCCommandStream::ResetDebugData()
{
  // m_uWritePacketsGot = 1;
  // m_uWritePacketsReleased = 0;
  // m_uReadPacketsGot = 0;
  // m_uReadPacketsReleased = 0;
  // m_uWriteBytesReceived = 0;
}

void CDCCommandStream::DisplayDebugData()
{
  printf("Write: \tgot:   %8lu,\treleased: %8lu,\tbytes: %8lu\n\r", m_uWritePacketsGot, m_uWritePacketsReleased, m_uWriteBytesReceived);
  printf("Read:  \tgot:   %8lu,\treleased: %8lu\n\r", m_uReadPacketsGot, m_uReadPacketsReleased);
  printf("Fifo:  \tcount: %8u,\tread:     %8u,\tWrite: %8u\n\r", m_uFifoUsedCount, m_uFifoReadPos, m_uFifoWritePos);
  printf("\n");
  ResetDebugData();
}
#endif
