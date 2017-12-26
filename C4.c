#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int token;
char *src, *old_src;
int poolsize;
int line;

int *text;
int *old_text;
int *stack;
char *data;

int *pc, *sp, *bp, ax, cycle;

enum {
    IMM, LI, LC, SI, SC, PUSH, JMP, JZ, JNZ,
    CALL, ENT, LEV, ADJ, LEA,
    XOR, OR, AND, EQ, NE, LT, LE, GT, GE,
    SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

enum {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Int, Char, Enum, Sizeof, If, Else, Return, While,
    Assign, Cond, Lor, Lan, Xor, Or, And, Eq, Ne, Lt, Le, Gt, Ge,
    Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

void next() {
    char *last_pos;
    return;
}

void expression(int level) {

}

void program() {
    next();
    while (token > 0) {
        printf("token is %c\n", token);
        next();
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