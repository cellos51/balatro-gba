import { describe, it, expect } from 'vitest';
import { createRuntime, navigateToPlaying, getHandState, readVar, waitForHandState } from '../helpers/runtime.js';
import { HandState, VAR } from '../helpers/game_variables.js';

describe('Hand actions', () => {
  it('playing a hand produces the expected outcome', async () => {
    const runtime = await createRuntime('hand-play');
    const engine = runtime.engine;

    await navigateToPlaying(runtime);

    // We should be in HAND_SELECT state with cards dealt
    expect(getHandState(runtime)).toBe(HandState.HAND_SELECT);

    const handsBefore = readVar(runtime, VAR.hands);
    const scoreBefore = readVar(runtime, VAR.score);

    // Select the first card (cursor starts on the first card in hand row)
    await engine.press('a');
    await engine.wait({ frames: 10 });

    // Play the hand by pressing L (KEY_PLAY_HAND)
    await engine.press('l');

    // Wait for the scoring sequence to complete.
    // After playing, hand_state goes through HAND_PLAY → HAND_PLAYING,
    // then eventually back to HAND_SELECT or HAND_DRAW for the next turn.
    // The score should change, so wait for hand_state to return to HAND_SELECT.
    await waitForHandState(runtime, HandState.HAND_SELECT, 1800);

    // Hands remaining should have decremented by 1
    expect(readVar(runtime, VAR.hands)).toBe(handsBefore - 1);

    // Score should have increased (we played at least a high card)
    expect(readVar(runtime, VAR.score)).toBeGreaterThan(scoreBefore);
  });

  it('discarding a hand produces the expected outcome', async () => {
    const runtime = await createRuntime('hand-discard');
    const engine = runtime.engine;

    await navigateToPlaying(runtime);

    expect(getHandState(runtime)).toBe(HandState.HAND_SELECT);

    const discardsBefore = readVar(runtime, VAR.discards);
    const scoreBefore = readVar(runtime, VAR.score);
    const handsBefore = readVar(runtime, VAR.hands);

    // Select the first card
    await engine.press('a');
    await engine.wait({ frames: 10 });

    // Discard by pressing R (KEY_DISCARD_HAND)
    await engine.press('r');

    // Wait for discard animation to complete and return to HAND_SELECT
    await waitForHandState(runtime, HandState.HAND_SELECT, 1800);

    // Discards should have decremented
    expect(readVar(runtime, VAR.discards)).toBe(discardsBefore - 1);

    // Score should NOT have changed (discarding doesn't score)
    expect(readVar(runtime, VAR.score)).toBe(scoreBefore);

    // Hands remaining should be unchanged
    expect(readVar(runtime, VAR.hands)).toBe(handsBefore);
  });
});
