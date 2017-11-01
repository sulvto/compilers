package type;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public class VoidTypeRef extends TypeRef {
    public VoidTypeRef(Token token) {
        super(token);
    }

    public VoidTypeRef() {
        super(null);
    }

    @Override
    public String toString() {
        return "void";
    }
}
