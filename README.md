# Multiplayer game Space Spacers

Space Spacers is a game / university project made by Ricardo Gutiérrez & David Valdivia in c++. The game is a top down shooter in space where you are a spaceship that can move arround and can shoot.

The controls are
  - A/D: Rotate spaceship
  - ↓: Accelerate
  - ←: Shoot

List of implemented tasks:
  - UDP virtual connection (Task completed without bugs)
    - Send PING: Here we send a PING message to the client every 1 second to assure everything is alright, if it doesn't arrive, we disconnect the player. (Made by David)
    - Handle players join/leave events: Assure that a max number of 8 players joining and leaving works correctly. (Made by Ricardo)
    
  - Replication manager (Task completed)
    It replicates the whole world state of the server to the client
    - Notify the spawn, update and destroy of the GameObject to all the clients (Replication client made by Ricardo, and replication server by David)
  
  Known bugs:
    - If you shoot very rapidly a laser stays still in the air. We with it may have relation with the client reads the world state.
  
  - Delievery manager (Task almost completed)
    It assures that the game works well with jitter or packet losses.
    - Packet delivery notification system (Made by Ricardo)
    - Overcome packet loss using redundancy (Made by David)

    
    Known bugs:
      - Shooting other players results in a crash.
      - Shooting doesn't work well with this system, and makes that sometimes the game crashes or freezes. Whe think this may happen in relation with processing timed out packages.
    
