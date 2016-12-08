package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Real extends Token {
    public final float value;

    public Real(float value) {
        super(Tag.REAL);
        this.value = value;
    }
}
