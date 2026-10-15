# TreeForge Bootstrap Menu — v1.0b1

Boot Manager menu provider for TreeForge Bootstrap on Pixel Tablet (`tangorpro`).

## Build

    ./menuctl build

Successful builds publish `out/development-provider/current` and retain the newest five successful runs.

## Verify

    ./menuctl verify

## Release

    ./release prepare

Release packages the exact current development provider into `out/releases/<version>/assets`. It does not rebuild it.

`VERSION` is authoritative.

## License

TreeForge Bootstrap Menu, Boot Manager menu code, host tooling,
profiles, tests, documentation, and provider metadata are licensed
under Apache-2.0. See `LICENSE` and `LICENSES/Apache-2.0.txt`.
