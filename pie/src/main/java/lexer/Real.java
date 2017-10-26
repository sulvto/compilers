package lexer;

/**
 * Created by sulvto on 17-10-26.
 */
public class Real extends Token {
    public final float value;

    public Real(float value,int line,int column) {
        super(Tag.REAL,line,column);
        this.value = value;
    }
}
