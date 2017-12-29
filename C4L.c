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

// opcodes
enum {
    LEA, IMM, JMP, JZ, JNZ,
    CALL, ENT, ADJ, LEV,
    LI, LC, SI, SC, PUSH,
    XOR, OR, AND, EQ, NE, LT, LE, GT, GE,
    SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

enum {//identifier.token
    Num = 128, Fun, Sys, Glo, Loc, Id,//类型
    Char, Else, Enum, If, Int, Return, Sizeof, While,//void, main 关键字 本身都是字符串
    Assign, Cond, Lor, Lan, Xor, Or, And, Eq, Ne, Lt, Le, Gt, Ge,//操作符
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
int token;//当前的token，从identifier.token代表或者第一个字符
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

token next() {//*token++
    char *last_pos;
    int hash;
    for (character = *src++; token != 0; character = *src++) {//找到一个可用的就退出

        // token_rep -> token_id
        //
        switch (character) {
            case token_reg:
                token == token_id;
                //
                line;
                src;
                Num
                { token_val }
                String
                { token_val, 写data }
                Opreator
                symbols {
                    {

                        Hash = len(last_pos)
                        Name = last_pos
                        current_id
                    }
                    keywords
                    { Token = Id }
                    void
                            main
                    ids
                    {

                    }
                };
                return ;
        };
    }

}

void program() {

    for (token = next(); token > 0;token = next()) {
        switch token {
            case def_reg
                fill symbols global
                if func
                    function();
        }
    }
}

void function() {//不能嵌套
    fill symbols loc//param loc var
    *++text = ENT;
    *++text = pos_local - index_of_bp;
    while (token != '}') {
        statement();
    }
    *++text = LEV;
    reset symbols global
}

void statement() {//可以嵌套
    switch token {
        case statement_reg:
            fill text
            expression(Assign);
            statement();
    }
}

void expression(int level) {//可以嵌套
    int tmp;
    int *id = current_id;
    int addr;
    if (token == Num) {
        *++text = IMM;
        *++text = token_val;
        expr_type = Int;
    } else if (token == '"') {
        *++text = IMM;
        *++text = token_val;
        next();
        while (token == '"') {
            next();
        }

        data = (char *) (((int) data + sizeof(int)) & (-sizeof(int)));//内存对齐
        expr_type = PTR;
    } else if (token == Sizeof) {
        next();
        match('(');
        expr_type = INT;
        if (token == Int) {
            next();
        } else if (token == Char) {
            next();
            expr_type = Char;
        }
        while (token == Mul) {
            next();
            expr_type += PTR;
        }
        match(')');
        *++text = IMM;
        *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);
        expr_type = INT;
    } else if (token == Id) {
        //func()
        //id
        //enum
        next();

        if (token == '(') {
            //func()
            tmp = 0;
            while (token != ')') {
                expression(Assign);
                *--text = PUSH;
                tmp++;
                if (token == ',')next();
            }
            next();
            if (id[Class] == Sys) {
                *--text = id[Value];
            } else if (id[Class] == Fun) {
                *--text = CALL;
                *--text = id[Value];
            } else {
                exit(-1);
            }
            if (tmp > 0) {
                *--text = ADJ;
                *--text = tmp;
            }
            expr_type = id[Type];
        } else if (id[Class] == Num) { //enum
            *--text = IMM;
            *--text = id[Value];
            expr_type = INT;
        } else {
            if (id[Class] == Glo) {
                *--text = IMM;
                *--text = id[Value];
            } else if (id[Class] == Loc) {
                *--text = LEA;
                *text = index_of_bp - id[Value];
            } else {
                exit(-1);
            }
            expr_type = id[Type];
            *--text = (expr_type == Char) ? LC : LI;
        }
    } else if (token == '(') {
        next();
        if (token == Int || token == Char) {
            tmp = (token == Int) ? INT : CHAR;
            while (token == MUL) {
                tmp += PTR;
            }
            match(')');
            expression(Inc);
            expr_type = tmp;
        } else {
            expression(Assign);
            match(')');
        }

    } else if (token == Mul) {//*P

    } else if (token == And) {

    } else if (token == '!') {

    } else if (token == '~') {

    } else if (token == Add) {

    } else if (token == Sub) {

    } else if (token == Inc) {

    } else if (token == Dec) {

    }

    while (token >= level) {
        tmp = expr_type;
        if (token == Assign) {

        } else if (token == Cond)
    }

}







int eval() {
    int op, *tmp;
    while (1) {
        op = *pc++;

        if (op == LEA) {
            ax = (int) (bp + *pc++);
        } else if (op == IMM) {
            ax = *pc++;
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
        } else if (op == ADJ) {
            sp += *pc++;
        } else if (op == LEV) {
            sp = bp;
            bp = (int *) *sp++;
            pc = (int *) *sp++;
        } else //无需pc参数
        if (op == LI) {
            ax = *(int *) ax;
        } else if (op == LC) {
            ax = *(char *) ax;
        } else if (op == SI) {
            *(int *) *sp++ = ax;
        } else if (op == SC) {
            *(char *) *sp++ = (char) ax;
        } else if (op == PUSH) {
            *--sp = ax;
        } else // 二元
        if (op == XOR) {
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
            printf("exit (%d)", *sp);
            return *sp;
        } else if (op == OPEN) {
            ax = open((char *) *(sp + 1), *sp);
        } else if (op == READ) {
            ax = read(*(sp + 2), (void *) *(sp + 1), *sp);
        } else if (op == CLOS) {
            ax = close(*sp);
        } else if (op == PRTF) {//?
            tmp = sp + pc[1];
            ax = printf((char *) tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
        } else if (op == MALC) {
            ax = (int) malloc(*sp);
        } else if (op == MSET) {
            ax = (int) memset((void *) *(sp + 2), *(sp + 1), *sp);
        } else if (op == MCMP) {
            ax = memcmp((void *) *(sp + 2), (void *) *(sp + 1), *sp);
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
    if (!(symbols = malloc(poolsize))) {
        printf("malloc symbols error");
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("malloc stack error");
        return -1;
    }
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(symbols, 0, poolsize);
    bp = sp = (int *) ((int) stack + poolsize);
    ax = 0;

    src = "char else enum if int return sizeof while "
            "open read close printf malloc memset memcmp exit void main";
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }
    i = OPEN;//func
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }
    next();
    current_id[Token] = Char;
    next();
    idmain = current_id;

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