package lexer;

/**
 * Created by sulvto on 17-10-26.
 */
public class Token {
    public int line;
    public int column;
    public final int tag;

    public Token(int tag, int line, int column) {
        this.tag = tag;
        this.line = line;
        this.column = column;
    }

    public void setLine(int line) {
        this.line = line;
    }

    public void setColumn(int column) {
        this.column = column;
    }
}
