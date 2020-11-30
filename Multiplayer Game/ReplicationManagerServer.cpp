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

		if (replicate_aux.action == ReplicationAction::Create || replicate_aux.action == ReplicationAction::Update) {


			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(replicate_aux.networkId);

			if (gameObject) {


				packet << gameObject->position.x;
				packet << gameObject->position.y;
				packet << gameObject->angle;
				packet << gameObject->size.x;
				packet << gameObject->size.y;

				if (gameObject->sprite->texture == App->modResources->spacecraft1)
					packet << (uint8)1;
				else if (gameObject->sprite->texture == App->modResources->spacecraft2)
					packet << (uint8)2;
				else if (gameObject->sprite->texture == App->modResources->spacecraft3)
					packet << (uint8)3;
				else
					packet << (uint8)0;

			}
		}
	}

	repCommand.clear();

}
