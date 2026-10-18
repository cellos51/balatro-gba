# E2E Tests

Runs the compiled ROM using [`@gba-kit/gba-node`](https://github.com/macabeus/gba-kit), a headless GBA emulator written in TypeScript.

## Setup

Before running the E2E tests locally, make sure you have:

- Node.js >= 22
- A built ROM and its ELF file (`build/balatro-gba.gba` + `build/balatro-gba.elf`). Build them as described in the [Build Instructions](../CONTRIBUTING.md#build-instructions), e.g. with Docker on Linux or macOS: `UID=$(id -u) GID=$(id -g) docker compose up`

```bash
# Install dependencies (pulls @gba-kit/* from npm)
cd tests_e2e
npm install
```

## Running

```bash
npm test            # single run
npm run test:watch  # watch mode
```

The tests read the ROM and ELF in `build/`, so rebuild the ROM after changing the game's source.

## Writing a new test

### 1. Create a test file

Add a file under [`tests/`](./tests/), e.g. `tests/shop.test.ts`:

```ts
import { describe, it, expect } from 'vitest';
import { createRuntime, navigateToPlaying, readVar, waitForHandState } from '../helpers/runtime.js';
import { HandState, VAR } from '../helpers/game_variables.js';

describe('Shop', () => {
  it('does something', async () => {
    const runtime = await createRuntime('shop-test');
    const engine = runtime.engine;

    // Navigate to a known state using helpers
    await navigateToPlaying(runtime);

    // Select the first card, then play the hand
    await engine.press('a');
    await engine.wait({ frames: 10 });
    await engine.press('l');

    // Wait for the game to reach a state, or fail after 1800 frames
    await waitForHandState(runtime, HandState.HAND_SELECT, 1800);

    // Assert on game variables
    expect(readVar(runtime, VAR.score)).toBeGreaterThan(0);
  });
});
```

### 2. Key concepts

- **`createRuntime(name)`** boots a fresh emulator with the ROM **and ELF** (so the engine owns the symbols + DWARF). Each test gets an isolated instance.
- **`readVar(runtime, path)`** reads a game variable by its C `symbol.field` path, such as `VAR.score` (`'g_game_vars.score'`).
- **`engine.press(button)`** presses a GBA button (`a`, `b`, `l`, `r`, `up`, `down`, `left`, `right`, `start`, `select`).
- **`engine.wait({ frames })`** advances the emulator by N frames (~60 fps).
- **`waitForGameState(runtime, state, timeout)`** and **`waitForHandState(runtime, state, timeout)`** advance until the game or hand state equals `state`, or throw after `timeout` frames.

> 🔖 Discover all the engine methods available by reading the [Scripting Guide](https://github.com/macabeus/gba-kit/blob/main/docs/scripting.md)

### 3. Navigation helpers

`helpers/runtime.ts` provides shortcuts to reach common game states:

| Helper                       | Destination                              |
| ---------------------------- | ---------------------------------------- |
| `skipDisclaimer(runtime)`    | Main menu                                |
| `startGame(runtime)`         | Blind select                             |
| `selectBlind(runtime)`       | Playing (cards being dealt)              |
| `waitForHandSelect(runtime)` | Playing (cards in hand, ready for input) |
| `navigateToPlaying(runtime)` | All of the above combined                |

### 4. Reading game variables

[`helpers/game_variables.ts`](./helpers/game_variables.ts) lists the variables the tests use as C paths in `VAR`, and the engine resolves each path from the build it was created with: the symbol address from the ELF's symbol table, and the field's byte offset and size from the variable's DWARF type:

```ts
export const VAR = {
  gameState: 'game_sm.state', // a field of a file-static struct
  handState: 's_hand.state',
  score: 'g_game_vars.score', // a field of a global struct
  // ...
};
```

`readVar` reads the right number of bytes and decodes bitfields, and the wait helpers take the same paths. A path can also name a nested field (`'g_game_vars.rng_info.seed'`) or a plain global.

If the game source moves a variable into a struct, or changes a field's type or size, nothing here needs updating. A renamed symbol or field changes one path in `VAR`.

> The enum constants (`GameState`, `HandState`, `BlindType`, `HandType`) are read from the build's DWARF too, so they never go stale when the game's enums change.
