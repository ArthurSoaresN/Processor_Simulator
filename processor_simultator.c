#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE (32 * 1024) 

void carregar_memoria(const char* filename, uint8_t* mem, uint32_t offset) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo HEX");
        exit(1);
    }
    
    uint32_t current_address = 0;
    unsigned int byte_val;
    
    char line[256];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '@') {
            sscanf(line, "@%x", &current_address);
        } else {
            char *token = strtok(line, " \t\n");

            while (token != NULL) {
                sscanf(token, "%x", &byte_val);
                if ((current_address - offset) < MEMORY_SIZE) {
                    mem[current_address - offset] = (uint8_t)byte_val;
                } else {
                    fprintf(stderr, "Aviso: Tentativa de escrever fora da memória alocada em 0x%08x\n", current_address);
                }
                current_address++;
                token = strtok(NULL, " \t\n");
            }
        }
    }
    fclose(file);
}
    
uint32_t read_word_from_mem(uint8_t* mem, uint32_t address, uint32_t offset) {
    uint32_t value = 0;
    value |= (uint32_t)mem[address - offset + 0];
    value |= (uint32_t)mem[address - offset + 1] << 8;
    value |= (uint32_t)mem[address - offset + 2] << 16;
    value |= (uint32_t)mem[address - offset + 3] << 24;
    return value;
}

