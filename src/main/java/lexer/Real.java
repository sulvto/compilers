package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Real extends Token {
    public final float value;

    public Real(float value,int line,int column) {
        super(Tag.REAL,line,column);
        this.value = value;
    }
}
