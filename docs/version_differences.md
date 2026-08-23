# Version differences

This repository supports the Rev 0 USA (`G4QE01`) and Japan (`G4QJ01`)
releases. They share almost all of their engine, game modes, rules, physics, and
AI. The Japanese release is best understood as a regional adaptation rather
than a substantially different revision of the game.

## Japanese presentation

Most of the additional Japanese code supports localized text and layouts. The
game loads dedicated Japanese font resources and changes text measurement and
wrapping so Japanese strings fit correctly. Button labels are slightly smaller,
some highlights and colours are adjusted, and longer buffers are used for cup
standings.

The Japanese release also selects alternate layouts for screens such as team
and captain selection, cup standings and trophies, popups, options, save/load,
tournament setup, and Strikers 101. It includes localized disc-error graphics
and a Japanese version of the sixth lesson movie. Goal, winner, team, and menu
messages use a few Japanese-specific formatting rules where a direct reuse of
the English presentation would not read naturally.

## Menus and memory cards

Controller-selection screens have extra controller indicators and more explicit
connected/disconnected presentation. The Japanese options menu can show a
dedicated message when no cheats have been unlocked, and several menus make
small visibility or callback adjustments to suit their localized layouts.

Save/load handling performs additional memory-card serial-number checks during
file operations. This lets it detect when the inserted card has changed and
route the player through the appropriate retry, format, or error flow.

## Gameplay

The version-specific code does not indicate a broad gameplay or balancing
revision. One confirmed gameplay-side change is that the Japanese release stops
a fielder's action-specific post-physics processing while that player is frozen.
The shared player update still runs first, but the remaining action triggers wait
until the frozen state clears.

## Binary identity

Both executables use the same entry point and overall section structure. The
Japanese `main.dol` is 13,792 bytes larger, mainly because of additional code
and read-only localization data. Those additions shift many later addresses, so
the project keeps separate symbol, split, and linker metadata for each release.

| Version | Region | `main.dol` size | SHA-1 |
| --- | --- | ---: | --- |
| `G4QE01` | USA | 3,210,784 bytes | `376d699c99b6b0949abe1b4ceccefdef7828d2b5` |
| `G4QJ01` | Japan | 3,224,576 bytes | `d116f02b778a4f69725fd1c00656012d16ebf94a` |

This is a summary of differences represented by the reconstructed executables
and their version-specific paths. It is not an exhaustive comparison of every
asset on the two retail discs.
