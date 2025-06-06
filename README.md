# Multi-Channel Diagnostic Test Rig

An embedded systems project supporting board-agnostic development for Raspberry Pi Pico W and STM32 Nucleo-F446RE platforms.

## Features

- Multi-platform support (Pico W, STM32 Nucleo-F446RE)
- Hardware abstraction layer (HAL)
- Web-based display simulation for Pico W
- Comprehensive testing framework
- Professional documentation structure

## Quick Start

1. Run setup script: `./scripts/setup/install_dependencies.sh`
2. Configure environment: `./scripts/setup/configure_environment.sh`
3. Build for target platform: `./build.sh pico` or `./build.sh nucleo`
4. Flash firmware: `./flash.sh`

## Project Structure

- `src/` - Core application logic
- `platforms/` - Platform-specific implementations
- `hardware/` - Hardware component drivers
- `web/` - Web interface for simulation
- `tests/` - Unit and integration tests
- `docs/` - Final documentation
- `documents/` - Raw planning and notes

## Development

See `docs/user/installation_guide.md` for detailed setup instructions.

## License

[Add your license here]
