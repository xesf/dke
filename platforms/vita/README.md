# PlayStation Vita

The Vita build stretches the 640 x 480 frame 4:3 into the 960 x 544 screen,
plays sound on one audio port and reads the buttons and the left stick. It
needs a Vita with HENkaku or Ensō.

This build compiles, but it has not been run on a Vita or in Vita3K yet.

## Build

Install [VitaSDK](https://vitasdk.org) in `~/vitasdk`, or set `VITASDK`, then:

```sh
make vita
```

This writes `build/vita/dke.vpk` (title ID `DKE000001`).

## Install

Install the VPK with VitaShell, then copy the game folder (`DATAS`, `SC`,
`BIGANIMS`, `DKE.CFG` and the rest of `bin/game`) to `ux0:data/dke`.

## Controls (provisional)

| Vita | Key |
|---|---|
| D-pad, left stick | Arrows |
| Cross | Space and Enter (action, OK) |
| Square, Triangle | Q, E |
| Circle | C (draw or sheathe) |
| L | Tab (special mode) |
| R | I (inventory) |
| Start | Esc (menu) |
| Select | P (pause) |
