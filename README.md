# Simple Makefile Maker

A very simple tool that automatically generates a Makefile from `.cpp` and `.hpp` files in your project directory.

## How It Works

This tool scans your current directory for all C++ source files (`.cpp`) and header files (`.hpp`), then generates a Makefile to compile them.

## Important Notes

- **Not recursive**: All `.cpp` and `.hpp` files must exist in the same directory for the tool to function properly
- No support for nested directories or subdirectories

## Usage

1. Add the `maker.out` executable to your project directory
2. Run the executable:
   ```bash
   ./maker.out
   ```
3. When prompted, enter the desired name for your target executable file
4. A `Makefile` will be generated in the current directory

Once the Makefile is created, you can build your project using:
```bash
make
```
