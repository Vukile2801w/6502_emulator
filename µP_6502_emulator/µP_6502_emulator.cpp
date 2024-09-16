#include <iostream>
#include <cassert>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;
using s32 = signed int;

struct Memory {
	static constexpr u32 MAX_MEM = 1024 * 64;  // 64KB

	Byte DATA[MAX_MEM];  // Full 64KB memory (65536 Bytes)

	// Read Byte
	Byte operator[](u32 Address) const {
		assert(Address < MAX_MEM);  // Ensure the address is within valid range
		return DATA[Address];
	}

	// Write Byte
	Byte& operator[](u32 Address) {
		assert(Address < MAX_MEM);  // Ensure the address is within valid range
		return DATA[Address];
	}

	// Write 2 Byte
	// Decrement 2 Cycles
	void Write_Word(u32& Cycle, Word Value, u32 Addres) {
		DATA[Addres] = Value & 0xFF; 
		DATA[Addres + 1] = (Value >> 8);
		Cycle -= 2;
	}

	// Method to initialize the memory
	void Restart() {
		for (u32 i = 0; i < MAX_MEM; i++) {
			DATA[i] = 0x00;
		}
	}
};

struct CPU {
	Word PC;	// Program counter (16-bit)
	Byte SP;	// Stack pointer (16-bit)
	Byte A;		// Accumulator register (8-bit)
	Byte X;		// Index Register X (8-bit)
	Byte Y;		// Index Register Y (8-bit)

	// Status flags:
	Byte C : 1;	// Carry
	Byte Z : 1;	// Zero
	Byte I : 1;	// Interrupt Disable
	Byte D : 1;	// Decimal Mode
	Byte B : 1;	// Break Command
	Byte V : 1;	// Overflow
	Byte N : 1;	// Negative


	/*
	- Load Register A
	Set of instruction that loading value 
	with specific way into register A
	*/
	struct LDA{

		/*
		- Load A Immidiate
		Load next Value from mem into register A
		opcode A9, take 2 cycles
		Set Z flag if A = 0
		Set N flag if 7th bit of A is set
		*/
		static constexpr Byte IM = 0xA9;
	
		/*
		- Load A Zero Page
		Use next Value form mem as Addres from Zero Page
		and then load that value into register A
		Set Z flag if A = 0
		Set N flag if 7th bit of A is set  
		*/
		static constexpr Byte ZP = 0xA5;
	
		/*
		- Load A Zero Page X
		Use next Value from mem as Addres
		from Zero Page and Value from X register
		and then load Value into A register
		*/
		static constexpr Byte ZP_X = 0xB5;

		/*
		- Load A Absolute
		Use next Word (16-bit) from mem as Address
		and then store Value from that Addres into
		A register. Opcode 0xAD. Take 4 Cycles.
		*/
		static constexpr Byte ABS = 0xAD;

		/*
		- Load A Absolute X
		Use next Word (16-bit) from mem as
		Address	+ Offset it with Value of X register
		and then store Value from that Addres into
		A register. Opcode 0xBD. Take 4 Cycles (+1 if page crossed).
		*/
		static constexpr Byte ABS_X = 0xBD;

		/*
		- Load A Absolute Y
		Use next Word (16-bit) from mem as
		Address	+ Offset it with Value of Y register
		and then store Value from that Addres into
		A register. Opcode 0xB9. Take 4 Cycles (+1 if page crossed).
		*/
		static constexpr Byte ABS_Y = 0xB9;

		/*
		- Load A Indirect X
		Use next Byte (8-bit) from mem as
		Address	in Zero Page (0x00 <-> 0xff) + Offset Value reg X
		Then read ZP[0] =  Addr + X as low byte
		and read ZP[1] Addr + X + 1 as hige byte 
		Word from that ZP[1] << 8 | ZP[0] is Addr
		in memory from where is readed Value,
		witch is Loaded to reg A  
		*/
		static constexpr Byte IND_X = 0xA1;

		/*
		- Load A Indirect Y
		Use next Byte (8-bit) from mem as
		Address	in Zero Page (0x00 <-> 0xff) + Offset Value reg Y
		Then read ZP[0] =  Addr + Y as low byte
		and read ZP[1] Addr + Y + 1 as hige byte
		Word from that ZP[1] << 8 | ZP[0] is Addr
		in memory from where is readed Value,
		witch is Loaded to reg A
		*/
		static constexpr Byte IND_Y = 0xB1;
	};

	/*
	- Jump to Subrutin
	Curent PC has been set to Stack pos
	And then PC has been set to nex Value 
	from mem. Opcode 0x20. Take 6 cycles
	*/
	static constexpr Byte JSR = 0x20;

	/*
	- Return From Subrutin
	Pull Word from Stack pos from mem
	Set Value to PC. Opcode 0x60. Take 6 cycles
	*/
	static constexpr Byte RTS = 0x60;



	// Return next Byte from memory
	// Decrement Cycles
	// Increment Program Counter
	Byte Fetch_Byte(u32& Cycles, Memory& mem) {
		Byte Value = mem[PC];

		Cycles--;
		std::cout << "Fetching Byte at PC=  0x" << std::hex << PC << ":  0x" << std::hex << static_cast<int>(Value) << " Cycles Left:" << Cycles <<std::endl;

		PC++;
		return Value;
	}

	// Return next 2 Byte from memory
	// Increment 2 times Program Counter
	// Decrement 2 times Cycles
	Word Fetch_Word(u32& Cycles, Memory& mem) {
		Word Data = mem[PC];
		PC++;

		Data |= (mem[PC] << 8);
		PC++;

		Cycles -= 2;
	
		return Data;
	}
	
