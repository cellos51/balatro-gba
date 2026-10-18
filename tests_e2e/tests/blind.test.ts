import { describe, it, expect } from 'vitest';
import { createRuntime, getGameState, skipDisclaimer, startGame, selectBlind, readVar } from '../helpers/runtime.js';
import { GameState, BlindType, VAR } from '../helpers/game_variables.js';

describe('Blind selection', () => {
  it('a blind can be selected', async () => {
    const runtime = await createRuntime('blind-select');

    await skipDisclaimer(runtime);
    await startGame(runtime);

    expect(getGameState(runtime)).toBe(GameState.BLIND_SELECT);
    const roundBefore = readVar(runtime, VAR.round);

    await selectBlind(runtime);

    expect(getGameState(runtime)).toBe(GameState.ROUND);
    // Round should have incremented
    expect(readVar(runtime, VAR.round)).toBe(roundBefore + 1);
  });

  it('a blind can be skipped', async () => {
    const runtime = await createRuntime('blind-skip');
    const engine = runtime.engine;

    await skipDisclaimer(runtime);
    await startGame(runtime);

    // We're at BLIND_SELECT for the small blind
    expect(getGameState(runtime)).toBe(GameState.BLIND_SELECT);
    expect(readVar(runtime, VAR.currentBlind)).toBe(BlindType.SMALL);

    // Wait for animation substate
    await engine.wait({ frames: 30 });

    // Press DOWN to move to "skip", then A to confirm
    await engine.press('down');
    await engine.wait({ frames: 5 });
    await engine.press('a');
    await engine.wait({ frames: 10 });

    // After skipping, current_blind should advance from SMALL to BIG
    expect(readVar(runtime, VAR.currentBlind)).toBe(BlindType.BIG);
    // Should still be on BLIND_SELECT (showing the big blind now)
    expect(getGameState(runtime)).toBe(GameState.BLIND_SELECT);
  });
});
