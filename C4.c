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

        switch (op) {
            case IMM:
                ax = *pc++;
                break;
            case LI:
                ax = *(int *) ax;
                break;
            case LC:
                ax = *(char *) ax;
                break;
            case SI:
                *(int *) *sp++ = ax;
                break;
            case SC:
                *(char *) *sp++ = (char) ax;
                break;
            case PUSH:
                *--sp = ax;
                break;
            case JMP:
                pc = (int *) *pc;
                break;
            case JZ:
                pc = ax ? pc + 1 : (int *) *pc;
                break;
            case JNZ:
                pc = ax ? (int *) *pc : pc + 1;
                break;

            case CALL:
                *--sp = (int) (pc + 1);
                pc = (int *) *pc;
                break;
            case ENT:
                *--sp = (int) bp;
                bp = sp;
                sp -= *pc++;
                break;
            case LEV:
                sp = bp;
                bp = (int *) *sp++;
                pc = (int *) *sp++;
                break;
            case ADJ:
                sp += *pc++;
                break;
            case LEA:
                ax = (int) (bp + *pc++);
                break;

            case XOR:
                ax = *sp++ ^ ax;
                break;
            case OR:
                ax = *sp++ | ax;
                break;
            case AND:
                ax = *sp++ & ax;
                break;
            case EQ:
                ax = *sp++ == ax;
                break;
            case NE:
                ax = *sp++ != ax;
                break;
            case LT:
                ax = *sp++ < ax;
                break;
            case LE:
                ax = *sp++ <= ax;
                break;
            case GT:
                ax = *sp++ > ax;
                break;
            case GE:
                ax = *sp++ >= ax;
                break;

            case SHL:
                ax = *sp++ << ax;
                break;
            case SHR:
                ax = *sp++ >> ax;
                break;
            case ADD:
                ax = *sp++ + ax;
                break;
            case SUB:
                ax = *sp++ - ax;
                break;
            case MUL:
                ax = *sp++ * ax;
                break;
            case DIV:
                ax = *sp++ / ax;
                break;
            case MOD:
                ax = *sp++ % ax;
                break;
            case EXIT:
                printf("exit %d", *sp);
                return *sp;
            case OPEN:
                break;
            case READ:
                break;
            case CLOS:
                break;
            case PRTF:
                break;
            case MALC:
                break;
            case MSET:
                break;
            case MCMP:
                break;
            default:
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