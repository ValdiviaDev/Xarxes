#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > sizeof(uint32)) //Input last sequence number
	{
		uint32 networkId;
		ReplicationAction repAction;

		packet >> networkId;
		packet >> repAction;

		switch (repAction) {
		case ReplicationAction::Create:
		{
			GameObject* go = Instantiate();
			if (go) {
				uint8 goType;

				packet >> go->position.x;
				packet >> go->position.y;
				packet >> go->angle;
				packet >> go->size.x;
				packet >> go->size.y;
				packet >> goType;

				//Create sprite
				go->sprite = App->modRender->addSprite(go);
				go->sprite->order = 5;

				//Put texture
				switch (goType) {
				case 0:
					go->sprite->texture = App->modResources->laser;
					break;
				case 1:
					go->sprite->texture = App->modResources->spacecraft1;
					break;
				case 2:
					go->sprite->texture = App->modResources->spacecraft2;
					break;
				case 3:
					go->sprite->texture = App->modResources->spacecraft3;
					break;
				}

				if (goType != 0)
				{
					
					go->collider = App->modCollision->addCollider(ColliderType::Player, go);
					go->collider->isTrigger = true;

					
					go->behaviour = new Spaceship;
					go->behaviour->gameObject = go;

					
				}
				else
				{
					
					go->collider = App->modCollision->addCollider(ColliderType::Laser, go);
					go->collider->isTrigger = true;
					go->sprite->order = 1;
				}

				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, networkId);
			}
			else
				LOG("THE GAMEOBJECT COULDN'T BE INSTANTIATED (ReplicationManagerClient.cpp)");

			break;
		}

		case ReplicationAction::Update:
		{
			vec2 position;
			packet >> position.x;
			packet >> position.y;

			float angle;
			packet >> angle;

			vec2 size;
			packet >> size.x;
			packet >> size.y;

			uint8 goType;
			packet >> goType;

			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);



			if (go) {
				go->position = position;
				go->angle = angle;
			}
			else { //Create GameObject if not created
				//TODO: Look why it crashes when entering here
				LOG("Creating GO inside Update (!!!)");
				go = Instantiate();

				go->position = position;
				go->angle = angle;
				go->size = size;

				//Create sprite
				go->sprite = App->modRender->addSprite(go);
				go->sprite->order = 5;

				//Put texture
				switch (goType) {
				case 0:
					go->sprite->texture = App->modResources->laser;
					break;
				case 1:
					go->sprite->texture = App->modResources->spacecraft1;
					break;
				case 2:
					go->sprite->texture = App->modResources->spacecraft2;
					break;
				case 3:
					go->sprite->texture = App->modResources->spacecraft3;
					break;
				}

				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, networkId);
			}

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
		}
	}
}
