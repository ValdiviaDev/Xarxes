#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
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

