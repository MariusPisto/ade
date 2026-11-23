# ADE: Apprenti Démineur de l'Extrême

Cooperative game built around a fake bomb to defuse. Project for the course
*OS embarqués pour l'Edge Computing* (SI5/M2, Université Côte d'Azur, 2026-2027).

- An **Arduino Uno** running **FreeRTOS** acts as an autonomous smart sensor/actuator (the bomb).
- A **Raspberry Pi** is the edge gateway: I2C master (polling only), relaying to a cloud over MQTT/HTTP.

## Repository

| File | Content |
|---|---|
| `CONCEPTION.md` | Detailed design: game rules, modules, pinout, FreeRTOS tasks, I2C protocol, gateway, experiments, schedule |
| `components.txt` | Inventory of the available hardware |
| `edge_rtos_project2026.pdf` | Project brief |

## Commit convention

Commit messages are written in **English** and follow
[Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/):

```
<type>(<optional scope>): <short summary>

<optional body: what and why, wrapped at 72 characters>
```

### Types

| Type | Use for |
|---|---|
| `feat` | A new feature (new module, new game mechanic, new gateway endpoint) |
| `fix` | A bug fix |
| `docs` | Documentation only (`README.md`, `CONCEPTION.md`, the article) |
| `refactor` | Code change that neither fixes a bug nor adds a feature |
| `perf` | Performance or timing improvement (e.g. reducing fault detection latency) |
| `test` | Adding or updating tests and measurements |
| `build` | Build system, dependencies, PlatformIO/Arduino configuration |
| `ci` | Continuous integration |
| `chore` | Maintenance that doesn't fit elsewhere (hardware inventory, cleanup) |
| `revert` | Reverts a previous commit |

### Scopes (optional)

Use the part of the project the commit touches, for example:
`arduino`, `rpi`, `cloud`, `perimeter`, `keypad`, `i2c`, `lcd`, `article`.

### Rules

- Summary in the imperative mood, lowercase, no final period, 72 characters max:
  `feat(perimeter): detect ultrasonic fault within 50 ms`, not `Added fault detection.`
- One logical change per commit.
- A breaking change (e.g. a change to the I2C protocol between the Uno and the RPi)
  adds `!` after the type and a `BREAKING CHANGE:` footer:

```
feat(i2c)!: add a checksum byte to status frames

BREAKING CHANGE: the RPi gateway must read 9 bytes instead of 8.
```

### Examples

```
feat(keypad): decode the analog keypad on A2
fix(lcd): restore the software I2C bus after a NACK
docs: update the pinout table in CONCEPTION.md
perf(perimeter): move the ultrasonic ping to Timer1
chore: add the DIP switch to components.txt
```
