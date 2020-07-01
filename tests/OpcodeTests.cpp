#include "catch.hpp"
#include "../opcode/Opcode.h"
#include "../OpcodeTable.h"
#include "../types.h"

TEST_CASE("Opcode class tests", "[Opcode]") {
    const OPCODE command = 0x1234;

    SECTION( "getLastNQuadbits" ) {
        REQUIRE( Opcode::getLastNQuadbits(command, 0) == 0x0000 );
        REQUIRE( Opcode::getLastNQuadbits(command, 1) == 0x0004 );
        REQUIRE( Opcode::getLastNQuadbits(command, 2) == 0x0034 );
        REQUIRE( Opcode::getLastNQuadbits(command, 3) == 0x0234 );
        REQUIRE( Opcode::getLastNQuadbits(command, 4) == 0x1234 );
    }

    SECTION( "getXRegister" ) {
        REQUIRE( Opcode::getXRegister(command) == 2 );
    }

    SECTION( "getYRegister" ) {
        REQUIRE( Opcode::getYRegister(command) == 3 );
    }
}

TEST_CASE("Call Machine Code", "[CallMachineCode]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    const OPCODE callMachineCode = 0x0111;
    try {
        table.executeOpcode(callMachineCode);
    } catch (...) {
        std::cout << "Successfully caught the exception." << std::endl;
    }
}

TEST_CASE("Clear screen", "[ClearScreen]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);
    for (int i = 0; i < DISPLAY_LENGTH; i++) {
        sysInfo.display[i] = 1;
    }

    const OPCODE clearDisplay = 0x00E0;
    table.executeOpcode(clearDisplay);

    std::array<unsigned char, DISPLAY_LENGTH> correctDisplayValues;
    correctDisplayValues.fill(0);

    REQUIRE(sysInfo.display == correctDisplayValues );
}

TEST_CASE("Return from subroutine", "[RetSub]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    sysInfo.stack[0] = 0x400;
    sysInfo.cpu.sp++;
    sysInfo.cpu.ip = 0x200;

    const OPCODE returnFromSubroutine = 0x00EE;

    table.executeOpcode(returnFromSubroutine);
    
    REQUIRE( sysInfo.cpu.sp == 0 );
    REQUIRE( sysInfo.cpu.ip == 0x400 );
}

TEST_CASE("Jump to line", "[Jump]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    const OPCODE jump = 0x1345;
    table.executeOpcode(jump);
    REQUIRE( sysInfo.cpu.ip == 0x345 );
}

TEST_CASE("Call Subroutine", "[CallSubroutine]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    sysInfo.cpu.ip = 0x200;

    const OPCODE call = 0x2400;
    table.executeOpcode(call);

    REQUIRE( sysInfo.cpu.ip == 0x400 );
    REQUIRE( sysInfo.cpu.sp == 1 );
    REQUIRE( sysInfo.stack[0] == 0x200 );
}

TEST_CASE("Skip if register equal to constant", "[SkipIfRegisterEqualToConstant]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    sysInfo.cpu.v[0] = 0x12;
    sysInfo.cpu.ip = 0x200;

    SECTION("Equal") {
        const OPCODE command = 0x3012;    
        table.executeOpcode(command);

        REQUIRE( sysInfo.cpu.ip == 0x202 );
    }

    SECTION("Not equal") {
        const OPCODE command = 0x3010;    
        table.executeOpcode(command);

        REQUIRE( sysInfo.cpu.ip == 0x200 );
    }
}

TEST_CASE("Skip if register not equal to constant", "[SkipIfRegisterNotEqualToConstant]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    sysInfo.cpu.v[0] = 0x12;
    sysInfo.cpu.ip = 0x200;

    SECTION("Equal") {
        const OPCODE command = 0x4012;    
        table.executeOpcode(command);
        REQUIRE( sysInfo.cpu.ip == 0x200 );
    }

    SECTION("Not equal") {
        const OPCODE command = 0x4010;    
        table.executeOpcode(command);

        REQUIRE( sysInfo.cpu.ip == 0x202 );
    }
}

TEST_CASE("Skip if register equal to register", "[SkipIfRegisterEqualToRegister]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    sysInfo.cpu.v[0] = 0x12;
    sysInfo.cpu.ip = 0x200;

    const OPCODE command = 0x5010;

    SECTION("Equal") {
        sysInfo.cpu.v[1] = 0x12;
        table.executeOpcode(command);
        REQUIRE( sysInfo.cpu.ip == 0x202 );
    }

    SECTION("Not equal") {
        sysInfo.cpu.v[1] = 0x13;
        table.executeOpcode(command);
        REQUIRE( sysInfo.cpu.ip == 0x200 );
    }
}

TEST_CASE("Set register to a constant", "[SetRegisterToConstant]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);

    // 6XNN
    const OPCODE command = 0x6012;
    table.executeOpcode(command);
    REQUIRE( sysInfo.cpu.v[0] == 0x12 );
}

TEST_CASE("Add constant to register", "[AddRegisterToConstant]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);
    sysInfo.cpu.v[0] = 0x3;

    // 7XNN
    const OPCODE command = 0x7001;
    table.executeOpcode(command);
    REQUIRE( sysInfo.cpu.v[0] == 0x4 );
}

TEST_CASE("Set register value from register", "[setRegisterFromRegister]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);
    sysInfo.cpu.v[1] = 0x12;

    // 7XNN
    const OPCODE command = 0x8010;
    table.executeOpcode(command);
    REQUIRE( sysInfo.cpu.v[0] == 0x12 );
}

TEST_CASE("Or", "[or]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);
    sysInfo.cpu.v[0] = 0x01;
    sysInfo.cpu.v[1] = 0x10;

    // 7XNN
    const OPCODE command = 0x8011;
    table.executeOpcode(command);
    REQUIRE( sysInfo.cpu.v[0] == 0x11 );
}

TEST_CASE("And", "[and]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);
    sysInfo.cpu.v[0] = 0x01;
    sysInfo.cpu.v[1] = 0x10;

    // 7XNN
    const OPCODE command = 0x8012;
    table.executeOpcode(command);
    REQUIRE( sysInfo.cpu.v[0] == 0x0 );
}

TEST_CASE("Xor", "[xor]") {
    SystemInformation sysInfo;
    OpcodeTable table(&sysInfo);
    sysInfo.cpu.v[0] = 0x10;
    sysInfo.cpu.v[1] = 0x11;

    // 7XNN
    const OPCODE command = 0x8013;
    table.executeOpcode(command);
    REQUIRE( sysInfo.cpu.v[0] == 0x01 );
}