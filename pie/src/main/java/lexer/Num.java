package lexer;

/**
 * Created by sulvto on 17-10-26.
 */
public class Num extends Token {
    public final long value;

    public Num(long value,int line,int column) {
        super(Tag.NUM,line,column);
        this.value = value;
    }
}
