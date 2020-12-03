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
	return !pendingAcknowledges.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	//Send a packet with: amount of acks and all the acknowledged sequence numbers from all received packets
	uint32 numOfAcks = pendingAcknowledges.size();

	packet << numOfAcks;

	for (uint32 sequenceNum : pendingAcknowledges) {
		packet << sequenceNum;

	}

	pendingAcknowledges.clear();

}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	//Get amount of acks from packet 
	uint32 numOfAcks;
	packet >> numOfAcks;


	for (int i = 0; i < numOfAcks; ++i)
	{

		uint32 sequenceNum;
		packet >> sequenceNum;

		//Look for the sequence numbers on the pending deliveries list
		for (std::list<Delivery*>::iterator del_it = pendingDeliveries.begin(); del_it != pendingDeliveries.end(); del_it++)
		{

			Delivery* del = *del_it;
			//If it exists, success! and delete it from the list
			if (del->sequenceNumber == sequenceNum)
			{

				if (del->delegate)
					del->delegate->OnDeliverySuccess(this);

				delete del->delegate;
				delete del;

				pendingDeliveries.erase(del_it);

				break;
			}
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
}

void DeliveryManager::clear()
{

	for (Delivery* del : pendingDeliveries) {
	
		delete del;
	}

	pendingAcknowledges.clear();
	pendingDeliveries.clear();

}

void DeliveryDelegate::OnDeliverySuccess(DeliveryManager* deliveryManager) 
{
}

void DeliveryDelegate::OnDeliveryFailure(DeliveryManager* deliveryManager)
{
}