	// Returne Byte from Addres in Memory
	// But dont increment PC
	// Decrement Cycles
	Byte Read_Byte(u32& Cycles, Memory& mem, Word Addres) {

		Byte Value = mem[Addres];

		Cycles--;

		std::cout << "Reading Byte at Addres= 0x" << std::hex << Addres << ": " << std::hex << static_cast<int>(Value)<< " Cycles Left:" << Cycles << std::endl;

		return Value;
	}

	// Return Word from Addres and Addres + 1 in Memory
	// But dont increment PC
	// Decrement Cycles
	Word Read_Word(u32& Cycle, Memory& mem, Word Addres) {
		
		Word low_byte = mem[Addres];
		Word high_byte = mem[Addres + 1];
		Word result = (high_byte << 8) | low_byte;

		Cycle -= 2;

		return result;
	}



	void Check_Flag_LDA() {
		Z = (A == 0);
		N = (A & 0b10000000) != 0;
	}

	/// <summary>
	/// Start executeing instruction from memory starting at PC (program counter)
	/// </summary>
	/// <param name="Cycles">- Number of cycle that going to execut</param>
	/// <param name="mem">- Instance of Memory</param>
	void Execute(u32 Cycles, Memory& mem) {
		while (Cycles > 0)
		{
			// Took next Instruction from mem 
			std::cout << std::endl;
			Byte Ins = Fetch_Byte(Cycles, mem);

			switch (Ins){
			case LDA::IM: {
				Byte Value = Fetch_Byte(Cycles, mem);


				A = Value;
				Z = (A == 0);
				Check_Flag_LDA();

				std::cout << "LDA Immediate, setting A to:  0x" << std::hex << static_cast<int>(A) << std::endl;

			}break;
			
			case LDA::ZP:
			{
				Byte Addres = Fetch_Byte(Cycles, mem);
				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;
				Check_Flag_LDA();
				std::cout << "LDA Zero Page, setting A to:  0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;

			}break;

			case LDA::ZP_X:
			{
				Byte Addres = Fetch_Byte(Cycles, mem);
				
				Addres += X;
				Cycles--;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;
				Check_Flag_LDA();
				std::cout << "LDA Zero Page With offset register X, setting A to:  0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;

			}break;
			case LDA::ABS:
			{
				Word Addres = Fetch_Word(Cycles, mem);
				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;
				Check_Flag_LDA();

				std::cout << "LDA Absolute, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case LDA::ABS_X:
			{
				Word Addres = Fetch_Word(Cycles, mem);

				if (Addres + X < Addres) {
					Cycles++;
				}

				Addres += X;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;

				Check_Flag_LDA();


				std::cout << "LDA Absolute With offset register X, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case LDA::ABS_Y:
			{
				Word Addres = Fetch_Word(Cycles, mem);

				if (Addres + Y < Addres) {
					Cycles++;
				}

				Addres += Y;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;

				Check_Flag_LDA();


				std::cout << "LDA Absolute With offset register Y, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case LDA::IND_X:
			{
				Byte Addres_ZP = Fetch_Byte(Cycles, mem);

				
				Addres_ZP += X;

				Word Addres = Read_Word(Cycles, mem, Addres_ZP);

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;

				Check_Flag_LDA();


				std::cout << "LDA Indirect With offset register X, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case LDA::IND_Y:
			{
				Byte Addres_ZP = Fetch_Byte(Cycles, mem);


				Addres_ZP += X;

				Word Addres = Read_Word(Cycles, mem, Addres_ZP);

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;

				Check_Flag_LDA();


				std::cout << "LDA Indirect With offset register X, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case JSR:
			{
				Word Sub_Addres = Fetch_Word(Cycles, mem);

				mem.Write_Word(Cycles, PC, 0x0100 + SP);
				SP += 2;
				PC = Sub_Addres;
				Cycles--;
				std::cout << "Going to Subroutin to: 0x" << std::hex << static_cast<int>(PC) << std::endl;

			}break;

			case RTS:
			{
				Word PC_Addres = Read_Word(Cycles, mem, 0x0100 + SP - 2);
				SP += 2;
				PC = PC_Addres;
				Cycles -= 3;
				std::cout << "Returnig from Subroutin to: 0x" << std::hex << static_cast<int>(PC) << std::endl;

			}break;


			default:
				std::cout << "Fuck!" << std::endl;
				return;
				
			}
		}
	}

	// Method to initialize the CPU (reset)
	void Restart(Memory& mem) {
		PC = 0xFFFA;    // Setting Program Counter at reset vector (0xFFFA)
		SP = 0x00;      // Setting Stack Pointer to 0xFF

		A = X = Y = 0x00;  // Setting A, X, Y registers to 0

		// Setting Flags
		C = Z = I = D = B = V = N = 0;

		mem.Restart();
	}
};

int main() {
	CPU cpu;
	Memory mem;

	cpu.Restart(mem);  // Initialize CPU with a reset

	Word origin = cpu.PC;
	u32 Cycles = 4;
	// Code For 6502
	
	mem[0xff00] = 0x65;
	mem[origin] = CPU::LDA::ABS;
	mem.Write_Word(Cycles, 0xff00, origin + 1);

	cpu.Y = 0x00AA;
	mem[0x00a9] = 0x56;
	mem[origin + 3] = CPU::LDA::ABS_Y;
	mem.Write_Word(Cycles, 0xffff, origin + 4);

	

	// End of Code

	cpu.Execute(9, mem);
	

	std::cout << "Accumulator (A) Value: 0x" << std::hex << static_cast<int>(cpu.A) << std::endl;

	/*
	std::cout << "Press any key to exit";
	std::cin.ignore(); 
	*/

	return 0;
}