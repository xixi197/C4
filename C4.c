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
                   (token == '_')) {
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') ||
                   (*src >= 'A' && *src <= 'Z') ||
                   (*src >= '0' && *src <= '9') ||
                   (*src == '_')) {
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
            token = current_id[Token] = Id;
            current_id[Hash] = hash;
            current_id[Name] = (int) last_pos;
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
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }
                if (token == '"') {
                    *data++ = (char) token_val;
                }
            }
            src++;//跳过最后一个符号
            if (token == '"') {
                token_val = (int) last_pos;
            } else {
                token = Num;
            }
            return;
        } else if (token == '/') {
            if (*src == '/') {
                while (*src != 0 && *src != '\n') {
                    src++;
                }
            } else {
                token = Div;
                return;
            }
        } else if (token == '=') {
            if (*src == '=') {
                src++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        } else if (token == '+') {
            if (*src == '+') {
                src++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        } else if (token == '-') {
            if (*src == '-') {
                src++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        } else if (token == '!') {
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        } else if (token == '<') {
            // parse '<=', '<<' or '<'
            if (*src == '=') {
                src++;
                token = Le;
            } else if (*src == '<') {
                src++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        } else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src++;
                token = Ge;
            } else if (*src == '>') {
                src++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        } else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        } else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        } else if (token == '^') {
            token = Xor;
            return;
        } else if (token == '%') {
            token = Mod;
            return;
        } else if (token == '*') {
            token = Mul;
            return;
        } else if (token == '[') {
            token = Brak;
            return;
        } else if (token == '?') {
            token = Cond;
            return;
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

void match(int t) {//常量
    if (token == t) {
        next();
    } else {
        printf("expect %c, got %c", t, token);
        exit(-1);
    }
}

int expr();

int factor() {
    int value = 0;
    if (token == '(') {
        match('(');
        value = expr();
        match(')');
    } else {
        value = token_val;
        match(Num);
    }
    return value;
}

int term_tail(int lvalue) {
    if (token == Mul) {
        match('*');
        int value = lvalue * factor();
        return term_tail(value);
    } else if (token == Div) {
        match('/');
        int value = lvalue / factor();
        return term_tail(value);
    } else {
        return lvalue;
    }
}

int term() {
    int lvalue = factor();
    return term_tail(lvalue);
}

int expr_tail(int lvalue) {
    if (token == Add) {
        match('+');
        int value = lvalue + term();
        return expr_tail(value);
    } else if (token == Sub) {
        match('-');
        int value = lvalue - term();
        return expr_tail(value);
    } else {
        return lvalue;
    }
}

int expr() {
    int lvalue = term();
    return expr_tail(lvalue);
}

int expr_type;

int index_of_bp;

void expression(int level) {//解析表达式
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
        if(token == Assign) {

        } else if (token == Cond)
    }

}

//program ::= {global_declaration}+
//
//global_declaration ::= enum_decl | variable_decl | function_decl
//
//enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'
//
//variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
//
//function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
//
//parameter_decl ::= type {'*'} id {',' type {'*'} id}
//
//body_decl ::= {variable_decl}, {statement}
//
//statement ::= non_empty_statement | empty_statement
//
//non_empty_statement ::= if_statement | while_statement | '{' statement '}'
//| 'return' expression | expression ';'
//
//if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]
//
//while_statement ::= 'while' '(' expression ')' non_empty_statement
void statement();

void variable_decl();

void if_statement() {
    match(If);
    match('(');
    expression(Assign);
    match(')');

    *++text = JZ;
    int *b = ++text;
    statement();
    if (token == Else) {
        next();

        *b = (int) text + 3;//text + 1 jmp, +2, b, + 3
        *++text = JMP;
        b = ++text;

        statement();
    }
    *b = (int) (text + 1);
}

void while_statement() {
    match(While);
    int *a = text + 1;
    expression(Assign);
    *++text = JZ;
    int *b = ++text;
    statement();
    *++text = JMP;
    *++text = (int) a;
    *b = (int) (text + 1);
}


void statement() {
    if (token == If) {
        if_statement();
    } else if (token == While) {
        while_statement();
    } else if (token == Return) {
        next();
        if (token != ';') {
            expression(Assign);
        }
        next();
        *++text = LEV;
    } else if (token == '{') {
        // { <statement> ... }
        next();

        while (token != '}') {
            statement();
        }

        next();
    } else if (token == ';') {
        // empty statement
        next();
    } else {//常规语句
        // a = b; or function_call();
        expression(Assign);
        match(';');
    }
}


void parameter_decl() {
//    type {'*'} id {',' type {'*'} id}
    int type;
    int params;
    params = 0;
    while (token != ')') {
        type = Int;
        if (token == Int) {
            next();
        } else if (token == Char) {
            type = CHAR;
            next();
        } else {
            exit(1);
        }
        while (token == Mul) {
            next();
            type += PTR;
        }

        if (token != Id) {
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            exit(-1);//重复
        }
        match(Id);

        //property
        current_id[BClass] = current_id[Class];
        current_id[Class] = Loc;
        current_id[BType] = current_id[Type];
        current_id[Type] = type;
        current_id[BValue] = current_id[Value];
        current_id[Value] = params++;//index

        if (token == ',') {
            next();
        }
    }
    index_of_bp = params + 1;//bp的位置
}

int basetype;


void body_decl() {
    int pos_local;
    int type;
    pos_local = index_of_bp;
    while (token == Int || token == Char) {
        basetype = (token == Int) ? INT : CHAR;
        next();
        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                next();
                type += PTR;
            }
//        type {'*'} id { ',' {'*'} id } ';'
//        type {'*'} id '(' parameter_decl ')' '{' body_decl '}'


            //id的属性
            if (token != Id) {
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                exit(-1);//重复
            }
            match(Id);

            //property
            current_id[BClass] = current_id[Class];
            current_id[Class] = Loc;
            current_id[BType] = current_id[Type];
            current_id[Type] = type;
            current_id[BValue] = current_id[Value];
            current_id[Value] = ++pos_local;//index

            if (token == ',') {
                next();
            }
        }
        next();
    }

    *++text = ENT;
    *++text = pos_local - index_of_bp;
    while (token != '}') {
        statement();
    }
    *++text = LEV;
}

void enum_decl() {
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            exit(1);
        }
        next();
        if (token == Assign) {
            next();
            if (token != Num) {
                exit(-1);
            }
            i = token_val;
            next();
        }
        current_id[Class] = Num;
        current_id[Type] = Int;
        current_id[Value] = i++;
        if (token == ',') { next(); }
    }
}


void function_decl() {
//function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
    match('(');
    parameter_decl();
    match(')');
    match('{');
    body_decl();
    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type] = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id += IdSize;
    }
}


void global_declaration() {
    int type;
    int i;
    basetype = INT;
    if (token == Enum) {
//        enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'
        next();
        if (token != '{') {
            match(Id);
        }
        if (token == '{') {
            next();
            enum_decl();
            match('}');
        }
        match(';');
        return;
    }
    if (token == Int) {
        next();
    } else if (token == Char) {//void?
        next();
        basetype = CHAR;
    }

    while (token != ';' && token != '}') {
        type = basetype;
        while (token == Mul) {
            next();
            type += PTR;
        }
//        type {'*'} id { ',' {'*'} id } ';'
//        type {'*'} id '(' parameter_decl ')' '{' body_decl '}'


        //id的属性
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int) (text + 1);//??
            function_decl();
        } else {
            current_id[Class] = Glo;
            current_id[Value] = (int) data;
            data = data + sizeof(int);//?
        }
        if (token == ',') {
            next();
        }
    }
    next();
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