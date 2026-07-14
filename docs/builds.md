# Build Notes

These binaries were built with the official KORG logue SDK template Makefiles and the bundled ARM GCC toolchain.

| Target | Platform | Output |
| --- | --- | --- |
| KORG Nu:Tekt NTS-1 digital kit mkI | `nutekt-digital` | `dist/pendy.ntkdigunit` |
| KORG minilogue xd | `minilogue-xd` | `dist/pendy.mnlgxdunit` |
| KORG prologue | `prologue` | `dist/pendy.prlgunit` |

## Build Evidence

All three builds completed with this size report:

```text
text: 7044
data:   48
bss:    16
dec:  7108
hex:  1bc4
```

Host smoke test:

```text
Pendy host peak q31: 310404032
```

## SHA-256

```text
72959eae0e7ae0689a13a68bcd40f439aa8069be1b64eb051a3dafdf7b7c4528  dist/pendy.ntkdigunit
e66f53096b42b84f06895c8562040bdc68d9aabafcb3eefb813df68df34ad49b  dist/pendy.mnlgxdunit
5dacae4fbb72d2ecfa0b957098435dfa4a1135a2093c721dcdfabb582a98931c  dist/pendy.prlgunit
```

