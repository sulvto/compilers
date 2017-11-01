package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Token {
    public int line;
    public int column;
    public final int type;
    public final String value;

    public Token(int type, int line, int column) {
        this(type, line, column, "");
    }

    public Token(int type, int line, int column, String value) {
        this.type = type;
        this.line = line;
        this.column = column;
        this.value = value;
    }

    public void setLine(int line) {
        this.line = line;
    }

    public void setColumn(int column) {
        this.column = column;
    }
}
