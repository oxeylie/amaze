# Amaze

> [!WARNING]
> The project is still in a state of very early development, for now core features are still being implemented. That means that the game is not yet in a playable state.

*In an out of time space, you wander, out of place, among what seems to be ruins of a forsaken and yet inexplicably familiar world.*

## Core concept

Amaze aims to be a TUI game with ascii graphics that simulates a 3d open-world, with multiplayer support.

> [!NOTE]
> While it is too early on to guarantee that all of the features stated above will be implemented as is, the development is heading toward those as its main goals. 

## Compilation

Amaze is not yet available on any package manager and needs to be compiled from source in order to be played. To do so follow the instructions below.  
*(Note that amaze is only meant to support GNU Linux & macOS platforms)*

```bash
git clone https://www.github.com/oxeylie/amaze amaze
cd amaze
make client
mv ./out/client ./client
```
