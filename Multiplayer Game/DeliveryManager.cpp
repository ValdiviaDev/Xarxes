#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	//Write the sequence number into the packet
	packet << nextOutgoingSeqNumber;

	//Store the new delivery into a list into the manager
	Delivery* del = new Delivery();
	del->sequenceNumber = nextOutgoingSeqNumber;
	del->dispatchTime = Time.time;
	del->delegate = new DeliveryDelegate();
	pendingDeliveries.push_back(del);

	nextOutgoingSeqNumber++;

	//Return the delivery
	return del;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	bool ret = false;
	uint32 sequenceNumRecv = 0;
	packet >> sequenceNumRecv;


	if (sequenceNumRecv == nextIncomingSeqNumber) {
		nextIncomingSeqNumber++;
		
		ret = true;
	}

	pendingAcknowledges.push_back(sequenceNumRecv);
	
	return ret;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	return true;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
}

void DeliveryManager::processTimedOutPackets()
{
}

void DeliveryManager::clear()
{
}

void DeliveryDelegate::OnDeliverySuccess(DeliveryManager* deliveryManager) 
{
}

void DeliveryDelegate::OnDeliveryFailure(DeliveryManager* deliveryManager)
{
}
