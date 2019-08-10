#include<iostream>
#include<cstdio>
#include<cctype>
#include<string>
#include<vector>
#include<algorithm>

enum InstName {
    unknown = -1,
    nop = 0,
    print = 10,
    jz = 20,
    movis = 21,
    loads = 22,
    stores = 23,
    muli = 30,
    divi = 31,
    modi = 32,
    addi = 33,
    subi = 34,
    shli = 35,
    shri = 36,
    lti = 37,
    gti = 38,
    lei = 39,
    gei = 40,
    eqi = 41,
    nei = 42,
    andi = 43,
    xori = 44,
    ori = 45,
    loadv = 60,
    storev = 61,
    mm = 80
};

std::string InstName2String(InstName name) {
    if (name == nop) return "nop";
    if (name == print) return "print";
    if (name == jz) return "jz";
    if (name == movis) return "movis";
    if (name == loads) return "loads";
    if (name == stores) return "stores";
    if (name == muli) return "muli";
    if (name == divi) return "divi";
    if (name == modi) return "modi";
    if (name == addi) return "addi";
    if (name == subi) return "subi";
    if (name == shli) return "shli";
    if (name == shri) return "shri";
    if (name == lti) return "lti";
    if (name == gti) return "gti";
    if (name == lei) return "lei";
    if (name == gei) return "gei";
    if (name == eqi) return "eqi";
    if (name == nei) return "nei";
    if (name == andi) return "andi";
    if (name == xori) return "xori";
    if (name == ori) return "ori";
    if (name == andi) return "andi";
    if (name == ori) return "ori";
    if (name == loadv) return "loadv";
    if (name == storev) return "storev";
    if (name == mm) return "mm";
    return "NULL";
}

InstName String2InstName(std::string name) {
    transform(name.begin(),name.end(),name.begin(),::tolower);
    if (name == "nop") return nop;
    if (name == "print") return print;
    if (name == "jz") return jz;
    if (name == "movis") return movis;
    if (name == "loads") return loads;
    if (name == "stores") return stores;
    if (name == "muli") return muli;
    if (name == "divi") return divi;
    if (name == "modi") return modi;
    if (name == "addi") return addi;
    if (name == "subi") return subi;
    if (name == "shli") return shli;
    if (name == "shri") return shri;
    if (name == "lti") return lti;
    if (name == "gti") return gti;
    if (name == "lei") return lei;
    if (name == "gei") return gei;
    if (name == "eqi") return eqi;
    if (name == "nei") return nei;
    if (name == "andi") return andi;
    if (name == "xori") return xori;
    if (name == "ori") return ori;
    if (name == "loadv") return loadv;
    if (name == "storev") return storev;
    if (name == "mm") return mm;
    return unknown;
}

enum OperandType {
    immediate = 0,
    reg = 1,
    reg_address = 2,
    address = 3,
    line = 4
};

struct Operand {
    OperandType type;
    int v;
    Operand(OperandType type_, int v_) : type(type_), v(v_) {}
};  

struct Inst {
    int pc;
    InstName name;
    std::vector<Operand> operand;
    Inst(int pc_, InstName name_, std::vector<Operand> operand_) : pc(pc_), name(name_), operand(operand_) {}
};

std::vector<Inst> getinput(char filename[]) {
    FILE *fpin;
    if ((fpin=fopen(filename,"r")) == NULL) {
        std::cout << "[ERROR] No input src " << filename << std::endl;
        exit(-1);
    }
    std::vector<Inst> inst_q;
    int N, M, pc, type, value;
    char name[30];
    fscanf(fpin, "%d", &N);
    for(int i = 0; i < N; i++) {
        name[0] = '\n';
        fscanf(fpin, "%d", &M);
        fscanf(fpin, "%d", &pc);
        fscanf(fpin, "%s", name);
        std::vector<Operand> this_operands;
        for(int j = 0; j < M - 2; j++) {
            fscanf(fpin, "%d%d", &type, &value);
            Operand single_operand(OperandType(type), value);
            this_operands.push_back(single_operand);
        }
        InstName str_name = String2InstName(std::string(name));
        if (str_name == -1) {
            std::cout << "[ERROR] wrong inst name" << std::endl;
        }
        Inst this_inst(pc, str_name, this_operands);
        inst_q.push_back(this_inst);
    }
    fclose(fpin);
    return inst_q;
} 

/*
int main(){
    std::vector<Inst> inst_q = getinput("eop.output1");
    for (int i = 0; i < inst_q.size(); i++)
        printf("# %d %d\n", inst_q[i].pc, inst_q[i].name);
    return 0;
}
*/