void write_word_to_mem(uint8_t* mem, uint32_t address, uint32_t offset, uint32_t data) {
    mem[address - offset + 0] = (uint8_t)(data & 0xFF);
    mem[address - offset + 1] = (uint8_t)((data >> 8) & 0xFF);
    mem[address - offset + 2] = (uint8_t)((data >> 16) & 0xFF);
    mem[address - offset + 3] = (uint8_t)((data >> 24) & 0xFF);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <input.hex> <output.out> [terminal.in] [terminal.out]\n", argv[0]);
        return 1;
    }
    FILE* output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        perror("Erro ao abrir o arquivo de saida (output.out)");
        return 1;
    }
    
    const uint32_t offset = 0x80000000;
    uint32_t x[32] = {0};
    const char* reg[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
    uint32_t pc = offset;
    uint8_t* mem = (uint8_t*)malloc(MEMORY_SIZE);

    if (mem == NULL) {
        perror("Erro ao alocar memória");
        fclose(output_file);
        return 1;
    }
    
    carregar_memoria(argv[1], mem, offset);
    x[2] = offset + MEMORY_SIZE;
    
    uint8_t controle = 1;
    
    while(controle) {
        x[0] = 0;

        if (pc < offset || pc >= (offset + MEMORY_SIZE)) {
             fprintf(stderr, "Erro: PC fora da memória alocada (0x%08x). Encerrando.\n", pc);
             break;
        }

        const uint32_t instruction = read_word_from_mem(mem, pc, offset);
        
        if(instruction == 0 && pc != offset) {
            break;
        }

        uint8_t rd_val = 0, rs1_val = 0, rs2_val = 0, funct3_val = 0, funct7_val = 0;
        int32_t imm_val = 0;
        const uint8_t opcode = instruction & 0x7F;

        switch(opcode) {
            
            case 0b0110111: { // LUI
                rd_val = (instruction >> 7) & 0x1F;
                imm_val = (int32_t)(instruction & 0xFFFFF000);
                x[rd_val] = (uint32_t)imm_val;
                fprintf(output_file, "0x%08x:lui        %s,0x%05x               %s=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val >> 12, reg[rd_val], x[rd_val]);
                break;
            }
            case 0b0010111: { // AUIPC
                rd_val = (instruction >> 7) & 0x1F;
                imm_val = (int32_t)(instruction & 0xFFFFF000);
                x[rd_val] = pc + (uint32_t)imm_val;
                fprintf(output_file, "0x%08x:auipc      %s,0x%05x               %s=0x%08x+0x%08x=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val >> 12, reg[rd_val], pc, imm_val, x[rd_val]);
                break;
            }
            
            case 0b1101111: { // JAL
                rd_val = (instruction >> 7) & 0x1F;
                imm_val = (int32_t)(((instruction & 0x80000000) >> 11) | (instruction & 0xFF000) | ((instruction >> 9) & 0x800) | ((instruction >> 20) & 0x7FE));
                if (imm_val & (1 << 20)) imm_val |= 0xFFE00000;
                
                uint32_t target_address = pc + imm_val;
                uint32_t return_address = pc + 4;

                fprintf(output_file, "0x%08x:jal        %s,0x%05x               pc=0x%08x,%s=0x%08x\n", pc, reg[rd_val], ((uint32_t)imm_val >> 1) & 0xFFFFF, target_address, reg[rd_val], return_address);
                if (rd_val != 0) x[rd_val] = return_address;
                pc = target_address;
                continue;
            }
             case 0b1100111: { // JALR
                rd_val = (instruction >> 7) & 0x1F;
                rs1_val = (instruction >> 15) & 0x1F;
                imm_val = (int32_t)instruction >> 20;
                
                uint32_t i_rs1_pre_jalr = x[rs1_val];
                uint32_t target_address = (i_rs1_pre_jalr + imm_val) & ~1;
                uint32_t return_address = pc + 4;
                
                fprintf(output_file, "0x%08x:jalr       %s,%s,0x%03x            pc=0x%08x+0x%08x,%s=0x%08x\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, i_rs1_pre_jalr, (uint32_t)imm_val, reg[rd_val], return_address);
                if (rd_val != 0) x[rd_val] = return_address;
                pc = target_address;
                continue;
            }

            case 0b1100011: { // Branches (B-Type)
                rs1_val = (instruction >> 15) & 0x1F;
                rs2_val = (instruction >> 20) & 0x1F;
                funct3_val = (instruction >> 12) & 0x07;
                
                imm_val = (int32_t)((((instruction >> 31) & 0x1) << 12) | (((instruction >> 7) & 0x1) << 11) | (((instruction >> 25) & 0x3F) << 5) | (((instruction >> 8) & 0xF) << 1));
                if (imm_val & (1 << 12)) imm_val |= 0xFFFFE000;
                
                uint8_t branch_taken = 0;
                uint32_t r_rs1_pre = x[rs1_val];
                uint32_t r_rs2_pre = x[rs2_val];
                uint32_t target_address = pc + imm_val;
                
                uint32_t imm_display = ((uint32_t)imm_val >> 1) & 0x7FF;

                switch(funct3_val) {
                    case 0b000: // BEQ
                        if (r_rs1_pre == r_rs2_pre) branch_taken = 1;
                        fprintf(output_file, "0x%08x:beq        %s,%s,0x%03x            (0x%08x==0x%08x)=%u->pc=0x%08x\n", pc, reg[rs1_val], reg[rs2_val], imm_display, r_rs1_pre, r_rs2_pre, branch_taken, branch_taken ? target_address : pc + 4);
                        break;
                    case 0b001: // BNE
                        if (r_rs1_pre != r_rs2_pre) branch_taken = 1;
                        fprintf(output_file, "0x%08x:bne        %s,%s,0x%03x            (0x%08x!=0x%08x)=%u->pc=0x%08x\n", pc, reg[rs1_val], reg[rs2_val], imm_display, r_rs1_pre, r_rs2_pre, branch_taken, branch_taken ? target_address : pc + 4);
                        break;
                    case 0b100: // BLT
                        if ((int32_t)r_rs1_pre < (int32_t)r_rs2_pre) branch_taken = 1;
                        fprintf(output_file, "0x%08x:blt        %s,%s,0x%03x            (0x%08x<0x%08x)=%u->pc=0x%08x\n", pc, reg[rs1_val], reg[rs2_val], ((uint32_t)imm_val + 4) & 0xFFF, x[rs1_val], x[rs2_val], branch_taken, branch_taken ? pc + imm_val : pc + 4);
                        break; 
                    case 0b101: // BGE
                        if ((int32_t)r_rs1_pre >= (int32_t)r_rs2_pre) branch_taken = 1;
                        fprintf(output_file, "0x%08x:bge        %s,%s,0x%03x            (0x%08x>=0x%08x)=%u->pc=0x%08x\n", pc, reg[rs1_val], reg[rs2_val], imm_display, r_rs1_pre, r_rs2_pre, branch_taken, branch_taken ? target_address : pc + 4);
                        break;
                    case 0b110: // BLTU
                        if (r_rs1_pre < r_rs2_pre) branch_taken = 1;
                        fprintf(output_file, "0x%08x:bltu       %s,%s,0x%03x            (0x%08x<0x%08x)=%u->pc=0x%08x\n", pc, reg[rs1_val], reg[rs2_val], imm_display, r_rs1_pre, r_rs2_pre, branch_taken, branch_taken ? target_address : pc + 4);
                        break;
                    case 0b111: // BGEU
                        if (r_rs1_pre >= r_rs2_pre) branch_taken = 1;
                        fprintf(output_file, "0x%08x:bgeu       %s,%s,0x%03x            (0x%08x>=0x%08x)=%u->pc=0x%08x\n", pc, reg[rs1_val], reg[rs2_val], imm_display, r_rs1_pre, r_rs2_pre, branch_taken, branch_taken ? target_address : pc + 4);
                        break;
                }
                
                if (branch_taken) {

                    if (funct3_val == 0b110) {
                        target_address = target_address + 4;
                    }

                    pc = target_address;
                    continue; // Pula para o próximo loop com o novo PC

                }

                break;
            }

            case 0b0000011: { // Loads (I-Type)
                rd_val = (instruction >> 7) & 0x1F;
                funct3_val = (instruction >> 12) & 0x07;
                rs1_val = (instruction >> 15) & 0x1F;
                imm_val = (int32_t)instruction >> 20;
                
                uint32_t mem_address = x[rs1_val] + imm_val;

                switch(funct3_val) {
                    case 0b000: { // LB
                        uint8_t byte = mem[mem_address - offset];
                        x[rd_val] = (int32_t)(int8_t)byte;
                        fprintf(output_file, "0x%08x:lb         %s,0x%03x(%s)           %s=mem[0x%08x]=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], reg[rd_val], mem_address, x[rd_val]);
                        break;
                    }
                    case 0b001: { // LH
                        uint16_t half = (uint16_t)mem[mem_address - offset] | ((uint16_t)mem[mem_address - offset + 1] << 8);
                        x[rd_val] = (int32_t)(int16_t)half;
                        fprintf(output_file, "0x%08x:lh         %s,0x%03x(%s)           %s=mem[0x%08x]=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], reg[rd_val], mem_address, x[rd_val]);
                        break;
                    }
                    case 0b010: { // LW
                        x[rd_val] = read_word_from_mem(mem, mem_address, offset);
                        fprintf(output_file, "0x%08x:lw         %s,0x%03x(%s)           %s=mem[0x%08x]=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], reg[rd_val], mem_address, x[rd_val]);
                        break;
                    }
                    case 0b100: { // LBU
                        uint8_t byte = mem[mem_address - offset];
                        x[rd_val] = byte;
                        fprintf(output_file, "0x%08x:lbu        %s,0x%03x(%s)           %s=mem[0x%08x]=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], reg[rd_val], mem_address, x[rd_val]);
                        break;
                    }
                    case 0b101: { // LHU
                        uint16_t half = (uint16_t)mem[mem_address - offset] | ((uint16_t)mem[mem_address - offset + 1] << 8);
                        x[rd_val] = half;
                        fprintf(output_file, "0x%08x:lhu        %s,0x%03x(%s)           %s=mem[0x%08x]=0x%08x\n", pc, reg[rd_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], reg[rd_val], mem_address, x[rd_val]);
                        break;
                    }
                }
                break;
            }

            case 0b0100011: { // Stores (S-Type)
                rs1_val = (instruction >> 15) & 0x1F;
                rs2_val = (instruction >> 20) & 0x1F;
                funct3_val = (instruction >> 12) & 0x07;
                imm_val = (int32_t)((((instruction >> 25) & 0x7F) << 5) | ((instruction >> 7) & 0x1F));
                if (imm_val & (1 << 11)) imm_val |= 0xFFFFF000;

                uint32_t mem_address = x[rs1_val] + imm_val;

                switch(funct3_val) {
                    case 0b000: // SB
                        fprintf(output_file, "0x%08x:sb         %s,0x%03x(%s)           mem[0x%08x]=0x%02x\n", pc, reg[rs2_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], mem_address, x[rs2_val] & 0xFF);
                        mem[mem_address - offset] = (uint8_t)x[rs2_val];
                        break;
                    case 0b001: // SH
                        fprintf(output_file, "0x%08x:sh         %s,0x%03x(%s)           mem[0x%08x]=0x%04x\n", pc, reg[rs2_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], mem_address, x[rs2_val] & 0xFFFF);
                        mem[mem_address - offset] = (uint8_t)(x[rs2_val] & 0xFF);
                        mem[mem_address - offset + 1] = (uint8_t)((x[rs2_val] >> 8) & 0xFF);
                        break;
                    case 0b010: // SW
                        fprintf(output_file, "0x%08x:sw         %s,0x%03x(%s)           mem[0x%08x]=0x%08x\n", pc, reg[rs2_val], (uint32_t)imm_val & 0xFFF, reg[rs1_val], mem_address, x[rs2_val]);
                        write_word_to_mem(mem, mem_address, offset, x[rs2_val]);
                        break;
                }
                break;
            }
            
            case 0b0010011: { // Immediate Arithmetic (I-Type)
                rd_val = (instruction >> 7) & 0x1F;
                funct3_val = (instruction >> 12) & 0x07;
                rs1_val = (instruction >> 15) & 0x1F;
                imm_val = (int32_t)instruction >> 20;
                
                uint32_t i_rs1_pre = x[rs1_val];

                switch(funct3_val) {
                    case 0b000: // ADDI
                        x[rd_val] = i_rs1_pre + imm_val;
                        fprintf(output_file, "0x%08x:addi       %s,%s,0x%03x           %s=0x%08x+0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, reg[rd_val], i_rs1_pre, (uint32_t)imm_val, x[rd_val]);
                        break;
                    case 0b010: // SLTI
                        x[rd_val] = ((int32_t)i_rs1_pre < imm_val) ? 1 : 0;
                        fprintf(output_file, "0x%08x:slti       %s,%s,0x%03x           %s=(0x%08x<0x%08x)=%u\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, reg[rd_val], i_rs1_pre, (uint32_t)imm_val, x[rd_val]);
                        break;
                    case 0b011: // SLTIU
                        x[rd_val] = (i_rs1_pre < (uint32_t)imm_val) ? 1 : 0;
                        fprintf(output_file, "0x%08x:sltiu      %s,%s,0x%03x           %s=(0x%08x<0x%08x)=%u\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, reg[rd_val], i_rs1_pre, (uint32_t)imm_val, x[rd_val]);
                        break;
                    case 0b100: // XORI
                        x[rd_val] = i_rs1_pre ^ imm_val;
                        fprintf(output_file, "0x%08x:xori       %s,%s,0x%03x           %s=0x%08x^0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, reg[rd_val], i_rs1_pre, (uint32_t)imm_val, x[rd_val]);
                        break;
                    case 0b110: // ORI
                        x[rd_val] = i_rs1_pre | imm_val;
                        fprintf(output_file, "0x%08x:ori        %s,%s,0x%03x          %s=0x%08x|0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, reg[rd_val], i_rs1_pre, (uint32_t)imm_val, x[rd_val]);
                        break;
                    case 0b111: // ANDI
                        x[rd_val] = i_rs1_pre & imm_val;
                        fprintf(output_file, "0x%08x:andi       %s,%s,0x%03x         %s=0x%08x&0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], (uint32_t)imm_val & 0xFFF, reg[rd_val], i_rs1_pre, (uint32_t)imm_val, x[rd_val]);
                        break;
                    case 0b001: { // SLLI
                        uint32_t shamt = imm_val & 0x1F;
                        x[rd_val] = i_rs1_pre << shamt;
                        fprintf(output_file, "0x%08x:slli       %s,%s,%u                %s=0x%08x<<%u=0x%08x\n", pc, reg[rd_val], reg[rs1_val], shamt, reg[rd_val], i_rs1_pre, shamt, x[rd_val]);
                        break;
                    }
                    case 0b101: { // SRLI, SRAI
                        uint32_t shamt = imm_val & 0x1F;
                        if ((imm_val >> 5) == 0b0000000) { // SRLI
                            x[rd_val] = i_rs1_pre >> shamt;
                            fprintf(output_file, "0x%08x:srli       %s,%s,%u                %s=0x%08x>>%u=0x%08x\n", pc, reg[rd_val], reg[rs1_val], shamt, reg[rd_val], i_rs1_pre, shamt, x[rd_val]);
                        } else { // SRAI
                            x[rd_val] = (int32_t)i_rs1_pre >> shamt;
                            fprintf(output_file, "0x%08x:srai       %s,%s,%u                %s=0x%08x>>>%u=0x%08x\n", pc, reg[rd_val], reg[rs1_val], shamt, reg[rd_val], i_rs1_pre, shamt, x[rd_val]);
                        }
                        break;
                    }
                }
                break;
            }

            case 0b0110011: { // Register-Register Arithmetic (R-Type)
                rd_val = (instruction >> 7) & 0x1F;
                funct3_val = (instruction >> 12) & 0x07;
                rs1_val = (instruction >> 15) & 0x1F;
                rs2_val = (instruction >> 20) & 0x1F;
                funct7_val = (instruction >> 25) & 0x7F;

                uint32_t r_rs1_pre = x[rs1_val];
                uint32_t r_rs2_pre = x[rs2_val];
                uint32_t shamt = r_rs2_pre & 0x1F;

                if (funct7_val == 0b0000001) { // M-extension
                    switch (funct3_val) {
                        case 0b000: // MUL
                            x[rd_val] = r_rs1_pre * r_rs2_pre;
                            fprintf(output_file, "0x%08x:mul        %s,%s,%s                %s=0x%08x*0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;

                        case 0b001:
                            x[rd_val] = (uint32_t)((((int64_t)(int32_t)r_rs1_pre) * ((int64_t)(int32_t)r_rs2_pre)) >> 32);
                            fprintf(output_file, "0x%08x:mulh       %s,%s,%s                %s=0x%08x*0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                        break;

                        case 0b011: // mulhu 
                            x[rd_val] = (uint32_t)((((uint64_t)r_rs1_pre) * ((uint64_t)r_rs2_pre)) >> 32);
                            fprintf(output_file, "0x%08x:mulhu      %s,%s,%s                %s=0x%08x*0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                        break;

                        case 0b100: // div (Signed Division)
                            if (r_rs2_pre == 0) {
                            x[rd_val] = 0xFFFFFFFF; // Divisão por zero resulta em -1
                            } else if ((int32_t)r_rs1_pre == 0x80000000 && (int32_t)r_rs2_pre == -1) {
                            x[rd_val] = 0x80000000; // Caso de overflow
                            } else {
                            x[rd_val] = (int32_t)r_rs1_pre / (int32_t)r_rs2_pre;
                            }
                            fprintf(output_file, "0x%08x:div        %s,%s,%s               %s=0x%08x/0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;

                        case 0b101: // divu (Unsigned Division)
                            if (r_rs2_pre == 0) {
                                x[rd_val] = 0xFFFFFFFF; // Divisão por zero resulta em 2^32 - 1
                            } else {
                                x[rd_val] = r_rs1_pre / r_rs2_pre;
                            }
                            fprintf(output_file, "0x%08x:divu           %s,%s,%s                %s=0x%08x/0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;

                        case 0b110: // rem (Signed Remainder)
                            if (r_rs2_pre == 0) {
                                x[rd_val] = r_rs1_pre; // Resto da divisão por zero é o próprio dividendo
                            } else if ((int32_t)r_rs1_pre == 0x80000000 && (int32_t)r_rs2_pre == -1) {
                                x[rd_val] = 0; // Caso de overflow do resto
                            } else {
                                x[rd_val] = (int32_t)r_rs1_pre % (int32_t)r_rs2_pre;
                            }
                            fprintf(output_file, "0x%08x:rem            %s,%s,%s                %s=0x%08x%%0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;

                        case 0b111: // remu (Unsigned Remainder)
                            if (r_rs2_pre == 0) {
                                x[rd_val] = r_rs1_pre;
                            } else {
                                x[rd_val] = r_rs1_pre % r_rs2_pre;
                            }
                            fprintf(output_file, "0x%08x:remu           %s,%s,%s                %s=0x%08x%%0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;
                    }
                } else {
                    switch (funct3_val) {

                    
                        case 0b000:
                            if (funct7_val == 0b0000000) { // ADD
                                x[rd_val] = r_rs1_pre + r_rs2_pre;
                                fprintf(output_file, "0x%08x:add        %s,%s,%s            %s=0x%08x+0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            } else { // SUB
                                x[rd_val] = r_rs1_pre - r_rs2_pre;
                                fprintf(output_file, "0x%08x:sub        %s,%s,%s            %s=0x%08x-0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            }
                            break;
                        case 0b001: // SLL
                            x[rd_val] = r_rs1_pre << shamt;
                            fprintf(output_file, "0x%08x:sll        %s,%s,%s                %s=0x%08x<<%u=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, shamt, x[rd_val]);
                            break;
                        case 0b010: // SLT
                            x[rd_val] = (int32_t)r_rs1_pre < (int32_t)r_rs2_pre ? 1 : 0;
                            fprintf(output_file, "0x%08x:slt        %s,%s,%s                %s=(0x%08x<0x%08x)=%u\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;
                        case 0b011: // SLTU
                             x[rd_val] = (r_rs1_pre < r_rs2_pre) ? 1 : 0;
                             fprintf(output_file, "0x%08x:sltu      %s,%s,%s                %s=(0x%08x<0x%08x)=%u\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;
                        case 0b100: // XOR
                            x[rd_val] = r_rs1_pre ^ r_rs2_pre;
                            fprintf(output_file, "0x%08x:xor        %s,%s,%s                %s=0x%08x^0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;
                        case 0b101:
                            if (funct7_val == 0b0000000) { // SRL
                                x[rd_val] = r_rs1_pre >> shamt;
                                fprintf(output_file, "0x%08x:srl        %s,%s,%s                %s=0x%08x>>%u=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, shamt, x[rd_val]);
                            } else { // SRA
                                x[rd_val] = (int32_t)r_rs1_pre >> shamt;
                                fprintf(output_file, "0x%08x:sra        %s,%s,%s                %s=0x%08x>>>%u=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, shamt, x[rd_val]);
                            }
                            break;
                        case 0b110: // OR
                            x[rd_val] = r_rs1_pre | r_rs2_pre;
                            fprintf(output_file, "0x%08x:or         %s,%s,%s                %s=0x%08x|0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;
                        case 0b111: // AND
                            x[rd_val] = r_rs1_pre & r_rs2_pre;
                            fprintf(output_file, "0x%08x:and        %s,%s,%s                %s=0x%08x&0x%08x=0x%08x\n", pc, reg[rd_val], reg[rs1_val], reg[rs2_val], reg[rd_val], r_rs1_pre, r_rs2_pre, x[rd_val]);
                            break;
                    }
                }
                break;
            }
            
            case 0b1110011: { // SYSTEM
                if (instruction == 0x00100073) { // EBREAK
                    fprintf(output_file, "0x%08x:ebreak\n", pc);
                    controle = 0;
                }
                break;
            }

            default:
                fprintf(output_file, "Erro: Instrução desconhecida [0x%08x] em pc=0x%08x\n", instruction, pc);
                controle = 0;
                break;
        }
    
        
        pc += 4;
    
    }

    fclose(output_file);
    free(mem);
    return 0;
}
