    // Don't even bother trying to compile on a non-Windows environment
    //    since USBI2CIO depends on Windows API.
#if !(defined(__WINDOWS__)||defined(_WIN16)||defined(_WIN32)||defined(_WIN64))
    #error "Not a Windows environment."
#endif

#include <windows.h>    // Dependency for UsbI2cIo

    /**********   Start Macro switches   **********/
//#define RUN_CHECK
//#define TEST_CORE
//#define USE_CPP
//#define NLOG
    /**********   End Macro switches    **********/

    /**********   Start Macro defines   **********/

#define TERMINATION_KEY     16u
#define TERMINATION_KEY2    17u

#define ADD_GLYPH           '+'
#define SUB_GLYPH           '-'
#define MUL_GLYPH           '*'
#define DIV_GLYPH           '/'

    // Choose whether or not to log progress
#ifndef NLOG
        // Visual Studio defaults to C++ projects,
        //  so take a pick at which one to use.
    #ifndef USE_CPP
        #include <stdio.h>

        #define LOG(STR) printf(STR)
        #define LOGNUM(NUM) printf("0x%x",NUM)
        #define LOGCHAR(CH) printf("%c",CH)
        #define LOGFLOAT(FNUM) printf("%d", FNUM)
    #else
        #include <iostream>

        #define LOG(STR) (std::cout << STR)
        #define LOGNUM(NUM) (std::cout << "0x" << std::hex << NUM)
        #define LOGCHAR(CH) (std::cout << CH)
        #define LOGFLOAT(FNUM) (std::cout << FNUM)
    #endif

    #define LOGNUMRETURN(NUM) LOGNUM(NUM); LOG("\n")
    #define LOGCHARRETURN(CH) LOGCHAR(CH); LOG("\n")
    #define LOGRETURN(STR) LOG(STR); LOG("\n")
    #define LOGFLOATRETURN(FNUM) LOGFLOAT(FNUM); LOG("\n")
#else
    #define LOG(STR)
    #define LOGRETURN(STR)
    #define LOGNUM(STR)
    #define LOGNUMRETURN(STR)
    #define LOGCHAR(STR)
    #define LOGCHARRETURN(STR)
    #define LOGFLOAT(FNUM)
    #define LOGFLOATRETURN(FNUM)
#endif

    // Test the core of the program without IO
    //  by replacing all API calls with dummy calls
