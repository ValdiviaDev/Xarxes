#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	ReplicationCommand replicate;
	replicate.action = ReplicationAction::Create;
	replicate.networkId = networkId;

	bool exists = false;

	for (int i = 0; i < repCommand.size(); i++) {

		if (repCommand[i].action == replicate.action && repCommand[i].networkId == replicate.networkId)
			exists = true;
	}

	if (!exists)
		repCommand.push_back(replicate);
}

void ReplicationManagerServer::update(uint32 networkId)
{
	ReplicationCommand replicate;
	replicate.action = ReplicationAction::Update;
	replicate.networkId = networkId;

	bool exists = false;

	for (int i = 0; i < repCommand.size(); i++) {

		if (repCommand[i].action == replicate.action && repCommand[i].networkId == replicate.networkId)
			exists = true;
	}

	if(!exists)
		repCommand.push_back(replicate);
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	ReplicationCommand replicate;
	replicate.action = ReplicationAction::Destroy;
	replicate.networkId = networkId;

	bool exists = false;

	for (int i = 0; i < repCommand.size(); i++) {

		if (repCommand[i].action == replicate.action && repCommand[i].networkId == replicate.networkId)
			exists = true;
	}

	if (!exists)
		repCommand.push_back(replicate);
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{

	for (int i = 0; i < repCommand.size(); i++) {

		ReplicationCommand replicate_aux = repCommand[i];
		packet << replicate_aux.networkId;
		packet << replicate_aux.action;

		switch (replicate_aux.action) {

		case ReplicationAction::Create:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(replicate_aux.networkId);

			if (gameObject) {
				
				LOG("GAMEOBJECT FOUND");
				
			}
		}
			break;

		case ReplicationAction::Update:


			break;

		case ReplicationAction::Destroy:


			break;


		}
		

		repCommand.clear();

	}




}
