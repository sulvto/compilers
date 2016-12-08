package lexer;

/**
 * Created by sulvto on 16-12-8.
 */
public class Num extends Token {
    private float value;

    public Num(float value) {
        super(Tag.NUM);
        this.value = value;
    }
}
