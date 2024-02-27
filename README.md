Author : Morry4C 

Email: worryanderson@gmail.com

Email: morry4c@163.com

./outputs/exe/make.ps1 = ScrapHDLCompiler.exe  -> log.txt

[Overall]
--------------------------------------------------------------------------------

- This is a 32-bit RSIC-V CPU in the game "Scrap Mechanic"
- 42022 logic gates
- 115197 wires

- Clock period：20 ticks (2Hz)
- RV32E instruction set (39 instructions)
- 5 stage pipeline：
  - (Instruction Fetch, IF)
  - (Instruction Decode, ID)
  - (Execution, EX)
  - (Memory, MEM)
  - (Write Back, WB)
- Data Forwarding
- Dynamic branch prediction
- Exception
- Single step debug mode



[Clock & Reset]
--------------------------------------------------------------------------------

- Clock period: 20 ticks, duty cycle: 15 %
- Asynchronous Reset, Synchronous Release



[Random Generator]
--------------------------------------------------------------------------------

- 8 order M Sequence Generator
- Polynomial: x^8 + x^4 + x^3 + x^2 + 1



[User Interface]
--------------------------------------------------------------------------------

- system input: clk_ena / reset_n / ext_intr
- gamepad input: up/down/left/right/X/Y/esc/ok
- debug input: step/halt/run
- status output：8 LEDs




[Input Sync]
--------------------------------------------------------------------------------

- clock domain cross, asynced input, synced output

  


[Key]
--------------------------------------------------------------------------------

- address range：0xC0000000 ~ 0xC0000003

- 32-bit gamepad status code output

  


[Led]
--------------------------------------------------------------------------------

- address range：0xC0000004

- width：8 bits

- setup time：2 ticks

- hold time：1 tick

- output delay：3 ticks

  


[Screen]
--------------------------------------------------------------------------------

- address range：0x80000000 ~ 0x800001FF
- size：16 * 16
- buffer：16 * 16 bits 
- vsync period：120 cycles（1 minute）



[Instruction ROM]
--------------------------------------------------------------------------------

- address range：0x00000000 ~ 0x000001FF

- size：512 bytes

- up to 128 instructions

  


[Data ROM]
--------------------------------------------------------------------------------

- address range：0x00001000 ~ 0x000010FF

- size：256 bytes

  


[Data RAM]
--------------------------------------------------------------------------------

- address range：0x10000000 ~ 0x100000FF
- size：256 bytes



[Bus Arbiter]
--------------------------------------------------------------------------------

- slave peripherals: Instruction ROM / Data ROM / Data RAM / Screen / Key / Led



[IF]
--------------------------------------------------------------------------------

- jump instruction detection

- program counter included

  


[ID]
--------------------------------------------------------------------------------

- generate control signal

- generate immediate & operand

- undefined instruction detection

  


[EX]
--------------------------------------------------------------------------------

- ALU included (add/sub/shift/compare/and/or/xor)

- output delay: 12 ticks

  


[MEM]
--------------------------------------------------------------------------------

- misalign detection

  


[WB]
--------------------------------------------------------------------------------

- write regfile

  

[Pipeline Register]
--------------------------------------------------------------------------------



[Regfile]
--------------------------------------------------------------------------------

- 15 registers (R1-R15)

- R0 is always 0

  


[Control Unit]
--------------------------------------------------------------------------------

- generate reset and enable signal of pipeline



[Hazard Detection Unit]
--------------------------------------------------------------------------------



[Data Forwarding Unit]
--------------------------------------------------------------------------------




[Branch Prediction Unit]
--------------------------------------------------------------------------------

- dynamic branch prediction with 2-bit branch predictor

- mispredict penalty: 3 cycles

  


[Branch History Table, BHT]
--------------------------------------------------------------------------------

- size：4 bytes, up to 16 history records

  


[Branch Target Buffer]
--------------------------------------------------------------------------------

- size：64 bytes, up to 16 history records



[Exception Interrupt Unit]
--------------------------------------------------------------------------------




[Exception Status Register]
--------------------------------------------------------------------------------

- SEPC: A 32-bit register used to hold the address of the affected instruction.
- SCAUSE: A register used to record the cause of the exception. (undefined/misalign/ecall/ext_intr)
