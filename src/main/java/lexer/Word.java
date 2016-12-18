package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Word extends Token {
    public String lexeme;

    public Word(String lexeme, int tag, int line, int column) {
        super(tag, line, column);
        this.lexeme = lexeme;
    }


    public Word(String lexeme, int tag) {
        super(tag, 0, 0);
        this.lexeme = lexeme;
    }

}
