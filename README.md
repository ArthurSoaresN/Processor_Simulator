# Simulador de Conjunto de Instruções (ISS) RISC-V RV32IM

Este programa segue o ciclo clássico de qualquer CPU: 

Busca (Fetch) -> Decodificação (Decode) -> Execução (Execute).

Ele cria a Máquina Virtual, aloca 32KB de memória RAM simulada (malloc), cria os 32 registradores do processador (x[32]), que são as "mãos" do processador para segurar números temporários.
define o PC (Program Counter), que aponta qual linha do código está sendo executada no momento.

Ele suporta a Extensão M do RISC-V, ou seja, ele sabe fazer multiplicações (MUL) e divisões (DIV, REM) nativamente.

Atividade proposta pelo professor [Bruno Otavio Piedade](http://lattes.cnpq.br/1063672636684815) do DCOMP-UFS da matéria de Arquitetura da Computadores.

## Instruções:

- ### beq (Branch If Equal)
    * Formato: B-type
    * Opcode: 0b1100011 (Decimal: 99, Hex: 0x63)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
- ### bne (Branch If Not Equal)
    * Formato: B-type
    * Opcode: 0b1100011 (Decimal: 99, Hex: 0x63)
    * Funct3: 0b001 (Decimal: 1, Hex: 0x1)
- ### blt (Branch If Less Than)
    * Formato: B-type
    * Opcode: 0b1100011 (Decimal: 99, Hex: 0x63)
    * Funct3: 0b100 (Decimal: 4, Hex: 0x4)
- ### bge (Branch If Greater than or Equal)
    * Formato: B-type
    * Opcode: 0b1100011 (Decimal: 99, Hex: 0x63)
    * Funct3: 0b101 (Decimal: 5, Hex: 0x5)
- ### bltu (Branch If Less Than Unsigned)
    * Formato: B-type
    * Opcode: 0b1100011 (Decimal: 99, Hex: 0x63)
    * Funct3: 0b110 (Decimal: 6, Hex: 0x6)
- ### bgeu (Branch If Greater than or Equal Unsigned)
    * Formato: B-type
    * Opcode: 0b1100011 (Decimal: 99, Hex: 0x63)
    * Funct3: 0b111 (Decimal: 7, Hex: 0x7)

###

- ### slti (Set Less Than Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b010 (Decimal: 2, Hex: 0x2)
- ### sltiu (Set Less Than Immediate Unsigned)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b011 (Decimal: 3, Hex: 0x3)
- ### lw (Load Word)
    * O que faz: Carrega uma palavra de 32 bits da memória para um registrador.
    * Formato: I-type
    * Opcode: 0b0000011 (Decimal: 3, Hex: 0x03)
    * Funct3: 0b010 (Decimal: 2, Hex: 0x2)
- ### lh (Load Half)
    * O que faz: Carrega uma meia-palavra de 16 bits da memória para um registrador, com extensão de sinal.
    * Formato: I-type
    * Opcode: 0b0000011 (Decimal: 3, Hex: 0x03)
    * Funct3: 0b001 (Decimal: 1, Hex: 0x1)
- ### lhu (Load Half Unsigned)
    * O que faz: Carrega uma meia-palavra de 16 bits da memória para um registrador, com extensão de zero.
    * Formato: I-type
    * Opcode: 0b0000011 (Decimal: 3, Hex: 0x03)
    * Funct3: 0b101 (Decimal: 5, Hex: 0x5)
- ### lb (Load Byte)
    * O que faz: Carrega um byte de 8 bits da memória para um registrador, com extensão de sinal.
    * Formato: I-type
    * Opcode: 0b0000011 (Decimal: 3, Hex: 0x03)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
- ### lbu (Load Byte Unsigned)
    * O que faz: Carrega um byte de 8 bits da memória para um registrador, com extensão de zero.
    * Formato: I-type
    * Opcode: 0b0000011 (Decimal: 3, Hex: 0x03)
    * Funct3: 0b100 (Decimal: 4, Hex: 0x4)
- ### addi (Add Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
- ### slli (Shift Left Logical Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b001 (Decimal: 1, Hex: 0x1)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### srli (Shift Right Logical Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b101 (Decimal: 5, Hex: 0x5)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### srai (Shift Right Arithmetic Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b101 (Decimal: 5, Hex: 0x5)
    * Funct7: 0b0100000 (Decimal: 32, Hex: 0x20)
- ### andi (AND Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b111 (Decimal: 7, Hex: 0x7)
- ### ori (OR Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b110 (Decimal: 6, Hex: 0x6)
- ### xori (XOR Immediate)
    * Formato: I-type
    * Opcode: 0b0010011 (Decimal: 19, Hex: 0x13)
    * Funct3: 0b100 (Decimal: 4, Hex: 0x4)

###

- ### and (AND)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b111 (Decimal: 7, Hex: 0x7)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### or (OR)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b110 (Decimal: 6, Hex: 0x6)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### xor (XOR)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b100 (Decimal: 4, Hex: 0x4)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### sll (Shift Left Logical)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b001 (Decimal: 1, Hex: 0x1)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### srl (Shift Right Logical)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b105 (Decimal: 5, Hex: 0x5)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### sra (Shift Right Arithmetic)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b101 (Decimal: 5, Hex: 0x5)
    * Funct7: 0b0100000 (Decimal: 32, Hex: 0x20)
- ### add (Add)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### sub (Subtract)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
    * Funct7: 0b0100000 (Decimal: 32, Hex: 0x20)
- ### mul (Multiply)
    * Formato: R-type (Extensão M)
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
    * Funct7: 0b0000001 (Decimal: 1, Hex: 0x01)
- ### div (Divide)
    * Formato: R-type (Extensão M)
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b100 (Decimal: 4, Hex: 0x4)
    * Funct7: 0b0000001 (Decimal: 1, Hex: 0x01)
- ### divu (Divide Unsigned)
    * Formato: R-type (Extensão M)
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b105 (Decimal: 5, Hex: 0x5)
    * Funct7: 0b0000001 (Decimal: 1, Hex: 0x01)
- ### rem (Remainder)
    * Formato: R-type (Extensão M)
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b110 (Decimal: 6, Hex: 0x6)
    * Funct7: 0b0000001 (Decimal: 1, Hex: 0x01)
- ### remu (Remainder Unsigned)
    * Formato: R-type (Extensão M)
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b111 (Decimal: 7, Hex: 0x7)
    * Funct7: 0b0000001 (Decimal: 1, Hex: 0x01)
- ### slt (Set Less Than)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b010 (Decimal: 2, Hex: 0x2)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)
- ### sltu (Set Less Than Unsigned)
    * Formato: R-type
    * Opcode: 0b0110011 (Decimal: 51, Hex: 0x33)
    * Funct3: 0b011 (Decimal: 3, Hex: 0x3)
    * Funct7: 0b0000000 (Decimal: 0, Hex: 0x00)

###

- ### sw (Store Word)
    * O que faz: Armazena uma palavra de 32 bits de um registrador na memória.
    * Formato: S-type
    * Opcode: 0b0100011 (Decimal: 35, Hex: 0x23)
    * Funct3: 0b010 (Decimal: 2, Hex: 0x2)
- ### sh (Store Half)
    * O que faz: Armazena uma meia-palavra de 16 bits de um registrador na memória.
    * Formato: S-type
    * Opcode: 0b0100011 (Decimal: 35, Hex: 0x23)
    * Funct3: 0b001 (Decimal: 1, Hex: 0x1)
- ### sb (Store Byte)
    * O que faz: Armazena um byte de 8 bits de um registrador na memória.
    * Formato: S-type
    * Opcode: 0b0100011 (Decimal: 35, Hex: 0x23)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)

