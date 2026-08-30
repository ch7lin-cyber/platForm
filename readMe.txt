System Platform Core

Reusable platform code shared by MCU products.

Main directories:
- L02_PlatformHAL: stable hardware-capability API and driver registration core.
- L03_SystemPlatform: serial, Modbus, services, algorithms, and execution abstraction.
- L04_FunctionBlock: IEC 61131-3 control, alarm, safety, event, and utility blocks.
- Tests: host-side unit tests.
- docs: architecture and integration boundaries.

MCU vendor drivers and board-specific BSP code intentionally remain in the
product repository. See docs/architecture.md for the integration contract.
