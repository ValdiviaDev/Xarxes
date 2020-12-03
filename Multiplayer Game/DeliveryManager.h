#pragma once
#include <list> 
// TODO(you): Reliability on top of UDP lab session

class DeliveryManager;

class DeliveryDelegate
{
public:

	void OnDeliverySuccess(DeliveryManager* deliveryManager);
	void OnDeliveryFailure(DeliveryManager* deliveryManager);
};

struct Delivery
{
	Delivery(uint32 sequenceNum);
	~Delivery();

	uint32 sequenceNumber = 0u;
	double dispatchTime = 0.0f;
	DeliveryDelegate* delegate = nullptr;
};


class DeliveryManager
{
public:

	//For senders to write a new seq. numbers into a packet.
	Delivery* writeSequenceNumber(OutputMemoryStream &packet);

	//For receivers to proccess the seq. number from an incoming packet.
	bool processSequenceNumber(const InputMemoryStream &packet);

	//For receivers to write ack'ed seq. numbers into a packet.
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream &packet);

	//For senders to process ack'ed seq. numbers from a packet.
	void processAckdSequenceNumbers(const InputMemoryStream &packet);
	void processTimedOutPackets();

	void clear();

private:
	//Private members (sender side)
	//- The next outgoing sequence number
	//- A list of pending deliveries

	uint32 nextOutgoingSeqNumber = 0u;
	std::list<Delivery*> pendingDeliveries;

	//Private members (receiver side)
	//- The next expected sequence number
	//- A list of sequence number pending ack

	uint32 nextIncomingSeqNumber = 0u;
	std::list<uint32> pendingAcknowledges;

};