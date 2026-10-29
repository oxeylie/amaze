# Game Architecure

Game architecure planning

## Client
### Threading 

**Threads**:
 - Main (Logic) (handles user-inputs, receive/sends server updates, sends graphic updates) 
 - Network (handles ATP stream, start/terminate connection, receive/sends state updates)
 - Sound (handles sound playback)
 - Rendering (handles display, orchestrate (load balance) workers to compute graphics)
   - Workers 1..x (graphic computation) | Maybe GPU instead

**Thread communication**:
 - Logic -> Network:     updates + disconnects `queue mutex`     + `queue wake (EVFILT_USER/eventfd)`
 - Network -> Logic:     updates + disconnects `queue mutex`
 - Logic -> Rendering:   loaded state/terrain  `hasmap + structs mutex`
 - (Rendering <-> Worker: screen buffer        `2d array mutexs`)
 - Logic -> Sound:       playback requests     `queue mutex`

### Headers

**graphics.h**:
 - Textures loading
 - Graphics computation
 - Graphics display
 - Movement interpolation
 - UI 

**sound.h**
 - Sound loading
 - Sound computation
 - Sound playback
 - Sound mix

## Server 
### Threading

**Threads**:
 - Main (Logic) (handles client updates, game ticking)
 - Network (handles ATP stream, accepts/blocks incoming connections, receive/sends state updates)
 - Terrain (handles saves/load world states, orchestrate (load balance) workers to generate terrain)
   - Workers 1..x (terrain generation computation)

**Thread communication**:
 - Logic -> Network:   updates + disconnects `queue mutex`   + `queue wake (EVFILT_USER/eventfd)`
 - Network -> Logic:   updates + disconnects `queue mutex`
 - Logic -> Terrain:   save/load requests    `queue mutex`
 - Terrain -> Logic:   load state            `queue mutex`
 - Terrain <-> Worker: terrain gen           `struct mutexs`
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

**network.h**:
 - Sockets
 - Events
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
