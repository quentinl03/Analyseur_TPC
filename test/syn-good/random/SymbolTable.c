int global_a;
int global_tab1[10];

int main(void) {

}

char f(int param_b, int param_tab[], int param_c) {
    int local_d;
    int local_tab2[10];

    return 'z';
}

int g(int param_e, int param_tab[], int param_f) {
    int local_h;
    int local_d;
    int local_tab3[10];

    return 0;
}

void h(
    int param_1,   int param_2[], int param_3, // On registers
    int param_4[], int param_5,   int param_6[], // On registers
    int param_7, int param_8[]) { // On stack

    // On stack
    int local_1;
    int local_2;
}

void test(int a) {

}
