#include "CHIP8.h"

#include <fmt/core.h>
#include <fstream>
#include <random>

CHIP8::CHIP8() {
    auto insertIndex = 0;
    for (const auto& e : fontset) {
        memory.at(insertIndex) = e;
        insertIndex++;
    }
}


auto CHIP8::load_ROM(std::string& path) -> int {
    std::ifstream file;
    file.open(path, std::ios::binary);

    if (!file.is_open()) {
        fmt::print("error: Unable to open file.\n");
        return -1;
    }

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});

    for (int i = 0; i < buffer.size(); i++) {
        memory[i + 0x200] = buffer[i];
    }

    file.close();

    return 0;
}

// Increment by 2 since every instruction is 2 bytes
// Whereas we can access our memory 1-byte at a time
// Since we're getting words of u8 instead of u16s
auto CHIP8::increment_pc() -> void {
    program_counter += 2;
}

auto CHIP8::cycle() -> void {
    // Read 1 byte (8 bits) from memory and store in opcode
    opcode = memory[program_counter] << 8 | memory[program_counter + 1];

    auto firstNibble = static_cast<uint8_t>(opcode >> 12);

    switch (firstNibble) {
        case 0x0:
            if (opcode == 0x00E0) {
                std::fill_n(graphics.begin(), graphics.size(), 0);
            } else if (opcode == 0x00EE) {
                sp -= 1;
                program_counter = stack[sp];
            }

            increment_pc();
            break;

        case 0x1:
            program_counter = opcode & 0x0FFF;
            break;

        case 0x2:
            stack[sp] = program_counter; // store programCounter on the stack
            sp += 1; // increment stack pointer
            program_counter = opcode & 0x0FFF; 
            break;

        case 0x3: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            if (registers[x] == kk) {
                increment_pc();
            }

            increment_pc();
            break;
        }

        case 0x4: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            if (registers[x] != kk) {
                increment_pc();
            }

            increment_pc();
            break;
        }
        
        case 0x5: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto y = static_cast<uint8_t>((opcode & 0x00F0) >> 4);

            if (registers[x] == registers[y]) {
                increment_pc();
            }

            increment_pc();
            break;
        }

        case 0x6: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            registers[x] = static_cast<uint8_t>(kk);

            increment_pc();
            break;
        }

        case 0x7: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            registers[x] += static_cast<uint8_t>(kk);

            increment_pc();
            break;
        }

        // ALU instructions
        case 0x8: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto y = static_cast<uint8_t>((opcode & 0x00F0) >> 4);
            auto m = static_cast<uint8_t>(opcode & 0X000F);

            switch (m) {
                case 0x0000:
                    registers[x] = registers[y];
                    break;

                case 0x0001:
                    registers[x] |= registers[y];
                    break;

                case 0x0002:
                    registers[x] &= registers[y];
                    break;

                case 0x0003:
                    registers[x] ^= registers[y];
                    break;

                case 0x0004: {
                    auto result = static_cast<uint16_t>(registers[x] + registers[y]);
                    if (result > 0xFF) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    registers[x] = static_cast<uint8_t>(result & 0x00FF);
                    break;
                }

                case 0x0005: {
                    if (registers[x] > registers[y]) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    registers[x] -= registers[y];
                    break;
                }

                case 0x0006: {
                    if ((registers[x] & 0b1) == 1) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    registers[x] >>= 1;
                    break;
                }

                case 0x0007: {
                    if (registers[y] > registers[x]) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    registers[x] = registers[y] - registers[x];
                    break;
                }

                case 0x000E: {
                    // 0x80 = 0b10000000
                    if ((registers[x] & 0x80) != 0) {
                        registers[0xF] = 1;
                    } else {
                        registers[0xF] = 0;
                    }

                    registers[x] <<= 1;
                    break;
                }
            }

            increment_pc();
            break;
        }

        case 0x9: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto y = static_cast<uint8_t>((opcode & 0x00F0) >> 4);

            if (registers[x] != registers[y]) {
                increment_pc();
            }

            increment_pc();
            break;
        }

        case 0xA: {
            index = opcode & 0x0FFF;
            increment_pc();
            break;
        }

        case 0xB: {
            program_counter = (opcode & 0x0FFF) + static_cast<uint16_t>(registers[0]);
            break;
        }

        case 0xC: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            std::random_device rd; // obtain a random number from hardware
            std::mt19937 gen{rd()}; // seed the generator
            std::uniform_int_distribution<> distribution{0x0, 0xFF}; // define the range

            registers[x] = distribution(gen) & kk;
            increment_pc();
            break;
        }

        case 0xD: {
            auto x = static_cast<uint8_t>(registers[(opcode & 0x0F00) >> 8]);
            auto y = static_cast<uint8_t>(registers[(opcode & 0x00F0) >> 4]);
            auto height = static_cast<uint8_t>(opcode & 0x000F);
            uint8_t pixel{0};

            registers[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[index + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (graphics[(x + xline + ((y + yline) * 64))] == 1) {
                            registers[0xF] = 1;
                        }
                        graphics[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            increment_pc();
            break;
        }

        case 0xE: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            if (kk == 0x9E) {
                if (keys[registers[x]] == 1) {
                    increment_pc();
                }
            } else if (kk == 0xA1) {
                if (keys[registers[x]] != 1) {
                    increment_pc();
                }
            }
            
            increment_pc();
            break;
        }

        // Misc instructions
        case 0xF: {
            auto x = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
            auto kk = static_cast<uint8_t>(opcode & 0x00FF);

            switch (kk) {
                case 0x7: {
                    registers[x] = delay_timer;

                    break;
                }

                case 0xA: {
                    auto keyPressed = false;

                    int i = 0;
                    for (const auto& key : keys) {
                        if (key != 0) {
                            registers[x] = static_cast<uint8_t>(i);
                            keyPressed = true;
                            break;
                        }

                        i++;
                    }

                    // Burn the cycle
                    if (!keyPressed) {
                        return;
                    }

                    break;
                }

                case 0x15: {
                    delay_timer = registers[x];
                    break;
                }

                case 0x18: {
                    sound_timer = registers[x];
                    break;
                }

                case 0x1E: {
                    index += registers[x];
                    break;
                }

                case 0x29: {
                    if (registers[x] < 16) {
                        index = registers[x] * 0x5;
                    }
                    break;
                }

                case 0x33: {
                    memory[index] = registers[x] / 100;
                    memory[index + 1] = (registers[x] / 10) % 10;
                    memory[index + 2] = registers[x] % 10;

                    break;
                }

                case 0x55: {
                    for (int i = 0; i < x; i++) {
                        memory[index + i] = registers[i];
                    }

                    index += x + 1;
                    break;
                }

                case 0x65: {
                    for (int i = 0; i < x; i++) {
                        registers[i] = memory[index + i];
                    }

                    break;
                }
            }
            
            increment_pc();
            break;
        }
    }
}