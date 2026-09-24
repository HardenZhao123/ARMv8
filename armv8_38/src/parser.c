#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "tokenizer.h"
#include "parser.h"

#define MAX_TOKENS 8

bool parse_line(const char *line, Table *symbols, IntermediateReps *out) {
    // skip the blank lines
    while (isspace(*line)) line++; // inspace expects and int
    // empty given
    if (*line == '\0') return false;
    
    // do tokenize
    char *tokens[MAX_TOKENS];
    size_t ntok = tokenize_line(line, tokens, MAX_TOKENS);
    if (ntok == 0) return false; // nothing tokenized

    // instruction aliases
    // cmp
    if (strcmp(tokens[0], "cmp") == 0 && ntok >= 3) {
        tokens[0] = "subs";
        const bool check_64 = tokens[1][0] == 'x';
        for (size_t i = ntok-1; i >= 1; i--) {
            tokens[i+1] = tokens[i];
        }
        tokens[1] = check_64 ? "xzr" : "wzr";
        ntok++;
    }
    // cmn
    else if (strcmp(tokens[0], "cmn") == 0 && ntok >= 3) {
        tokens[0] = "adds";
        const bool check_64 = tokens[1][0] == 'x';
        for (size_t i = ntok-1; i >= 1; i--) {
            tokens[i+1] = tokens[i];
        }
        tokens[1] = check_64 ? "xzr" : "wzr";
        ntok++;
    }
    // neg
    else if (strcmp(tokens[0], "neg") == 0 && ntok >= 3) {
        tokens[0] = "sub";
        const bool check_64 = tokens[1][0] == 'x';
        for (size_t i = ntok-1; i >= 2; i--) {
            tokens[i+1] = tokens[i];
        }
        tokens[2] = check_64 ? "xzr" : "wzr";
        ntok++;
    }
    // negs
    else if (strcmp(tokens[0], "negs") == 0 && ntok >= 3) {
        tokens[0] = "subs";
        const bool check_64 = tokens[1][0] == 'x';
        for (size_t i = ntok-1; i >= 2; i--) {
            tokens[i+1] = tokens[i];
        }
        tokens[2] = check_64 ? "xzr" : "wzr";
        ntok++;
    }
    // tst
    else if (strcmp(tokens[0], "tst") == 0 && ntok >= 3) {
        tokens[0] = "ands";
        const bool check_64 = tokens[1][0] == 'x';
        for (size_t i = ntok-1; i >= 1; i--) {
            tokens[i+1] = tokens[i];
        }
        tokens[1] = check_64 ? "xzr" : "wzr";
        ntok++;
    }
    // mov
    else if (strcmp(tokens[0], "mov") == 0 && ntok == 3) {
        const bool check_64 = tokens[1][0] == 'x';
        tokens[0] = "orr";
        tokens[3] = tokens[2];
        tokens[2] = check_64 ? "xzr" : "wzr";
        ntok = 4;
    }
    // mvn
    else if (strcmp(tokens[0], "mvn") == 0 && ntok >= 3) {
        tokens[0] = "orn";
        const bool check_64 = tokens[1][0] == 'x';
        for (size_t i = ntok - 1; i >= 2; --i) {
            tokens[i + 1] = tokens[i];
        }
        tokens[2] = check_64 ? "xzr" : "wzr";
        ntok++;
    }
    // mul
    else if (strcmp(tokens[0], "mul") == 0 && ntok == 4) {
        const bool check_64 = tokens[1][0] == 'x';
        tokens[0] = "madd";
        tokens[4] = check_64 ? "xzr" : "wzr";
        ntok = 5;
    }
    // mneg
    else if (strcmp(tokens[0], "mneg") == 0 && ntok == 4) {
        const bool check_64 = tokens[1][0] == 'x';
        tokens[0] = "msub";
        tokens[4] = check_64 ? "xzr" : "wzr";
        ntok = 5;
    }

    // identify the mnemonic
    const char *mn = tokens[0];

    // .int
    if (strcmp(mn, ".int") == 0 && ntok == 2) {
        out->type = special;
        // out->op = ldr;
        out->special_imm = (int32_t)strtol(tokens[1], NULL, 0);
        return true;
    }

    // add, adds, sub, subs
    if ((strcmp(mn, "add") == 0 || 
        strcmp(mn, "adds") == 0 || 
        strcmp(mn, "sub") == 0 ||
        strcmp(mn, "subs") == 0)
        && (ntok == 4 || ntok == 6)) {
        // determine opcode
        if (strcmp(mn, "add")==0) out->op = add;
        else if (strcmp(mn, "adds")==0) out->op = adds;
        else if (strcmp(mn, "sub")==0)  out->op = sub;
        else out->op = subs;
        // immediate form:
        if (tokens[3][0] == '#') {
            out->type = data_processing_imm;

            // determine the opcode
            if (strcmp(mn, "add")==0) out->op = add;
            else if (strcmp(mn, "adds")==0) out->op = adds;
            else if (strcmp(mn, "sub")==0)  out->op = sub;
            else out->op = subs;

            out->arith_imm.Rd = tokens[1][1] == 'z'? 31 : atoi(tokens[1] + 1);
            out->arith_imm.Rn = atoi(tokens[2] + 1);
            // parse imm and optional shift
            const char *imm_str = tokens[3] + 1;
            const int imm_val = (int)strtol(imm_str, NULL, 0);
            out->arith_imm.imm = (uint16_t)imm_val;
            // check the lsl #12
            out->arith_imm.shift_left_12 = false;
            out->arith_imm.has_optional = (ntok == 6);
            if (out->arith_imm.has_optional && strcmp(tokens[4], "lsl") == 0 && strcmp(tokens[5], "#12") == 0) {
                out->arith_imm.shift_left_12 = true;
            }
            // check whether 64-bit
            out->arith_imm.is_64 = tokens[2][0] == 'x';
            return true;
        }
        // register form
        else {
            out->type = data_processing_reg;

            // determine the opcode
            if (strcmp(mn, "add")==0) out->op = addr;
            else if (strcmp(mn, "adds")==0) out->op = addsr;
            else if (strcmp(mn, "sub")==0)  out->op = subr;
            else out->op = subsr;

            out->arith_reg.Rd = tokens[1][1] == 'z'? 31 : atoi(tokens[1] + 1);
            out->arith_reg.Rn = atoi(tokens[2] + 1);
            out->arith_reg.Rm = atoi(tokens[3] + 1);
            // optional shift
            out->arith_reg.has_optional = (ntok >= 6);
            if (out->arith_reg.has_optional) {
                // token[4] = shift, token[5] = #imm
                if (strcmp(tokens[4], "lsl")==0) out->arith_reg.shift_type = lsl;
                else if (strcmp(tokens[4], "lsr")==0) out->arith_reg.shift_type = lsr;
                else if (strcmp(tokens[4], "asr")==0) out->arith_reg.shift_type = asr;
                else if (strcmp(tokens[4], "ror")==0) out->arith_reg.shift_type = ror;
                out->arith_reg.shift_amount = (uint8_t)strtol(tokens[5]+1, NULL, 0);
            } else {
                // set a default
                out->arith_reg.shift_type = lsl;
                out->arith_reg.shift_amount = 0;
            }
            // check whether 64-bit
            out->arith_reg.is_64 = tokens[2][0] == 'x' || tokens[3][0] == 'x';
            return true;
        }
    }

    // and, ands, bic, bics, eor, orr, eon, orn
    if (strcmp(tokens[0], "and") == 0 ||
        strcmp(tokens[0], "ands") == 0 ||
        strcmp(tokens[0], "bic") == 0 ||
        strcmp(tokens[0], "bics") == 0 ||
        strcmp(tokens[0], "eor") == 0 ||
        strcmp(tokens[0], "orr") == 0 ||
        strcmp(tokens[0], "eon") == 0 ||
        strcmp(tokens[0], "orn") == 0) {
        out->type = data_processing_reg;
        // determine opcode
        if (strcmp(tokens[0], "and") == 0) out->op = and_op;
        else if (strcmp(tokens[0], "ands") == 0) out->op = ands;
        else if (strcmp(tokens[0], "bic") == 0) out->op = bic;
        else if (strcmp(tokens[0], "bics") == 0) out->op = bics;
        else if (strcmp(tokens[0], "eor") == 0) out->op = eor;
        else if (strcmp(tokens[0], "orr") == 0) out->op = orr;
        else if (strcmp(tokens[0], "eon") == 0) out->op = eon;
        else out->op = orn;
        out->bitlogic.Rd = tokens[1][1] == 'z' ? 31 : atoi(tokens[1] + 1);
        out->bitlogic.Rn = tokens[2][1] == 'z' ? 31 : atoi(tokens[2] + 1);
        out->bitlogic.Rm = tokens[3][1] == 'z' ? 31 : atoi(tokens[3] + 1);
        out->bitlogic.has_optional = ntok == 6;
        if (out->bitlogic.has_optional) {
            if (strcmp(tokens[4], "lsl") == 0) out->bitlogic.shift_type = lsl;
            else if (strcmp(tokens[4], "lsr") == 0) out->bitlogic.shift_type = lsr;
            else if (strcmp(tokens[4], "asr") == 0) out->bitlogic.shift_type = asr;
            else if (strcmp(tokens[4], "ror") == 0) out->bitlogic.shift_type = ror;
            out->bitlogic.shift_amount = (uint8_t)strtol(tokens[5]+1, NULL, 0);
        } else {
            out->bitlogic.shift_type = lsl;
            out->bitlogic.shift_amount = 0;
        }
        out->bitlogic.is_64 = tokens[1][0] == 'x' || tokens[2][0] == 'x' || tokens[3][0] == 'x';
        return true;
    }

    // movk, movn, movz
    if (strcmp(mn, "movn") == 0 || 
        strcmp(mn, "movz") == 0 || 
        strcmp(mn, "movk") == 0) {
        out->type = data_processing_imm;

        // determine opcode
        if (strcmp(mn, "movn") == 0) out->op = movn;
        else if (strcmp(mn, "movz") == 0) out->op = movz;
        else out->op = movk;

        out->wide_move.Rd = atoi(tokens[1] + 1);
        const int imm = (int)strtol(tokens[2] + 1, NULL, 0);
        out->wide_move.imm = (uint16_t)imm;
        // optional shift amount (hw * 16): only allowed values {0,16,32,48}
        out->wide_move.has_optional = ntok == 5 && strcmp(tokens[3], "lsl") == 0;
        if (out->wide_move.has_optional) {
            const int shift_amount_value = (int)strtol(tokens[4] + 1, NULL, 0);
            out->wide_move.shift_amount = (uint8_t)shift_amount_value;
        } else {
            out->wide_move.shift_amount = 0;
        }
        out->wide_move.is_64 = tokens[1][0] == 'x';
        return true;
    }

    // madd, msub (Rd, Rn, Rm, Ra)
    if (strcmp(tokens[0], "madd") == 0||
        strcmp(tokens[0], "msub") == 0) {
        out->type = data_processing_reg;
        if (strcmp(tokens[0], "madd") == 0) out->op = madd;
        else out->op = msub;
        out->multiply.Rd = atoi(tokens[1] + 1);
        out->multiply.Rn = atoi(tokens[2] + 1);
        out->multiply.Rm = atoi(tokens[3] + 1);
        out->multiply.Ra = tokens[4][1] == 'z' ? 31 : atoi(tokens[4] + 1);
        out->multiply.is_64 = tokens[1][0] == 'x';
        return true;
    }

    // b
    if (strcmp(tokens[0], "b") == 0 && ntok == 2) {
        out->type = branch;
        out->op = b;
        const char *lit = tokens[1];

        if (lit[0] == '#') {
            // immediate‐address
            out->branch_address = (uint64_t)strtol(lit + 1, NULL, 0);
        }
        else {
            // label from symbol table
            out->branch_address = get_entry(symbols, lit);
        }
        return true;
    }

    // b.cond
    if (ntok == 2 && tokens[0][0] == 'b' && tokens[0][1] == '.') {
        out->type = branch;
        out->op = b_cond;
        const char *lit = tokens[1];
        const char *cond_str = tokens[0] + 2;
        static const struct { const char *mnemonic; uint8_t encoding; } cond_map[] = {
            {"eq",0x0}, {"ne",0x1},
            {"ge",0xa}, {"lt",0xb},
            {"gt",0xc}, {"le",0xd},
            {"al",0xe},
        };
        // check the encoding exist
        out->branch_cond.cond = 0xf;
        for (int i = 0; i < sizeof(cond_map) / sizeof(cond_map[0]); i++) {
            if (strcmp(cond_str, cond_map[i].mnemonic) == 0) {
                out->branch_cond.cond = cond_map[i].encoding;
            }
        }
        if (out->branch_cond.cond == 0xf) return false;

        // look for branch address
        if (lit[0] == '#') {
            // immediate‐address
            out->branch_cond.branch_address = (uint64_t)strtol(lit + 1, NULL, 0);
        }
        else {
            // label from symbol table
            out->branch_cond.branch_address = get_entry(symbols, lit);
        }
        return true;
    }

    // br
    if (strcmp(tokens[0], "br") == 0 && ntok == 2) {
        out->type = branch;
        out->op = br;
        out->branch_reg = atoi(tokens[1] + 1);
        return true;
    }

    // str, ldr
    if (strcmp(tokens[0], "ldr") == 0 || strcmp(tokens[0], "str") == 0) {
        out->type = single_data_transfer;
        // check whether load or store
        const bool is_load = tokens[0][0] == 'l';
        out->op = is_load ? ldr : str;

        out->load_store.Rt = atoi(tokens[1] + 1);
        out->load_store.is_64  = tokens[1][0] == 'x';

        // examples:
        // zero unsigned: {"str", "x0", "x1"}
        // unsigned: {"str", "x0", "x1", "#<imm>]"}
        // pre: {"str", "x0", "x1", "#<simm>]!"}
        // post: {"str", "x0", "x1]", "#<simm>"}
        // register: {"str", "x0", "x1", "x2]"}

        // load literal
        if (is_load && tokens[2][0] != 'x') {
            out->load_store.ls_type = ls_literal;
            if (tokens[2][0] == '#') {
                out->load_store.label_address = (uint64_t)strtol(tokens[2] + 1, NULL, 0);
            } else {
                out->load_store.label_address = get_entry(symbols, tokens[2]);
            }
            return true;
        }

        // zero unsigned
        if (ntok == 3 && tokens[2][0] == 'x') {
            out->load_store.ls_type = ls_unsigned;
            out->load_store.Xn = atoi(tokens[2] + 1);
            out->load_store.imm = 0;
            return true;
        }

        // unsigned offset
        if (ntok == 4 && tokens[3][0] == '#' && tokens[3][strlen(tokens[3]) - 1] == ']') {
            out->load_store.ls_type = ls_unsigned;
            out->load_store.Xn = atoi(tokens[2] + 1);
            out->load_store.imm = (uint16_t)strtol(tokens[3] + 1, NULL, 0);
            return true;
        }

        // pre-indexed
        if (ntok == 4 && tokens[3][0] == '#' && tokens[3][strlen(tokens[3]) - 1] == '!') {
            static char buf[64];
            const size_t len = strlen(tokens[3]);
            memcpy(buf, tokens[3] + 1, len - 2);
            buf[len - 2] = '\0';
            out->load_store.ls_type = ls_signed_pre;
            out->load_store.Xn = atoi(tokens[2] + 1);
            out->load_store.simm = (int16_t)strtol(buf, NULL, 0);
            return true;
        }

        // post-index
        if (ntok == 4 && tokens[2][strlen(tokens[2])-1] == ']' && tokens[3][0] == '#') {
            out->load_store.ls_type = ls_signed_post;
            out->load_store.Xn = atoi(tokens[2] + 1);
            out->load_store.simm = (int16_t)strtol(tokens[3] + 1, NULL, 0);
            return true;
        }

        // register offset
        if (ntok == 4 && tokens[3][strlen(tokens[3]) - 1] == ']' && tokens[3][0] == 'x') {
            out->load_store.ls_type = ls_register;
            out->load_store.Xn = atoi(tokens[2] + 1);
            out->load_store.Xm = atoi(tokens[3] + 1);
            return true;
        }

        // This is unrecognized
        return false;
    }

    fprintf(stderr, "Unrecognized instruction: '%s'", line);
    return false;
}
