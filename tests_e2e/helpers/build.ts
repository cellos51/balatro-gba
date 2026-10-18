import { statSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const BUILD_DIR = join(dirname(fileURLToPath(import.meta.url)), '..', '..', 'build');

export const ROM_PATH = join(BUILD_DIR, 'balatro-gba.gba');
export const ELF_PATH = join(BUILD_DIR, 'balatro-gba.elf');

for (const path of [ROM_PATH, ELF_PATH]) {
  if (!statSync(path, { throwIfNoEntry: false })) {
    throw new Error(`${path} not found. Build the ROM first, as described in tests_e2e/README.md`);
  }
}
