import { readFileSync } from 'node:fs';
import { DebugInfo } from '@gba-kit/debug-info';
import { ELF_PATH } from './build.js';

const debug = DebugInfo.fromElf(new Uint8Array(readFileSync(ELF_PATH)));

// Game variables the tests read and wait on
export const VAR = {
  gameState: 'game_sm.state',
  handState: 's_hand.state',
  round: 'g_game_vars.round',
  currentBlind: 'g_game_vars.current_blind',
  hands: 'g_game_vars.hands',
  discards: 'g_game_vars.discards',
  score: 'g_game_vars.score',
} as const;

/**
 * Read a C enum's constants from the DWARF. Throws if the enum isn't in the ELF.
 *
 * @argument stripPrefix Drops the prefix on the enum member names
 */
function buildEnum(name: string, stripPrefix = ''): Record<string, number> {
  const values = debug.enumValues(name);
  if (!values) {
    throw new Error(`Enum '${name}' not found in DWARF (${ELF_PATH})`);
  }
  return Object.fromEntries(
    Object.entries(values).map(([key, value]) => [
      key.startsWith(stripPrefix) ? key.slice(stripPrefix.length) : key,
      value,
    ]),
  );
}

// Enum constants, read from the build's DWARF
export const GameState = buildEnum('GameState', 'GAME_STATE_');
export const HandState = buildEnum('HandState');
export const BlindType = buildEnum('BlindType', 'BLIND_TYPE_');
export const HandType = buildEnum('HandType');
