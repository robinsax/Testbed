# Game Systems

## Core Stats

- Energy
  - Regenerated with items
- Stamina
  - Regenerated passively using energy
- Health
  - HP is regenerated with items
  - Robots are made of parts. Each part has its' own HP pool
  - Parts can take "persistent damage" that:
    - Is removed with items
    - Prevents their HP pool from being healed
    - Drains energy over time
    - Has other debuffs depending on the part

## States

- Walk
- Sprint
  - Drains stamina
- Jump
  - Drains stamina
- Aim
  - Including firing while aiming
  - Can't sprint or jump
- Hipfire
  - Can't sprint
- Crouch
  - Can't jump
- Strafe
  - Can't sprint
- Squint
  - Can't sprint 
- Lean
  - Can't sprint
- Check status
  - Can't aim or hipfire
- Inventory
  - Can't move at all, but can still check status, and use items
- Interacting
  - Can't sprint, aim, or hipfire

## Inventory

- Always take items, even when inventory full (drop others)
- "Hands" is its' own slot
- Inventory management drag and drop
  - Can interact with nearby world items
    - Allow camera backoff to see entire reach radius
  - Can quick take to hands and quick drop with mouse buttons
- Always use items from hands
- Quick slots
  - Can bind 1-5 keys to specific items and/or item types (context dependent)














--------------

# TODO

- Robot part system
  - As meshes
  - Proper proxies
- Refactor to SYSTEMS
- Replicated item blueprints
  - Allow validate action
  - Allow extend animation context
- Magazine and projectile replications, recoil
  - Magazine cap.
  - Local projectiles
- Fix item attachment and rotation control
- More animation keys
  - Better proxy anchor setup
  - Natural rest movement
  - Local move offsets
- Inventory intellegence (drop/move to force take, etc.)
- Quick slot system
- Audio system
- De Test* and create proper bases
- Blueprint exposures
- Player lifecycle
- Magazine reload
- Core item variations
- Emergency energy recharge somehow
- Special items (wallhack, etc.)
- Drones



- Strings to UENUMS (once final sets known)
