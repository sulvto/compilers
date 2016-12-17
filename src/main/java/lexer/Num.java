package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Num extends Token {
    public final long value;

    public Num(long value) {
        super(Tag.NUM);
        this.value = value;
    }
}
