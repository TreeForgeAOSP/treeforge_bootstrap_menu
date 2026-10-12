# TreeForge Bootstrap Menu — v1.0b1

Boot Manager menu provider for TreeForge Bootstrap on Pixel Tablet (`tangorpro`).

## Verify

```bash
./menuctl verify
```

## Package

```bash
./menuctl package
```

Bootstrap resolves the latest published compatible provider automatically. Unreleased development can be selected explicitly with `--menu-provider PATH`.

## Source

- `VERSION` — authoritative menu and Boot Manager versions
- `profiles/` — menu profiles
- `userspace/` — freestanding menu runtime fragments

Generated release output is not source.
