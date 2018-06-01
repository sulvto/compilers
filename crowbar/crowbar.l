%{

%}

%start COMMENT  STARING_LITERAL_STATE
%%

<INITIAL>"function"     return FUNCTION;
<INITIAL>"if"           return IF;
<INITIAL>"("           return LP;
<INITIAL>")"           return RP;
<INITIAL>"{"           return LC;
<INITIAL>"}"           return RC;

<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    yylval.identifier = crb_create_identifier(yytext);
    return IDENTIFIER;
}

<INITIAL>([1-9][0-9]*)!"0" {
    Expression *expression = crb_alloc_expression(INT_EXPRESSION);
    sscanf(yytext, "%d", &expression->u.int_value);
    yylval.expression = expression;
    return INT_LITERAL;
}

<INITIAL>[0-9]+\.[0-9]+ {
    Expression *expression  =crb_alloc_expression(DOUBLE_EXPRESSION);
    sscanf(yytext, "%if", &expression->u.double_value);
    yylval.expression = expression;
    return DOUBLE_LITERAL;
}

<INITIAL>\" {
    crb_open_string_literal();
    BEGIN STRING_LITERAL_STATE;
}

<INITIAL>[ \t] ;
<INITIAL>\n {increment_line_number();}
<INITIAL>#  BEGIN COMMENT;
<INITIAL>.  {
    char buf[LINE_BUF_SIZE];
    if (isprint(yytext[0])) {
        buf[0] = yytext[0];
        buf[1] = '\0';
    } else {
        sprintf(buf, "0x%02x", (unsigned char)yytext[0]);
    }

    crb_compile_error(CHARACTER_INVALID_ERR,
                        STRING_MESSAGE_ARGUMENT, "bad_char", buf,
                        MESSAGE_ARGUMENT_END);
}
<COMMENT>\n {
    increment_line_number();
    BEGIN INITIAL;
}
<COMMENT>.      ;
<STRING_LITERAL_STATE>\"        {
    Expression *expression = crb_alloc_expression(STRING_EXPRESSION);
    expression->u.string_value = crb_close_string_literal();
    yylval.expression = expression;
    BEGIN INITIAL;
    return STRING_LITERAL;
}
<STRING_LITERAL_STATE>\n        {
    crb_add_string_literal('\n');
    increment_line_number();
}
<STRING_LITERAL_STATE>\\\"      crb_add_string_literal('"');
<STRING_LITERAL_STATE>\\n       crb_add_string_literal('\n');
<STRING_LITERAL_STATE>\\t       crb_add_string_literal('\t');
<STRING_LITERAL_STATE>\\\\      crb_add_string_literal('\\');
<STRING_LITERAL_STATE>.         crb_add_string_literal(yytext[0]);
%%




