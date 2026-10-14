# TreeForge Bootstrap Menu — v1.0b1

Boot Manager menu provider for TreeForge Bootstrap on Pixel Tablet (`tangorpro`).

## Build

    ./menuctl build

A successful build verifies the provider and promotes it to `out/development-provider/current`. The newest five successful runs are retained.

## Verify

    ./menuctl verify

## Package

    ./menuctl package

Package archives the exact current development provider into `.release/`. It does not rebuild it.

`VERSION` is authoritative.