#ifdef TEST_CORE
    #define DAPI_OpenDeviceInstance(A,B)        \
            NU_PTR;                             \
            LOG("Opening dummy port: ");        \
            LOGRETURN(#A)
    #define DAPI_CloseDeviceInstance(A)         \
            LOG("Closing dummy port: ");        \
            LOGRETURN(#A)
    #define DAPI_ConfigIoPorts(A,B)             \
            LOG("Configuring dummy port: ");    \
            LOGRETURN(#A);                      \
            LOG("\tConfigration: ");            \
            LOGNUMRETURN(B)
    #define DAPI_WriteIoPorts(A,B,C)            \
            LOG("Writing to dummy port: ");     \
            LOGRETURN(#A);                      \
            LOG("\tWrote to: ");                \
            LOGNUMRETURN(C);                    \
            LOG("\tWrote: ");                   \
            LOGNUMRETURN(B)
    #define DAPI_ReadIoPorts(A, B)              \
            LOG("Reading from dummy port: ");   \
            LOGRETURN(#A);                      \
            LOG("\tRead to: ");                 \
            LOGNUMRETURN(B);                    \
            *B = 0x0;
            
#else
            // Contains API for communicating with USBI2CIO board
    #ifdef __cplusplus
        extern "C" {
            #include "UsbI2cIo.h"
        }
    #else
        #include "UsbI2cIo.h"
    #endif
#endif
    /**********   End Macro defines     **********/

    /**********   Start type aliasing   **********/
#ifdef USE_CPP
#include <cstdint>

#define NU_PTR          nullptr
#define UINT_TYPE       std::uint32_t
#define SMALL_UINT_TYPE std::uint8_t
//#define SMALL_UINT_TYPE unsigned short
#else
#include <stdint.h>

#define NU_PTR          NULL
#define UINT_TYPE       uint32_t
#define SMALL_UINT_TYPE uint8_t
//#define SMALL_UINT_TYPE unsigned short
#endif

#ifndef USE_CPP
    #define state_type      SMALL_UINT_TYPE
    #define ent_num_state   0u
    #define ent_op_state    1u
    #define ent_fin_state   2u

    #define input_type      SMALL_UINT_TYPE
    #define dig_input       0u
    #define op_input        1u
    #define ent_input       2u
    #define no_input        3u
    #define clr_input        4u
#else
    enum state_type : SMALL_UINT_TYPE {
        ent_num_state = 0u,
        ent_op_state,
        ent_fin_state,
    };

    enum input_type : SMALL_UINT_TYPE {
        dig_input = 0u,
        op_input,
        ent_input,
        no_input,
        clr_input
    };
#endif

#ifdef __cplusplus
    #define BOOLEAN     bool
    #define TRUE__      true
    #define FALSE__     false
#else
    #define BOOLEAN     SMALL_UINT_TYPE
    #define TRUE__        1
    #define FALSE__       0
#endif

typedef struct {
    float operand[2];
    SMALL_UINT_TYPE index;
    SMALL_UINT_TYPE op_code;
} expression_data;

    /**********   End type aliasing     **********/

    /**********   Start function prototypes   **********/
    // Help make port writing legible.
    //  In terms of USBI2CIO:
    //      - A1 ~ A0 - A3
    //      - A2 ~ A4 - A7
    //      - B1 ~ B0 - B3
    //      - B2 ~ B4 - B7
    //      - C  ~ C4 - C7
UINT_TYPE embed(
    UINT_TYPE A1, UINT_TYPE A2,
    UINT_TYPE B1, UINT_TYPE B2,
    UINT_TYPE C
    );
UINT_TYPE find_lsob(UINT_TYPE);

    // Debugging program
void test_hardware(HANDLE);

    // Main program
void run_calculator(HANDLE);

    // Compute an expression based on its data
float compute(expression_data*);

    // Store digit attempts to push digit to lsd of current number
    //  being used based on available space and the current state
    //  of the calculator.
    //  This function returns whether or not the storing of the
    //  digit was successful.
BOOLEAN store_dig(
    expression_data* exp_ptr, SMALL_UINT_TYPE* mag_ptr,
    state_type* state_ptr, SMALL_UINT_TYPE new_dig,
    SMALL_UINT_TYPE* ntd_ptr
    );
BOOLEAN store_op(
    expression_data* exp_ptr, SMALL_UINT_TYPE* mag_ptr,
    state_type* state_ptr, SMALL_UINT_TYPE new_op
    );

    // Determine type of input and its associated code number.
    //    For digit input, the code is the digit itself.
    //    For operation input:
    //        - Addition          --> ADD_GLYPH
    //        - Subtraction       --> SUB_GLYPH
    //        - Multiplication    --> MUL_GLYPH
    //        - Division          --> DIV_GLYPH
    //    For Enter and Clear inputs, the code is 0.
input_type decode_input_type(SMALL_UINT_TYPE, SMALL_UINT_TYPE*);

    // Display number to the seven segment displays
void display_dig(HANDLE, SMALL_UINT_TYPE dig_to_display, UINT_TYPE C_enable);
void display_num(HANDLE, float num_to_display);
void display_err(HANDLE);

    // Initial state is defined as all seven segment displays turned off
    //   and latches disabled.
void set_initial_state(HANDLE);

    // Wait for keypad input and return the key that was pressed.
    //  Guaranteed to catch a single input.
    //  0 - 15 denotes key on keypad from bottom to top then left to right
    //  TERMINATION_KEY2 denotes combo key to terminate program
    //  TERMINATION_KEY denotes combo key to terminate test program
    //    13 denotes the clear key
    //    5 denotes the enter key
SMALL_UINT_TYPE wait_for_key(HANDLE, UINT_TYPE delay);

BOOLEAN check_null(void*);
    /**********   End function prototypes   **********/

    /**********   Start function definitions   **********/
int main(){
    HANDLE h_instance = INVALID_HANDLE_VALUE;

    // Attempt to connect to device
    h_instance = DAPI_OpenDeviceInstance("UsbI2cIo", 0);

    if (h_instance == INVALID_HANDLE_VALUE){
        LOGRETURN("Unable to open handle to USBI2CIO board.");
        return 0xBAD;
    }

    LOGRETURN("Opened handle to USBI2CIO board.");

    // Set Ports A and C as output. First half of port B
    //  will be output while the other half will be input.
    DAPI_ConfigIoPorts(h_instance, embed(0, 0, 0, 0xF, 0));

#ifdef RUN_CHECK
    test_hardware(h_instance);
#endif

    set_initial_state(h_instance);

    run_calculator(h_instance);

    set_initial_state(h_instance);

    Sleep(100);
    DAPI_CloseDeviceInstance(h_instance);
    LOGRETURN("Program Terminated.");

    return 0;
}

void set_initial_state(HANDLE h_inst){
    // Turn off all SSDs
    LOGRETURN("Turning off all displays.");
    DAPI_WriteIoPorts(h_inst, embed(0xF, 0xF, 0, 0, 0xF), embed(0xF, 0xF, 0, 0, 0xF));
    Sleep(10);
    // Disable latches
    LOGRETURN("Disabling all latches.");
    DAPI_WriteIoPorts(h_inst, embed(0xF, 0xF, 0, 0, 0), embed(0, 0, 0, 0, 0xF));
}

UINT_TYPE embed(
    UINT_TYPE A1, UINT_TYPE A2,
    UINT_TYPE B1, UINT_TYPE B2,
    UINT_TYPE C
){
    return A1 | (A2<<4) | (B1<<8) | (B2<<12) | (C<<16);
}

UINT_TYPE find_lsob(UINT_TYPE target){
    UINT_TYPE toreturn(0u);
    while(target){
        ++toreturn;
        target >>= 1;
    }
    return toreturn;
}

void test_hardware(HANDLE h_instance){
    // Run visual check on seven segment displays
    LOGRETURN("Running visual check...");
    // Turn on all LEDs
    DAPI_WriteIoPorts(h_instance, embed(0, 0, 0, 0, 0xF), embed(0xF, 0xF, 0, 0, 0xF));
    Sleep(4000);

    // Turn off one segment at a time
    UINT_TYPE lit_bit(1);
#define TEST_DELY__ 333
    for (; lit_bit < (1u << 7); lit_bit <<= 1){
        // Remember that only seven of eight pins are
        //  used for display, so there will be a pause.
        DAPI_WriteIoPorts(h_instance, embed(0xF, 0xF, 0, 0, 0), lit_bit);
        Sleep(TEST_DELY__);
    }
    LOGRETURN("Displaying 1 number at a time.");

    // Turn on one number at a time
    lit_bit = 0u;
    for (; lit_bit < 10u; ++lit_bit){
        display_dig(h_instance, lit_bit, 0xF);
        Sleep(TEST_DELY__ * 3);
    }

    // Display "1337"
    LOGRETURN("Displaying \"1337\"");
    display_num(h_instance, 133.7f);
    Sleep(1500);

    // Display "ERRO"
    LOGRETURN("Displaying \"ERRO\"");
    display_num(h_instance, 1337.0f);
    Sleep(1500);

    // Run semi-infinite loop to test keypad input
    LOGRETURN("Reading keypad...");
    SMALL_UINT_TYPE key_code;
    while((key_code = wait_for_key(h_instance, 5)) != TERMINATION_KEY2){
        display_dig(h_instance, key_code, 0x4);
        SMALL_UINT_TYPE sub_code;
        LOG("\tInput type was a");
        switch (decode_input_type(key_code, &sub_code)){
            case dig_input:
                LOG(" digit ( ");
                LOGNUM(sub_code);
                LOG(" )");
                break;
            case op_input:
                LOG("n operation ( ");
                switch (sub_code){
                    case 1: LOG("+"); break;
                    case 2: LOG("-"); break;
                    case 3: LOG("*"); break;
                    case 4: LOG("/"); break;
                    default:    break;
                }
                LOG(" )");
                break;
            case ent_input:
                LOGRETURN("n ENTER");
                break;
            case clr_input:
                LOGRETURN(" clear");
                break;
            default: break;
        }
        LOGRETURN("");
    }
}

void run_calculator(HANDLE h_inst){
    LOGRETURN("Starting calculator program.");
    display_num(h_inst, 0.0f);
    SMALL_UINT_TYPE key_code = TERMINATION_KEY;

        // Let a negative number indicate that the storage is empty
    expression_data exp;
            // Initialize data inside the object
        exp.operand[0] = exp.operand[1] = -1.0f;
        exp.index = exp.op_code = 0u;
    SMALL_UINT_TYPE magnitude = 0u, num_to_display = 0u;
    state_type state = ent_num_state;
    bool cleared_already = false;

    while((key_code = wait_for_key(h_inst, 150)) != TERMINATION_KEY){
        SMALL_UINT_TYPE unknown_code = 0u;
        LOG("\tDecoding input type: "); 
        switch(decode_input_type(key_code, &unknown_code)){
            case clr_input:
                LOGRETURN("Clear");
                if(cleared_already){
                    exp.operand[0u] = 0.0f;
                    exp.operand[1u] = -1.0f;
                    exp.index = magnitude = num_to_display = exp.op_code = 0u;
                    state = ent_fin_state;
                } else if (
                    (exp.operand[0u] > 999.9f || exp.operand[0u] < 0.0f) ||
                    state == ent_fin_state
                ){
                    exp.operand[0u] = 0.0f;
                } else if(state == ent_num_state) {
                    magnitude = 0u;
                    exp.operand[exp.index] = 0.0f;
                }
                cleared_already = true;
                display_num(h_inst, exp.operand[num_to_display]);
                continue;
            case dig_input:
                cleared_already = false;
                LOGRETURN("Digit");
                if (!store_dig(&exp, &magnitude, &state, unknown_code, &num_to_display)){
                    display_err(h_inst);
                    break;
                }
                display_num(h_inst, exp.operand[num_to_display]);
                break;
            case op_input:
                cleared_already = false;
                LOGRETURN("Operation");
                if (!store_op(&exp, &magnitude, &state, unknown_code)){
                    display_err(h_inst);
                }
                break;
            case ent_input:
                cleared_already = false;
                LOGRETURN("ENTER");
                if (
                    exp.operand[0u] < 0.0f ||
                    exp.operand[1u] < 0.0f
                ){
                    display_err(h_inst);
                    break;
                }
                display_num(
                    h_inst,
                    (state != ent_fin_state)*
                    ( -1.0f * (!exp.index) + exp.index * compute(&exp) )
                    );
                 state = ent_fin_state;
                 num_to_display = 0u;
                break;
            case no_input:  break;
            default:
                cleared_already = false;
                display_err(h_inst);
                LOG("Invalid input type (key code : ");
                LOGNUM(key_code);
                LOGRETURN(")");
                break;
        }
        LOG("\tExpression is now: ");
            LOGFLOAT(exp.operand[0u]);
            LOG(" ");
            LOGCHAR(exp.op_code);
            LOG(" ");
            LOGFLOATRETURN(exp.operand[1u]);
    }
}

float compute(expression_data* exp_ptr){
    if(check_null(exp_ptr)){
        return 0.0f;
    }

    switch(exp_ptr->op_code){
        case ADD_GLYPH:
            exp_ptr->operand[0u] += exp_ptr->operand[1u];
            break;
        case SUB_GLYPH:
            exp_ptr->operand[0u] -= exp_ptr->operand[1u];
            break;
        case MUL_GLYPH:
            exp_ptr->operand[0u] *= exp_ptr->operand[1u];
            break;
        case DIV_GLYPH:
            exp_ptr->operand[0u] /= exp_ptr->operand[1u];
            break;
        default:
            LOGRETURN("Invalid operation code.");
            return -1.0f;
    }
    exp_ptr->operand[1u] = -1.0f;
    exp_ptr->index = exp_ptr->op_code = 0u;
    return exp_ptr->operand[0u];
}

BOOLEAN store_dig(
    expression_data* exp_ptr, SMALL_UINT_TYPE* mag_ptr,
    state_type* state_ptr, SMALL_UINT_TYPE new_dig,
    SMALL_UINT_TYPE* ntd_ptr
){
    if(
        check_null(exp_ptr) || check_null(state_ptr) ||
        check_null(mag_ptr) || check_null(ntd_ptr)
    ){
        return FALSE__;
    }

#define MAX_MAGNITUDE 2u

    switch(*state_ptr){
    // Calculation was finished, so reset all values
    //  and store new digit. Also go to the entering
    //  number state.
        case ent_fin_state:
            *mag_ptr = 0u;
            exp_ptr->operand[0u] = -1.0f;
            *state_ptr = ent_num_state;
            *ntd_ptr = 0u;
            exp_ptr->index = 0u;
    // Program is ready to accept a new digit.
        case ent_num_state:
                // There were already MAX_MAGNITUDE digits stored
            if (*mag_ptr == MAX_MAGNITUDE){
                return FALSE__;
            }
            if (!(*mag_ptr || new_dig)){
                exp_ptr->operand[exp_ptr->index] = 0.0f;
                return TRUE__;
            }
                // Push digits left if necessary then insert new digit
            exp_ptr->operand[exp_ptr->index] *= (*mag_ptr > 0u)*10.0f;
            exp_ptr->operand[exp_ptr->index] += new_dig;
                // Update magnitude. If magnitude is at max, update
                //  index as well.
            ++(*mag_ptr);
            if (exp_ptr->index)    *ntd_ptr = exp_ptr->index;
            if(*mag_ptr == MAX_MAGNITUDE){
                *mag_ptr = 0u;
                if(!exp_ptr->index){
                    ++exp_ptr->index;
                }
                *state_ptr = ent_op_state;
            }
            return TRUE__;
    // Program was expecting an operator.
    //  Guarantee that expression is not affected.
        case ent_op_state:
            return FALSE__;
    // Should never happen
        default:
            LOGRETURN("Invalid state");
            return FALSE__;
    }
}

BOOLEAN store_op(
    expression_data* exp_ptr, SMALL_UINT_TYPE* mag_ptr,
    state_type* state_ptr, SMALL_UINT_TYPE new_op
){
    if(check_null(exp_ptr) || check_null(state_ptr) || check_null(mag_ptr)){
        return FALSE__;
    }

    switch (*state_ptr){
        case ent_op_state:
            exp_ptr->op_code = new_op;
            *state_ptr = ent_num_state;
            return TRUE__;
        case ent_num_state:
            if (exp_ptr->index){
                return FALSE__;
            } else if (!(*mag_ptr)) {
                return FALSE__;
            }
            exp_ptr->op_code = new_op;
            exp_ptr->index = 1u;
            *mag_ptr = 0u;
            return TRUE__;
        default:
            return FALSE__;
    }
}

input_type decode_input_type(SMALL_UINT_TYPE key_code, SMALL_UINT_TYPE* i_code){
    if (check_null(i_code)){
        return no_input;
    }

    switch (key_code){
    // Operations
        case 0x0:
            LOGRETURN("Pressed division key");
            *i_code = DIV_GLYPH;
            return op_input;
        case 0x1:
            LOGRETURN("Pressed multiplication key");
            *i_code = MUL_GLYPH;
            return op_input;
        case 0x2:
            LOGRETURN("Pressed subtraction key");
            *i_code = SUB_GLYPH;
            return op_input;
        case 0x3:
            LOGRETURN("Pressed addition key");
            *i_code = ADD_GLYPH;
            return op_input;
    // Enter and clear
        case 0x4:
            *i_code = 0;
            return ent_input;
        case 0xC:
            *i_code = 0;
            return clr_input;
    // Digits
        case 0x8:
            *i_code = 0;
            LOGRETURN("Entered number: 0");
            return dig_input;
        default:
            {
                   SMALL_UINT_TYPE
                       row = 4u-(key_code % 4u),
                       col = 2u-(key_code / 4u)-1u
                       ;
                   *i_code = row * 3u + col;
            }
            LOG("Entered number: ");
            LOGNUMRETURN(*i_code+1u);
            return dig_input;
    }
}

void display_dig(HANDLE h_inst, SMALL_UINT_TYPE num, UINT_TYPE C_enable){
    UINT_TYPE bin_to_display = 0x0;
    switch(num){
                // DCBA  GFE
        case 0: // 0000 0100
            bin_to_display = embed(0, 0x4, 0, 0, C_enable);
            break;
        case 1: // 1001 0111
            bin_to_display = embed(0x9, 0x7, 0, 0, C_enable);
            break;
        case 2: // 0100 0010
            bin_to_display = embed(0x4, 0x2, 0, 0, C_enable);
            break;
        case 3: // 0000 0011
            bin_to_display = embed(0, 0x3, 0, 0, C_enable);
            break;
        case 4: // 1001 0001
            bin_to_display = embed(0x9, 0x1, 0, 0, C_enable);
            break;
        case 5: // 0010 0001
            bin_to_display = embed(0x2, 0x1, 0, 0, C_enable);
            break;
        case 6: // 0010 0000
            bin_to_display = embed(0x2, 0, 0, 0, C_enable);
            break;
        case 7: // 1000 0111
            bin_to_display = embed(0x8, 0x7, 0, 0, C_enable);
            break;
        case 8: // 0000 0000
            bin_to_display = embed(0, 0, 0, 0, C_enable);
            break;
        case 9: // 0000 0001
            bin_to_display = embed(0, 0x1, 0, 0, C_enable);
            break;
        case 10: // 1000 0000
            bin_to_display = embed(0x8, 0, 0, 0, C_enable);
            break;
        case 11: // 0011 0000
            bin_to_display = embed(0x3, 0, 0, 0, C_enable);
            break;
        case 12: // 0110 0100
            bin_to_display = embed(0x6, 0x4, 0, 0, C_enable);
            break;
        case 13: // 0001 0010
            bin_to_display = embed(0x1, 0x2, 0, 0, C_enable);
            break;
        case 14: // 0110 0000
            bin_to_display = embed(0x6, 0, 0, 0, C_enable);
            break;
        case 15: // 1110 0000
            bin_to_display = embed(0xE, 0, 0, 0, C_enable);
            break;
        default:    // Non-hexadecimal digit
            LOGRETURN("\tInvalid digit.");
            break;
    }
    DAPI_WriteIoPorts(h_inst, bin_to_display, embed(0xF, 0xF, 0, 0, 0xF));
    Sleep(5);
        // Remember to disable latches in case
    DAPI_WriteIoPorts(h_inst, 0, embed(0, 0, 0, 0, 0xF));
}

void display_num(HANDLE h_inst, float num){
    // Cannot display negative number or number
    //  greater than or equal to 1000.0
    if((num >= 1000.0f) || (num < 0.0f)){
        LOG("\tInvalid number: ");
        LOGFLOATRETURN(num);
        display_dig(h_inst, 0xE, 0x1);
        display_dig(h_inst, 0xA, 0x2);
        display_dig(h_inst, 0xA, 0x4);
        display_dig(h_inst, 0x0, 0x8);
        return;
    }

    UINT_TYPE trunc = 
#ifdef __cpluplus
        static_cast<UINT_TYPE>
#else
        UINT_TYPE
#endif
        (num*10.0f + 0.5f);
    UINT_TYPE i(0x8);
    for (; i; i >>= 1, trunc /= 10){
        display_dig(h_inst, trunc%10, i);
    }
}

void display_err(HANDLE h_inst){
    display_num(h_inst, -1.0f);
}

SMALL_UINT_TYPE wait_for_key(HANDLE h_inst, UINT_TYPE delay){
    static SMALL_UINT_TYPE col_bit = 1u;
    UINT_TYPE col = 0u;
        // Polling method
    for(;;){
        DAPI_WriteIoPorts(
            h_inst,
            embed(0, 0, col_bit, 0, 0),
            embed(0, 0, 0xF, 0, 0)
            );
        Sleep(5);
        long input;
#ifdef TEST_CORE
        input = 0x0;
#endif
        DAPI_ReadIoPorts(h_inst, &input);
        UINT_TYPE row((input & embed(0, 0, 0, 0xF, 0)) >> 12);
        switch(row){
            case 0xB:   return TERMINATION_KEY;  // Terminate program
            case 0xD:    return TERMINATION_KEY2;    // Terminate test program
            case 0x0:   break;      // No buttons pressed
            default:
                    // Count only the least significant ON bit
                LOG("Input is: ");
                LOGNUMRETURN(input);
                LOG("Pressed key #");
                col = find_lsob(col_bit);
                row = find_lsob(row);
                    LOGNUM((col-1) * 4u + row - 1u);
                    LOG("\t(Column: ");
                    LOGNUM(col);
                    LOG(", Row: ");
                    LOGNUM(row);
                    LOGRETURN(")");
                    Sleep(delay);
                    return (col-1u) * 4u + row - 1u;
        }

            // Cycle through the four output bits to keypad,
            //  or ready the next column for output.
        col_bit <<= 1;
        col_bit = (col_bit%(1u<<4)) | (col_bit==(1u<<4));
    }
}

BOOLEAN check_null(void* ptr){
    return
        ptr == NULL
#if __cplusplus >= 201103L
        || ptr == nullptr
#endif
        ;
}
    /**********   End function definitions      **********/