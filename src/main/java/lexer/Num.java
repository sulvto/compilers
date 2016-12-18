package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Num extends Token {
    public final long value;

    public Num(long value,int line,int column) {
        super(Tag.NUM,line,column);
        this.value = value;
    }
}
