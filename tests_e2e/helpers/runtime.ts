import { HeadlessRuntime } from '@gba-kit/gba-node';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { mkdirSync } from 'node:fs';
import { ELF_PATH, ROM_PATH } from './build.js';
import { GameState, HandState, VAR } from './game_variables.js';

const __dirname = dirname(fileURLToPath(import.meta.url));

/**
 * Create a fresh HeadlessRuntime for a test.
 * Each test gets its own output directory under tests_e2e/output/<testName>.
 */
export async function createRuntime(testName: string): Promise<HeadlessRuntime> {
  const outputDir = join(__dirname, '..', 'output', testName);
  mkdirSync(outputDir, { recursive: true });

  return HeadlessRuntime.create({
    romPath: ROM_PATH,
    elfPath: ELF_PATH,
    outputDir,
    logFn: () => {},
  });
}

/**
 * Read a game variable by its C `symbol` or `symbol.field` path (e.g.
 * `'g_game_vars.score'`)
 */
export function readVar(runtime: HeadlessRuntime, path: string): number {
  return runtime.engine.readVariable(path);
}

/** Get the current game state enum value. */
export function getGameState(runtime: HeadlessRuntime): number {
  return readVar(runtime, VAR.gameState);
}

/** Get the current hand state enum value. */
export function getHandState(runtime: HeadlessRuntime): number {
  return readVar(runtime, VAR.handState);
}

/** Advance until the game state is `state`, or throw after `timeout` frames. */
export async function waitForGameState(runtime: HeadlessRuntime, state: number, timeout: number): Promise<void> {
  await runtime.engine.wait({ memory: { address: VAR.gameState, equals: state }, timeout });
}

/** Advance until the hand state is `state`, or throw after `timeout` frames. */
export async function waitForHandState(runtime: HeadlessRuntime, state: number, timeout: number): Promise<void> {
  await runtime.engine.wait({ memory: { address: VAR.handState, equals: state }, timeout });
}

/**
 * Advance from boot past the disclaimer screen by pressing A.
 * Returns with game_state === MAIN_MENU.
 */
export async function skipDisclaimer(runtime: HeadlessRuntime): Promise<void> {
  const engine = runtime.engine;

  // The game reaches the splash screen about 11 frames after power-on
  await engine.wait({ frames: 30 });
  await engine.press('a');

  await waitForGameState(runtime, GameState.MAIN_MENU, 60);
}

/**
 * From the main menu, start a fresh run.
 * PLAY is selected by default, which opens the RUN_SETUP screen (deck/seed). From
 * there the Play button (row below the deck, first column) starts the run, which
 * runs through GAME_START and lands on BLIND_SELECT.
 * Returns with game_state === BLIND_SELECT.
 */
export async function startGame(runtime: HeadlessRuntime): Promise<void> {
  const engine = runtime.engine;

  // Wait a few frames on main menu for things to settle
  await engine.wait({ frames: 10 });

  // PLAY → RUN_SETUP
  await engine.press('a');
  await waitForGameState(runtime, GameState.RUN_SETUP, 300);

  // RUN_SETUP opens on the "change deck" row; move down to the Play/seed row
  // (Play is the first button there) and confirm to start the run.
  await engine.wait({ frames: 10 });
  await engine.press('down');
  await engine.wait({ frames: 5 });
  await engine.press('a');

  // GAME_START is transient (sets up the deck) and transitions to BLIND_SELECT.
  await waitForGameState(runtime, GameState.BLIND_SELECT, 300);
}

/**
 * From blind select, select (play) the current blind.
 * selection_y starts at 0 = "select blind", so just press A.
 * Waits through the animation substates until GAME_STATE_ROUND.
 */
export async function selectBlind(runtime: HeadlessRuntime): Promise<void> {
  const engine = runtime.engine;

  // Wait for the start animation to finish (substate goes to BLIND_SELECT=1)
  await engine.wait({ frames: 30 });

  // Press UP to ensure we're on "select" (y=0), then A to confirm
  await engine.press('up');
  await engine.wait({ frames: 5 });
  await engine.press('a');

  await waitForGameState(runtime, GameState.ROUND, 600);
}

/**
 * Wait for cards to be dealt and hand_state to become HAND_SELECT.
 */
export async function waitForHandSelect(runtime: HeadlessRuntime): Promise<void> {
  await waitForHandState(runtime, HandState.HAND_SELECT, 600);
}

/**
 * Navigate from boot to the ROUND state with cards dealt.
 * Combines: skip disclaimer → start game → select blind → wait for hand.
 */
export async function navigateToPlaying(runtime: HeadlessRuntime): Promise<void> {
  await skipDisclaimer(runtime);
  await startGame(runtime);
  await selectBlind(runtime);
  await waitForHandSelect(runtime);
}
