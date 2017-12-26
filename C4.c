#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


char *src, *old_src;
int poolsize;
int line;

int *text;
int *old_text;
int *stack;
char *data;

//eval
int *pc, *sp, *bp, ax, cycle;

enum {
    IMM, LI, LC, SI, SC, PUSH, JMP, JZ, JNZ,
    CALL, ENT, LEV, ADJ, LEA,
    XOR, OR, AND, EQ, NE, LT, LE, GT, GE,
    SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

enum {//identifier.token
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,//void, main
    Assign, Cond, Lor, Lan, Xor, Or, And, Eq, Ne, Lt, Le, Gt, Ge,
    Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

struct identifier {
    int token;
    int hash;
    char *name;
    int type;//char int ptr
    int class;
    int value;
    int Btype;
    int Bclass;
    int Bvalue;
};
int token;
int token_val;
int *current_id;
int *symbols;
enum {
    Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize
};
enum {
    CHAR, INT, PTR
};
int *idmain;

void next() {
    char *last_pos;
    int hash;
    while (token = *src) {
        src++;

        if (token == '\n') {
            line++;
        } else if (token == '#') {
            while (*src != 0 && *src != '\n') {
                src++;
            }
        } else if ((token >= 'a' && token <= 'z') ||
                   (token >= 'A' && token <= 'Z') ||
                   (token = '_')) {
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') ||
                   (*src >= 'A' && *src <= 'Z') ||
                   (*src >= '0' && *src <= '9') ||
                   (*src = '_')) {
                hash = hash * 146 + *src++;
            }//last_pos-src is token
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp(last_pos, (char *) current_id[Name], src - last_pos)) {//去除碰撞
                    token = current_id[Token];
                    return;
                }
                current_id += IdSize;
            }
            current_id[Hash] = hash;
            current_id[Name] = (int) last_pos;
            token = current_id[Token] = Id;
            return;
        } else if (token >= '0' && token <= '9') {//dec(12) hex(0x123) oct(012)
            token_val = token - '0';
            if (token_val > 0) {//dec
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val * 10 + *src++ - '0';
                }
            } else {
                if (*src == 'x' || *src == 'X') {//hex
                    token = *++src;
                    while ((token >= 'a' && token <= 'f') ||
                           (token >= 'A' && token <= 'F') ||
                           (token >= '0' && token <= '9')) {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } else {//oct
                    while (*src >= '0' && *src <= '8') {
                        token_val = token_val * 8 + *src++ - '0';
                    }
                }
            }
            token = Num;
            return;
        } else if (token == '"' || token == '\'') {

        } else if (token == '/') {
            if (*src == '/') {
                while (*src != 0 && *src != '\n') {
                    src++;
                }
            } else {
                token = Div;
                return;
            }
        } else if (token == '~' ||
                   token == ';' ||
                   token == '{' ||
                   token == '}' ||
                   token == '(' ||
                   token == ')' ||
                   token == ']' ||
                   token == ',' ||
                   token == ':') {
            // directly return the character as token;
            return;
        }
    }
}

void expression(int level) {

}
void global_declaration(){

}
void program() {
    next();
    while (token > 0) {
        global_declaration();
    }
}

int eval() {
    int op, *tmp;
    while (1) {
        op = *pc++;

        if (op == IMM) {
            ax = *pc++;
        } else if (op == LI) {
            ax = *(int *) ax;
        } else if (op == LC) {
            ax = *(char *) ax;
        } else if (op == SI) {
            *(int *) *sp++ = ax;
        } else if (op == SC) {
            *(char *) *sp++ = (char) ax;
        } else if (op == PUSH) {
            *--sp = ax;
        } else if (op == JMP) {
            pc = (int *) *pc;
        } else if (op == JZ) {
            pc = ax ? pc + 1 : (int *) *pc;
        } else if (op == JNZ) {
            pc = ax ? (int *) *pc : pc + 1;
        } else if (op == CALL) {
            *--sp = (int) (pc + 1);
            pc = (int *) *pc;
        } else if (op == ENT) {
            *--sp = (int) bp;
            bp = sp;
            sp -= *pc++;
        } else if (op == LEV) {
            sp = bp;
            bp = (int *) *sp++;
            pc = (int *) *sp++;
        } else if (op == ADJ) {
            sp += *pc++;
        } else if (op == LEA) {
            ax = (int) (bp + *pc++);
        } else if (op == XOR) {
            ax = *sp++ ^ ax;
        } else if (op == OR) {
            ax = *sp++ | ax;
        } else if (op == AND) {
            ax = *sp++ & ax;
        } else if (op == EQ) {
            ax = *sp++ == ax;
        } else if (op == NE) {
            ax = *sp++ != ax;
        } else if (op == LT) {
            ax = *sp++ < ax;
        } else if (op == LE) {
            ax = *sp++ <= ax;
        } else if (op == GT) {
            ax = *sp++ > ax;
        } else if (op == GE) {
            ax = *sp++ >= ax;
        } else if (op == SHL) {
            ax = *sp++ << ax;
        } else if (op == SHR) {
            ax = *sp++ >> ax;
        } else if (op == ADD) {
            ax = *sp++ + ax;
        } else if (op == SUB) {
            ax = *sp++ - ax;
        } else if (op == MUL) {
            ax = *sp++ * ax;
        } else if (op == DIV) {
            ax = *sp++ / ax;
        } else if (op == MOD) {
            ax = *sp++ % ax;
        } else if (op == EXIT) {
            printf("exit %d", *sp);
            return *sp;
        } else if (op == OPEN) {
        } else if (op == READ) {
        } else if (op == CLOS) {
        } else if (op == PRTF) {
        } else if (op == MALC) {
        } else if (op == MSET) {
        } else if (op == MCMP) {
        } else {
            printf("unknown op %d", op);
            return -1;
        }
    }
}

int main(int argc, char **argv) {
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024;
    line = 1;

//    if ((fd = open(*argv, 0)) < 0) {
//        printf("open error %s\n", *argv);
//        return -1;
//    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("malloc src error");
        return -1;
    }
//    if ((i == read(fd, src, poolsize - 1)) <= 0) {
//        printf("read error");
//        return -1;
//    }
//    src[i] = 0;
//    close(fd);

    if (!(text = old_text = malloc(poolsize))) {
        printf("malloc text error");
        return -1;
    }
    if (!(data = malloc(poolsize))) {
        printf("malloc data error");
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("malloc stack error");
        return -1;
    }
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);
    bp = sp = (int *) ((int) stack + poolsize);
    ax = 0;

    src = "char else enum if int return sizeof while "
            "open read close printf malloc memset memcmp exit void main";
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Token] = i++;
    }

    i = 0;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 40;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    program();
    return eval();
}