###

## INSTRUÇÕES DE CONTROLE DE FLUXO E EXCEÇÃO:

- ### ecall (Environment Call)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
- ### ebreak (Environment Breakpoint)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
 (imm[0]) é 1. Usada para depuração, gerando uma exceção de breakpoint.
- ### mret (Machine Return from Trap)
    * Formato: R-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b000 (Decimal: 0, Hex: 0x0)
    * Funct7: 0b0011000 (Decimal: 24, Hex: 0x18)

## INSTRUÇÕES DE ACESSO AOS CSRs (REGISTRADOR-REGISTRADOR):

- ### csrrw (CSR Read and Write)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b001 (Decimal: 1, Hex: 0x1)
    * Operação: rd = csr, csr = rs1
- ### csrrs (CSR Read and Set)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b010 (Decimal: 2, Hex: 0x2)
    * Operação: rd = csr, csr = csr | rs1
- ### csrrc (CSR Read and Clear)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b011 (Decimal: 3, Hex: 0x3)
    * Operação: rd = csr, csr = csr & ~rs1

## INSTRUÇÕES DE ACESSO AOS CSRs (REGISTRADOR-IMEDIATO):

- ### csrrwi (CSR Read and Write Immediate)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b101 (Decimal: 5, Hex: 0x5)
    * Operação: rd = csr, csr = zero_extension(imm)
- ### csrrsi (CSR Read and Set Immediate)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b110 (Decimal: 6, Hex: 0x6)
    * Operação: rd = csr, csr = csr | zero_extension(imm)
- ### csrrci (CSR Read and Clear Immediate)
    * Formato: I-type (variante)
    * Opcode: 0b1110011 (Decimal: 115, Hex: 0x73)
    * Funct3: 0b111 (Decimal: 7, Hex: 0x7)
    * Operação: rd = csr, csr = csr & ~zero_extension(imm)