package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Word extends Token {
    public String lexeme;

    public Word(String lexeme, int tag) {
        super(tag);
        this.lexeme = lexeme;
    }

    public static final Word
            AND = new Word("&&", Tag.AND), OR = new Word("||", Tag.OR),
            EQ = new Word("==", Tag.EQ), NE = new Word("!=", Tag.NE),
            LE = new Word("<=", Tag.LE), GE = new Word(">=", Tag.GE),
            TRUE = new Word("true", Tag.TRUE),
            FALSE = new Word("false", Tag.FALSE),
            VOID = new Word("void", Tag.VOID),
            CHAR = new Word("char", Tag.CHAR),
            UNION = new Word("union", Tag.UNION),
            ENUM = new Word("enum", Tag.ENUM),
            STATIC = new Word("static", Tag.STATIC),
            EXTERN = new Word("extern", Tag.EXTERN),
            CONST = new Word("const", Tag.CONST),
            IF = new Word("if", Tag.IF),
            ELSE = new Word("else", Tag.ELSE),
            SWITCH = new Word("switch", Tag.SWITCH),
            CASE = new Word("case", Tag.CASE),
            DEFAULT = new Word("default", Tag.DEFAULT),
            WHILE = new Word("while", Tag.WHILE),
            DO = new Word("do", Tag.DO),
            FOR = new Word("for", Tag.FOR),
            RETURN = new Word("return", Tag.RETURN),
            BREAK = new Word("break", Tag.BREAK),
            CONTINUE = new Word("continue", Tag.CONTINUE),
            GOTO = new Word("goto", Tag.GOTO),
            TYPEDEF = new Word("typedef", Tag.TYPEDEF),
            IMPORT = new Word("import", Tag.IMPORT),
            SIZEOF = new Word("sizeof", Tag.SIZEOF);

}
