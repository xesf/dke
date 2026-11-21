# Dark Earth walkthrough as an engine test script

Source: the "Dark Earth Solution" by the people at Kalisto Entertainment (condensed; the full text
was supplied by the project owner). Use it as the reference route for headless playthrough tests
(`DKE_AUTOPLAY`, see `src/autoplay.c`) and for manual checks of each chapter.

Controls: `C` draws the weapon, `Tab` switches Light/Dark mode, `Space` jumps, ducks and searches,
`Ctrl` attacks (alternate scheme) or Ctrl+arrows (default). See README.

## Provisional room map

Derived from the personas each level script uses (`src/levels/lieuNN*.c`). Verify before relying on it.

| Level | Evidence | Probable place |
|---|---|---|
| 1 | Kabak, Arkhan 0/1 | Barracks: bedroom, training room |
| 2, 3, 4, 10, 33 | Kalhi 13/33 | Upper Town squares, barracks, platform to the Lower Town |
| 5 | Bogdaran | Bogdaran's chambers (oiler) |
| 6 | Dhorkan, Phedoria | Provost's office |
| 7, 22, 27, 29 | Phedoria, Zed, Lory | Temple, crypt, Lory's office |
| 8, 9, 20, 23 | Guardians | Barracks floors, prison, platform |
| 12, 50 | Bandor, Khorad | Builders' tower |
| 14, 54 | Provider, Malrot | Providers' area |
| 15 | Danrys | Danrys's home (Lower Town) |
| 16, 19 | Maor, Delia | Tavern and backstage |
| 18 | Armal | Armal Sadak's boat (Yong/Othello) |
| 21, 24, 34 | Sunseers, Thanandar, Sordos | Hospital, Sordos's office |
| 25, 46 | Rylsadhar | Father's apartments, secret laboratory |
| 28 | Archivist | Archive room |
| 30, 31, 32 | Konkalites, Gork, Bork | Sewers and Konkalite hideout |
| 36 | Lodar | Ramparts |
| 39 | Leona | Leona's cave |
| 40-44, 51-53 | no persona | Tomb, labyrinth, finale, credits (52) |
| 45 | Delia | Tavern |

## Story route and checkpoints

Each step is a milestone a bot or a tester can check (a script variable changes, an item appears,
a room change happens).

### 1. Barracks
1. Wake up, dress, take the sword. Leave the salt beef on the bedside table (needed later).
2. Train with Zed in the weapons room; Phedoria joins. Leave the axe there.
3. Go downstairs. The provost's door is guarded: be in Light mode and speak to the guard.
4. Provost Dhorkan orders you to guard the Council chamber.
5. Run to the temple of the Sun God (left of the barracks exit). Overhear Lory and Rylsadhar.
6. Join Jedar at the Council entrance with the sword drawn.
7. Cinematic: two councillors and Jedar are killed. Fight the assassin.
8. Cinematic: Lory is attacked; you take a cup of poison; Zed shoots with the Python 357.

### 2. Contaminated
1. Wake in the hospital. Dulnys attacks; kill him and take Thanandar (the weapon). The knife is in
   the other part of the room.
2. Thanandar the Healer enters. Talk to him. Do not go outside until he tells Phedoria all is well.
3. In his laboratory take the gas canister (right) and the knife (left). Save at the Sun symbol.
4. Go to your father's apartments (small square with the fountain). Take the empty flask from the
   shelves in the entrance. Speak to Rylsadhar; Kalhi arrives. Take the document under his bed.
5. Platform guards are watching: use Light mode to talk past them.
6. Temple, Council chamber, staircase on the left, left door: the archivist Sesekan. Examine the
   mirror from Before (right of the door), read the spell book, use the document on the mechanism
   to open the hideout and take the metal plate.
7. Barracks: stohll barrel on the table, fill the empty gourd. Optionally fetch the meat from your
   bedroom. Upstairs take the axe; speak to Phedoria in Light mode without the axe in hand.
8. Go out on the balcony (glass doors behind the staircase).

### 3. Escape
1. Left door: Bogdaran's chamber. Take the copper objects and lead plates, climb, take the key from
   the shelves, open the chest by the door and take Bogdaran's old uniform.
2. Balcony, opposite door: the prison. Kill the jailer, take his weapon and the cell keys.
3. Open the cell: Kalhi. Give her the old uniform.
4. Follow her as a prisoner. When the guard leaves, hide behind the cart in front of the lift, then
   use the console to go down. (Alternative: fight through; collect a spitrod from the platform guard.)

### 4. Lower City
1. Follow Kalhi to Danrys. Talk to him.
2. Cross the square, find the breastplate in the area, save.
3. Leave left, reach the tavern. Watch the show; follow Delia backstage. Give her the mirror from
   Before; she gives the password for Armal Sadak. (Alternative: Danrys gives it after one visit.)
4. Armal's boat (right of the tavern). Speak to him behind the table on the left; play Yong
   (Othello, level 18). Each game costs a weapon whatever the result.

