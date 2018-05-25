/**
 * Created by sulvto on 17-10-26.
 */
public class Token {
    public int line;
    public int column;
    public final int type;
    public String text;

    public Token(int type) {
        this(type, "");
    }

    public Token(int type, String text) {
        this(type, text, 1, 1);
    }

    public Token(int type, String text, int line, int column) {
        this.type = type;
        this.text = text;
        this.line = line;
        this.column = column;
    }

    @Override
    public String toString() {
        return Type.valueOf(type) + (text.length() > 0 ? ":" + text : "");
    }
}

