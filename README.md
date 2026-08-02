# auth-bypass-swd

Bypassing a password check on an STM32F401 "Black Pill" by manipulating CPU registers live over SWD, using OpenOCD and GDB. A hands-on hardware security exercise from **LSEC** (Liga Acadêmica de Segurança Cibernética, Centro de Informática - UFPE).

> This project is for education and authorized security testing only. Only run it against hardware you own or have explicit permission to test.

## What this is

The target firmware (`alvo.ino`) prints a `Login:` prompt over UART and checks the input against a hardcoded password with `strcmp()`. On its own, that check is fine. But the STM32F401's Serial Wire Debug (SWD) pins were left enabled, and SWD talks directly to the CPU, not to the application. That means a debugger can pause execution mid-comparison and simply overwrite the result in the register before the `if` statement ever reads it.

This repo contains the vulnerable target sketch, a UART bridge sketch (for when you don't have a dedicated USB-to-serial adapter on hand), and the commands used to reproduce the bypass end to end: from opening the debug session to dumping and reverse engineering the firmware itself.

The technique follows the approach described in *Practical IoT Hacking* by Fotios Chantzis, Ioannis Stais, Paulino Calderon, Evangelos Deirmentzoglou, and Beau Woods (No Starch Press, 2021), originally demonstrated on an STM32F103. This repo adapts it to the STM32F401 "Black Pill."

A full write-up with explanations is available [here](https://medium.com/@rafaeruucharlotte/debugging-trust-how-a-debug-port-can-bypass-your-firmwares-password-check-cea66e07a078?postPublishedType=initial)

## Hardware

| Item | Role |
|---|---|
| STM32F401 "Black Pill" (ARM Cortex-M4) | Target board |
| ST-Link V2 | SWD debug probe, bridges OpenOCD to the target |
| USB-to-UART adapter (or a spare ESP32 running `bridge.ino`) | UART console access |

### Wiring

**ST-Link V2 to STM32F401 (SWD):**

| ST-Link | STM32F401 |
|---|---|
| SWDIO | SWDIO |
| SWCLK | SWCLK |
| GND | GND |
| 3.3V | 3.3V |

**UART bridge to STM32F401:**

| Bridge | STM32F401 |
|---|---|
| AD0 | PA3 (RX) |
| AD1 | PA2 (TX) |
| GND | GND |

Baud rate: 9600 on both ends.

## Software

- [OpenOCD](https://openocd.org/) (0.12.0 or newer)
- [GDB multiarch](https://www.gnu.org/software/gdb/) (`gdb-multiarch` on Debian/Ubuntu/Kali)
- Arduino IDE with STM32duino board support, to build and flash `alvo.ino`
- `arm-none-eabi-binutils` (for `objcopy` / `objdump`), if you want to dump and disassemble the firmware
- [Ghidra](https://ghidra-sre.org/) and [binwalk](https://github.com/ReFirmLabs/binwalk), optional, for reverse engineering the dumped binary

## Repo structure

```
.
├── alvo/
│   └── alvo.ino          # Vulnerable target firmware
├── bridge/
│   └── bridge.ino        # UART passthrough sketch (ESP32)
└── README.md
```

## Usage

### 1. Flash the target

Open `alvo/alvo.ino` in the Arduino IDE, select your STM32F401 "Black Pill" board, and flash it via the ST-Link.

### 2. Wire everything up

Connect the ST-Link to the SWD pins and your UART adapter (or `bridge/bridge.ino` on an ESP32) as described above.

### 3. Open the UART console

```
sudo dmesg                        # find the device, e.g. /dev/ttyUSB0
sudo minicom -D /dev/ttyUSB0 -b 9600
```

You should see the `Login:` prompt.

### 4. Start OpenOCD

```
sudo openocd -f /usr/share/openocd/scripts/interface/stlink.cfg \
             -f /usr/share/openocd/scripts/target/stm32f4x.cfg
```

This opens a GDB server on port `3333` and a Telnet control channel on port `4444`.

### 5. Attach GDB

```
gdb-multiarch -q --eval-command="target remote localhost:3333" alvo.ino.elf
```

### 6. Break on the password check

```
(gdb) break validate
(gdb) continue
```

Type any wrong password into the UART console. The breakpoint fires.

### 7. Bypass it

`strcmp()` returns its result in register `r0` (ARM calling convention), where zero means the strings matched:

```
(gdb) set $r0 = 0
(gdb) continue
```

The console prints `ACCESS GRANTED`, despite never receiving the correct password.

### 8. (Optional) Dump and reverse the firmware

Through the OpenOCD Telnet session (`telnet localhost 4444`):

```
flash banks
dump_image firmware_completo.bin 0x08000000 0x40000
```

Then inspect it with `binwalk -E`, `strings`, `arm-none-eabi-objdump`, or import it into Ghidra (ARM Cortex little-endian) for a full disassembly.

## Mitigations

For anyone shipping a real product on this or a similar chip:

- **Enable Readout Protection (RDP).** STM32 chips support RDP levels that block tools like OpenOCD from connecting or reading flash without first performing a full erase.
- **Disable or physically remove debug access before shipping.** Blow the relevant fuses, or strip access to the SWD/JTAG pins on the production board.

A debug interface left active cancels out whatever security the firmware thinks it has, regardless of how the application-level password check was written.

## Credits

Rafael and Ivison, LSEC (Liga Acadêmica de Segurança Cibernética) - Centro de Informática, UFPE.