### 5. Sewers
1. At the end of Armal's passage, jump into the water (Space) at the part with no barrier.
2. Combat mode. Kill two Konkalites; search the water for the other two weapons (Space).
3. Staircase, save, watch Sordos's meeting. Kill the approaching Konkalite.
4. Offer the two door guards stohll, or fight. Enter Sordos's apartments.
5. Fight Sordos and Thanandar (no escape). Loot: Sordos's weapon, key, weapon design; Thanandar's
   red key and the potion (save it).
6. Open the chest with Sordos's key: the proof of Dhorkan's alliance.
7. Alternative: skip Sordos and fight Thanandar at the hospital instead (second visit).
8. Special: if a Konkalite beats you after Thanandar's death you are caged; go Dark and rage twice
   to break the door.

### 6. Traitor
1. Search the barrels in the Konkalite hall for the handle.
2. Give Danrys the design and the handle: flame-thrower.
3. Barracks, Dhorkan's door, show the document to Phedoria. Enter the office; Dhorkan flees.
4. Take the stiletto from the left drawer; insert it in the wall opening; the passage stays open.
5. Down the steps: your dying father. Follow the passage to the Lower City.
6. Armal's, turn left to the ramparts: Kalhi. Return to the Upper City by the platform.
7. Temple: Thanandar (2.6 m) beats you; Zed takes the crypt passage. Follow him down.
8. Zed corrupts the Heart of Light and gives you the Tear of Life. Alternative: if Thanandar died
   earlier Zed is in the Konkalite jail; kill him after the scene and your father corrupts the Heart.
9. Dark mode (Tab): loot Zed.

### 7. The lights go out
1. Leave the crypt; Lory and Phedoria. Follow Lory to her office. She holds crystal part 1 (ring).
2. Builders' building: guard in blue (stohll after something salty, or kill him). Lift up.
3. Take the parchment from the workbench, threaten it at a fire: Bandor identifies himself.
   Show him part 1: he gives part 2.
4. Lower City bunker (red cross): fight Dhorkan, loot part 3 and the map from Before.
5. Put the map in the box by the door. Search the bunker: the electronic component, then the gun.
6. Secret passage to Dhorkan's office, then the prison. Kill the guard, open the right cell, kill
   the Konkalite. In Dark mode hit the bed: part 4 falls out. The key is complete.

### 8. Outside (optional)
1. Bunker staircase to the ramparts. Help Lodar against the Konkalite; get the helmet.
2. Sewers ladder with the aqualung. Underwater creature; ammunition under a ribcage.
3. Beach: two Shankreatures (use firearms). Poison in the cave. Cave with the giant snake.
4. Leona's shelter: information on the Tomb. Drinking spring water reduces Darkness (twice per
   life level).
5. Alternative crystal: rough crystal near the snake pond, cut by Danrys (needs the small axe).

### 9. Bogdaran
1. Danrys tells you to ask Bogdaran about the object from Before.
2. Speak to Bogdaran, then fake a hit in combat mode. Give him the object; he asks for Delia.
3. Fetch Delia (she needs you presentable: drink the potion or spring water). Maor attacks at the
   exit; beat him. Return with the lift.
4. Upper City: creatures about. Fight the one in the barracks. Take Delia to Bogdaran's lab.
5. Bogdaran oils the object. Alternative: use the large oil container by the staircase.

### 10. Explosion
1. Father's apartments: two guards attack. Use the crystal key on the bedroom wall lock.
2. Secret laboratory: parchments (Runka tomb, translation). Telescope: Dark mode, hit the table,
   take the hidden key.
3. Main square well: open the grille facing the Builders' tower, throw the object, run without
   turning. The square now has a hole.
4. Fountain: take the pump blade, put it on the propeller axle, go to the other end, Dark mode,
   jump. Crash at the bottom of the well.

### 11. The Tomb
1. Follow the small passage behind you; the labyrinth floor room; the small door on the far side
   closes behind you; the cliff.
2. Return to the big door. Left panel: RUNKA-SUN-PROTECTION. Right panel:
   PLANET-DESTRUCTION-SHANKR. Right lever validates. Save.
3. Labyrinth: keep to the right. Rotating blades: Space to duck, Space again to stand. Save point
   at the end.
4. Thanandar (monster). Stun him with firearms, run to the door, push both levers; left light
   down, right light up, before he reaches you.

### 12. Epilogue
The door opens, Thanandar explodes, the Runka heals Arkhan. Credits (level 52).

## Using this for testing

- Jump straight to a chapter with `DKE_GOTO=<room>:<from>` (the `from` room must link to `<room>`,
  otherwise the entry position and collision toggles differ from a real arrival; level 24 needs
  `from=27`).
- Night-only levels (40, 43, 44, 51) need `DKE_NIGHT=1`.
- Run a bot with `DKE_AUTOPLAY=1 DKE_FASTCLOCK=33 DKE_NOAUDIO=1 DKE_NOMOVIE=1`; add
  `DKE_AUTOPLAY_DBG=1` for walk logs and `DKE_COLLDBG=1` for collision logs.
- Known results (2026-11-21): all 55 levels load and run 20000 frames with the blind bot and no
  crash; the blind bot loops between levels 2 and 3 and cannot follow the story. A route-driven
  bot built from the checkpoints above is the next step.
