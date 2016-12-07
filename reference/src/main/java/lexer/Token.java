package lexer;

/**
 * Created by sulvto on 16-11-6.
 */
public class Token {
    public final int tag;

    public Token(int t) {
        tag = t;
    }

    @Override
    public String toString() {
        return "" + (char) tag;
    }
}
