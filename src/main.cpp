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
			DATA[i] = 0xEA;
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
		Cycles 4. Opcode 0xB5
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
	- Load Register X
	Set of instruction that loading value
	with specific way into register X
	*/
	struct LDX {
		/*
		- Load X Immediate
		Load next Value from mem into register X
		opcode 0xA2, take 2 cycles
		Set Z flag if X = 0
		Set N flag if 7th bit of X is set
		*/
		static constexpr Byte IM = 0xA2;
		/*
		- Load X Zero Page
		Use next Value form mem as Addres from Zero Page
		and then load that value into register X
		Set Z flag if X = 0
		Set N flag if 7th bit of X is set  
		*/
		static constexpr Byte ZP = 0xA6;
		/*
		-Load X Zero Page Y
		Use next Value from mem as Addres
		from Zero Page and Value from Y register
		and then load Value into X register
		Cycles 4. Opcode 0xB6
		*/		
		static constexpr Byte ZP_Y = 0xB6;
		
		/*
		- Load X Absolute
		Use next Word (16-bit) from mem as Address
		and then store Value from that Addres into
		X register. Opcode 0xAD. Take 4 Cycles.
		*/
		static constexpr Byte ABS = 0xAE;

		/*
		- Load X Absolute Y
		Use next Word (16-bit) from mem as
		Address	+ Offset it with Value of Y register
		and then store Value from that Addres into
		X register. Opcode 0xB9. Take 4 Cycles (+1 if page crossed).
		*/
		static constexpr Byte ABS_Y = 0xBE;

	};

	/*
	- Load Register Y
	Set of instruction that loading value
	with specific way into register Y
	*/
	struct LDY {
		/*
		- Load Y Immediate
		Load next Value from mem into register Y
		opcode 0xA2, take 2 cycles
		Set Z flag if Y = 0
		Set N flag if 7th bit of Y is set
		*/
		static constexpr Byte IM = 0xA0;
		/*
		- Load Y Zero Page
		Use next Value form mem as Addres from Zero Page
		and then load that value into register Y
		Set Z flag if Y = 0
		Set N flag if 7th bit of Y is set
		*/
		static constexpr Byte ZP = 0xA4;
		/*
		-Load Y Zero Page X
		Use next Value from mem as Addres
		from Zero Page and Value from X register
		and then load Value into Y register
		Cycles 4. Opcode 0xB4
		*/
		static constexpr Byte ZP_X = 0xB4;

		/*
		- Load Y Absolute
		Use next Word (16-bit) from mem as Address
		and then store Value from that Addres into
		Y register. Opcode 0xAC. Take 4 Cycles.
		*/
		static constexpr Byte ABS = 0xAC;

		/*
		- Load Y Absolute X
		Use next Word (16-bit) from mem as
		Address	+ Offset it with Value of X register
		and then store Value from that Addres into
		X register. Opcode 0xBC. Take 4 Cycles (+1 if page crossed).
		*/
		static constexpr Byte ABS_X = 0xBC;

	};

	/*
	Arithmetical Operation
	*/
	struct ART_OP
	{
		static constexpr Byte INY = 0xC8; // Increment Reg Y
		static constexpr Byte INX = 0xE8; // Increment Reg X
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

	/*
	Jump Absolut
	Next Word form mem is addres where is
	PC is going to be set
	*/
	static constexpr Byte JMP_ABS = 0x4C;

	static constexpr Byte EXIT = 0xFF;

	/*
	Jump Indirect
		*/
	static constexpr Byte JMP_IND = 0x6C;

	/*
	No Operation
	Make no changes to CPU,
	Increment PC and Decrement Cycles regulary
	Opcode 0xEA, Take 2 Cycles
	*/
	static constexpr Byte NOP = 0xEA;

	// Return next Byte from memory
	// Decrement Cycles
	// Increment Program Counter
	Byte Fetch_Byte(u32& Cycles, Memory& mem) {
		Byte Value = mem[PC];

		Cycles--;
		//std::cout << "Fetching Byte at PC=  0x" << std::hex << PC << ":  0x" << std::hex << static_cast<int>(Value) << " Cycles Left:" << Cycles <<std::endl;

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

		//std::cout << "Reading Byte at Addres= 0x" << std::hex << Addres << ": " << std::hex << static_cast<int>(Value)<< " Cycles Left:" << Cycles << std::endl;

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



	void Check_Flag(Byte Register) {
		Z = (Register == 0);
		N = (Register & 0b10000000) != 0;
	}

	/// <summary>
	/// Start executeing instruction from memory starting at PC (program counter)
	/// </summary>
	/// <param name="Cycles">- Number of cycle that going to execut</param>
	/// <param name="mem">- Instance of Memory</param>
	void Execute(u32 Cycles, Memory& mem) {
		while (Cycles > 0 and Cycles + 1 > 0)
		{
			// Took next Instruction from mem 
			//std::cout << std::endl;
			Byte Ins = Fetch_Byte(Cycles, mem);

			switch (Ins) {

				// ================================================================== //
				// =========================       LDA       ======================== //
				// ================================================================== //

			case LDA::IM: {
				Byte Value = Fetch_Byte(Cycles, mem);


				A = Value;
				Check_Flag(A);

				//std::cout << "LDA Immediate, setting A to:  0x" << std::hex << static_cast<int>(A) << std::endl;

			}break;

			case LDA::ZP:
			{
				Byte Addres = Fetch_Byte(Cycles, mem);
				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;
				Check_Flag(A);
				//std::cout << "LDA Zero Page, setting A to:  0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;

			}break;

			case LDA::ZP_X:
			{
				Byte Addres = Fetch_Byte(Cycles, mem);

				Addres += X;
				Cycles--;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;
				Check_Flag(A);
				std::cout << "LDA Zero Page With offset register X, setting A to:  0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;

			}break;

			case LDA::ABS:
			{
				Word Addres = Fetch_Word(Cycles, mem);
				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;
				Check_Flag(A);

				//std::cout << "LDA Absolute, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
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

				Check_Flag(A);


				//std::cout << "LDA Absolute With offset register X, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
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

				Check_Flag(A);


				//std::cout << "LDA Absolute With offset register Y, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case LDA::IND_X:
			{
				Byte Addres_ZP = Fetch_Byte(Cycles, mem);


				Addres_ZP += X;

				Word Addres = Read_Word(Cycles, mem, Addres_ZP);

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;

				Check_Flag(A);


				//std::cout << "LDA Indirect With offset register X, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			case LDA::IND_Y:
			{
				Byte Addres_ZP = Fetch_Byte(Cycles, mem);


				Addres_ZP += X;

				Word Addres = Read_Word(Cycles, mem, Addres_ZP);

				Byte Value = Read_Byte(Cycles, mem, Addres);

				A = Value;

				Check_Flag(A);


				//std::cout << "LDA Indirect With offset register X, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			// ================================================================== //
			// =========================       LDX       ======================== //
			// ================================================================== //

			case LDX::IM: {
				Byte Value = Fetch_Byte(Cycles, mem);


				X = Value;
				Check_Flag(X);

			}break;

			case LDX::ZP: {
				Byte Value = Fetch_Byte(Cycles, mem);


				X = Value;
				Check_Flag(X);

			}break;

			case LDX::ZP_Y: {
				Byte Addres = Fetch_Byte(Cycles, mem);

				Addres += Y;
				Cycles--;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				X = Value;
				Check_Flag(X);

			}break;

			case LDX::ABS: {
				Word Addres = Fetch_Word(Cycles, mem);
				Byte Value = Read_Byte(Cycles, mem, Addres);

				X = Value;
				Check_Flag(X);


			}break;

			case LDX::ABS_Y:
			{
				Word Addres = Fetch_Word(Cycles, mem);

				if (Addres + Y < Addres) {
					Cycles++;
				}

				Addres += Y;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				X = Value;

				Check_Flag(X);


				//std::cout << "LDA Absolute With offset register Y, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			// ================================================================== //
			// =========================       LDY       ======================== //
			// ================================================================== //

			case LDY::IM: {
				Byte Value = Fetch_Byte(Cycles, mem);


				Y = Value;
				Check_Flag(Y);

			}break;

			case LDY::ZP: {
				Byte Value = Fetch_Byte(Cycles, mem);


				Y = Value;
				Check_Flag(Y);

			}break;

			case LDY::ZP_X: {
				Byte Addres = Fetch_Byte(Cycles, mem);

				Addres += X;
				Cycles--;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				Y = Value;
				Check_Flag(Y);

			}break;

			case LDY::ABS: {
				Word Addres = Fetch_Word(Cycles, mem);
				Byte Value = Read_Byte(Cycles, mem, Addres);

				Y = Value;
				Check_Flag(Y);


			}break;

			case LDY::ABS_X:
			{
				Word Addres = Fetch_Word(Cycles, mem);

				if (Addres + X < Addres) { Cycles++; }

				Addres += X;

				Byte Value = Read_Byte(Cycles, mem, Addres);

				Y = Value;

				Check_Flag(Y);


				//std::cout << "LDA Absolute With offset register Y, setting A to: 0x" << std::hex << static_cast<int>(A) << " From: 0x" << std::hex << static_cast<int>(Addres) << std::endl;
			}break;

			// ================================================================== //
			// ========       arithmetic operation with Register        ========= //
			// ================================================================== //

			case ART_OP::INY:
			{
				Y++;
				Cycles--;
				Check_Flag(Y);
			}break;

			case ART_OP::INX:
			{
				X++;
				Cycles--;
				Check_Flag(Y);
			}break;

			// ================================================================== //
			// ================================================================== //

			case JSR:
			{
				Word Sub_Addres = Fetch_Word(Cycles, mem);

				mem.Write_Word(Cycles, PC, 0x0100 + SP);
				SP += 2;
				PC = Sub_Addres;
				Cycles--;
				//std::cout << "Going to Subroutin to: 0x" << std::hex << static_cast<int>(PC) << std::endl;

			}break;

			case RTS:
			{
				Word PC_Addres = Read_Word(Cycles, mem, 0x0100 + SP - 2);
				SP += 2;
				PC = PC_Addres;
				Cycles -= 3;
				//std::cout << "Returnig from Subroutin to: 0x" << std::hex << static_cast<int>(PC) << std::endl;

			}break;

			case JMP_ABS:
			{
				Word Addres = Fetch_Word(Cycles, mem);
				PC = Addres;
			}break;

			case JMP_IND:
			{
				Word Adders = Fetch_Word(Cycles, mem);
				Word Addres_for_Jump = Read_Word(Cycles, mem, Adders);
				PC = Addres_for_Jump;
			}break;

			case NOP:
			{
				PC++;
				Cycles -= 2;
			}break;

			case EXIT: {
				Cycles = 0;
			}break;

			default: {
				printf("Operation not found ");
				std::cout << "Ins:  0x" << std::hex << static_cast<int>(Ins);
				std::cout << std::endl;

			}break;
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




void Code(Memory& mem, CPU& cpu) 
{
	Word origin = cpu.PC;
	Word origin_2 = 0x1000;

	mem[origin] = CPU::JMP_ABS;
	mem[origin + 1] = 0x00;
	mem[origin + 2] = 0x10;

	mem[origin_2] = CPU::LDA::ZP_X;
	mem[origin_2 + 1] = 0x00;
	mem[origin_2 + 2] = 0x00;
	mem[origin_2 + 3] = CPU::ART_OP::INX;
	mem[origin_2 + 4] = CPU::JMP_ABS;
	mem[origin_2 + 5] = 0x00;
	mem[origin_2 + 6] = 0x10;
}









int main() {
	CPU cpu;
	Memory mem;

	cpu.Restart(mem);  // Initialize CPU with a reset

	u32 Cycles = 4;
	// Code For 6502
	
	Code(mem, cpu);

	// End of Code

	cpu.Execute(1024 * 64, mem);
	

	std::cout << "Accumulator (A) Value: 0x" << std::hex << static_cast<int>(cpu.A) << std::endl;

	/*
	std::cout << "Press any key to exit";
	std::cin.ignore(); 
	*/

	return 0;
}


