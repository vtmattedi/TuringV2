# Turing Recover

Goal: reconstruct the Turing Garden NightMare Board REV 2.0 schematic from manufacturing evidence, without depending on the original EasyEDA project.

## Source of truth priority
1. Copper connectivity in Gerber/ODB++
2. BOM / assembly render / silkscreen labels
3. Physical-board measurements and continuity tests
4. Firmware behavior and GPIO names as corroborating evidence

## Scope now
- Reconstruct schematic only.
- Do not rebuild PCB yet.
- Preserve uncertainty explicitly; do not turn inferred connections into facts.

## Canonical recovered representation
- `model/components.yaml`: identified components and values.
- `model/connectors.yaml`: connector labels/pin semantics visible on the board.
- `model/nets.yaml`: recovered electrical nets; starts empty and is filled only after trace validation.
- `model/evidence-map.yaml`: provenance/confidence for every recovered fact.
- `schematic/`: eventual KiCad schematic/project.

## Evidence currently available
- Top/bottom Gerbers, masks, silkscreen, outline, drills.
- ODB++/Genesis CAM job (`3961371a_y32`).
- Top/bottom assembly renders.
- Partial recovered BOM supplied from manufacturing data.
- Three raw 4 MiB ESP32 flash reads.
- Live board is functional and can be queried/tested.
