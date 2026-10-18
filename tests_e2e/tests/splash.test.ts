import { describe, it, expect } from 'vitest';
import { createRuntime, getGameState, waitForGameState } from '../helpers/runtime.js';
import { GameState } from '../helpers/game_variables.js';

describe('Splash screen', () => {
  it('pressing any key dismisses the splash', async () => {
    const runtime = await createRuntime('splash-key-press');
    const engine = runtime.engine;

    // The game reaches the splash screen about 11 frames after power-on
    await engine.wait({ frames: 30 });
    expect(getGameState(runtime)).toBe(GameState.SPLASH_SCREEN);

    // Press a button
    await engine.press('a');

    // Should transition to main menu
    await waitForGameState(runtime, GameState.MAIN_MENU, 60);
    expect(getGameState(runtime)).toBe(GameState.MAIN_MENU);
  });

  it('auto-dismisses after 10 seconds without input', async () => {
    const runtime = await createRuntime('splash-auto-dismiss');
    const engine = runtime.engine;

    // The game reaches the splash screen about 11 frames after power-on
    await engine.wait({ frames: 30 });
    expect(getGameState(runtime)).toBe(GameState.SPLASH_SCREEN);

    // The splash lasts 600 frames (10s at 60fps) from the frame it starts on.
    // 590 frames after power-on it is still showing.
    await engine.wait({ frames: 560 });
    expect(getGameState(runtime)).toBe(GameState.SPLASH_SCREEN);

    // Now wait for the auto-dismiss transition
    await waitForGameState(runtime, GameState.MAIN_MENU, 60);
    expect(getGameState(runtime)).toBe(GameState.MAIN_MENU);
  });
});
