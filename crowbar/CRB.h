//
// Created by sulvto on 18-6-5.
//

#ifndef CROWBAR_CRB_H
#define CROWBAR_CRB_H

typedef struct CRB_Interpreter_tag CRB_Interpreter;

CRB_Interpreter *CRB_create_interpreter(void);

void CRB_compile(CRB_Interpreter *interpreter, FILE *fp);

void CRB_interpret(CRB_Interpreter *interpreter);

void CRB_dispose_interpret(CRB_Interpreter *interpreter);


#endif //CROWBAR_CRB_H
