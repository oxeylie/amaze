# Game Architecure

Game architecure planning

## Client
### Threading 

**Threads**:
 - Main (Logic) (handles user-inputs, receive/sends server updates, sends graphic updates) 
 - Network (handles ATP stream, start/terminate connection, receive/sends state updates)
 - Rendering (handles display, orchestrate (load balance) workers to compute graphics)
   - Workers 1..x (graphic computation)
 - (Music)

**Thread communication**:
 - Logic -> Network:     updates + disconnects `queue mutex`     + `queue wake (EVFILT_USER/eventfd)`
 - Network -> Logic:     updates + disconnects `queue mutex`     + `mutex condition`
 - Logic -> Rendering:   loaded state/terrain  `hasmap + structs mutex`
 - Rendering <-> Worker: screen buffer         `2d array mutexs` + `mutex condition`

### Headers

**graphics.h**:
 - Textures loading
 - Graphics computation
 - Graphics display
 - Movement interpolation
 - UI 

## Server 
### Threading

**Threads**:
 - Main (Logic) (handles client updates, game ticking)
 - Network (handles ATP stream, accepts/blocks incoming connections, receive/sends state updates)
 - Terrain (handles saves/load world states, orchestrate (load balance) workers to generate terrain)
   - Workers 1..x (terrain generation computation)

**Thread communication**:
 - Logic -> Network:   updates + disconnects `queue mutex`   + `queue wake (EVFILT_USER/eventfd)`
 - Network -> Logic:   updates + disconnects `queue mutex`   + `mutex condition`
 - Logic -> Terrain:   save/load requests    `queue mutex`   + `mutex condition`
 - Terrain -> Logic:   load state            `queue mutex`   + `mutex condition` 
 - Terrain <-> Worker: terrain gen           `struct mutexs` + `mutex condition`
 - Terrain <-> Logic:  loaded terrain        `hashmap mutex` + `atomic writes` 

### Headers

**engine.h**:
 - Game loop
 - Updates
 - Interaction check
 - Movement check

**terrain.h**:
 - Terrain load/save 
 - Terrain format 
 - Generation algorithm

## Common
### Headers

**atp.h**:
 - Sockets
 - Requests format
 - Handshake & identity

**engine.h**:
 - Interactions
 - Movement
 - Geometry
 - Data storage
 - Infos
 - Players
 - Loaded zone
