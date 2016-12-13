package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Type extends Word {
    public int width;

    public Type(String lexeme, int tag, int width) {
        super(lexeme, tag);
        this.width = width;
    }

    public String name() {
        return lexeme;
    }

    // TODO unsigned
    public static Type INT = new Type("int", Tag.BASIC, 4), FLOAT = new Type("float", Tag.BASIC, 8), SHORT = new Type("short", Tag.BASIC, 2), CHAR = new Type("char", Tag.BASIC, 1), BOOL = new Type("bool", Tag.BASIC, 1);
}
