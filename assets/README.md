# Assets

This directory contains demo recordings and visual assets.

## Generating demo.gif

```bash
# Prerequisites:
pip install asciinema
cargo install --git https://github.com/asciinema/agg

# Record and convert:
./scripts/record_demo.sh
```

The generated `demo.gif` is embedded in the project README.

## Manual recording alternative

If `asciinema` is unavailable, use `terminalizer`:

```bash
npm install -g terminalizer
terminalizer record demo --config scripts/terminalizer.yml
terminalizer render demo -o assets/demo.gif
```

Or simply use a screen recording tool and convert to GIF with:

```bash
ffmpeg -i recording.mp4 -vf "fps=10,scale=800:-1" assets/demo.gif
```
