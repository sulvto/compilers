//
// Created by sulvto on 18-6-5.
//

static CRB_Interpreter *st_current_interpreter;


void crb_set_current_interpreter(CRB_Interpreter *interpreter) {
    st_current_interpreter = interpreter;
}