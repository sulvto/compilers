package lexer;

/**
 * Created by sulvto on 16-11-6.
 */
public class Real extends Token {
    public final float value;
    public Real(float v){
        super(Tag.REAL);
        value = v;
    }

    @Override
    public String toString() {
        return "" + value;
    }
}
