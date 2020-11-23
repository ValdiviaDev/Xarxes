#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	//A
	uint32 networkId;
	ReplicationAction repAction;

	packet >> networkId;
	packet >> repAction;

	switch (repAction) {
	case ReplicationAction::Create:
	{
		GameObject* go = Instantiate();
		App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, networkId);
		//TODO: Fill
		
		break;
	}

	case ReplicationAction::Update:
	{
		GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);
		//TODO: Fill
		break;
	}

	case ReplicationAction::Destroy:
	{
		GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);

		if (go) {

			App->modLinkingContext->unregisterNetworkGameObject(go);
			Destroy(go);
		}

		break;
	}

	case ReplicationAction::None:
		//Nothing
		break;
	}

}
