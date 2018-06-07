//
// Created by sulvto on 18-6-7.
//

#ifndef DIKSAM_DIKSAMC_H
#define DIKSAM_DIKSAMC_H


typedef enum {
    EXPRESSION_STATEMENT = 1,
    IF_STATEMENT,
    SWITCH_STATEMENT,
    WHILE_STATEMENT,
    DO_WHILE_STATEMENT,
    FOR_STATEMENT,
    FOREACH_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    TRY_STATEMENT,
    THROW_STATEMENT,
    DECLARATION_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1,
} StatementType;

struct Statement_tag {
    StatementType   type;
    int             line_number;
    union {
        // TODO
    } u;
};

typedef struct StatementList_tag {
    Statement                   *statement;
    struct StatementList_tag    *next;
} StatementList;

#endif //DIKSAM_DIKSAMC_H
